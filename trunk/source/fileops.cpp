/****************************************************************************
 * fileops.cpp
 * by dimok
 *
 * File operations for the WiiXplorer
 * Handling all the needed file operations
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>

#include "PromptWindows.h"
#include "fileops.h"

#define BLOCKSIZE               400*1024      //400KB

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
 * CopyFile
 *
 * Copy the file from source filepath to destination filepath
 ***************************************************************************/
int CopyFile(const char * src, const char * dest) {

	u32 blksize;
	u32 read = 1;

	char temp[MAXPATHLEN];
	sprintf(temp, "%s", src);
	char * filename = strrchr(temp-2, '/')+1;

	FILE * source = fopen(src, "rb");

	if(!source){
		return -2;
	}

    fseek(source, 0, SEEK_END);
    u32 sizesrc = ftell(source);
    rewind(source);

    if(sizesrc < BLOCKSIZE)
        blksize = sizesrc;
    else
        blksize = BLOCKSIZE;

	void * buffer = malloc(blksize);

	if(buffer == NULL){
	    //no memory
        fclose(source);
		return -1;
	}

	FILE * destination = fopen(dest, "wb");

    if(destination == NULL) {
        free(buffer);
        fclose(source);
        return -3;
    }

    u32 done = 0;
    while (read > 0) {
        //Display progress
        ShowProgress(done, sizesrc, filename);
        read = fread(buffer, 1, blksize, source);
        fwrite(buffer, 1, read, destination);
        done += read;
    }

    fclose(source);
    //get size of written file
    u32 sizedest = ftell(destination);

    fclose(destination);
    free(buffer);

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
int CopyDirectory(const char * src, const char * dest) {

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
        }
        else if((st.st_mode & S_IFDIR) == 0) {
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
bool CreateSubfolder(const char * fullpath) {

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
 * RemoveDirectory
 *
 * Remove a directory and its content recursively
 ***************************************************************************/
int RemoveDirectory(char * dirpath) {

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
        }
        else if((st.st_mode & S_IFDIR) == 0) {
            char currentname[MAXPATHLEN];
            snprintf(currentname, sizeof(currentname), "%s%s", dirpath, filename);
            RemoveFile(currentname);
            //Display Throbber rotating
            ShowProgress(0, 0, filename, THROBBER);
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
bool RemoveFile(char * filepath) {

    if(remove(filepath) != 0)
        return false;

    return true;
}
