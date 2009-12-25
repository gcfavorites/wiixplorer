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
 * Archive
 *
 * Archive Class
 * for WiiXplorer 2009
 ***************************************************************************/
#include <stdio.h>

#include "Archive.h"

Archive::Archive(const char  * filepath)
{
    szFile = NULL;
    zipFile = NULL;

    char checkbuffer[6];

    FILE * file = fopen(filepath, "rb");
    if(!file)
        return;

    fread(&checkbuffer, 1, 6, file);
    fclose(file);

    if(IsZipFile(checkbuffer))
        zipFile = new ZipFile(filepath);

    if(Is7ZipFile(checkbuffer))
        szFile = new SzFile(filepath);
}

Archive::~Archive()
{
    if(zipFile)
        delete zipFile;

    if(szFile)
        delete szFile;

    zipFile = NULL;
    szFile = NULL;
}

ArchiveFileStruct * Archive::GetFileStruct(int ind)
{
    if(zipFile)
        return zipFile->GetFileStruct(ind);

    if(szFile)
        return szFile->GetFileStruct(ind);

    return NULL;
}

u32 Archive::GetItemCount()
{
    if(zipFile)
        return zipFile->GetItemCount();

    if(szFile)
        return szFile->GetItemCount();

    return 0;
}

int Archive::ExtractFile(int ind, const char *destpath, bool withpath)
{
    if(zipFile)
        return zipFile->ExtractFile(ind, destpath, withpath);

    if(szFile)
        return 0;//szFile->ExtractFile(ind, destpath, withpath);

    return 0;
}

int Archive::ExtractAll(const char * destpath)
{
    if(zipFile)
        return zipFile->ExtractAll(destpath);

    if(szFile)
        return 0;//szFile->ExtractAll(destpath);

	return 0;
}

bool Archive::IsZipFile (const char *buffer)
{
	unsigned int *check;

	check = (unsigned int *) buffer;

	if (check[0] == 0x504b0304)
		return true;

	return false;
}

bool Archive::Is7ZipFile(const char *buffer)
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
