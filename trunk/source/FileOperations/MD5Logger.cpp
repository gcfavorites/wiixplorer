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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <sys/dir.h>
#include <algorithm>
#include "MD5Logger.hpp"
#include "Language/gettext.h"
#include "FileOperations/fileops.h"
#include "FileOperations/MD5.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PromptWindows.h"
#include "Tools/tools.h"

#define BLOCKSIZE   102400

extern bool sizegainrunning;

MD5Logger::MD5Logger()
	: LogFile(NULL), FolderCounter(0), FileCounter(0), ErrorCounter(0)
{
}

MD5Logger::~MD5Logger()
{
	CloseLog();
}

bool MD5Logger::OpenLog(const char * path)
{
	LogFile = fopen(path, "ab");
	if(!LogFile)
		LogFile = fopen(path, "wb");

	return (LogFile != NULL);
}

void MD5Logger::CloseLog()
{
	if(LogFile)
		fclose(LogFile);

	LogFile = NULL;
}

bool MD5Logger::LogMD5(const char * logpath, ItemMarker * Marker, bool showprogress)
{
	if(!OpenLog(logpath))
	{
		ShowError(tr("MD5 log file can't be created."));
		return false;
	}

	FolderCounter = 0;
	FileCounter = 0;
	ErrorCounter = 0;
//	int ProgressMode = ((Marker->GetItemcount() > 1) || Marker->IsItemDir(0)) ? MULTI_PROGRESSBAR : PROGRESSBAR;

//	StartProgress(tr("Calculating MD5..."), ProgressMode);

//	if(ProgressMode == MULTI_PROGRESSBAR)
		CalcTotalSize(Marker);

	fprintf(LogFile, tr("--------------------------------------------------\n"));
	fprintf(LogFile, tr("WiiXplorer MD5 Calculator\n"));

	char currentpath[1024];

	for(int i = 0; i < Marker->GetItemcount(); i++)
	{
		fprintf(LogFile, "\n");

		if(Marker->IsItemDir(i))
		{
			snprintf(currentpath, sizeof(currentpath), "%s/", Marker->GetItemPath(i));
			fprintf(LogFile, "%s %s\n\n", tr("Checking directory:"), currentpath);
			CalculateDirectory(currentpath, showprogress);
		}
		else
		{
			snprintf(currentpath, sizeof(currentpath), "%s", Marker->GetItemPath(i));
			fprintf(LogFile, "%s %s\n\n", tr("Checking file:"), currentpath);
			CalculateFile(currentpath, showprogress);
		}
	}

	fprintf(LogFile, tr("--------------------------------------------------\n"));
	fprintf(LogFile, tr("Checked %i file(s) in %i folder(s).\n"), FileCounter, FolderCounter);
	fprintf(LogFile, tr("Encountered %i error(s).\n"), ErrorCounter);
	fprintf(LogFile, tr("--------------------------------------------------\n\n"));

	StopProgress();

	CloseLog();

	if(ErrorCounter > 0)
		WindowPrompt(tr("Hashing complete."), fmt(tr("%i error(s) encountered. Check MD5.log for more details."), ErrorCounter), tr("OK"));
	else
		WindowPrompt(tr("Hashing successfully finished."), tr("Log was saved in MD5.log"), tr("OK"));

	return true;
}

bool MD5Logger::CalculateFile(const char * filepath, bool showprogress)
{
	if(!LogFile || !filepath)
	{
		++ErrorCounter;
		return false;
	}

	char * filename = strrchr(filepath, '/');
	if(!filename)
	{
		fprintf(LogFile, tr("Error - %s is not a file\n"), filepath);
		++ErrorCounter;
		return false;
	}

	filename++;

	u8 Hash[32];
	auth_md5Ctx ctx[1];
	int read = 0;
	u64 done = 0;
	u64 filesize = FileSize(filepath);

	FILE * file = fopen(filepath, "rb");

	if (file == NULL || filesize == 0)
	{
		fprintf(LogFile, tr("Error - Could not open file: %s\n"), filepath);
		++ErrorCounter;
		return false;
	}

	auth_md5InitCtx(ctx);

	u8 * buffer = (u8 *) malloc(BLOCKSIZE);

	if(!buffer)
	{
		fclose(file);
		fprintf(LogFile, tr("Error - Not enough memory on: %s\n"), filepath);
		++ErrorCounter;
		return false;
	}

	do
	{
//		if(actioncanceled)
		{
			fclose(file);
			free(buffer);
			return false;
		}

		if(showprogress)
			ShowProgress(done, filesize, filename);

		read = fread(buffer, 1, BLOCKSIZE, file);

		if(read > 0)
			auth_md5SumCtx(ctx, buffer, read);

		done += read;
	}
	while(read > 0);

	fclose(file);
	free(buffer);

	auth_md5CloseCtx(ctx, Hash);

	char HashString[100];
	memset(HashString, 0, sizeof(HashString));

	MD5ToString(Hash, HashString);

	++FileCounter;

	fprintf(LogFile, "%s - %i. %s\n", HashString, FileCounter, filepath);

	return true;
}

