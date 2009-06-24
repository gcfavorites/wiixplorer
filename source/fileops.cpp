/****************************************************************************
 * fileops.cpp
 * by dimok
 *
 * File operations for the Wiixplorer
 * Handling copying and deleteing/removing files/directories
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>

#include "fileops.h"
#include "filebrowser.h"
#include "PromptWindows.h"

#define BLOCKSIZE               3*1048576      //3MB

bool findfile(const char * filename, const char * path)
{
    DIR *dir;
    struct dirent *file;

    dir = opendir(path);

    char temp[11];
    while ((file = readdir(dir)))
    {
        snprintf(temp,sizeof(temp),"%s",file->d_name);
        if (!strncmpi(temp,filename,11))
            {
            //WindowPrompt(path, filename,"go" ,0);
            closedir(dir);
            return true;
            }
        }
      closedir(dir);
      return false;
}

bool checkfile(char * path)
{
    FILE * f;
    f = fopen(path,"rb");
    if(f) {
    fclose(f);
    return true;
    }
    fclose(f);
    return false;
}

int CopyFile(const char * src, const char * dest) {

	u32 blksize;
	u32 read = 1;

	FILE * source = fopen(src, "rb");

	if(!source){
		return -2;
	}

    fseek(source, 0, SEEK_END);
    u64 sizesrc = ftell(source);
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

    while (read > 0) {
        read = fread(buffer, 1, blksize, source);
        fwrite(buffer, 1, read, destination);
    }

    //get size of written file
    fseek(destination , 0 , SEEK_END);
    u64 sizedest = ftell(destination);

    fclose(source);
    fclose(destination);
    free(buffer);

    if(sizesrc != sizedest) {
        return -4;
    }

	return 1;
}

int CopyDirectory(char * src, char * dest)
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
        }
        else if((st.st_mode & S_IFDIR) == 0) {
            char currentname[MAXPATHLEN];
            char destname[MAXPATHLEN];
            subfoldercreate(dest);
            snprintf(currentname, sizeof(currentname), "%s%s", src, filename);
            snprintf(destname, sizeof(destname), "%s%s", dest, filename);
            CopyFile(currentname, destname);
        }
	}
	dirclose(dir);

    return 1;
}

bool subfoldercreate(char * fullpath) {
        //check forsubfolders
    char dircheck[300];
    char dirnoslash[300];
    char * pch = NULL;
    u32 cnt = 0;
    struct stat st;

    snprintf(dirnoslash, strlen(fullpath), "%s", fullpath);

    if(stat(fullpath, &st) != 0) {
        pch = strrchr(dirnoslash, '/');
        cnt = pch-dirnoslash;
        snprintf(dircheck, cnt+2, "%s", dirnoslash);
        subfoldercreate(dircheck);
    };

    if (mkdir(dirnoslash, 0777) == -1) {
        return false;
    }

    return true;
}
