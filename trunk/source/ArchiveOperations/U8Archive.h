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
 * U8Archive.h
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef _U8ARCHIVE_H_
#define _U8ARCHIVE_H_

#include "WiiArchive.h"

using namespace std;

struct IMD5Header
{
	u32 fcc;
	u32 filesize;
	u8 zeroes[8];
	u8 crypto[16];
} __attribute__((packed));

struct IMETHeader
{
	u8 zeroes[64];
	u32 fcc;
	u8 unk[8];
	u32 iconSize;
	u32 bannerSize;
	u32 soundSize;
	u32 flag1;
	u8 names[7][84];
	u8 zeroes_2[0x348];
	u8 crypto[16];
} __attribute__((packed));

struct U8Header
{
	u32 fcc;
	u32 rootNodeOffset;
	u32 headerSize;
	u32 dataOffset;
	u8 zeroes[16];
} __attribute__((packed));

struct U8Entry
{
	struct
	{
		u32 fileType : 8;
		u32 nameOffset : 24;
	};
	u32 fileOffset;
	union
	{
		u32 fileLength;
		u32 numEntries;
	};
} __attribute__((packed));

class U8Archive : public WiiArchive
{
    public:
		//!Constructor
        U8Archive(const char *filepath);
        //!Overload
        U8Archive(const u8 * Buffer, u32 Size);
		//!Destructor
		~U8Archive();
    protected:
        //!Parse the archive
        bool ParseFile();
        bool ParseU8Header(u32 U8HeaderOffset);
        //!Get the U8Filename
        void U8Filename(const U8Entry * fst, int fstoffset, int i, string & Filename);
};

#endif
