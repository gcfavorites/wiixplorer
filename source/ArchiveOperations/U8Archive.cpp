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
 * U8Archive.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"
#include "U8Archive.h"

extern bool actioncanceled;

static char *u8Filename(const U8Entry *fst, int i)
{
	return (char *)(fst + fst[0].numEntries) + fst[i].nameOffset;
}

U8Archive::U8Archive(const char *filepath)
{
    FileBuffer = NULL;
    FileSize = 0;

    u8 * buffer = NULL;
    u64 filesize = 0;
    LoadFileToMem(filepath, &buffer, &filesize);

    if(buffer)
    {
        LoadFile(buffer, filesize);
        free(buffer);
    }
}

U8Archive::U8Archive(const u8 * Buffer, u32 Size)
{
    FileBuffer = NULL;
    FileSize = 0;

    if(Buffer)
    {
        LoadFile(Buffer, Size);
    }
}

U8Archive::~U8Archive()
{
    ClearList();

    if(FileBuffer)
        free(FileBuffer);
}

bool U8Archive::LoadFile(const u8 * Buffer, u32 Size)
{
    if(FileBuffer)
        free(FileBuffer);

    FileBuffer = NULL;
    FileSize = 0;

    if(!Buffer)
        return false;

    FileBuffer = (u8 *) malloc(Size);
    if(!FileBuffer)
        return false;

    FileSize = Size;

    memcpy(FileBuffer, Buffer, FileSize);

    return ParseFile();
}

bool U8Archive::ParseFile()
{
    if(!FileBuffer)
        return false;

    ClearList();

	const IMETHeader * IMET_Header = (IMETHeader *) FileBuffer;
	const IMD5Header * IMD5_Header = (IMD5Header *) FileBuffer;
	const U8Header   * U8_Header = (U8Header *) FileBuffer;
	u32 U8HeaderOffset = 0;

    //It's opening.bnr
	if (IMET_Header->fcc == 0x494D4554 /*"IMET"*/)
	{
	    //Add header.imet as a file
        AddListEntrie("header.imet", sizeof(IMETHeader), sizeof(IMETHeader), false, 0, 0, U8Arch);
        BufferOffset.push_back(0);

        U8HeaderOffset = sizeof(IMETHeader);

        const U8Header * bnrArcHdr = (U8Header *)(IMET_Header + 1);

        return ParseU8Header(bnrArcHdr, U8HeaderOffset);
	}

	//It's icon.bin/banner.bin/sound.bin
	else if(IMD5_Header->fcc == 0x494D4435 /* IMD5 */)
	{
	    const u8 * BinBuffer = (const u8 *)(IMD5_Header+1);
	    FileSize = ((IMD5Header *) BinBuffer)->filesize;
        U8HeaderOffset = sizeof(IMD5Header);

        if(*((u32*) BinBuffer) == 0x4C5A3737 /* LZ77 */)
        {
            u8 * UncBinBuffer = uncompressLZ77(BinBuffer, FileSize-sizeof(IMD5Header), &FileSize);
            if(!UncBinBuffer)
                return false;

            free(FileBuffer);
            BinBuffer = UncBinBuffer;
            FileBuffer = UncBinBuffer;
            U8HeaderOffset = 0;
        }

        const U8Header * bnrArcHdr = (U8Header *) BinBuffer;

        return ParseU8Header(bnrArcHdr, U8HeaderOffset);
	}
	//It's a direct U8Archive...weird but oh well...
	else if(U8_Header->fcc == 0x55AA382D /* U.8- */)
	{
        return ParseU8Header(U8_Header, 0);
	}

    //Unknown U8Archive
    ClearList();
    free(FileBuffer);
    FileBuffer = NULL;
    FileSize = 0;

    return false;
}

