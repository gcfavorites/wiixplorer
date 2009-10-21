#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <ogcsys.h>

#include "devicemounter.h"
#include "title.h"

/*-------------------------------------------------------------
 from any title deleter
titles.c -- functions for grabbing all titles of a certain type

Copyright (C) 2008 tona
-------------------------------------------------------------*/

u32 __titles_init = 0;
u32 __num_titles;
static u64 __title_list[MAX_TITLES] ATTRIBUTE_ALIGN(32);

s32 __getTitles() {
    s32 ret;
    ret = ES_GetNumTitles(&__num_titles);
    if (ret <0)
        return ret;
    if (__num_titles > MAX_TITLES)
        return -1;
    ret = ES_GetTitles(__title_list, __num_titles);
    if (ret <0)
        return ret;
    __titles_init = 1;
    return 0;
}

s32 getTitles_TypeCount(u32 type, u32 *count) {
    s32 ret = 0;
    u32 type_count;
    if (!__titles_init)
        ret = __getTitles();
    if (ret <0)
        return ret;
    int i;
    type_count = 0;
    for (i=0; i < __num_titles; i++) {
        u32 upper, lower;
        upper = __title_list[i] >> 32;
        lower = __title_list[i] & 0xFFFFFFFF;
        if ((upper == type)&&
                ((lower !=0x48414741)&&//this filters out haga,haaa, hafa.  dupe factory channels that don't load
                 (lower !=0x48414141)&&//since we dont care about apps that dont load for what we are doing
                 (lower !=0x48414641)))
            type_count++;
    }
    *count = type_count;
    return ret;
}

s32 getTitles_Type(u32 type, u32 *titles, u32 count) {
    s32 ret = 0;
    u32 type_count;
    if (!__titles_init)
        ret = __getTitles();
    if (ret <0)
        return ret;
    int i;
    type_count = 0;
    for (i=0; type_count < count && i < __num_titles; i++) {
        u32 upper, lower;
        upper = __title_list[i] >> 32;
        lower = __title_list[i] & 0xFFFFFFFF;
        if ((upper == type)&&
                ((lower !=0x48414741)&&
                 (lower !=0x48414141)&&
                 (lower !=0x48414641))) {
            titles[type_count]=lower;
            type_count++;
        }
    }
    if (type_count < count)
        return -2;
    __titles_init = 0;
    return 0;
}


//this function expects initialize be called before it is called
// if not, it will fail miserably and catch the wii on fire and kick you in the nuts
s32 WII_BootHBC()
{
	u32 tmdsize;
	u64 tid = 0;
	u64 *list;
	u32 titlecount;
	s32 ret;
	u32 i;

	ret = ES_GetNumTitles(&titlecount);
	if(ret < 0)
		return WII_EINTERNAL;

	list = memalign(32, titlecount * sizeof(u64) + 32);

	ret = ES_GetTitles(list, titlecount);
	if(ret < 0) {
		free(list);
		return WII_EINTERNAL;
	}
	
	for(i=0; i<titlecount; i++) {
		if (list[i]==TITLE_ID(0x00010001,0x4A4F4449) 
			|| list[i]==TITLE_ID(0x00010001,0x48415858))
		{
			tid = list[i];
			break;
		}
	}
	free(list);

	if(!tid)
		return WII_EINSTALL;

	if(ES_GetStoredTMDSize(tid, &tmdsize) < 0)
		return WII_EINSTALL;

	return WII_LaunchTitle(tid);
}



