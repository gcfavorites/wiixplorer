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
 * RarFile.cpp
 *
 * RarFile Class
 * for WiiXplorer 2009
 ***************************************************************************/
#include <ogcsys.h>
#include <string.h>

#include "RarFile.h"

RarFile::RarFile(const char *filepath)
{
    ArcList = NULL;
    this->LoadList(filepath);
}

RarFile::~RarFile()
{
    if(ArcList)
        urarlib_freelist(ArcList);

    ClearList();
}

bool RarFile::LoadList(const char * filepath)
{
    urarlib_list((void *) filepath, (ArchiveList_struct*)&ArcList);
    u32 index = 0;

    ClearList();

    while(ArcList != NULL)
    {
        char *realfilename = strdup(ArcList->item.Name);
        u32 strlength = strlen(realfilename)+1;
        u32 ch = 0;
        //switch windows crap '\' to unix '/'
        for(ch = 0; ch < strlength; ch++)
        {
            if(realfilename[ch] == '\\')
                realfilename[ch] = '/';
        }

        ArchiveFileStruct * TempStruct = new ArchiveFileStruct;
        TempStruct->filename = new char[strlength];
        snprintf(TempStruct->filename, strlength, "%s", realfilename);
        TempStruct->length = ArcList->item.UnpSize;
        TempStruct->comp_length = ArcList->item.PackSize;
        if(!ArcList->item.FileCRC)
            TempStruct->isdir = true;
        else
            TempStruct->isdir = false;
        TempStruct->fileindex = index;
        TempStruct->ModTime = (u64) ArcList->item.FileTime;
        TempStruct->archiveType = RAR;

        RarStructure.push_back(TempStruct);
        free(realfilename);
        TempStruct = NULL;
        realfilename = NULL;

        index++;

        ArcList = (ArchiveList_struct*)ArcList->next;
    }

    urarlib_freelist(ArcList);
    ArcList = NULL;

    return true;
}

void RarFile::ClearList()
{
    for(u32 i = 0; i < RarStructure.size(); i++)
    {
        if(RarStructure.at(i)->filename != NULL)
        {
            delete [] RarStructure.at(i)->filename;
            RarStructure.at(i)->filename = NULL;
        }
    }

    RarStructure.clear();
}

ArchiveFileStruct * RarFile::GetFileStruct(int ind)
{
    if(ind > (int) RarStructure.size() || ind < 0)
        return NULL;

    return RarStructure.at(ind);
}

u32 RarFile::GetItemCount()
{
    return RarStructure.size();
}

int RarFile::ExtractFile(int fileindex, const char * outpath, bool withpath)
{
	return 0;
}

int RarFile::ExtractAll(const char * destpath)
{
	return 0;
}
