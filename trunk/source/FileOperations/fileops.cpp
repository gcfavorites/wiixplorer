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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "fileops.h"
#include "filebrowser.h"

using namespace std;

#define BLOCKSIZE			   71680	  //70KB
#define VectorResize(List) if(List.capacity()-List.size() == 0) List.reserve(List.size()+100)

static bool replaceall = false;
static bool replacenone = false;

/****************************************************************************
 * GetReplaceChoice
 *
 * Get the user choice if he wants to replace a file or not
 ***************************************************************************/
static int GetReplaceChoice(const char * filename)
{
	std::string progressText = ProgressWindow::Instance()->GetTitle();
	StopProgress();

	int choice = WindowPrompt(fmt("%s %s", tr("File already exists:"), filename), tr("Do you want to replace this file?"), tr("Yes"), tr("No"), tr("Yes to all"), tr("No to all"));

	StartProgress(progressText.c_str());

	if(choice == 3)
	{
		replaceall = true;
		replacenone = false;
		return choice;
	}
	else if(choice == 0)
	{
		replaceall = false;
		replacenone = true;
		return choice;
	}
	else
	{
		replaceall = false;
		return choice;
	}
}

/****************************************************************************
 * ResetReplaceChoice
 *
 * This should be called after the process is done
 ***************************************************************************/
void ResetReplaceChoice()
{
	replaceall = false;
	replacenone = false;
}

/****************************************************************************
 * CheckFile
 *
 * Check if file is existing
 ***************************************************************************/
bool CheckFile(const char * filepath)
{
	if(!filepath)
		return false;

	if(strchr(filepath, '/') == NULL)
		return false;

	struct stat filestat;
	int length = strlen(filepath);

	char * dirnoslash = (char *) malloc(length+2);
	if(!dirnoslash) return false;

	strcpy(dirnoslash, filepath);

	while(dirnoslash[length-1] == '/')
	{
		dirnoslash[length-1] = '\0';
		--length;
	}

	char * notRoot = strrchr(dirnoslash, '/');
	if(!notRoot)
		strcat(dirnoslash, "/");

	int ret = stat(dirnoslash, &filestat);

	free(dirnoslash);

	return (ret == 0);
}

/****************************************************************************
 * FileSize
 *
 * Get filesize in bytes. u64 for files bigger than 4GB
 ***************************************************************************/
u64 FileSize(const char * filepath)
{
  struct stat filestat;

  if (!filepath || stat(filepath, &filestat) != 0)
	return 0;

  return filestat.st_size;
}

/****************************************************************************
 * LoadFileToMem
 *
 * Load up the file into a block of memory
 ***************************************************************************/
int LoadFileToMem(const char *filepath, u8 **inbuffer, u32 *size)
{
	if(!filepath)
		return -1;

	int ret = -1;
	u32 filesize = FileSize(filepath);
	char * filename = strrchr(filepath, '/');
	if(filename)
		filename++;

	*inbuffer = NULL;
	*size = 0;

	FILE *file = fopen(filepath, "rb");

	if (file == NULL)
		return -1;

	u8 *buffer = (u8 *) malloc(filesize);
	if (buffer == NULL)
	{
		fclose(file);
		return -2;
	}

	u32 done = 0;
	u32 blocksize = BLOCKSIZE;

	do
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			free(buffer);
			fclose(file);
			return -10;
		}

		if(blocksize > filesize-done)
			blocksize = filesize-done;

		ret = fread(buffer+done, 1, blocksize, file);
		if(ret < 0)
		{
			free(buffer);
			fclose(file);
			return -3;
		}

		ShowProgress(done, filesize, filename);

		done += ret;

	}
	while(ret > 0);

	fclose(file);

	if (done != filesize)
	{
		free(buffer);
		return -3;
	}

	*inbuffer = buffer;
	*size = filesize;

	return 1;
}

/****************************************************************************
 * LoadFileToMemWithProgress
 *
 * Load up the file into a block of memory, while showing a progress dialog
 ***************************************************************************/
int LoadFileToMemWithProgress(const char *progressText, const char *filepath, u8 **inbuffer, u32 *size)
{
	StartProgress(progressText);
	int ret = LoadFileToMem(filepath, inbuffer, size);
	StopProgress();

	if(ret == -1) {
		ShowError(tr("Can not open the file."));
	}
	else if(ret == -2) {
		ShowError(tr("Not enough memory."));
	}
	else if(ret == -3) {
		ShowError(tr("Error while reading file."));
	}
	else if(ret == -10) {
		ShowError(tr("Action cancelled."));
	}
	return ret;
}

