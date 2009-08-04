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
 * fileops.cpp
 * File operations for the Wii-FileXplorer
 * Handling all the needed file operations
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>

#include "Prompts/ProgressWindow.h"
#include "fileops.h"
#include "filebrowser.h"

#define BLOCKSIZE               50*1024      //50KB

extern bool sizegainrunning;

/****************************************************************************
 * FindFile
 *
 * Check if file is available in the given directory
 ***************************************************************************/
bool FindFile(const char * filename, const char * path)
{
    DIR *dir;
    struct dirent *file;

    dir = opendir(path);

    char temp[11];
    while ((file = readdir(dir)))
    {
        snprintf(temp,sizeof(temp),"%s",file->d_name);
        if (!strncmpi(temp,filename,11)) {
            closedir(dir);
        return true;
        }
    }
    closedir(dir);
    return false;
}

/****************************************************************************
 * CheckFile
 *
 * Check if file is existing
 ***************************************************************************/
bool CheckFile(const char * filepath)
{
    FILE * f;
    f = fopen(filepath,"rb");
    if(f) {
    fclose(f);
    return true;
    }
    fclose(f);
    return false;
}

/****************************************************************************
 * FileSize
 *
 * Get filesize in bytes. u64 for files bigger than 4GB
 ***************************************************************************/
u64 FileSize(const char * filepath)
{
  struct stat filestat;

  if (stat(filepath, &filestat) != 0)
    return 0;

  return filestat.st_size;
}

/****************************************************************************
 * CopyFile
 *
 * Copy the file from source filepath to destination filepath
 ***************************************************************************/
int CopyFile(const char * src, const char * dest)
{

	u32 blksize;
	u32 read = 1;
    u64 sizesrc = FileSize(src);

	char temp[MAXPATHLEN];
	sprintf(temp, "%s", src);
	char * filename = strrchr(temp-2, '/')+1;

	FILE * source = fopen(src, "rb");

	if(!source){
		return -2;
	}

    if(sizesrc < BLOCKSIZE)
        blksize = sizesrc;
    else
        blksize = BLOCKSIZE;

	u8 * buffer = new unsigned char[blksize];

	if(buffer == NULL){
	    //no memory
        fclose(source);
		return -1;
	}

	FILE * destination = fopen(dest, "wb");

    if(destination == NULL) {
        delete buffer;
        fclose(source);
        return -3;
    }

    u64 done = 0;
    do {
        //Display progress
        ShowProgress(done, sizesrc, filename);
        read = fread(buffer, 1, blksize, source);
        fwrite(buffer, 1, read, destination);
        done += read;
    } while (read > 0);

    fclose(source);
    fclose(destination);
    delete buffer;

    //get size of written file
    u64 sizedest = FileSize(dest);

    if(sizesrc != sizedest) {
        return -4;
    }

	return 1;
}

/****************************************************************************
 * CopyDirectory
 *
 * Copy recursive a complete source path to destination path
 ***************************************************************************/
int CopyDirectory(const char * src, const char * dest)
{

    struct stat st;
    DIR_ITER *dir = NULL;
    char filename[MAXPATHLEN];

    dir = diropen(src);
    if(dir == NULL) {
        return -1;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
            char currentname[MAXPATHLEN];
            char destname[MAXPATHLEN];
            snprintf(currentname, sizeof(currentname), "%s%s/", src, filename);
            snprintf(destname, sizeof(destname), "%s%s/", dest, filename);
            CopyDirectory(currentname, destname);
            }
        } else {
            char currentname[MAXPATHLEN];
            char destname[MAXPATHLEN];
            CreateSubfolder(dest);
            snprintf(currentname, sizeof(currentname), "%s%s", src, filename);
            snprintf(destname, sizeof(destname), "%s%s", dest, filename);
            CopyFile(currentname, destname);
        }
	}
	dirclose(dir);

    return 1;
}

