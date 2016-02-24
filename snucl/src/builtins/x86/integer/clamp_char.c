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
#include <integer/min.h>
#include <integer/max.h>

schar clamp(schar x, schar minval, schar maxval)
{
	return min(max(x, minval), maxval);
}

char2 clamp(char2 x, char2 minval, char2 maxval)
{
	char2 rst;
	rst[0] = min(max(x[0], minval[0]), maxval[0]);
	rst[1] = min(max(x[1], minval[1]), maxval[1]);
	return rst;
}

char3 clamp(char3 x, char3 minval, char3 maxval)
{
	char3 rst;
	for (int i = 0; i < 3; i++)
		rst[i] = min(max(x[i], minval[i]), maxval[i]);
	return rst;
}

char4 clamp(char4 x, char4 minval, char4 maxval)
{
	char4 rst;
	for (int i = 0; i < 4; i++)
		rst[i] = min(max(x[i], minval[i]), maxval[i]);
	return rst;
}

char8 clamp(char8 x, char8 minval, char8 maxval)
{
	char8 rst;
	for (int i = 0; i < 8; i++)
		rst[i] = min(max(x[i], minval[i]), maxval[i]);
	return rst;
}

char16 clamp(char16 x, char16 minval, char16 maxval)
{
	char16 rst;
	for (int i = 0; i < 16; i++)
		rst[i] = min(max(x[i], minval[i]), maxval[i]);
	return rst;
}


char2 clamp(char2 x, schar minval, schar maxval)
{
	char2 rst;
	rst[0] = min(max(x[0], minval), maxval);
	rst[1] = min(max(x[1], minval), maxval);
	return rst;
}

char3 clamp(char3 x, schar minval, schar maxval)
{
	char3 rst;
	for (int i = 0; i < 3; i++)
		rst[i] = min(max(x[i], minval), maxval);
	return rst;
}

char4 clamp(char4 x, schar minval, schar maxval)
{
	char4 rst;
	for (int i = 0; i < 4; i++)
		rst[i] = min(max(x[i], minval), maxval);
	return rst;
}

char8 clamp(char8 x, schar minval, schar maxval)
{
	char8 rst;
	for (int i = 0; i < 8; i++)
		rst[i] = min(max(x[i], minval), maxval);
	return rst;
}

char16 clamp(char16 x, schar minval, schar maxval)
{
	char16 rst;
	for (int i = 0; i < 16; i++)
		rst[i] = min(max(x[i], minval), maxval);
	return rst;
}