/****************************************************************************
 * CreateSubfolder
 *
 * Create recursive all subfolders to the given path
 ***************************************************************************/
bool CreateSubfolder(const char * fullpath)
{
	if(!fullpath)
		return false;

	if(CheckFile(fullpath))
		return true;

	string dirpath(fullpath);

	int length = dirpath.size()-1;
	while(dirpath[length] == '/')
	{
		dirpath.erase(length);
		--length;
	}

	string subpath(dirpath);
	size_t pos = subpath.rfind('/');
	if(pos == string::npos)
		return false;

	if(subpath.size()-1 > pos)
		subpath.erase(pos+1);

	bool result = CreateSubfolder(subpath.c_str());
	if(!result)
		return false;

	return (mkdir(dirpath.c_str(), 0777) != -1);
}

/****************************************************************************
 * CopyFile
 *
 * Copy the file from source filepath to destination filepath
 ***************************************************************************/
int CopyFile(const char * src, const char * dest)
{
	u32 read;
	u32 wrote;

	char * filename = strrchr(src, '/');
	if(filename)
		filename++;
	else
		return -1;

	bool fileexist = CheckFile(dest);

	if(fileexist == true)
	{
		int choice = -1;
		if(!replaceall && !replacenone)
			choice = GetReplaceChoice(filename);

		if(replacenone || choice == 2)
			return 1;
	}

	u64 sizesrc = FileSize(src);

	FILE * source = fopen(src, "rb");

	if(!source)
		return -2;

	u32 blksize = BLOCKSIZE;

	u8 * buffer = (u8 *) malloc(blksize);

	if(buffer == NULL){
		//no memory
		fclose(source);
		return -1;
	}

	FILE * destination = fopen(dest, "wb");

	if(destination == NULL)
	{
		free(buffer);
		fclose(source);
		return -3;
	}

	u64 done = 0;

	do
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			fclose(source);
			fclose(destination);
			free(buffer);
			RemoveFile(dest);
			return -10;
		}

		if(blksize > sizesrc - done)
			blksize = sizesrc - done;

		read = fread(buffer, 1, blksize, source);
		if(read != blksize)
			break;

		wrote = fwrite(buffer, 1, read, destination);
		if(wrote != read)
			break;

		//Display progress
		ShowProgress(done, sizesrc, filename);

		done += wrote;
	}
	while (read > 0);

	free(buffer);
	fclose(source);
	fclose(destination);

	if(sizesrc != done)
	{
		RemoveFile(dest);
		return -4;
	}

	return 1;
}

/****************************************************************************
* ClearList
*
* Clearing a vector list
****************************************************************************/
static inline void ClearList(vector<char *> &List)
{
	for(u32 i = 0; i < List.size(); ++i)
	{
		if(List[i])
		{
			free(List[i]);
			List[i] = NULL;
		}
	}
	List.clear();
	vector<char *>().swap(List);
}

/****************************************************************************
* ListDirectory
*
* Create a list of files/dirs of a directory
****************************************************************************/
int ListDirectory(string &path, vector<char *> &DirList, vector<char *> &FileList)
{
	struct dirent *dirent = NULL;
	DIR *dir = NULL;

	dir = opendir(path.c_str());
	if(dir == NULL)
		return -2;

	while ((dirent = readdir(dir)) != 0)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			ClearList(DirList);
			ClearList(FileList);
			closedir(dir);
			return -10;
		}

		if(dirent->d_type & DT_DIR)
		{
			if(strcmp(dirent->d_name,".") != 0 && strcmp(dirent->d_name,"..") != 0)
			{
				VectorResize(DirList);
				DirList.push_back(strdup(dirent->d_name));
			}
		}
		else
		{
			VectorResize(FileList);
			FileList.push_back(strdup(dirent->d_name));
		}
	}

	closedir(dir);

	return 0;
}

/****************************************************************************
 * CopyDirectory
 *
 * Copy recursive a complete source path to destination path
 ***************************************************************************/