/****************************************************************************
 * CreateSubfolder
 *
 * Create recursive all subfolders to the given path
 ***************************************************************************/
bool CreateSubfolder(const char * fullpath)
{

    //check/create subfolders
    struct stat st;

    char dirnoslash[MAXPATHLEN];
    snprintf(dirnoslash, strlen(fullpath), "%s", fullpath);

    if(stat(fullpath, &st) != 0) {
        char dircheck[MAXPATHLEN];
        char * pch = NULL;
        u32 cnt = 0;
        pch = strrchr(dirnoslash, '/');
        cnt = pch-dirnoslash;
        snprintf(dircheck, cnt+2, "%s", dirnoslash);
        CreateSubfolder(dircheck);
    };

    if (mkdir(dirnoslash, 0777) == -1) {
        return false;
    }

    return true;
}


/****************************************************************************
 * MoveDirectory
 *
 * Move recursive a complete source path to destination path
 ***************************************************************************/
int MoveDirectory(char * src, const char * dest)
{

    struct stat st;
    DIR_ITER *dir = NULL;
    char filename[MAXPATHLEN];

    dir = diropen(src);
    if(dir == NULL) {
        return -1;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
            char currentname[MAXPATHLEN];
            char destname[MAXPATHLEN];
            snprintf(currentname, sizeof(currentname), "%s%s/", src, filename);
            snprintf(destname, sizeof(destname), "%s%s/", dest, filename);
            MoveDirectory(currentname, destname);
            }
        } else {
            char currentname[MAXPATHLEN];
            char destname[MAXPATHLEN];
            CreateSubfolder(dest);
            snprintf(currentname, sizeof(currentname), "%s%s", src, filename);
            snprintf(destname, sizeof(destname), "%s%s", dest, filename);
            CopyFile(currentname, destname);
            RemoveFile(currentname);
        }
	}
	dirclose(dir);

	int pos = strlen(src)-1;
	src[pos] = '\0';

    if(remove(src) != 0)
        return -1;

    return 1;
}


/****************************************************************************
 * RemoveDirectory
 *
 * Remove a directory and its content recursively
 ***************************************************************************/
int RemoveDirectory(char * dirpath)
{

    struct stat st;
    DIR_ITER *dir = NULL;
    char filename[MAXPATHLEN];

    dir = diropen(dirpath);
    if(dir == NULL) {
        return -1;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
            char currentname[MAXPATHLEN];
            snprintf(currentname, sizeof(currentname), "%s%s/", dirpath, filename);
            RemoveDirectory(currentname);
            }
        } else {
            char currentname[MAXPATHLEN];
            snprintf(currentname, sizeof(currentname), "%s%s", dirpath, filename);
            RemoveFile(currentname);
            //Display Throbber rotating
            ShowProgress(0, 1, filename);
        }
	}

	dirclose(dir);

	int pos = strlen(dirpath)-1;
	dirpath[pos] = '\0';

    if(remove(dirpath) != 0)
        return -1;

    return 1;
}

/****************************************************************************
 * RemoveFile
 *
 * Delete the file from a given filepath
 ***************************************************************************/
bool RemoveFile(char * filepath)
{

    if(remove(filepath) != 0)
        return false;

    return true;
}

/****************************************************************************
 * GetFolderSize
 *
 * Get recursivly complete foldersize
 ***************************************************************************/
void GetFolderSize(const char * folderpath, u64 &foldersize, u32 &filecount)
{
    struct stat st;
    DIR_ITER *dir = NULL;
    char filename[MAXPATHLEN];

    dir = diropen(folderpath);
    if(dir == NULL) {
        return;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if(!sizegainrunning)
            return;

        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                char currentname[MAXPATHLEN];
                snprintf(currentname, sizeof(currentname), "%s%s/", folderpath, filename);
                GetFolderSize(currentname, foldersize, filecount);
            }
        } else {
            filecount = filecount + 1;
            foldersize = foldersize + st.st_size;
        }
	}
	dirclose(dir);
}
