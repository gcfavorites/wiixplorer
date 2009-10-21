/*-------------------------------------------------------------
 from any title deleter and wad manager 1.4
title.h --

Copyright (C) 2008 tona and/or waninkoko
-------------------------------------------------------------*/
#include <gccore.h>
#include <ogcsys.h>
#include <string.h>
#include <stdio.h>
#include <fat.h>
#include <malloc.h>

#ifndef _TITLE_H_
#define _TITLE_H_

#ifdef __cplusplus
extern "C" {
#endif
    /* Constants */
#define BLOCK_SIZE			1024
#define MAX_TITLES			256
#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))

// Get the number of titles on the Wii of a given type
    s32 getTitles_TypeCount(u32 type, u32 *count);

// Get the list of titles of this type
    s32 getTitles_Type(u32 type, u32 *titles, u32 count);

//boot HBC in either HAXX or JODI locations
	s32 WII_BootHBC();

#ifdef __cplusplus
}
#endif

#endif