static int InternalCopyDirectory(string &src, string &dest)
{
	vector<char *> DirList;
	vector<char *> FileList;

	int ret = ListDirectory(src, DirList, FileList);
	if(ret < 0)
		return ret;

	//! Ensure that empty directories are created
	CreateSubfolder(dest.c_str());

	for(u32 i = 0; i < FileList.size(); ++i)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			ClearList(DirList);
			ClearList(FileList);
			return -10;
		}

		if(!FileList[i])
			continue;

		int srcSize = src.size();
		int destSize = dest.size();
		src += FileList[i];
		dest += FileList[i];
		free(FileList[i]);
		FileList[i] = NULL;
		int res = CopyFile(src.c_str(), dest.c_str());
		if(res < 0) ret = res;
		src.erase(srcSize);
		dest.erase(destSize);
	}

	ClearList(FileList);

	for(u32 i = 0; i < DirList.size(); ++i)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			ClearList(DirList);
			ClearList(FileList);
			return -10;
		}

		if(!DirList[i])
			continue;

		int srcSize = src.size();
		int destSize = dest.size();
		src += DirList[i];
		src += '/';
		dest += DirList[i];
		dest += '/';
		free(DirList[i]);
		DirList[i] = NULL;
		int res = InternalCopyDirectory(src, dest);
		if(res < 0) ret = res;
		src.erase(srcSize);
		dest.erase(destSize);
	}

	ClearList(DirList);

	return ret;

}

int CopyDirectory(const char * src, const char * dest)
{
	if(!src || !dest)
		return -1;

	string srcString(src);
	string destString(dest);

	if(srcString.size() > 1 && srcString[srcString.size()-1] != '/')
		srcString += '/';
	if(destString.size() > 1 && destString[destString.size()-1] != '/')
		destString += '/';

	return InternalCopyDirectory(srcString, destString);
}

/****************************************************************************
 * MoveDirectory
 *
 * Move recursive a complete source path to destination path
 ***************************************************************************/
static int InternalMoveDirectory(string &src, string &dest)
{
	bool samedevices = CompareDevices(src.c_str(), dest.c_str());
	vector<char *> DirList;
	vector<char *> FileList;

	int ret = ListDirectory(src, DirList, FileList);
	if(ret < 0)
		return ret;

	//! Ensure that empty directories are created
	CreateSubfolder(dest.c_str());

	for(u32 i = 0; i < FileList.size(); ++i)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			ClearList(DirList);
			ClearList(FileList);
			return -10;
		}

		if(!FileList[i])
			continue;

		int srcSize = src.size();
		int destSize = dest.size();
		src += FileList[i];
		dest += FileList[i];

		int res = MoveFile(src.c_str(), dest.c_str());
		if(res < 0) ret = res;

		if(samedevices)
		{
			//Display progress
			ShowProgress(1, 1, FileList[i]);
		}

		free(FileList[i]);
		FileList[i] = NULL;
		src.erase(srcSize);
		dest.erase(destSize);
	}

	ClearList(FileList);

	for(u32 i = 0; i < DirList.size(); ++i)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			ClearList(DirList);
			ClearList(FileList);
			return -10;
		}

		if(!DirList[i])
			continue;

		int srcSize = src.size();
		int destSize = dest.size();
		src += DirList[i];
		src += '/';
		dest += DirList[i];
		dest += '/';
		free(DirList[i]);
		DirList[i] = NULL;

		int res = InternalMoveDirectory(src, dest);
		if(res < 0) ret = res;
		src.erase(srcSize);
		dest.erase(destSize);
	}

	ClearList(DirList);

	if(ret < 0)
		return ret;

	string srcCopy(src);

	while(srcCopy[srcCopy.size()-1] == '/')
		srcCopy.erase(srcCopy.size()-1);

	ret = remove(srcCopy.c_str());

	return (ret != 0 ? -1 : 0);
}

int MoveDirectory(const char * src, const char * dest)
{
	if(!src || !dest)
		return -1;

	string srcString(src);
	string destString(dest);

	if(srcString.size() > 1 && srcString[srcString.size()-1] != '/')
		srcString += '/';
	if(destString.size() > 1 && destString[destString.size()-1] != '/')
		destString += '/';

	return InternalMoveDirectory(srcString, destString);
}

/****************************************************************************
 * MoveFile
 *
 * Move a file from srcpath to destdir
 ***************************************************************************/
int MoveFile(const char *srcpath, const char *destdir)
{
	if(CompareDevices(srcpath, destdir))
	{
		if(CheckFile(destdir))
		{
			int choice = -1;
			if(!replaceall && !replacenone)
			{
				const char * filename = strrchr(destdir, '/');
				choice = GetReplaceChoice(filename ? filename+1 : destdir);
			}

			if(replacenone || choice == 2)
				return 1;

			else if(replaceall || choice == 1)
				RemoveFile(destdir);
		}

		if(RenameFile(srcpath, destdir))
			return 1;
	}
	else
	{
		int res = CopyFile(srcpath, destdir);
		if(res < 0)
			return res;

		if(RemoveFile(srcpath))
			return 1;
	}

	return -1;
}

/****************************************************************************
 * RemoveDirectory
 *
 * Remove a directory and its content recursively
 ***************************************************************************/
