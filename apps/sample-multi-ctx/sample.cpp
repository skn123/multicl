/*****************************************************************************/
/*                                                                           */
/* Copyright (c) 2011-2013 Seoul National University.                        */
/* All rights reserved.                                                      */
/*                                                                           */
/* Redistribution and use in source and binary forms, with or without        */
/* modification, are permitted provided that the following conditions        */
/* are met:                                                                  */
/*   1. Redistributions of source code must retain the above copyright       */
/*      notice, this list of conditions and the following disclaimer.        */
/*   2. Redistributions in binary form must reproduce the above copyright    */
/*      notice, this list of conditions and the following disclaimer in the  */
/*      documentation and/or other materials provided with the distribution. */
/*   3. Neither the name of Seoul National University nor the names of its   */
/*      contributors may be used to endorse or promote products derived      */
/*      from this software without specific prior written permission.        */
/*                                                                           */
/* THIS SOFTWARE IS PROVIDED BY SEOUL NATIONAL UNIVERSITY "AS IS" AND ANY    */
/* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE    */
/* DISCLAIMED. IN NO EVENT SHALL SEOUL NATIONAL UNIVERSITY BE LIABLE FOR ANY */
/* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS   */
/* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)     */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       */
/* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  */
/* ANY WAY OUT OF THE USE OF THIS  SOFTWARE, EVEN IF ADVISED OF THE          */
/* POSSIBILITY OF SUCH DAMAGE.                                               */
/*                                                                           */
/* Contact information:                                                      */
/*   Center for Manycore Programming                                         */
/*   Department of Computer Science and Engineering                          */
/*   Seoul National University, Seoul 151-744, Korea                         */
/*   http://aces.snu.ac.kr                                                   */
/*                                                                           */
/* Contributors:                                                             */
/*   Jungwon Kim, Sangmin Seo, Gangwon Jo, Jun Lee, Jeongho Nah,             */
/*   Jungho Park, Junghyun Kim, and Jaejin Lee                               */
/*                                                                           */
/*****************************************************************************/

#include <CL/cl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"

#if defined(SAMPLE_CPU)
#define PLATFORM_NAME "SnuCL CPU"
#elif defined(SAMPLE_SINGLE)
#define PLATFORM_NAME "SnuCL Single"
#elif defined(SAMPLE_CLUSTER)
#define PLATFORM_NAME "SnuCL Cluster"
#else
#define PLATFORM_NAME "NVIDIA CUDA"
#endif

#define CHECK_ERROR(err)                                   \
	if (err != CL_SUCCESS) {                                 \
		printf("[%s:%d] error %d\n", __FILE__, __LINE__, err); \
		exit(EXIT_FAILURE);                                    \
	}

const char* kernel_src =
"__kernel void sample(__global int* dst, __global int* src, int offset) {\n"
"  int id = get_global_id(0);\n"
"  dst[id] = src[id] + offset;\n"
"}\n";

