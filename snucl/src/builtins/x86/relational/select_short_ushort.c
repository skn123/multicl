/*****************************************************************************/
/* Copyright (C) 2010, 2011 Seoul National University                        */
/* and Samsung Electronics Co., Ltd.                                         */
/*                                                                           */
/* Contributed by Sangmin Seo <sangmin@aces.snu.ac.kr>, Jungwon Kim          */
/* <jungwon@aces.snu.ac.kr>, Jaejin Lee <jlee@cse.snu.ac.kr>, Seungkyun Kim  */
/* <seungkyun@aces.snu.ac.kr>, Jungho Park <jungho@aces.snu.ac.kr>,          */
/* Honggyu Kim <honggyu@aces.snu.ac.kr>, Jeongho Nah                         */
/* <jeongho@aces.snu.ac.kr>, Sung Jong Seo <sj1557.seo@samsung.com>,         */
/* Seung Hak Lee <s.hak.lee@samsung.com>, Seung Mo Cho                       */
/* <seungm.cho@samsung.com>, Hyo Jung Song <hjsong@samsung.com>,             */
/* Sang-Bum Suh <sbuk.suh@samsung.com>, and Jong-Deok Choi                   */
/* <jd11.choi@samsung.com>                                                   */
/*                                                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This file is part of the SNU-SAMSUNG OpenCL runtime.                      */
/*                                                                           */
/* The SNU-SAMSUNG OpenCL runtime is free software: you can redistribute it  */
/* and/or modify it under the terms of the GNU Lesser General Public License */
/* as published by the Free Software Foundation, either version 3 of the     */
/* License, or (at your option) any later version.                           */
/*                                                                           */
/* The SNU-SAMSUNG OpenCL runtime is distributed in the hope that it will be */
/* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  */
/* Public License for more details.                                          */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with the SNU-SAMSUNG OpenCL runtime. If not, see                    */
/* <http://www.gnu.org/licenses/>.                                           */
/*****************************************************************************/

#include <cl_cpu_ops.h>

// 7. select_short_ushort

#define MSB_USHORT   0x8000

short select(short a, short b, ushort c) {
	return	c ? b : a;
}

short2 select(short2 a, short2 b, ushort2 c) {
  ushort2 msb = c & MSB_USHORT;
  short2 rst;
  rst[0] = msb[0] ? b[0] : a[0];
  rst[1] = msb[1] ? b[1] : a[1];
  return rst;
}

short3 select(short3 a, short3 b, ushort3 c) {
  ushort3 msb = c & MSB_USHORT;
  short3 rst;
  rst[0] = msb[0] ? b[0] : a[0];
  rst[1] = msb[1] ? b[1] : a[1];
  rst[2] = msb[2] ? b[2] : a[2];
  return rst;
}

short4 select(short4 a, short4 b, ushort4 c) {
  ushort4 msb = c & MSB_USHORT;
  short4 rst;
  rst[0] = msb[0] ? b[0] : a[0];
  rst[1] = msb[1] ? b[1] : a[1];
  rst[2] = msb[2] ? b[2] : a[2];
  rst[3] = msb[3] ? b[3] : a[3];
  return rst;
}

short8 select(short8 a, short8 b, ushort8 c) {
  ushort8 msb = c & MSB_USHORT;
  short8 rst;
  rst[0] = msb[0] ? b[0] : a[0];
  rst[1] = msb[1] ? b[1] : a[1];
  rst[2] = msb[2] ? b[2] : a[2];
  rst[3] = msb[3] ? b[3] : a[3];
  rst[4] = msb[4] ? b[4] : a[4];
  rst[5] = msb[5] ? b[5] : a[5];
  rst[6] = msb[6] ? b[6] : a[6];
  rst[7] = msb[7] ? b[7] : a[7];
  return rst;
}

short16 select(short16 a, short16 b, ushort16 c) {
  ushort16 msb = c & MSB_USHORT;
  short16 rst;
  rst[0] = msb[0] ? b[0] : a[0];
  rst[1] = msb[1] ? b[1] : a[1];
  rst[2] = msb[2] ? b[2] : a[2];
  rst[3] = msb[3] ? b[3] : a[3];
  rst[4] = msb[4] ? b[4] : a[4];
  rst[5] = msb[5] ? b[5] : a[5];
  rst[6] = msb[6] ? b[6] : a[6];
  rst[7] = msb[7] ? b[7] : a[7];
  rst[8] = msb[8] ? b[8] : a[8];
  rst[9] = msb[9] ? b[9] : a[9];
  rst[10] = msb[10] ? b[10] : a[10];
  rst[11] = msb[11] ? b[11] : a[11];
  rst[12] = msb[12] ? b[12] : a[12];
  rst[13] = msb[13] ? b[13] : a[13];
  rst[14] = msb[14] ? b[14] : a[14];
  rst[15] = msb[15] ? b[15] : a[15];
  return rst;
}

