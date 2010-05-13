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
#include <string.h>
#include <math.h>
#include "Resample.h"

void Init3BandState(EQState *es,s32 lowfreq,s32 highfreq,s32 mixfreq)
{
	memset(es,0,sizeof(EQState));

	es->lg = 1.0;
	es->mg = 1.0;
	es->hg = 1.0;

	es->lf = 2.0F*sinf(M_PI*((f32)lowfreq/(f32)mixfreq));
	es->hf = 2.0F*sinf(M_PI*((f32)highfreq/(f32)mixfreq));
}

int ConvertMonoToStereo(const u8 * source, u8 * dest, int size, int maxsize)
{
    u32 counter = 0;
    s16 * src = (s16 *) source;
    s16 * dst = (s16 *) dest;
    u8 * buf_end = dest+maxsize;

    while(counter < size && ((u8 *) dst) < buf_end)
    {
        *dst++ = *src++;
        *dst++ = *src;
        counter += 2;
    }

    return counter*2;
}

int Convert8BitTo16Bit(const u8 * source, u8 * dest, int size, int maxsize)
{
    u32 counter = 0;
    s16 * dst = (s16 *) dest;
    u8 * buf_end = dest+maxsize;

    while(counter < size && ((u8 *) dst) < buf_end)
    {
        *dst++ = (((s16) *source++)-127)*255;
        counter++;
    }

    return counter*2;
}
