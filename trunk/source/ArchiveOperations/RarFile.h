/***************************************************************************
 * Copyright (C) 2009
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
 * RarFile.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef _RARFILE_H_
#define _RARFILE_H_

#include <vector>
#include <algorithm >

#include "7ZipFile.h"
#include "unrarlib/unrarlib.h"


class RarFile
{
    public:
		//!Constructor
        RarFile(const char *filepath);
		//!Destructor
		~RarFile();
		//!Get the archive file structure
        ArchiveFileStruct * GetFileStruct(int fileIndx);
		//!Extract file from a 7zip to file
        int ExtractFile(int fileindex, const char * outpath, bool withpath = false);
		//!Extract all files from the RAR to a path
        int ExtractAll(const char * destpath);
		//!Get the total amount of items inside the archive
        u32 GetItemCount();

    private:
        bool LoadList(const char * filepath);
        void ClearList();

        ArchiveList_struct * ArcList;
        std::vector<ArchiveFileStruct *> RarStructure;
};

#endif
