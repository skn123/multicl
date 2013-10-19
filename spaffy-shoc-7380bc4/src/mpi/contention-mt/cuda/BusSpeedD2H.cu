#include <stdio.h>
#include "cudacommon.h"
#include "OptionParser.h"
#include "ResultDatabase.h"
#include <sys/time.h>
#include "misc_defs.h"

extern unsigned long long mpido;
extern unsigned long long cudado;
extern unsigned long long mpidone;
extern unsigned long long cudadone;
extern pthread_barrier_t mpitest_barrier;
//extern cudaStream_t g_cuda_default_stream;

// ****************************************************************************
// Function: addBenchmarkSpecOptions
//
// Purpose:
//   Add benchmark specific command line argument parsing.
//
//   -nopinned
//   This option controls whether page-locked or "pinned" memory is used.
//   The use of pinned memory typically results in higher bandwidth for data
//   transfer between host and device.
//
// Arguments:
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Jeremy Meredith
// Creation: September 08, 2009
//
// Modifications:
//
// ****************************************************************************
void addBenchmarkSpecOptions(OptionParser &op)
{
    op.addOption("nopinned", OPT_BOOL, "",
                 "disable usage of pinned (pagelocked) memory", 'p');
}

// ****************************************************************************
// Function: runBenchmark
//
// Purpose:
//   Measures the bandwidth of the bus connecting the host processor to the
//   OpenCL device.  This benchmark repeatedly transfers data chunks of various
//   sizes across the bus to the host from the device and calculates the
//   bandwidth for each chunk size.
//
// Arguments:
//  resultDB: the benchmark stores its results in this ResultDatabase
//  op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Jeremy Meredith
// Creation: September 08, 2009
//
// Modifications:
//    Jeremy Meredith, Wed Dec  1 17:05:27 EST 2010
//    Added calculation of latency estimate.
//
// ****************************************************************************
void RunBenchmark(ResultDatabase &resultDB,
                  OptionParser &op)
{
    bool verbose = op.getOptionBool("verbose");
    bool pinned  = !op.getOptionBool("nopinned");
	int cur_cpu_core;
	struct timeval t_start;
	struct timeval t_end;
	MPIACCGetCPUCore_thread(&cur_cpu_core);
	MPIACCSetCPUCore_thread(1);

	int cur_device;
	cudaGetDevice(&cur_device);
	CHECK_CUDA_ERROR();
	//cudaSetDevice(0);
	CHECK_CUDA_ERROR();

    // Sizes are in kb
    //int nSizes  = 20;
    //int sizes[20] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,
	//	     32768,65536,131072,262144,524288};
    int nSizes  = 10;
#if 0
    int sizes[10] = {1024,2048,4096,8192,16384,
		     32768,65536,131072,262144,524288};
#else
    int sizes[10] = {1,2,4,8,16,
		     32,64,128,256,512};
#endif
    /*
	int maxmsg_sz = op.getOptionInt("MPImaxmsg");
    int nSizes  = 1;
	int sizes[1] = {maxmsg_sz/1024};
	*/
	int iterations = 10;
	int skip = 4;
    long long numMaxFloats = 1024 * (sizes[nSizes-1]) / 4;

    // Create some host memory pattern
    float *hostMem1;
    float *hostMem2;
    if (pinned)
    {
	    if (verbose) cout << "using pinned memory\n";
        cudaMallocHost((void**)&hostMem1, sizeof(float)*numMaxFloats);
        cudaError_t err1 = cudaGetLastError();
        cudaMallocHost((void**)&hostMem2, sizeof(float)*numMaxFloats);
        cudaError_t err2 = cudaGetLastError();
	while (err1 != cudaSuccess || err2 != cudaSuccess)
	{
	    // free the first buffer if only the second failed
	    if (err1 == cudaSuccess)
	        cudaFreeHost((void*)hostMem1);

	    // drop the size and try again
	    if (verbose) cout << " - dropping size allocating pinned mem\n";
	    --nSizes;
	    if (nSizes < 1)
	    {
		cerr << "Error: Couldn't allocated any pinned buffer\n";
		return;
	    }
	    numMaxFloats = 1024 * (sizes[nSizes-1]) / 4;
            cudaMallocHost((void**)&hostMem1, sizeof(float)*numMaxFloats);
            err1 = cudaGetLastError();
            cudaMallocHost((void**)&hostMem2, sizeof(float)*numMaxFloats);
            err2 = cudaGetLastError();
	}
   }
    else
    {
        hostMem1 = new float[numMaxFloats];
        hostMem2 = new float[numMaxFloats];
    }
    for (int i=0; i<numMaxFloats; i++)
        hostMem1[i] = i % 77;

    float *device;
    cudaMalloc((void**)&device, sizeof(float) * numMaxFloats);
    while (cudaGetLastError() != cudaSuccess)
    {
	// drop the size and try again
	if (verbose) cout << " - dropping size allocating device mem\n";
	--nSizes;
	if (nSizes < 1)
	{
	    cerr << "Error: Couldn't allocated any device buffer\n";
	    return;
	}
	numMaxFloats = 1024 * (sizes[nSizes-1]) / 4;
        cudaMalloc((void**)&device, sizeof(float) * numMaxFloats);
    }

	cudaStream_t g_cuda_default_stream;
	cudaStreamCreate(&g_cuda_default_stream);
	CHECK_CUDA_ERROR();
    cudaMemcpy(device, hostMem1,
               numMaxFloats*sizeof(float), cudaMemcpyHostToDevice);
    cudaThreadSynchronize();
    const unsigned int passes = op.getOptionInt("passes");


    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    CHECK_CUDA_ERROR();

	if(cudado != 1)
	{
	    int rc = pthread_barrier_wait(&mpitest_barrier);
		if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
		{
			printf("Could not wait on barrier\n");
			exit(-1);
	    }
	}
	
	int cur_dev;
	cudaGetDevice(&cur_dev);
	CHECK_CUDA_ERROR();
    cout << "[CUDA-Task] Running CUDA benchmarks on device: " << cur_dev << "\n";
    // Three passes, forward and backward both
	int iter = 0;
	gettimeofday(&t_start, NULL);
#ifndef LONG_GPU_RUNS
	do {
    //for (int pass = 0; pass < passes; pass++) {
#else
    for (int pass = 0; pass < passes; pass++)
    {
#endif
        //cout << "Running benchmarks, pass: " << iter << "\n";
        // store the times temporarily to estimate latency
        //float times[nSizes];
        // Step through sizes forward on even passes and backward on odd
        //int i = 0;
        //int i = 6;
		int i = 9;
		//for (int i = 0; i < nSizes; i++)
        {
            int sizeIndex;
           // if ((iter % 2) == 0)
                sizeIndex = i;
           // else
           //     sizeIndex = (nSizes - 1) - i;

            int nbytes = sizes[sizeIndex] * 1024;

            for (int idx = 0; idx < iterations + skip; idx++) 
			{
            //if(idx == skip) 
			//	cudaEventRecord(start, g_cuda_default_stream);
			if(pinned)
			{
				printf("CUDA Thread Stream (D2H): %p\n", g_cuda_default_stream);
            	cudaMemcpyAsync(hostMem2, device,
               			nbytes, cudaMemcpyDeviceToHost, g_cuda_default_stream);
			}
			else
            	cudaMemcpy(hostMem2, device,
                       nbytes, cudaMemcpyDeviceToHost);
            }
			//cudaEventRecord(stop, g_cuda_default_stream);
            //cudaEventSynchronize(stop);
			cudaStreamSynchronize(g_cuda_default_stream);
            float t = 0;
            //cudaEventElapsedTime(&t, start, stop);
			t/=iterations;
            //times[sizeIndex] = t;

            // Convert to GB/sec
            if (verbose)
            {
                cerr << "size " <<sizes[sizeIndex] << "k took " << t <<
                        " ms\n";
            }

            double speed = (double(sizes[sizeIndex]) * 1024. / (1000*1000)) / t;
            char sizeStr[256];
            sprintf(sizeStr, "% 7dkB", sizes[sizeIndex]);
            resultDB.AddResult("ReadbackSpeed", sizeStr, "GB/sec", speed);
            resultDB.AddResult("ReadbackTime", sizeStr, "ms", t);
        }
		iter++;
	//resultDB.AddResult("ReadbackLatencyEstimate", "1-2kb", "ms", times[0]-(times[1]-times[0])/1.);
	//resultDB.AddResult("ReadbackLatencyEstimate", "1-4kb", "ms", times[0]-(times[2]-times[0])/3.);
	//resultDB.AddResult("ReadbackLatencyEstimate", "2-4kb", "ms", times[1]-(times[2]-times[1])/1.);
#ifdef LONG_GPU_RUNS
    }
	cudadone = 1;
#else
	//}
	} while(mpidone != 1);
#endif
	gettimeofday(&t_end, NULL);
	long long t_time = (t_end.tv_sec - t_start.tv_sec) * 1e6 
													 + ((t_end.tv_usec - t_start.tv_usec));
	printf("CUDA D2H Total Time for %d iters: %lld us\n", iter, t_time);

    // Cleanup
	printf("Done with CUDA Tests...\n");
	cudaStreamDestroy(g_cuda_default_stream);
	CHECK_CUDA_ERROR();
    cudaFree((void*)device);
    CHECK_CUDA_ERROR();
    if (pinned)
    {
        cudaFreeHost((void*)hostMem1);
        CHECK_CUDA_ERROR();
        cudaFreeHost((void*)hostMem2);
        CHECK_CUDA_ERROR();
    }
    else
    {
        delete[] hostMem1;
        delete[] hostMem2;
        cudaEventDestroy(start);
	    cudaEventDestroy(stop);
    }
	//cudaSetDevice(cur_device);
    CHECK_CUDA_ERROR();
	MPIACCSetCPUCore_thread(cur_cpu_core);
}
