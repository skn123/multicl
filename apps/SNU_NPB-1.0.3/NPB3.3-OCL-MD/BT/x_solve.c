//-------------------------------------------------------------------------//
//                                                                         //
//  This benchmark is an OpenCL version of the NPB BT code for multiple    //
//  devices. This OpenCL version is developed by the Center for Manycore   //
//  Programming at Seoul National University and derived from the MPI      //
//  Fortran versions in "NPB3.3-MPI" developed by NAS.                     //
//                                                                         //
//  Permission to use, copy, distribute and modify this software for any   //
//  purpose with or without fee is hereby granted. This software is        //
//  provided "as is" without express or implied warranty.                  //
//                                                                         //
//  Information on NPB 3.3, including the technical report, the original   //
//  specifications, source code, results and information on how to submit  //
//  new results, is available at:                                          //
//                                                                         //
//           http://www.nas.nasa.gov/Software/NPB/                         //
//                                                                         //
//  Send comments or suggestions for this OpenCL version to                //
//  cmp@aces.snu.ac.kr                                                     //
//                                                                         //
//          Center for Manycore Programming                                //
//          School of Computer Science and Engineering                     //
//          Seoul National University                                      //
//          Seoul 151-744, Korea                                           //
//                                                                         //
//          E-mail:  cmp@aces.snu.ac.kr                                    //
//                                                                         //
//-------------------------------------------------------------------------//

//-------------------------------------------------------------------------//
// Authors: Sangmin Seo, Jungwon Kim, Jun Lee, Gangwon Jo, Jeongho Nah,    //
//          and Jaejin Lee                                                 //
//-------------------------------------------------------------------------//

#include "header.h"
#include "work_lhs.h"

static void x_unpack_solve_info(int stage);
static void x_send_solve_info(int stage);
static void x_send_backsub_info(int stage);
static void x_unpack_backsub_info(int stage);
static void x_backsubstitute(int first, int last, int stage);
static void x_solve_cell(int first, int last, int stage);


//---------------------------------------------------------------------
// 
// Performs line solves in X direction by first factoring
// the block-tridiagonal matrix into an upper triangular matrix, 
// and then performing back substitution to solve for the unknow
// vectors of each line.  
// 
// Make sure we treat elements zero to cell_size in the direction
// of the sweep.
// 
//---------------------------------------------------------------------
void x_solve()
{
  int i, stage;
  int first, last;

  if (timeron) timer_start(t_xsolve);
  //---------------------------------------------------------------------
  // in our terminology stage is the number of the cell in the x-direction
  // i.e. stage = 1 means the start of the line stage=ncells means end
  //---------------------------------------------------------------------
  for (stage = 1; stage <= ncells; stage++) {
    //---------------------------------------------------------------------
    // set last-cell flag
    //---------------------------------------------------------------------
    if (stage == ncells) {
      last = 1;
    } else {
      last = 0;
    }

    if (stage == 1) {
      //---------------------------------------------------------------------
      // This is the first cell, so solve without receiving data
      //---------------------------------------------------------------------
      first = 1;
      // lhsx(c);
      x_solve_cell(first, last, stage-1);
    } else {
      //---------------------------------------------------------------------
      // Not the first cell of this line, so receive info from
      // processor working on preceeding cell
      //---------------------------------------------------------------------
      first = 0;
      if (timeron) timer_start(t_xcomm);
      //---------------------------------------------------------------------
      // overlap computations and communications
      //---------------------------------------------------------------------
      // lhsx(c)
      //---------------------------------------------------------------------
      // wait for completion
      //---------------------------------------------------------------------
      if (timeron) timer_stop(t_xcomm);
      //---------------------------------------------------------------------
      // install C'(istart) and rhs'(istart) to be used in this cell
      //---------------------------------------------------------------------
      x_unpack_solve_info(stage-1);
      x_solve_cell(first, last, stage-1);
    }

    if (last == 0) x_send_solve_info(stage-1);
  }

  for (i = 0; i < num_devices; i++) {
    cl_int ecode = clFinish(cmd_queue[i]);
    clu_CheckError(ecode, "clFinish()");
  }

  //---------------------------------------------------------------------
  // now perform backsubstitution in reverse direction
  //---------------------------------------------------------------------
  for (stage = ncells; stage >= 1; stage--) {
    first = 0;
    last = 0;
    if (stage == 1) first = 1;
    if (stage == ncells) {
      last = 1;
      //---------------------------------------------------------------------
      // last cell, so perform back substitute without waiting
      //---------------------------------------------------------------------
      x_backsubstitute(first, last, stage-1);
    } else {
      if (timeron) timer_start(t_xcomm);
      if (timeron) timer_stop(t_xcomm);
      x_unpack_backsub_info(stage-1);
      x_backsubstitute(first, last, stage-1);
    }
    if (first == 0) x_send_backsub_info(stage-1);
  }

  if (timeron) timer_stop(t_xsolve);
}


