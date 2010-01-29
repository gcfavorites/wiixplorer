#ifndef __TOOLS_H
#define __TOOLS_H

#include <gctypes.h>

typedef struct _TimeStruct
{
    u32 tm_sec;            /* seconds after the minute - [0,59] */
    u32 tm_min;            /* minutes after the hour - [0,59] */
    u32 tm_hour;           /* hours since midnight - [0,23] */
    u32 tm_mday;           /* day of the month - [1,31] */
    u32 tm_mon;            /* months since January - [0,11] */
    u32 tm_year;           /* years - [1980..2044] */
} TimeStruct;

int ROUND2FOUR(double x);
bool TimePassed(int limit);
void ConvertDosDate(u64 ulDosDate, TimeStruct * ptm);
void ConvertNTFSDate(u64 ulNTFSDate,  TimeStruct * ptm);

#endif /* SVNREV_H */
