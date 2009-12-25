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
 * 7ZipFile.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef _7ZIPFILE_H_
#define _7ZIPFILE_H_

extern "C" {
#include "sevenzip/7zCrc.h"
#include "sevenzip/7zIn.h"
#include "sevenzip/7zExtract.h"
}

typedef struct
{
	char * filename; // full filename
	size_t length; // uncompressed file length in 64 bytes for sizes higher than 4GB
	size_t comp_length; // compressed file length in 64 bytes for sizes higher than 4GB
	bool isdir; // 0 - file, 1 - directory
	u32 fileindex; // fileindex number
} ArchiveFileStruct;

typedef struct _SzFileInStream
{
   ISzInStream InStream;
   off_t offset; // offset of the file
   size_t len; // length of the file
   size_t pos;  // current position of the file pointer
   FILE * pFile;  // current file pointer
   Byte tempBuffer[2048];  // current file pointer
} SzFileInStream;


class SzFile
{
    public:
		//!Constructor
        SzFile(const char *filepath);
		//!Destructor
		~SzFile();
		//!Check if it is a 7zip file
        bool Is7ZipFile (const char *buffer);
		//!Get the archive file structure
        ArchiveFileStruct * GetFileStruct(int fileIndx);
		//!Extract file from a 7zip to file
        int ExtractFile(int fileindex, const char * outpath, bool withpath = false);
		//!Extract all files from the 7zip to a path
        int ExtractAll(const char * destpath);
		//!Get the total amount of items inside the archive
        u32 GetItemCount();

    private:
        void DisplayError(SZ_RESULT res);

        ArchiveFileStruct CurArcFile;
        SZ_RESULT SzResult;
        SzFileInStream SzArchiveStream;
        CArchiveDatabaseEx SzArchiveDb;
        ISzAlloc SzAllocImp;
        ISzAlloc SzAllocTempImp;
        UInt32 SzBlockIndex;
        CFileItem * SzFileItem;

        static SZ_RESULT SzFileReadImp(void *object, void **buffer, size_t maxRequiredSize, size_t *processedSize);
        static SZ_RESULT SzFileSeekImp(void *object, CFileSize pos);
};

#endif
