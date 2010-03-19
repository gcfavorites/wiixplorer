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

U8Archive::U8Archive(const char *filepath)
{
    File = NULL;
    FileBuffer = NULL;
    FileSize = 0;
    FromMem = false;

    LoadFile(filepath);
}

U8Archive::U8Archive(const u8 * Buffer, u32 Size)
{
    File = NULL;
    FileBuffer = NULL;
    FileSize = 0;
    FromMem = true;

    if(Buffer)
    {
        LoadFile(Buffer, Size);
    }
}

U8Archive::~U8Archive()
{
    CloseFile();
}

void U8Archive::CloseFile()
{
    ClearList();

    if(FileBuffer)
        free(FileBuffer);

    if(File)
        fclose(File);

    File = NULL;
    FileBuffer = NULL;
    FileSize = 0;
    FromMem = false;
}

bool U8Archive::LoadFile(const char * filepath)
{
    if(!filepath)
        return false;

    CloseFile();

    File = fopen(filepath, "rb");
    if(!File)
        return false;

    fseek(File, 0, SEEK_END);

    FileSize = ftell(File);
    rewind(File);

    FromMem = false;

    return ParseFile();
}

bool U8Archive::LoadFile(const u8 * Buffer, u32 Size)
{
    if(!Buffer)
        return false;

    CloseFile();

    FileBuffer = (u8 *) malloc(Size);
    if(!FileBuffer)
        return false;

    FileSize = Size;

    FromMem = true;

    memcpy(FileBuffer, Buffer, FileSize);

    return ParseFile();
}

bool U8Archive::ParseFile()
{
    if(!FileBuffer && !File)
        return false;

    ClearList();

    IMETHeader IMET_Header;
    ReadFile(&IMET_Header, sizeof(IMETHeader), 0);

	IMD5Header IMD5_Header;
	memcpy(&IMD5_Header, &IMET_Header, sizeof(IMD5Header));
	U8Header U8_Header;
	memcpy(&U8_Header, &IMET_Header, sizeof(U8Header));
	u32 U8HeaderOffset = 0;

    //It's opening.bnr
	if (IMET_Header.fcc == 'IMET')
	{
	    //Add header.imet as a file
        AddListEntrie("header.imet", sizeof(IMETHeader), sizeof(IMETHeader), false, 0, 0, U8Arch);
        BufferOffset.push_back(0);

        U8HeaderOffset = sizeof(IMETHeader);

        return ParseU8Header(U8HeaderOffset);
	}

	//It's icon.bin/banner.bin/sound.bin
	else if(IMD5_Header.fcc == 'IMD5')
	{
	    FileSize = IMD5_Header.filesize;
	    u32 LZ77Magic = 0;
        U8HeaderOffset = sizeof(IMD5Header);
        ReadFile(&LZ77Magic, sizeof(u32), U8HeaderOffset);

        if(LZ77Magic == 'LZ77')
        {
            FileSize -= sizeof(IMD5Header);
            u8 * BinBuffer = (u8 *) malloc(FileSize);
            if(!BinBuffer)
                return false;

            ReadFile(BinBuffer, FileSize, U8HeaderOffset);

            u8 * UncBinBuffer = uncompressLZ77(BinBuffer, FileSize, &FileSize);
            if(!UncBinBuffer)
                return false;

            u32 tmpSize = FileSize;
            free(BinBuffer);
            CloseFile();

            FileBuffer = UncBinBuffer;
            FileSize = tmpSize;
            FromMem = true;
            U8HeaderOffset = 0;
        }

        return ParseU8Header(U8HeaderOffset);
	}
	//It's a direct U8Archive...weird but oh well...
	else if(U8_Header.fcc == 0x55AA382D /* U.8- */)
	{
        return ParseU8Header(0);
	}

    //Unknown U8Archive
    CloseFile();

    return false;
}