bool U8Archive::ParseU8Header(const U8Header * bnrArcHdr, u32 U8HeaderOffset)
{
    if(bnrArcHdr->fcc != 0x55AA382D /* U.8- */)
    {
        ClearList();
        free(FileBuffer);
        FileBuffer = NULL;
        FileSize = 0;
        return false;
    }

    const U8Entry * fst = (const U8Entry *) (((const u8 *) bnrArcHdr) + bnrArcHdr->rootNodeOffset);

    char filename[MAXPATHLEN];
	char directory[MAXPATHLEN];
	strcpy(directory, "");
	u32 dir_stack[100];
	int dir_index = 0;

	for (u32 i = 1; i < fst[0].numEntries; ++i)
    {
        snprintf(filename, sizeof(filename), "%s%s", directory, u8Filename(fst, i));

        bool isDir = (fst[i].fileType == 0) ? false : true;

        if(isDir)
        {
            dir_stack[++dir_index] = fst[i].fileLength;
            char tmp[MAXPATHLEN];
            snprintf(tmp, sizeof(tmp), "%s/", u8Filename(fst, i));
            strncat(directory, tmp, sizeof(directory));
        }

        AddListEntrie(filename, fst[i].fileLength, fst[i].fileLength, isDir, GetItemCount(), 0, U8Arch);
        BufferOffset.push_back(U8HeaderOffset+fst[i].fileOffset);

        while (dir_stack[dir_index] == i+1 && dir_index > 0)
        {
            if(directory[strlen(directory)-1] == '/')
                directory[strlen(directory)-1] = '\0';

            char * ptr = strrchr(directory, '/');
            if(ptr)
            {
                ptr++;
                ptr[0] = '\0';
            }
            dir_index--;
        }
    }

    return true;
}

ArchiveFileStruct * U8Archive::GetFileStruct(int ind)
{
    if(ind > (int) PathStructure.size() || ind < 0)
        return NULL;

    return PathStructure.at(ind);
}

void U8Archive::AddListEntrie(const char * filename, size_t length, size_t comp_length, bool isdir, u32 index, u64 modtime, u8 Type)
{
    if(!filename)
        return;

    ArchiveFileStruct * TempStruct = new ArchiveFileStruct;
    TempStruct->filename = new char[strlen(filename)+1];
    sprintf(TempStruct->filename, "%s", filename);
    TempStruct->length = length;
    TempStruct->comp_length = comp_length;
    TempStruct->isdir = isdir;
    TempStruct->fileindex = index;
    TempStruct->ModTime = modtime;
    TempStruct->archiveType = Type;

    PathStructure.push_back(TempStruct);
    TempStruct = NULL;
}

void U8Archive::ClearList()
{
    for(u32 i = 0; i < PathStructure.size(); i++)
    {
        if(PathStructure.at(i)->filename != NULL)
        {
            delete [] PathStructure.at(i)->filename;
            PathStructure.at(i)->filename = NULL;
        }
    }

    PathStructure.clear();
    BufferOffset.clear();
}

int U8Archive::ExtractFile(int ind, const char *dest, bool withpath)
{
    if(!FileBuffer)
        return -1;

    ArchiveFileStruct * File = GetFileStruct(ind);
    if(!File)
        return -2;

	char * RealFilename = strrchr(File->filename, '/');
	if(RealFilename)
        RealFilename += 1;
    else
        RealFilename = File->filename;

	char writepath[MAXPATHLEN];
	if(withpath)
        snprintf(writepath, sizeof(writepath), "%s/%s", dest, File->filename);
    else
        snprintf(writepath, sizeof(writepath), "%s/%s", dest, RealFilename);

    if(File->isdir)
    {
        strncat(writepath, "/", sizeof(writepath));
        CreateSubfolder(writepath);
        return 1;
    }

    char * temppath = strdup(writepath);
    char * pointer = strrchr(temppath, '/');
    if(pointer)
    {
        pointer += 1;
        pointer[0] = '\0';
    }

    CreateSubfolder(temppath);

    free(temppath);
    temppath = NULL;

    u32 blocksize = 1024*50;
    const u8 * buffer = FileBuffer+BufferOffset.at(File->fileindex);
	u32 filesize = File->length;

    FILE *pfile = fopen(writepath, "wb");
    if(!pfile)
    {
        StopProgress();
        fclose(pfile);
        WindowPrompt(tr("Could not extract file:"), File->filename, "OK");
        return -3;
    }

    u32 done = 0;

    do
    {
        if(actioncanceled)
        {
            usleep(20000);
            fclose(pfile);
            StopProgress();
            return -10;
        }

        ShowProgress(done, filesize, RealFilename);

        if(filesize - done < blocksize)
            blocksize = filesize - done;

        int ret = fwrite(buffer+done, 1, blocksize, pfile);
        if(ret < 0)
        {
            usleep(20000);
            fclose(pfile);
            StopProgress();
            return -3;
        }

        done += ret;
    }
    while(done < filesize);

    fclose(pfile);

    return done;
}

int U8Archive::ExtractAll(const char * destpath)
{
    StartProgress(tr("Extracting files..."));

    for(u32 i = 0; i < PathStructure.size(); i++)
    {
        int ret = ExtractFile(PathStructure.at(i)->fileindex, destpath, true);
        if(ret < 0)
            return ret;
    }

    StopProgress();

    return 1;
}
