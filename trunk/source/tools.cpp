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
