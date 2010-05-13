/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * The functions from this files are taken from libmad.
 * All credits goes to the guys from libmad.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef RESAMPLE_H_
#define RESAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gccore.h>

typedef struct _eqstate_s
{
	f32 lf;
	f32 f1p0;
	f32 f1p1;
	f32 f1p2;
	f32 f1p3;

	f32 hf;
	f32 f2p0;
	f32 f2p1;
	f32 f2p2;
	f32 f2p3;

	f32 sdm1;
	f32 sdm2;
	f32 sdm3;

	f32 lg;
	f32 mg;
	f32 hg;
} EQState;

typedef union
{
	struct {
		u16 hi;
		u16 lo;
	} aword;
	u32 adword;
} dword;

void Init3BandState(EQState * es, s32 lowfreq, s32 highfreq, s32 mixfreq);
int Convert8BitTo16Bit(const u8 * source, u8 * dest, int size, int maxsize);
int ConvertMonoToStereo(const u8 * source, u8 * dest, int size, int maxsize);

#ifdef __cplusplus
}
#endif

#endif
