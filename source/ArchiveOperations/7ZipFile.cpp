/****************************************************************************
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
 * ZipFile.cpp
 *
 * ZipFile Class
 * for WiiXplorer 2009
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"
#include "7ZipFile.h"
#include "Language/gettext.h"

#define BLOCKSIZE   2048

extern bool actioncanceled;

// 7zip error list
static const char * szerrormsg[9] = {
   tr("File is corrupt."), // 7z: Data error
   tr("Archive contains too many files."), // 7z: Out of memory
   tr("File is corrupt (CRC mismatch)."), // 7z: CRC Error
   tr("File uses unsupported compression settings."), // 7z: Not implemented
   tr("File is corrupt."), // 7z: Fail
   tr("Failed to read file data."), // 7z: Data read failure
   tr("File is corrupt."), // 7z: Archive error
   tr("File uses too high of compression settings (dictionary size is too large)."), // 7z: Dictionary too large
   tr("Process canceled."), // 7z: Dictionary too large
};

static const char * currentfilename = NULL;
static u32 currentfilesize = 0;

SzFile::SzFile(const char *filepath)
{
	SzBlockIndex = 0xFFFFFFFF;

	SzArchiveStream.offset = 0;
	SzArchiveStream.len = FileSize(filepath);
	SzArchiveStream.pos = 0;
	//SzArchiveStream.tempBuffer = (Byte*) malloc(BLOCKSIZE);

	if(!filepath || !SzArchiveStream.len)
	{
	    SzResult = 1;
		return;
	}

	SzArchiveStream.pFile = fopen (filepath, "rb");
	if(!SzArchiveStream.pFile)
	{
	    SzResult = 1;
		return;
	}

    char checkbuffer[6];
    memset(checkbuffer, 0, 6);
    fread(&checkbuffer, 1, 6, SzArchiveStream.pFile);
    rewind(SzArchiveStream.pFile);

    if(!Is7ZipFile(checkbuffer))
	{
	    SzResult = 1;
        return;
	}

	// set handler functions for reading data from SD/USB/SMB/DVD
	SzArchiveStream.InStream.Read = SzFileReadImp;
	SzArchiveStream.InStream.Seek = SzFileSeekImp;

	// set default 7Zip SDK handlers for allocation and freeing memory
	SzAllocImp.Alloc = SzAlloc;
	SzAllocImp.Free = SzFree;
	SzAllocTempImp.Alloc = SzAllocTemp;
	SzAllocTempImp.Free = SzFreeTemp;

	// prepare CRC and 7Zip database structures
	InitCrcTable();
	SzArDbExInit(&SzArchiveDb);

	// open the archive
	SzResult = SzArchiveOpen(&SzArchiveStream.InStream, &SzArchiveDb, &SzAllocImp,
			&SzAllocTempImp);

	if (SzResult != SZ_OK)
	{
		DisplayError(SzResult);
	}
}

SzFile::~SzFile()
{
    //if(SzArchiveStream.tempBuffer)
        //free(SzArchiveStream.tempBuffer);

	if(SzArchiveDb.Database.NumFiles > 0)
		SzArDbExFree(&SzArchiveDb, SzAllocImp.Free);

    fclose(SzArchiveStream.pFile);
}

bool SzFile::Is7ZipFile (const char *buffer)
{
	unsigned int *check;
	check = (unsigned int *) buffer;

	// 7z signature
	static Byte Signature[6] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};

	int i;
	for(i = 0; i < 6; i++)
		if(buffer[i] != Signature[i])
			return false;

	return true; // 7z archive found
}

ArchiveFileStruct * SzFile::GetFileStruct(int ind)
{
    if(ind > (int) SzArchiveDb.Database.NumFiles || ind < 0)
        return NULL;

    CFileItem * SzFileItem = SzArchiveDb.Database.Files + ind;

    CurArcFile.filename = SzFileItem->Name;
    CurArcFile.length = SzFileItem->Size;
    CurArcFile.comp_length = 0;
    CurArcFile.isdir = SzFileItem->IsDirectory;
    CurArcFile.fileindex = ind;

    return &CurArcFile;
}

void SzFile::DisplayError(SZ_RESULT res)
{
	StopProgress();
	WindowPrompt("7z decompression failed:", szerrormsg[(res - 1)], "OK");
}

u32 SzFile::GetItemCount()
{
	return SzArchiveDb.Database.NumFiles;
}

int SzFile::ExtractFile(int fileindex, const char * outpath, bool withpath)
{
    if(SzResult != SZ_OK)
        return -1;

    if(!GetFileStruct(fileindex))
        return -2;

    //startup timer
    ShowProgress(0, 100, " ");

	// reset variables
	size_t SzBlockIndex = 0xFFFFFFFF;
	size_t SzOffset = 0;
    size_t SzSizeProcessed = 0;
    size_t SzBufferSize = 0;

	currentfilename = CurArcFile.filename;
	currentfilesize = CurArcFile.length;

	char * RealFilename = strrchr(currentfilename, '/')+1;

	char writepath[MAXPATHLEN];
	if(!RealFilename || withpath)
        snprintf(writepath, sizeof(writepath), "%s/%s", outpath, CurArcFile.filename);
    else
        snprintf(writepath, sizeof(writepath), "%s/%s", outpath, RealFilename);

    if(CurArcFile.isdir)
    {
        strncat(writepath, "/", sizeof(writepath));
        CreateSubfolder(writepath);
        return 1;
    }

    char * temppath = strdup(writepath);
    char * pointer = strrchr(temppath, '/')+1;
    if(pointer)
        pointer[0] = '\0';

    CreateSubfolder(temppath);

    free(temppath);
    temppath = NULL;

	FILE * wFile = fopen(writepath, "wb");
	if(!wFile)
        return -3;

	// Extract
	SzResult = SzExtract3(&SzArchiveStream.InStream, &SzArchiveDb, fileindex,
                            &SzBlockIndex, wFile, &SzBufferSize, &SzOffset,
                            &SzSizeProcessed, &SzAllocImp, &SzAllocTempImp);

    fclose(wFile);
	currentfilename = NULL;
	currentfilesize = 0;

	// check for errors
	if(SzResult != SZ_OK)
	{
		// display error message
		DisplayError(SzResult);
		return -4;
	}

	return 1;
}

int SzFile::ExtractAll(const char * destpath)
{
    if(!destpath)
        return -5;

    StartProgress("Extracting files...");

    for(u32 i = 0; i < SzArchiveDb.Database.NumFiles; i++)
	{
        CFileItem * SzFileItem = SzArchiveDb.Database.Files + i;

        char path[MAXPATHLEN];
        snprintf(path, sizeof(path), "%s/%s", destpath, SzFileItem->Name);
        u32 filesize = SzFileItem->Size;

        if(!filesize)
            continue;

        if(SzFileItem->IsDirectory)
            continue;

        char * pointer = strrchr(path, '/')+1;

        if(pointer)
        {
            //cut off filename
            pointer[0] = '\0';
        }
        else
            continue; //shouldn't ever happen but to be sure, skip the file if it does

        CreateSubfolder(path);

        ExtractFile(i, path);
	}
	StopProgress();

	return 1;
}

// function used by the 7zip SDK to read data from SD/USB/DVD/SMB
SZ_RESULT SzFile::SzFileReadImp(void *object, void **buffer, size_t maxRequiredSize, size_t *processedSize)
{
	u32 sizeread = 0;

	if(maxRequiredSize == 0)
		return SZ_OK;

    if(actioncanceled)
        return SZE_FAILREAD;

	// the void* object is a SzFileInStream
	SzFileInStream *s = (SzFileInStream *) object;

    ShowProgress(s->pos, currentfilesize, currentfilename);

	if (maxRequiredSize > BLOCKSIZE)
		maxRequiredSize = BLOCKSIZE;

	// read data
	sizeread = fread(s->tempBuffer, 1, maxRequiredSize, s->pFile);

	if(sizeread <= 0)
		return SZE_FAILREAD;

	*buffer = s->tempBuffer;
	*processedSize = sizeread;
	s->pos += sizeread;

	return SZ_OK;
}

// function used by the 7zip SDK to change the filepointer
SZ_RESULT SzFile::SzFileSeekImp(void *object, CFileSize pos)
{
	// the void* object is a SzFileInStream
	SzFileInStream *s = (SzFileInStream *) object;

	// check if the 7z SDK wants to move the pointer to somewhere after the EOF
	if (pos >= s->len)
		return SZE_FAIL;

	// save new position and return
	if(fseek(s->pFile, (long)pos, SEEK_SET) != 0)
		return SZE_FAIL;

	s->pos = pos;
	return SZ_OK;
}
