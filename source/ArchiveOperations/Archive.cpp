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
#include <string.h>

#include "Archive.h"

Archive::Archive(const char  * filepath)
{
    szFile = NULL;
    zipFile = NULL;
    rarFile = NULL;
    u8File = NULL;

    char checkbuffer[6];
    memset(checkbuffer, 0, sizeof(checkbuffer));

    FILE * file = fopen(filepath, "rb");
    if(!file)
        return;

    int ret = 1;

    while(checkbuffer[0] == 0 && ret > 0)
        ret = fread(&checkbuffer, 1, 1, file);

    fread(&checkbuffer[1], 1, 5, file);
    fclose(file);

    if(IsZipFile(checkbuffer))
        zipFile = new ZipFile(filepath);

    if(Is7ZipFile(checkbuffer))
        szFile = new SzFile(filepath);

    if(IsRarFile(checkbuffer))
        rarFile = new RarFile(filepath);

    if(IsU8ArchiveFile(checkbuffer))
    {
        u8File = new U8Archive(filepath);
    }
}

Archive::~Archive()
{
    if(zipFile)
        delete zipFile;

    if(szFile)
        delete szFile;

    if(rarFile)
        delete rarFile;

    if(u8File)
        delete u8File;

    zipFile = NULL;
    szFile = NULL;
    rarFile = NULL;
    u8File = NULL;
}

ArchiveFileStruct * Archive::GetFileStruct(int ind)
{
    if(zipFile)
        return zipFile->GetFileStruct(ind);

    if(szFile)
        return szFile->GetFileStruct(ind);

    if(rarFile)
        return rarFile->GetFileStruct(ind);

    if(u8File)
        return u8File->GetFileStruct(ind);

    return NULL;
}

u32 Archive::GetItemCount()
{
    if(zipFile)
        return zipFile->GetItemCount();

    if(szFile)
        return szFile->GetItemCount();

    if(rarFile)
        return rarFile->GetItemCount();

    if(u8File)
        return u8File->GetItemCount();

    return 0;
}

int Archive::ExtractFile(int ind, const char *destpath, bool withpath)
{
    if(zipFile)
        return zipFile->ExtractFile(ind, destpath, withpath);

    if(szFile)
        return szFile->ExtractFile(ind, destpath, withpath);

    if(rarFile)
        return rarFile->ExtractFile(ind, destpath, withpath);

    if(u8File)
        return u8File->ExtractFile(ind, destpath, withpath);

    return 0;
}

int Archive::ExtractAll(const char * destpath)
{
    if(zipFile)
        return zipFile->ExtractAll(destpath);

    if(szFile)
        return szFile->ExtractAll(destpath);

    if(rarFile)
        return rarFile->ExtractAll(destpath);

    if(u8File)
        return u8File->ExtractAll(destpath);

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
	// 7z signature
	int i;
	for(i = 0; i < 6; i++)
		if(buffer[i] != k7zSignature[i])
			return false;

	return true; // 7z archive found
}

bool Archive::IsRarFile(const char *buffer)
{
	// Rar signature    Rar!\x1A\a\0
	Byte Signature[6] = {'R', 'a', 'r', 0x21, 0x1a, 0x07};

	int i;
	for(i = 0; i < 6; i++)
		if(buffer[i] != Signature[i])
			return false;

	return true; // RAR archive found
}

bool Archive::IsU8ArchiveFile(const char *buffer)
{
	char SignatureIMET[4] = {'I', 'M', 'E', 'T'};
	char SignatureIMD5[4] = {'I', 'M', 'D', '5'};

	int i;
	bool sign_passed = true;

	for(i = 0; i < 4; i++)
		if(buffer[i] != SignatureIMET[i])
			sign_passed = false;

    if(!sign_passed)
    {
        sign_passed = true;
        for(i = 0; i < 4; i++)
            if(buffer[i] != SignatureIMD5[i])
                sign_passed = false;
    }

	return sign_passed;
}
