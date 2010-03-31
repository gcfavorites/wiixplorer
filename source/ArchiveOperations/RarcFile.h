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
 * RarcFile.h
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef _RARCFILE_H_
#define _RARCFILE_H_

#include "WiiArchive.h"

typedef struct
{
  u32 magic; //'RARC'
  u32 size; //size of the file
  u32 unknown;
  u32 dataStartOffset; //where does the actual data start? You have to add 0x20 to this value.
  u32 unknown2[4];

  u32 numNodes;
  u32 unknown3[2];
  u32 fileEntriesOffset;
  u32 unknown4;
  u32 stringTableOffset; //where is the string table stored? You have to add 0x20 to this value.
  u32 unknown5[2];
} RarcHeader;

typedef struct
{
  char type[4];
  u32 filenameOffset; //directory name, offset into string table
  u16 unknown;
  u16 numFileEntries; //how many files belong to this node?
  u32 firstFileEntryOffset;
} RarcNode;

typedef struct
{
  u16 id; //file id. If this is 0xFFFF, then this entry is a subdirectory link
  u16 unknown;
  u16 unknown2;
  u16 filenameOffset; //file/subdir name, offset into string table
  u32 dataOffset; //offset to file data (for subdirs: index of Node representing the subdir)
  u32 dataSize; //size of data
  u32 zero; //seems to be always '0'
} RarcFileEntry;

class RarcFile : public WiiArchive
{
    public:
		//!Constructor
        RarcFile(const char *filepath);
        //!Overload
        RarcFile(const u8 * Buffer, u32 Size);
		//!Destructor
		~RarcFile();
    protected:
        //!Parse the archive
        bool ParseFile();
        bool ParseRarcHeader();
        void ParseNode(RarcNode * Node, string & parentDirectory);
        //!Get the Filename
        void GetFilename(int offset, string & Filename);

        RarcHeader Header;
        int ItemIndex;
};

#endif