static int InternalRemoveDirectory(string &dirpath)
{
	vector<char *> DirList;
	vector<char *> FileList;

	int ret = ListDirectory(dirpath, DirList, FileList);
	if(ret < 0)
		return ret;

	for(u32 i = 0; i < FileList.size(); ++i)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			ClearList(DirList);
			ClearList(FileList);
			return -10;
		}

		if(!FileList[i])
			continue;

		int stringSize = dirpath.size();
		dirpath += FileList[i];
		if(!RemoveFile(dirpath.c_str()))
			ret = -1;
		dirpath.erase(stringSize);

		//Display progress
		ShowProgress(1, 1, FileList[i]);

		free(FileList[i]);
		FileList[i] = NULL;
	}

	ClearList(FileList);

	for(u32 i = 0; i < DirList.size(); ++i)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			ClearList(DirList);
			ClearList(FileList);
			return -10;
		}

		if(!DirList[i])
			continue;

		int stringSize = dirpath.size();
		dirpath += DirList[i];
		dirpath += '/';
		free(DirList[i]);
		DirList[i] = NULL;
		int res = InternalRemoveDirectory(dirpath);
		if(res < 0) ret = res;
		dirpath.erase(stringSize);
	}

	ClearList(DirList);

	if(ret < 0)
		return ret;

	string srcCopy(dirpath);

	while(srcCopy[srcCopy.size()-1] == '/')
		srcCopy.erase(srcCopy.size()-1);

	ret = remove(srcCopy.c_str());

	return (ret != 0 ? -1 : 1);

}

int RemoveDirectory(const char * dirpath)
{
	if(!dirpath)
		return -1;

	string destString(dirpath);

	if(destString.size() > 1 && destString[destString.size()-1] != '/')
		destString += '/';

	return InternalRemoveDirectory(destString);
}

/****************************************************************************
 * RemoveFile
 *
 * Delete the file from a given filepath
 ***************************************************************************/
bool RemoveFile(const char * filepath)
{
	return (remove(filepath) == 0);
}

/****************************************************************************
 * RenameFile
 *
 * Rename the file from a given srcpath to a given destpath
 ***************************************************************************/
bool RenameFile(const char * srcpath, const char * destpath)
{
	return (rename(srcpath, destpath) == 0);
}

/****************************************************************************
 * GetFolderSize
 *
 * Get recursivly complete foldersize
 ***************************************************************************/
void GetFolderSize(const char * folderpath, u64 &foldersize, u32 &filecount, const bool &bCancel)
{
	struct dirent *dirent = NULL;
	DIR *dir = opendir(folderpath);
	if(dir == NULL)
		return;

	char *filename = (char *) malloc(MAXPATHLEN);
	if(!filename)
	{
		closedir(dir);
		return;
	}

	memset(filename, 0, MAXPATHLEN);

	struct stat * st = (struct stat *) malloc(sizeof(struct stat));
	if(!st)
	{
		free(filename);
		closedir(dir);
		return;
	}

	vector<char *> DirList;

	while ((dirent = readdir(dir)) != 0)
	{
		if(bCancel)
		{
			ClearList(DirList);
			break;
		}

		snprintf(filename, MAXPATHLEN-1, "%s/%s", folderpath, dirent->d_name);

		if(stat(filename, st) != 0)
			continue;

		if(st->st_mode & S_IFDIR)
		{
			if(strcmp(dirent->d_name,".") != 0 && strcmp(dirent->d_name,"..") != 0)
			{
				VectorResize(DirList);
				DirList.push_back(strdup(dirent->d_name));
			}
		}
		else
		{
			++filecount;
			foldersize += st->st_size;
		}
	}

	closedir(dir);
	free(filename);
	free(st);

	for(u32 i = 0; i < DirList.size(); ++i)
	{
		if(!DirList[i])
			continue;

		string currentname(folderpath);
		currentname += DirList[i];
		currentname += '/';
		free(DirList[i]);
		DirList[i] = NULL;

		GetFolderSize(currentname.c_str(), foldersize, filecount, bCancel);
	}
}

/****************************************************************************
 * CompareDevices
 *
 * Compare if its the devices are equal
 ***************************************************************************/
bool CompareDevices(const char *src, const char *dest)
{
	if(!src || !dest)
		return false;

	char *device1 = strchr(src, ':');
	char *device2 = strchr(dest, ':');

	if(!device1 || !device2)
		return false;

	int position1 = device1-src+1;
	int position2 = device2-dest+1;

	char temp1[50];
	char temp2[50];

	snprintf(temp1, position1, "%s", src);
	snprintf(temp2, position2, "%s", dest);

	if(strcasecmp(temp1, temp2) == 0)
		return true;

	return false;
}