int main(int argc, char** argv) {
	//cl_device_type    DEV_TYPE = CL_DEVICE_TYPE_GPU;
	cl_device_type    DEV_TYPE = CL_DEVICE_TYPE_ALL;
	cl_uint           num_platforms;
	cl_platform_id*   platforms;
	size_t            platform_name_size;
	char*             platform_name;
	cl_platform_id    snucl_platform = NULL;
	cl_device_id*     devices;
	const cl_uint 			num_devices = 3;
	cl_context_properties context_properties[3];
	const cl_uint num_contexts = 3;
	cl_context        *contexts;
	cl_command_queue  command_queue;
	cl_program        program;
	cl_kernel         kernel;
	int*              host_src;
	int*              host_dst;
	cl_mem            buffer_src;
	cl_mem            buffer_dst;
	cl_int            offset;
	size_t            local  = 4;
	size_t            global = local * 8;
	size_t            size   = global;
	int               i;
	cl_int            err;

	MPI_Init(&argc, &argv);

	err = clGetPlatformIDs(0, NULL, &num_platforms);
	CHECK_ERROR(err);
	printf("%u platforms are detected.\n", num_platforms);

	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
	err = clGetPlatformIDs(num_platforms, platforms, NULL);
	CHECK_ERROR(err);

	for (i = 0; i < num_platforms; i++) {
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL,
				&platform_name_size);
		CHECK_ERROR(err);

		platform_name = (char*)malloc(sizeof(char) * platform_name_size);
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platform_name_size,
				platform_name, NULL);
		CHECK_ERROR(err);

		printf("Platform %d: %s\n", i, platform_name);
		if (strcmp(platform_name, PLATFORM_NAME) == 0)
		{
			printf("Choosing Platform %d: %s\n", i, platform_name);
			snucl_platform = platforms[i];
		}
		free(platform_name);
	}

	if (snucl_platform == NULL) {
		printf("%s platform is not found.\n", PLATFORM_NAME);
		//exit(EXIT_FAILURE);
	}

	devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices);
	err = clGetDeviceIDs(snucl_platform, DEV_TYPE, num_devices, devices, NULL);
	CHECK_ERROR(err);

	contexts = (cl_context*)malloc(sizeof(cl_context) * num_contexts);
	context_properties[0] = CL_CONTEXT_PLATFORM;
	context_properties[1] = (cl_context_properties)snucl_platform;
	context_properties[2] = NULL;

	for(cl_uint idx = 0; idx < num_contexts; idx++)
	{
		// 1-1 mapping between devices and contexts
		char device_name[1000];
		err = clGetDeviceInfo(devices[idx], CL_DEVICE_NAME, 1000, &device_name, NULL);
		CHECK_ERROR(err);
		cl_device_type device_type;
		err = clGetDeviceInfo(devices[idx], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);
		CHECK_ERROR(err);
		printf("Device %u is of type %d is %s.\n", idx, device_type, device_name);
		contexts[idx] = clCreateContext(context_properties, 1, &devices[idx], NULL, NULL, &err);
		CHECK_ERROR(err);
	}

	for(cl_uint idx = 0; idx < num_contexts; idx++)
	{
		cl_device_id device;
		err = clGetContextInfo(contexts[idx], CL_CONTEXT_DEVICES, sizeof(cl_device_id), &device, NULL);
		CHECK_ERROR(err);
		char device_name[1000];
		err = clGetDeviceInfo(device, CL_DEVICE_NAME, 1000, &device_name, NULL);
		CHECK_ERROR(err);
		cl_device_type device_type;
		err = clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);
		CHECK_ERROR(err);
		printf("Context %u has a device of type %d whose name is %s.\n", idx, device_type, device_name);
		command_queue = clCreateCommandQueue(contexts[idx], device, 0, &err);
		CHECK_ERROR(err);

		program = clCreateProgramWithSource(contexts[idx], 1, (const char**)&kernel_src,
				NULL, &err);
		CHECK_ERROR(err);
		err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
		CHECK_ERROR(err);
		kernel = clCreateKernel(program, "sample", &err);
		CHECK_ERROR(err);

		host_src = (int*)calloc(size, sizeof(int));
		host_dst = (int*)calloc(size, sizeof(int));
		for (i = 0; i < size; i++)
			host_src[i] = i * 10;

		buffer_src = clCreateBuffer(contexts[idx], CL_MEM_READ_ONLY, sizeof(int) * size,
				NULL, &err);
		CHECK_ERROR(err);
		buffer_dst = clCreateBuffer(contexts[idx], CL_MEM_WRITE_ONLY, sizeof(int) * size,
				NULL, &err);
		CHECK_ERROR(err);

		err = clEnqueueWriteBuffer(command_queue, buffer_src, CL_TRUE, 0,
				sizeof(int) * size, host_src, 0, NULL, NULL);
		CHECK_ERROR(err);

		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&buffer_dst);
		CHECK_ERROR(err);
		err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&buffer_src);
		CHECK_ERROR(err);
		offset = 100;
		err = clSetKernelArg(kernel, 2, sizeof(cl_int), (void*)&offset);
		CHECK_ERROR(err);

		err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global, &local,
				0, NULL, NULL);
		CHECK_ERROR(err);

		err = clEnqueueReadBuffer(command_queue, buffer_dst, CL_TRUE, 0,
				sizeof(int) * size, host_dst, 0, NULL, NULL);
		CHECK_ERROR(err);

		for (i = 0; i < size; i++)
			printf("[%2d] %d\n", i, host_dst[i]);

		free(host_src);
		free(host_dst);

		clReleaseMemObject(buffer_src);
		clReleaseMemObject(buffer_dst);
		clReleaseKernel(kernel);
		clReleaseProgram(program);
		clReleaseCommandQueue(command_queue);
	}

	for(cl_uint i = 0; i < num_contexts; i++)
		clReleaseContext(contexts[i]);

	free(platforms);
	free(devices);
	free(contexts);
	MPI_Finalize();
	return 0;
}