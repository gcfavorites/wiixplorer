/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <wchar.h>
#include <time.h>
#include <math.h>

#include "Prompts/PromptWindows.h"
#include "tools.h"

extern "C" void ShowError(const char * format, ...)
{
	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		WindowPrompt(tr("Error:"), tmp, tr("OK"));
		free(tmp);
	}
	va_end(va);
}

extern "C" bool TimePassed(int limit)
{
	static time_t starttime = 0;
	time_t timer = time(NULL);

	if (starttime == 0)
        starttime = timer;

    if(difftime(timer, starttime) >= limit)
    {
        starttime = 0;
        return true;
    }

    return false;
}

#define PERIOD_4 (4 * 365 + 1)
#define PERIOD_100 (PERIOD_4 * 25 - 1)
#define PERIOD_400 (PERIOD_100 * 4 + 1)
extern "C" void ConvertNTFSDate(u64 ulNTFSDate,  TimeStruct * ptm)
{
    unsigned year, mon, day, hour, min, sec;
    u64 v64 = ulNTFSDate;
    u8 ms[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    unsigned temp;
    u32 v;
    v64 /= 10000000;
    sec = (unsigned)(v64 % 60);
    v64 /= 60;
    min = (unsigned)(v64 % 60);
    v64 /= 60;
    hour = (unsigned)(v64 % 24)+1;
    v64 /= 24;

    v = (u32)v64;

    year = (unsigned)(1601 + v / PERIOD_400 * 400);
    v %= PERIOD_400;

    temp = (unsigned)(v / PERIOD_100);
    if (temp == 4)
    temp = 3;
    year += temp * 100;
    v -= temp * PERIOD_100;

    temp = v / PERIOD_4;
    if (temp == 25)
    temp = 24;
    year += temp * 4;
    v -= temp * PERIOD_4;

    temp = v / 365;
    if (temp == 4)
    temp = 3;
    year += temp;
    v -= temp * 365;

    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
    ms[1] = 29;
    for (mon = 1; mon <= 12; mon++)
    {
    unsigned s = ms[mon - 1];
    if (v < s)
        break;
    v -= s;
    }
    day = (unsigned)v + 1;

    ptm->tm_mday = (u32)day;
    ptm->tm_mon =  (u32)mon;
    ptm->tm_year = (u32)year;

    ptm->tm_hour = (u32)hour;
    ptm->tm_min =  (u32)min;
    ptm->tm_sec =  (u32)sec;
}

extern "C" void ConvertDosDate(u64 ulDosDate, TimeStruct * ptm)
{
    u32 uDate;
    uDate = (u32)(ulDosDate>>16);
    ptm->tm_mday = (u32)(uDate&0x1f) ;
    ptm->tm_mon =  (u32)((((uDate)&0x1E0)/0x20)) ;
    ptm->tm_year = (u32)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (u32) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (u32) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (u32) (2*(ulDosDate&0x1f)) ;
}

extern "C" const char * fmt(const char * format, ...)
{
    static char strChar[512];
    memset(strChar, 0, sizeof(strChar));
	char * tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        snprintf(strChar, sizeof(strChar), tmp);
		free(tmp);
		return (const char *) strChar;
	}
	va_end(va);

	return NULL;
}

extern "C" const wchar_t * wfmt(const char * format, ...)
{
    static wchar_t strWChar[512];
    memset(strWChar, 0, sizeof(strWChar));

	if(!format)
        return (const wchar_t *) &strWChar;

	if(strcmp(format, "") == 0)
        return (const wchar_t *) &strWChar;

	char * tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        int	bt;
        int strlength = strlen(tmp);
        bt = mbstowcs(strWChar, tmp, (strlength < 512) ? strlength : 512 );
        free(tmp);

        if(bt > 0)
            return (const wchar_t *) &strWChar;
	}
	va_end(va);

	return NULL;
}

extern "C" bool char2wchar_t(const char * strChar, wchar_t * dest)
{
    if(!strChar || !dest)
        return false;

    int	bt;
    bt = mbstowcs(dest, strChar, strlen(strChar));
    if (bt > 0) {
        dest[bt] = (wchar_t) '\0';
        return true;
    }

    return false;
}

extern "C" void EncryptString(const char *src, char *dst)
{
	u32 id;
	char sid[9], tmp[3];

	ES_GetDeviceID(&id);
	sprintf(sid, "%08x", id);

	dst[0] = 0;
	for (u32 i = 0; i < strlen(src); i++)
	{
		sprintf(tmp, "%02x", src[i] ^ sid[i%8]);
		strcat(dst, tmp);
	}
}

extern "C" void DecryptString(const char *src, char *dst)
{
	u32 id;
	char sid[9];

	ES_GetDeviceID(&id);
	sprintf(sid, "%08x", id);

	for (u32 i = 0; i < strlen(src); i += 2)
	{
		char c = (src[i] >= 'a' ? (src[i] - 'a') + 10 : (src[i] - '0')) << 4;
		c += (src[i+1] >= 'a' ? (src[i+1] - 'a') + 10 : (src[i+1] - '0'));
		dst[i>>1] = c ^ sid[(i>>1)%8];
	}
	dst[strlen(src)>>1] = 0;
}

static inline u32 le32(u32 i)
{
	return ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i & 0xFF0000) >> 8) | ((i & 0xFF000000) >> 24);
}

static inline u16 le16(u16 i)
{
	return ((i & 0xFF) << 8) | ((i & 0xFF00) >> 8);
}

extern "C" u8 * uncompressLZ77(const u8 *inBuf, u32 inLength, u32 * size)
{
	u8 *buffer = NULL;
	if (inLength <= 0x8 || *((const u32 *)inBuf) != 0x4C5A3737 /*"LZ77"*/ || inBuf[4] != 0x10)
		return NULL;

	u32 uncSize = le32(((const u32 *)inBuf)[1] << 8);

	const u8 *inBufEnd = inBuf + inLength;
	inBuf += 8;

	buffer = (u8 *) malloc(uncSize);

	if (!buffer)
		return buffer;

	u8 *bufCur = buffer;
	u8 *bufEnd = buffer + uncSize;

	while (bufCur < bufEnd && inBuf < inBufEnd)
	{
		u8 flags = *inBuf;
		++inBuf;
		for (int i = 0; i < 8 && bufCur < bufEnd && inBuf < inBufEnd; ++i)
		{
			if ((flags & 0x80) != 0)
			{
				const LZ77Info &info = *(const LZ77Info *)inBuf;
				inBuf += sizeof (LZ77Info);
				int length = info.length + 3;
				if (bufCur - info.offset - 1 < buffer || bufCur + length > bufEnd)
					return buffer;
				memcpy(bufCur, bufCur - info.offset - 1, length);
				bufCur += length;
			}
			else
			{
				*bufCur = *inBuf;
				++inBuf;
				++bufCur;
			}
			flags <<= 1;
		}
	}

	*size = uncSize;

	return buffer;
}

extern "C" u32 CheckIMD5Type(const u8 * buffer, int length)
{
    if(*((u32 *) buffer) != 'IMD5')
    {
        return *((u32 *) buffer);
    }

    const u8 * file = buffer+32;

    if(*((u32 *) file) != 'LZ77')
    {
        return *((u32 *) file);
    }

    u32 uncSize = 0;
    u8 * uncompressed_data = uncompressLZ77(file, length-32, &uncSize);
    if(!uncompressed_data)
        return 0;

    u32 * magic = (u32 *) uncompressed_data;
    u32 Type = magic[0];
    free(uncompressed_data);

    return Type;
}
