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
#ifndef __TOOLS_H
#define __TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>
#include <ogc/lwp_watchdog.h>

typedef struct _TimeStruct
{
    u32 tm_sec;            /* seconds after the minute - [0,59] */
    u32 tm_min;            /* minutes after the hour - [0,59] */
    u32 tm_hour;           /* hours since midnight - [0,23] */
    u32 tm_mday;           /* day of the month - [1,31] */
    u32 tm_mon;            /* months since January - [0,11] */
    u32 tm_year;           /* years - [1980..2044] */
} TimeStruct;

struct LZ77Info
{
	u16 length : 4;
	u16 offset : 12;
} __attribute__((packed));

bool TimePassed(int limit);
void ConvertDosDate(u64 ulDosDate, TimeStruct * ptm);
void ConvertNTFSDate(u64 ulNTFSDate,  TimeStruct * ptm);

void ShowError(const char * format, ...);
const char * fmt(const char * format, ...);
const wchar_t * wfmt(const char * format, ...);
bool char2wchar_t(const char * src, wchar_t * dest);

void EncryptString(const char *src, char *dst);
void DecryptString(const char *src, char *dst);

u8 * uncompressLZ77(const u8 *inBuf, u32 inLength, u32 * uncSize);
u32 CheckIMD5Type(const u8 * buffer, int length);

#ifdef __cplusplus
}

class Timer
{
    public:
        Timer() { starttick = gettime(); };
        ~Timer() { };
        float elapsed() { return (float) (gettime()-starttick)/(1000.0f*TB_TIMER_CLOCK); };
        float elapsed_millisecs() { return 1000.0f*elapsed(); };
        void reset() { starttick = gettime(); }
    protected:
        u64 starttick;
};

#endif //__cplusplus

#endif /* SVNREV_H */
