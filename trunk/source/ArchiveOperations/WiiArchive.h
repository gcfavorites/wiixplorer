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
 * WiiArchive.h
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef _WIIARCHIVE_H_
#define _WIIARCHIVE_H_

#include <string>
#include "7ZipFile.h"

using namespace std;

class WiiArchive
{
    public:
		//!Constructor
        WiiArchive(const char *filepath);
        //!Overload
        WiiArchive(const u8 * Buffer, u32 Size);
		//!Destructor
		~WiiArchive();
		//!Load the file
        virtual bool LoadFile(const char * filepath);
		//!Load the file from a buffer
        virtual bool LoadFile(const u8 * Buffer, u32 Size);
		//!Get the archive file structure
        virtual ArchiveFileStruct * GetFileStruct(int fileIndx);
		//!Extract a file
		virtual int ExtractFile(int ind, const char *dest, bool withpath = false);
		//!Extract all files
		virtual int ExtractAll(const char *dest);
		//!Get the total amount of items inside the archive
        virtual u32 GetItemCount() { return PathStructure.size(); };
		//!Close File
		virtual void CloseFile();

    protected:
        //!Add archive listname
        void AddListEntrie(const char * filename, size_t length, size_t comp_length, bool isdir, u32 index, u64 modtime, u8 Type);
        //!Clear the list
        void ClearList();
        //!Raw read from the file
        size_t ReadFile(void * buffer, size_t size, off_t offset);

        bool FromMem;
        FILE * File;
        u8 * FileBuffer;
        u32 FileSize;
        vector<ArchiveFileStruct *> PathStructure;
        vector<u32> BufferOffset;
};

#endif
