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

// int
int min(int x, int y){
	return (x<y)? x:y;
}
int2 min(int2 x, int2 y){
	int2 rst;
	rst.v[0]=(x.v[0]<y.v[0])? x.v[0]:y.v[0];
	rst.v[1]=(x.v[1]<y.v[1])? x.v[1]:y.v[1];
  return rst;
}
int3 min(int3 x, int3 y){
	int3 rst;
	rst.v[0]=(x.v[0]<y.v[0])? x.v[0]:y.v[0];
	rst.v[1]=(x.v[1]<y.v[1])? x.v[1]:y.v[1];
	rst.v[2]=(x.v[2]<y.v[2])? x.v[2]:y.v[2];
	return rst;
}
int4 min(int4 x, int4 y){
	int4 rst;
	rst.v[0]=(x.v[0]<y.v[0])? x.v[0]:y.v[0];
	rst.v[1]=(x.v[1]<y.v[1])? x.v[1]:y.v[1];
	rst.v[2]=(x.v[2]<y.v[2])? x.v[2]:y.v[2];
	rst.v[3]=(x.v[3]<y.v[3])? x.v[3]:y.v[3];
	return rst;
}
int8 min(int8 x, int8 y){
	int8 rst;

	rst.v[0]=(x.v[0]<y.v[0])? x.v[0]:y.v[0];
	rst.v[1]=(x.v[1]<y.v[1])? x.v[1]:y.v[1];
	rst.v[2]=(x.v[2]<y.v[2])? x.v[2]:y.v[2];
	rst.v[3]=(x.v[3]<y.v[3])? x.v[3]:y.v[3];
	rst.v[4]=(x.v[4]<y.v[4])? x.v[4]:y.v[4];
	rst.v[5]=(x.v[5]<y.v[5])? x.v[5]:y.v[5];
	rst.v[6]=(x.v[6]<y.v[6])? x.v[6]:y.v[6];
	rst.v[7]=(x.v[7]<y.v[7])? x.v[7]:y.v[7];
	return rst;
}
int16 min(int16 x, int16 y){
	int16 rst;
	rst.v[0]=rst.v[1]=rst.v[2]=rst.v[3]=rst.v[4]=rst.v[5]=rst.v[6]=rst.v[7]=0;
	rst.v[8]=rst.v[9]=rst.v[10]=rst.v[11]=rst.v[12]=rst.v[13]=rst.v[14]=rst.v[15]=0;

	rst.v[0]=(x.v[0]<y.v[0])? x.v[0]:y.v[0];
	rst.v[1]=(x.v[1]<y.v[1])? x.v[1]:y.v[1];
	rst.v[2]=(x.v[2]<y.v[2])? x.v[2]:y.v[2];
	rst.v[3]=(x.v[3]<y.v[3])? x.v[3]:y.v[3];
	rst.v[4]=(x.v[4]<y.v[4])? x.v[4]:y.v[4];
	rst.v[5]=(x.v[5]<y.v[5])? x.v[5]:y.v[5];
	rst.v[6]=(x.v[6]<y.v[6])? x.v[6]:y.v[6];
	rst.v[7]=(x.v[7]<y.v[7])? x.v[7]:y.v[7];
	rst.v[8]=(x.v[8]<y.v[8])? x.v[8]:y.v[8];
	rst.v[9]=(x.v[9]<y.v[9])? x.v[9]:y.v[9];
	rst.v[10]=(x.v[10]<y.v[10])? x.v[10]:y.v[10];
	rst.v[11]=(x.v[11]<y.v[11])? x.v[11]:y.v[11];
	rst.v[12]=(x.v[12]<y.v[12])? x.v[12]:y.v[12];
	rst.v[13]=(x.v[13]<y.v[13])? x.v[13]:y.v[13];
	rst.v[14]=(x.v[14]<y.v[14])? x.v[14]:y.v[14];
	rst.v[15]=(x.v[15]<y.v[15])? x.v[15]:y.v[15];
	return rst;
}


