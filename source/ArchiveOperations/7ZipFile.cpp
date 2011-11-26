/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>

#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"
#include "7ZipFile.h"

// 7zip error list
static const char * szerrormsg[10] = {
   tr("File is corrupt."), // 7z: Data error
   tr("Not enough memory."), // 7z: Out of memory
   tr("File is corrupt (CRC mismatch)."), // 7z: CRC Error
   tr("File uses unsupported compression settings."), // 7z: Not implemented
   tr("File is corrupt."), // 7z: Fail
   tr("Failed to read file data."), // 7z: Data read failure
   tr("File is corrupt."), // 7z: Archive error
   tr("File uses too high of compression settings (dictionary size is too large)."), // 7z: Dictionary too large
   tr("Can't open file."),
   tr("Process canceled."),
};

SzFile::SzFile(const char *filepath)
{
	if(InFile_Open(&archiveStream.file, filepath))
	{
		SzResult = 9;
		return;
	}

	FileInStream_CreateVTable(&archiveStream);
	LookToRead_CreateVTable(&lookStream, False);

	lookStream.realStream = &archiveStream.s;
	LookToRead_Init(&lookStream);

	// set default 7Zip SDK handlers for allocation and freeing memory
	SzAllocImp.Alloc = SzAlloc;
	SzAllocImp.Free = SzFree;
	SzAllocTempImp.Alloc = SzAllocTemp;
	SzAllocTempImp.Free = SzFreeTemp;

	// prepare CRC and 7Zip database structures
	CrcGenerateTable();
	SzArEx_Init(&SzArchiveDb);

	// open the archive
	SzResult = SzArEx_Open(&SzArchiveDb, &lookStream.s, &SzAllocImp, &SzAllocTempImp);


	if (SzResult != SZ_OK)
	{
		DisplayError(SzResult);
	}
}

SzFile::~SzFile()
{
	SzArEx_Free(&SzArchiveDb, &SzAllocImp);

	File_Close(&archiveStream.file);
}

bool SzFile::Is7ZipFile (const char *buffer)
{
	// 7z signature
	int i;
	for(i = 0; i < 6; i++)
		if(buffer[i] != k7zSignature[i])
			return false;

	return true; // 7z archive found
}

ArchiveFileStruct * SzFile::GetFileStruct(int ind)
{
	if(ind > (int) SzArchiveDb.db.NumFiles || ind < 0)
		return NULL;

	CSzFileItem * SzFileItem = SzArchiveDb.db.Files + ind;

	CurArcFile.filename = SzFileItem->Name;
	CurArcFile.length = SzFileItem->Size;
	CurArcFile.comp_length = 0;
	CurArcFile.isdir = SzFileItem->IsDir;
	CurArcFile.fileindex = ind;
	if(SzFileItem->MTimeDefined)
		CurArcFile.ModTime = (u64) (SzFileItem->MTime.Low  | ((u64) SzFileItem->MTime.High << 32));
	else
		CurArcFile.ModTime = 0;
	CurArcFile.archiveType = SZIP;

	return &CurArcFile;
}

void SzFile::DisplayError(SRes res)
{
	StopProgress();
	WindowPrompt(tr("7z decompression failed:"), szerrormsg[(res - 1)], tr("OK"));
}

u32 SzFile::GetItemCount()
{
	if(SzResult != SZ_OK)
		return 0;

	return SzArchiveDb.db.NumFiles;
}

int SzFile::ExtractFile(int fileindex, const char * outpath, bool withpath)
{
	if(SzResult != SZ_OK)
		return -1;

	if(!GetFileStruct(fileindex))
		return -2;

	// reset variables
	UInt32 SzBlockIndex = 0xFFFFFFFF;
	size_t SzOffset = 0;
	size_t SzSizeProcessed = 0;
	Byte * outBuffer = 0;
	size_t outBufferSize = 0;

	char * RealFilename = strrchr(CurArcFile.filename, '/');

	char writepath[MAXPATHLEN];
	if(!RealFilename || withpath)
		snprintf(writepath, sizeof(writepath), "%s/%s", outpath, CurArcFile.filename);
	else
		snprintf(writepath, sizeof(writepath), "%s/%s", outpath, RealFilename+1);

	if(CurArcFile.isdir)
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

	//startup timer
	ShowProgress(0, CurArcFile.length, (RealFilename ? RealFilename+1 : CurArcFile.filename));

	// Extract
	SzResult = SzAr_Extract(&SzArchiveDb, &lookStream.s, fileindex,
							&SzBlockIndex, &outBuffer, &outBufferSize,
							&SzOffset, &SzSizeProcessed,
							&SzAllocImp, &SzAllocTempImp);

//	if(actioncanceled)
		SzResult = 10;

	if(SzResult == SZ_OK)
	{
		FILE * wFile = fopen(writepath, "wb");

		//not quite right and needs to be changed
		u32 done = 0;
		if(!wFile)
			done = CurArcFile.length;

		do
		{
			ShowProgress(done, CurArcFile.length, (RealFilename ? RealFilename+1 : CurArcFile.filename));
			int wrote = fwrite(outBuffer, 1, 51200, wFile);
			done += wrote;

			if(wrote == 0)
				break;

		} while(done < CurArcFile.length);

		fclose(wFile);
	}
	if(outBuffer)
	{
		IAlloc_Free(&SzAllocImp, outBuffer);
		outBuffer = NULL;
	}

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

	StartProgress(tr("Extracting files..."));

	for(u32 i = 0; i < SzArchiveDb.db.NumFiles; i++)
	{
//		if(actioncanceled)
			return -10;

		CSzFileItem * SzFileItem = SzArchiveDb.db.Files + i;

		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/%s", destpath, SzFileItem->Name);
		u32 filesize = SzFileItem->Size;

		if(!filesize)
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