bool U8Archive::ParseU8Header(u32 U8HeaderOffset)
{
    u32 U8Magic;
    ReadFile(&U8Magic, sizeof(u32), U8HeaderOffset);

    if(U8Magic != 0x55AA382D /* U.8- */)
    {
        CloseFile();
        return false;
    }

    u32 rootNodeOffset = 0;
    ReadFile(&rootNodeOffset, sizeof(u32), U8HeaderOffset+sizeof(u32));

    u32 fstOffset = U8HeaderOffset + rootNodeOffset;
    U8Entry * MainFST = (U8Entry *) malloc(sizeof(U8Entry));
    if(!MainFST)
    {
        CloseFile();
        return false;
    }

    ReadFile(MainFST, sizeof(U8Entry), fstOffset);

    u32 fstNums = MainFST[0].numEntries;
    U8Entry * fst = (U8Entry *) realloc(MainFST, fstNums*sizeof(U8Entry));
    if(!fst)
    {
        free(MainFST);
        CloseFile();
        return false;
    }

    ReadFile(fst, fstNums*sizeof(U8Entry), fstOffset);

    char filename[MAXPATHLEN];
	char directory[MAXPATHLEN];
	strcpy(directory, "");
	u32 dir_stack[100];
	int dir_index = 0;

	for (u32 i = 1; i < fstNums; ++i)
    {
        string RealFilename;
        U8Filename(fst, fstOffset, i, RealFilename);
        snprintf(filename, sizeof(filename), "%s%s", directory, RealFilename.c_str());

        bool isDir = (fst[i].fileType == 0) ? false : true;

        if(isDir)
        {
            dir_stack[++dir_index] = fst[i].numEntries;
            char tmp[MAXPATHLEN];
            snprintf(tmp, sizeof(tmp), "%s/", RealFilename.c_str());
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
            else
            {
                directory[0] = '\0';
            }
            dir_index--;
        }
    }

    free(fst);

    return true;
}

void U8Archive::U8Filename(const U8Entry * fst, int fstoffset, int i, string & Filename)
{
    u32 nameoffset = fstoffset+fst[0].numEntries*sizeof(U8Entry)+fst[i].nameOffset;
    int n = -1;
    char Char;

    do
    {
        n++;
        ReadFile(&Char, 1, nameoffset+n);
        Filename.push_back(Char);
    }
    while(Char != 0 && nameoffset+n < FileSize);
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
        if(PathStructure.at(i) != NULL)
        {
            delete PathStructure.at(i);
            PathStructure.at(i) = NULL;
        }
    }

    PathStructure.clear();
    BufferOffset.clear();
}

size_t U8Archive::ReadFile(void * buffer, size_t size, off_t offset)
{
    if(!FileBuffer && !File)
        return -1;

	if(FromMem)
	{
        memcpy(buffer, FileBuffer+offset, size);
        return size;
	}

    fseek(File, offset, SEEK_SET);
    return fread(buffer, 1, size, File);
}

int U8Archive::ExtractFile(int ind, const char *dest, bool withpath)
{
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
    u8 * buffer = (u8 *) malloc(1024*50);
    if(!buffer)
    {
        return -3;
    }

    u32 FileOffset = BufferOffset.at(File->fileindex);
	u32 filesize = File->length;

    FILE *pfile = fopen(writepath, "wb");
    if(!pfile)
    {
        free(buffer);
        fclose(pfile);
        return -3;
    }

    u32 done = 0;

    do
    {
        if(actioncanceled)
        {
            free(buffer);
            fclose(pfile);
            return -10;
        }

        ShowProgress(done, filesize, RealFilename);

        if(filesize - done < blocksize)
            blocksize = filesize - done;

        int ret = ReadFile(buffer, blocksize, FileOffset+done);
        if(ret < 0)
        {
            fclose(pfile);
            free(buffer);
            return -3;
        }

        ret = fwrite(buffer, 1, ret, pfile);
        if(ret < 0)
        {
            fclose(pfile);
            free(buffer);
            return -3;
        }

        done += ret;
    }
    while(done < filesize);

    fclose(pfile);
    free(buffer);

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