uchar2 min(uchar2 x, int y){
	uchar2 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
  return rst;
}
uchar3 min(uchar3 x, int y){
	uchar3 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
  return rst;
}
uchar4 min(uchar4 x, int y){
	uchar4 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
	rst.v[3]=(x.v[3]<y)? x.v[3]:y;
  return rst;
}
uchar8 min(uchar8 x, int y){
	uchar8 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
	rst.v[3]=(x.v[3]<y)? x.v[3]:y;
	rst.v[4]=(x.v[4]<y)? x.v[4]:y;
	rst.v[5]=(x.v[5]<y)? x.v[5]:y;
	rst.v[6]=(x.v[6]<y)? x.v[6]:y;
	rst.v[7]=(x.v[7]<y)? x.v[7]:y;
  return rst;
}
uchar16 min(uchar16 x, int y){
	uchar16 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
	rst.v[3]=(x.v[3]<y)? x.v[3]:y;
	rst.v[4]=(x.v[4]<y)? x.v[4]:y;
	rst.v[5]=(x.v[5]<y)? x.v[5]:y;
	rst.v[6]=(x.v[6]<y)? x.v[6]:y;
	rst.v[7]=(x.v[7]<y)? x.v[7]:y;
	rst.v[8]=(x.v[8]<y)? x.v[8]:y;
	rst.v[9]=(x.v[9]<y)? x.v[9]:y;
	rst.v[10]=(x.v[10]<y)? x.v[10]:y;
	rst.v[11]=(x.v[11]<y)? x.v[11]:y;
	rst.v[12]=(x.v[12]<y)? x.v[12]:y;
	rst.v[13]=(x.v[13]<y)? x.v[13]:y;
	rst.v[14]=(x.v[14]<y)? x.v[14]:y;
	rst.v[15]=(x.v[15]<y)? x.v[15]:y;
	return rst;
}

uchar2 min(uchar2 x, uint y){
	uchar2 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
  return rst;
}
uchar3 min(uchar3 x, uint y){
	uchar3 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
  return rst;
}
uchar4 min(uchar4 x, uint y){
	uchar4 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
	rst.v[3]=(x.v[3]<y)? x.v[3]:y;
  return rst;
}
uchar8 min(uchar8 x, uint y){
	uchar8 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
	rst.v[3]=(x.v[3]<y)? x.v[3]:y;
	rst.v[4]=(x.v[4]<y)? x.v[4]:y;
	rst.v[5]=(x.v[5]<y)? x.v[5]:y;
	rst.v[6]=(x.v[6]<y)? x.v[6]:y;
	rst.v[7]=(x.v[7]<y)? x.v[7]:y;
  return rst;
}
uchar16 min(uchar16 x, uint y){
	uchar16 rst;
	rst.v[0]=(x.v[0]<y)? x.v[0]:y;
	rst.v[1]=(x.v[1]<y)? x.v[1]:y;
	rst.v[2]=(x.v[2]<y)? x.v[2]:y;
	rst.v[3]=(x.v[3]<y)? x.v[3]:y;
	rst.v[4]=(x.v[4]<y)? x.v[4]:y;
	rst.v[5]=(x.v[5]<y)? x.v[5]:y;
	rst.v[6]=(x.v[6]<y)? x.v[6]:y;
	rst.v[7]=(x.v[7]<y)? x.v[7]:y;
	rst.v[8]=(x.v[8]<y)? x.v[8]:y;
	rst.v[9]=(x.v[9]<y)? x.v[9]:y;
	rst.v[10]=(x.v[10]<y)? x.v[10]:y;
	rst.v[11]=(x.v[11]<y)? x.v[11]:y;
	rst.v[12]=(x.v[12]<y)? x.v[12]:y;
	rst.v[13]=(x.v[13]<y)? x.v[13]:y;
	rst.v[14]=(x.v[14]<y)? x.v[14]:y;
	rst.v[15]=(x.v[15]<y)? x.v[15]:y;
	return rst;
}