//---------------------------------------------------------------------
// unpack C'(-1) and rhs'(-1) for
// all j and k
//---------------------------------------------------------------------
static void x_unpack_solve_info(int stage)
{
  int i, c;
  size_t d0_size, d1_size;
  cl_int ecode;

  int (*slice)[MAXCELLS][3] = (int (*)[MAXCELLS][3])g_slice;

  size_t x_usi_lws[3], x_usi_gws[3], temp;

  d0_size = JMAX;
  d1_size = KMAX;

  x_usi_lws[0] = d0_size < work_item_sizes[0] ?
                 d0_size : work_item_sizes[0];
  temp = max_work_group_size / x_usi_lws[0];
  x_usi_lws[1] = d1_size < temp ? d1_size : temp;
  x_usi_gws[0] = clu_RoundWorkSize(d0_size, x_usi_lws[0]);
  x_usi_gws[1] = clu_RoundWorkSize(d1_size, x_usi_lws[1]);

  for (i = 0; i < num_devices; i++) {
    c = slice[i][stage][0];

    ecode  = clSetKernelArg(k_x_usi[i], 0, sizeof(cl_mem), &m_lhsc[i]);
    ecode |= clSetKernelArg(k_x_usi[i], 1, sizeof(cl_mem), &m_rhs[i]);
    ecode |= clSetKernelArg(k_x_usi[i], 2, sizeof(cl_mem), &m_out_buffer[i]);
    ecode |= clSetKernelArg(k_x_usi[i], 3, sizeof(int), &c);
    clu_CheckError(ecode, "clSetKernelArg()");

    ecode = clEnqueueNDRangeKernel(cmd_queue[i],
                                   k_x_usi[i],
                                   2, NULL,
                                   x_usi_gws,
                                   x_usi_lws,
                                   0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueNDRangeKernel()");
  }

  CHECK_FINISH();
}


//---------------------------------------------------------------------
// pack up and send C'(iend) and rhs'(iend) for
// all j and k
//---------------------------------------------------------------------
static void x_send_solve_info(int stage)
{
  int i, c;
  int buffer_size;
  size_t d0_size, d1_size;
  cl_int ecode;

  int (*slice)[MAXCELLS][3] = (int (*)[MAXCELLS][3])g_slice;

  size_t x_ssi_lws[3], x_ssi_gws[3], temp;

  d0_size = JMAX;
  d1_size = KMAX;

  x_ssi_lws[0] = d0_size < work_item_sizes[0] ?
                 d0_size : work_item_sizes[0];
  temp = max_work_group_size / x_ssi_lws[0];
  x_ssi_lws[1] = d1_size < temp ? d1_size : temp;
  x_ssi_gws[0] = clu_RoundWorkSize(d0_size, x_ssi_lws[0]);
  x_ssi_gws[1] = clu_RoundWorkSize(d1_size, x_ssi_lws[1]);

  //---------------------------------------------------------------------
  // pack up buffer
  //---------------------------------------------------------------------
  for (i = 0; i < num_devices; i++) {
    c = slice[i][stage][0];

    ecode  = clSetKernelArg(k_x_ssi[i], 0, sizeof(cl_mem), &m_lhsc[i]);
    ecode |= clSetKernelArg(k_x_ssi[i], 1, sizeof(cl_mem), &m_rhs[i]);
    ecode |= clSetKernelArg(k_x_ssi[i], 2, sizeof(cl_mem), &m_in_buffer[i]);
    ecode |= clSetKernelArg(k_x_ssi[i], 3, sizeof(cl_mem), &m_cell_size[i]);
    ecode |= clSetKernelArg(k_x_ssi[i], 4, sizeof(int), &c);
    clu_CheckError(ecode, "clSetKernelArg()");

    ecode = clEnqueueNDRangeKernel(cmd_queue[i],
                                   k_x_ssi[i],
                                   2, NULL,
                                   x_ssi_gws,
                                   x_ssi_lws,
                                   0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueNDRangeKernel()");
  }

  buffer_size = MAX_CELL_DIM*MAX_CELL_DIM*(BLOCK_SIZE*BLOCK_SIZE+BLOCK_SIZE);

  for (i = 0; i < num_devices; i++) {
    ecode = clFinish(cmd_queue[i]);
    clu_CheckError(ecode, "clFinish()");
  }

  //---------------------------------------------------------------------
  // send buffer 
  //---------------------------------------------------------------------
  if (timeron) timer_start(t_xcomm);
  int dst;
  size_t cb = sizeof(double) * buffer_size;
  for (i = 0; i < num_devices; i++) {
    dst = successor[i][0];
    ecode = clEnqueueCopyBuffer(cmd_queue[dst],
                                m_in_buffer[i],
                                m_out_buffer[dst],
                                0, 0, cb,
                                0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueCopyBuffer()");
  }
  for (i = 0; i < num_devices; i++) {
    ecode = clFinish(cmd_queue[i]);
    clu_CheckError(ecode, "clFinish()");
  }
  if (timeron) timer_stop(t_xcomm);
}


//---------------------------------------------------------------------
// pack up and send U(istart) for all j and k
//---------------------------------------------------------------------
static void x_send_backsub_info(int stage)
{
  int i, c;
  int buffer_size;
  size_t d0_size, d1_size;
  cl_int ecode;

  int (*slice)[MAXCELLS][3] = (int (*)[MAXCELLS][3])g_slice;

  size_t x_sbi_lws[3], x_sbi_gws[3], temp;

  d0_size = JMAX;
  d1_size = KMAX;

  x_sbi_lws[0] = d0_size < work_item_sizes[0] ?
                 d0_size : work_item_sizes[0];
  temp = max_work_group_size / x_sbi_lws[0];
  x_sbi_lws[1] = d1_size < temp ? d1_size : temp;
  x_sbi_gws[0] = clu_RoundWorkSize(d0_size, x_sbi_lws[0]);
  x_sbi_gws[1] = clu_RoundWorkSize(d1_size, x_sbi_lws[1]);

  //---------------------------------------------------------------------
  // Send element 0 to previous processor
  //---------------------------------------------------------------------
  for (i = 0; i < num_devices; i++) {
    c = slice[i][stage][0];

    ecode  = clSetKernelArg(k_x_sbi[i], 0, sizeof(cl_mem), &m_rhs[i]);
    ecode |= clSetKernelArg(k_x_sbi[i], 1, sizeof(cl_mem), &m_in_buffer[i]);
    ecode |= clSetKernelArg(k_x_sbi[i], 2, sizeof(int), &c);
    clu_CheckError(ecode, "clSetKernelArg()");

    ecode = clEnqueueNDRangeKernel(cmd_queue[i],
                                   k_x_sbi[i],
                                   2, NULL,
                                   x_sbi_gws,
                                   x_sbi_lws,
                                   0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueNDRangeKernel()");
  }

  buffer_size = MAX_CELL_DIM*MAX_CELL_DIM*BLOCK_SIZE;

  for (i = 0; i < num_devices; i++) {
    ecode = clFinish(cmd_queue[i]);
    clu_CheckError(ecode, "clFinish()");
  }

  if (timeron) timer_start(t_xcomm);
  int dst;
  size_t cb = sizeof(double) * buffer_size;
  for (i = 0; i < num_devices; i++) {
    dst = predecessor[i][0];
    ecode = clEnqueueCopyBuffer(cmd_queue[dst],
                                m_in_buffer[i],
                                m_out_buffer[dst],
                                0, 0, cb,
                                0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueCopyBuffer()");
  }
  for (i = 0; i < num_devices; i++) {
    ecode = clFinish(cmd_queue[i]);
    clu_CheckError(ecode, "clFinish()");
  }
  if (timeron) timer_stop(t_xcomm);
}


//---------------------------------------------------------------------
// unpack U(isize) for all j and k
//---------------------------------------------------------------------
static void x_unpack_backsub_info(int stage)
{
  int i, c;
  size_t d0_size, d1_size;
  cl_int ecode;

  int (*slice)[MAXCELLS][3] = (int (*)[MAXCELLS][3])g_slice;

  size_t x_ubi_lws[3], x_ubi_gws[3], temp;

  d0_size = JMAX;
  d1_size = KMAX;

  x_ubi_lws[0] = d0_size < work_item_sizes[0] ?
                 d0_size : work_item_sizes[0];
  temp = max_work_group_size / x_ubi_lws[0];
  x_ubi_lws[1] = d1_size < temp ? d1_size : temp;
  x_ubi_gws[0] = clu_RoundWorkSize(d0_size, x_ubi_lws[0]);
  x_ubi_gws[1] = clu_RoundWorkSize(d1_size, x_ubi_lws[1]);

  for (i = 0; i < num_devices; i++) {
    c = slice[i][stage][0];

    ecode  = clSetKernelArg(k_x_ubi[i], 0, sizeof(cl_mem),&m_backsub_info[i]);
    ecode |= clSetKernelArg(k_x_ubi[i], 1, sizeof(cl_mem), &m_out_buffer[i]);
    ecode |= clSetKernelArg(k_x_ubi[i], 2, sizeof(int), &c);
    clu_CheckError(ecode, "clSetKernelArg()");

    ecode = clEnqueueNDRangeKernel(cmd_queue[i],
                                   k_x_ubi[i],
                                   2, NULL,
                                   x_ubi_gws,
                                   x_ubi_lws,
                                   0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueNDRangeKernel()");
  }

  CHECK_FINISH();
}


//---------------------------------------------------------------------
// back solve: if last cell, then generate U(isize)=rhs(isize)
// else assume U(isize) is loaded in un pack backsub_info
// so just use it
// after call u(istart) will be sent to next cell
//---------------------------------------------------------------------
static void x_backsubstitute(int first, int last, int stage)
{
  int i, c;
  size_t d0_size, d1_size;
  cl_int ecode;

  int (*slice)[MAXCELLS][3] = (int (*)[MAXCELLS][3])g_slice;

  for (i = 0; i < num_devices; i++) {
    c = slice[i][stage][0];

    size_t x_bs_lws[3], x_bs_gws[3], temp;

    d0_size = max_cell_size[i][1];
    d1_size = max_cell_size[i][2];

    x_bs_lws[0] = d0_size < work_item_sizes[0] ? d0_size : work_item_sizes[0];
    temp = max_work_group_size / x_bs_lws[0];
    x_bs_lws[1] = d1_size < temp ? d1_size : temp;
    x_bs_gws[0] = clu_RoundWorkSize(d0_size, x_bs_lws[0]);
    x_bs_gws[1] = clu_RoundWorkSize(d1_size, x_bs_lws[1]);

    ecode  = clSetKernelArg(k_x_bs[i], 0, sizeof(cl_mem), &m_rhs[i]);
    ecode |= clSetKernelArg(k_x_bs[i], 1, sizeof(cl_mem), &m_lhsc[i]);
    ecode |= clSetKernelArg(k_x_bs[i], 2, sizeof(cl_mem), &m_backsub_info[i]);
    ecode |= clSetKernelArg(k_x_bs[i], 3, sizeof(cl_mem), &m_cell_size[i]);
    ecode |= clSetKernelArg(k_x_bs[i], 4, sizeof(cl_mem), &m_start[i]);
    ecode |= clSetKernelArg(k_x_bs[i], 5, sizeof(cl_mem), &m_end[i]);
    ecode |= clSetKernelArg(k_x_bs[i], 6, sizeof(int), &last);
    ecode |= clSetKernelArg(k_x_bs[i], 7, sizeof(int), &c);
    clu_CheckError(ecode, "clSetKernelArg()");

    ecode = clEnqueueNDRangeKernel(cmd_queue[i],
                                   k_x_bs[i],
                                   2, NULL,
                                   x_bs_gws,
                                   x_bs_lws,
                                   0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueNDRangeKernel()");
  }

  CHECK_FINISH();
}


//---------------------------------------------------------------------
// performs guaussian elimination on this cell.
// 
// assumes that unpacking routines for non-first cells 
// preload C' and rhs' from previous cell.
// 
// assumed send happens outside this routine, but that
// c'(IMAX) and rhs'(IMAX) will be sent to next cell
//---------------------------------------------------------------------
static void x_solve_cell(int first, int last, int stage)
{
  int j, i, c;
  size_t d0_size, d1_size;
  cl_int ecode;

  int (*slice)[MAXCELLS][3] = (int (*)[MAXCELLS][3])g_slice;

  for (j = 0; j < num_devices; j++) {
	  for (i = 0; i < actual_num_devices; i++) {

		  size_t x_sc_lws[3], x_sc_gws[3], temp;

		  if (X_SOLVE_DIM[i] == 2) { // GPU
			  d0_size = max_cell_size[j][1];
			  d1_size = max_cell_size[j][2];

			  x_sc_lws[0] = d0_size < work_item_sizes[0] ?
				  d0_size : work_item_sizes[0];
			  temp = max_work_group_size / x_sc_lws[0];
			  x_sc_lws[1] = d1_size < temp ? d1_size : temp;
			  x_sc_gws[0] = clu_RoundWorkSize(d0_size, x_sc_lws[0]);
			  x_sc_gws[1] = clu_RoundWorkSize(d1_size, x_sc_lws[1]);
		  } else { // CPU
			  d0_size = max_cell_size[j][2];
			  x_sc_lws[0] = 1; //d0_size / max_compute_units;
			  x_sc_gws[0] = clu_RoundWorkSize(d0_size, x_sc_lws[0]);
		  }
    	ecode = clSetKernelLaunchConfiguration(devices[i],
                                   k_x_sc[j],
                                   X_SOLVE_DIM[i], NULL,
                                   x_sc_gws,
                                   x_sc_lws
                                   );
   		clu_CheckError(ecode, "clEnqueueNDRangeKernel()");
	  }
  }

  for (i = 0; i < num_devices; i++) {
	size_t x_sc_lws[3] = {1,1,1}, x_sc_gws[3]={1,1,1};
    c = slice[i][stage][0];
    ecode  = clSetKernelArg(k_x_sc[i], 0, sizeof(cl_mem), &m_qs[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 1, sizeof(cl_mem), &m_rho_i[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 2, sizeof(cl_mem), &m_u[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 3, sizeof(cl_mem), &m_rhs[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 4, sizeof(cl_mem), &m_lhsc[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 5, sizeof(cl_mem), &m_lhsa[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 6, sizeof(cl_mem), &m_lhsb[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 7, sizeof(cl_mem), &m_fjac[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 8, sizeof(cl_mem), &m_njac[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 9, sizeof(cl_mem), &m_cell_size[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 10, sizeof(cl_mem), &m_start[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 11, sizeof(cl_mem), &m_end[i]);
    ecode |= clSetKernelArg(k_x_sc[i], 12, sizeof(int), &first);
    ecode |= clSetKernelArg(k_x_sc[i], 13, sizeof(int), &last);
    ecode |= clSetKernelArg(k_x_sc[i], 14, sizeof(int), &c);
    clu_CheckError(ecode, "clSetKernelArg()");

    ecode = clEnqueueNDRangeKernel(cmd_queue[i],
                                   k_x_sc[i],
                                   3, NULL,
                                   x_sc_gws,
                                   x_sc_lws,
                                   0, NULL, NULL);
    clu_CheckError(ecode, "clEnqueueNDRangeKernel()");
  }

  CHECK_FINISH();
}