bool MD5Logger::CalculateDirectory(const char * path, bool showprogress)
{
	if(!LogFile || !path)
	{
		++ErrorCounter;
		return false;
	}

	DIR_ITER *dir = diropen(path);

	if(dir == NULL)
	{
		fprintf(LogFile, tr("Error - Could not open directory: %s\n"), path);
		++ErrorCounter;
		return false;
	}

	char *filename = (char *) malloc(1024);

	if(!filename)
	{
		dirclose(dir);
		fprintf(LogFile, tr("Error - Not enough memory on: %s\n"), path);
		++ErrorCounter;
		return false;
	}

	struct stat st;
	std::vector<char *> DirList;
	std::vector<char *> FileList;

	while (dirnext(dir,filename,&st) == 0)
	{
//		if(actioncanceled)
		{
			free(filename);
			ClearList(DirList);
			ClearList(FileList);
			dirclose(dir);
			++FolderCounter;
			return false;
		}

		if(st.st_mode & S_IFDIR)
		{
			if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0)
			{
				if(DirList.capacity()-DirList.size() == 0)
					DirList.reserve(DirList.size()+100);

				DirList.push_back(strdup(filename));
			}
		}
		else
		{
			if(FileList.capacity()-FileList.size() == 0)
				FileList.reserve(FileList.size()+100);

			FileList.push_back(strdup(filename));
		}
	}
	dirclose(dir);
	free(filename);
	filename = NULL;

	if(FileList.size() > 1)
		SortList(FileList);
	if(DirList.size() > 1)
		SortList(DirList);

	for(u32 i = 0; i < FileList.size(); i++)
	{
		if(FileList[i])
		{
			char currentname[strlen(path)+strlen(FileList[i])+2];
			sprintf(currentname, "%s%s", path, FileList[i]);
			free(FileList[i]);
			FileList[i] = NULL;

			CalculateFile(currentname, showprogress);
		}
	}
	FileList.clear();
	ClearList(FileList);

	for(u32 i = 0; i < DirList.size(); i++)
	{
		if(DirList[i])
		{
			char currentname[strlen(path)+strlen(DirList[i])+2];
			sprintf(currentname, "%s%s/", path, DirList[i]);
			free(DirList[i]);
			DirList[i] = NULL;

			CalculateDirectory(currentname, showprogress);
		}
	}
	DirList.clear();
	ClearList(DirList);

	++FolderCounter;

	return true;
}

void MD5Logger::ClearList(std::vector<char *> &List)
{
	for(u32 i = 0; i < List.size(); i++)
	{
		if(List[i])
			free(List[i]);
		List[i] = NULL;
	}
	List.clear();
	std::vector<char *>().swap(List);
}

void MD5Logger::SortList(std::vector<char *> & List)
{
	std::sort(List.begin(), List.end(), SortCallback);
}

bool MD5Logger::SortCallback(const char * path1, const char * path2)
{
	if(stricmp(path1, path2) > 0)
		return false;
	else
		return true;
}

void MD5Logger::CalcTotalSize(ItemMarker * Process)
{
	char filepath[1024];
	sizegainrunning = true;
	u64 TotalSize = 0;
	u32 TotalFileCount = 0;

	for(int i = 0; i < Process->GetItemcount(); i++)
	{
		if(Process->IsItemDir(i) == true)
		{
			snprintf(filepath, sizeof(filepath), "%s/", Process->GetItemPath(i));
			GetFolderSize(filepath, TotalSize, TotalFileCount);
		}
		else
		{
			TotalSize += FileSize(Process->GetItemPath(i));
			++TotalFileCount;
		}
	}

	sizegainrunning = false;

	ProgressWindow::Instance()->SetTotalValues(TotalSize, TotalFileCount);
}
