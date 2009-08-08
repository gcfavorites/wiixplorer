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
#include "Prompts/PromptWindows.h"
#include "fileops.h"
#include "filebrowser.h"

#define BLOCKSIZE               50*1024      //50KB


typedef struct {
    char    *dir;
} SubDirList;

extern bool sizegainrunning;

extern bool actioncanceled;

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
 * LoadFileToMem
 *
 * Load up the file into a block of memory
 ***************************************************************************/
int LoadFileToMem(const char *filepath, u8 **inbuffer, u64 *size)
{
    int ret;
	char temp[MAXPATHLEN];
	sprintf(temp, "%s", filepath);
	char * filename = strrchr(temp-2, '/')+1;

    FILE *file = fopen(filepath, "rb");

    if (file == NULL) {
        return -1;
    }

    u64 filesize = FileSize(filepath);

    u8 *tempbuffer = (u8 *) malloc(BLOCKSIZE);
    if (tempbuffer == NULL) {
        fclose(file);
        return -2;
    }

    u64 done = 0;

    u8 *readbuffer = (u8 *) malloc(BLOCKSIZE);
    if(!readbuffer) {
            free(tempbuffer);
            fclose(file);
            return -2;
    }

    do {
        if(actioncanceled) {
            usleep(20000);
            free(readbuffer);
            free(tempbuffer);
            fclose(file);
            return -10;
        }

        ShowProgress(done, filesize, filename);
        ret = fread(tempbuffer, 1, BLOCKSIZE, file);
        done += ret;

        readbuffer = (u8 *) realloc(readbuffer, done);
        if(!readbuffer) {
            free(readbuffer);
            free(tempbuffer);
            fclose(file);
            return -2;
        }

        memcpy(readbuffer+(done-ret), tempbuffer, ret);

    } while(done < filesize);

    free(tempbuffer);
    fclose(file);

    if (done != filesize) {
        free(readbuffer);
        return -3;
    }

    *inbuffer = readbuffer;
    *size = filesize;

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
        if(actioncanceled) {
            usleep(20000);
            fclose(source);
            fclose(destination);
            delete buffer;
            RemoveFile((char *) dest);
            return -10;
        }

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

    char *filename = (char *) malloc(MAXPATHLEN);

    if(!filename)
        return -2;

    SubDirList *dirlist = (SubDirList *) malloc(sizeof(SubDirList));

    if(!dirlist) {
        free(filename);
        filename = NULL;
        return -3;
    }

    u32 dircount = 0;

    memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

    dir = diropen(src);
    if(dir == NULL) {
        free(dirlist);
        dirlist = NULL;
        free(filename);
        filename = NULL;
        return -1;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if(actioncanceled) {
            for(u32 i = 0; i < dircount; i++) {
                if(dirlist[i].dir) {
                    free(dirlist[i].dir);
                    dirlist[i].dir = NULL;
                }
            }
            free(dirlist);
            dirlist = NULL;
            free(filename);
            filename = NULL;
            dirclose(dir);
            return -10;
        }

        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                dirlist = (SubDirList*) realloc(dirlist, (dircount+1)* sizeof(SubDirList));

                if(!dirlist) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].dir) {
                            free(dirlist[i].dir);
                            dirlist[i].dir = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -4;
                }

                memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

                dirlist[dircount].dir = (char*) malloc(strlen(filename)+2);
                if(!dirlist[dircount].dir) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].dir) {
                            free(dirlist[i].dir);
                            dirlist[i].dir = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -5;
                }
                sprintf(dirlist[dircount].dir, "%s/", filename);
                dircount++;
            }
        } else {
            char currentname[strlen(src)+strlen(filename)+2];
            char destname[strlen(dest)+strlen(filename)+2];
            CreateSubfolder(dest);
            snprintf(currentname, sizeof(currentname), "%s%s", src, filename);
            snprintf(destname, sizeof(destname), "%s%s", dest, filename);
            CopyFile(currentname, destname);
        }
	}
	dirclose(dir);
    free(filename);
    filename = NULL;

    for(u32 i = 0; i < dircount; i++) {
        char currentname[strlen(src)+strlen(dirlist[i].dir)+2];
        char destname[strlen(dest)+strlen(dirlist[i].dir)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", src, dirlist[i].dir);
        snprintf(destname, sizeof(destname), "%s%s", dest, dirlist[i].dir);
        CopyDirectory(currentname, destname);

        //done free entry memory now
        if(dirlist[i].dir) {
            free(dirlist[i].dir);
            dirlist[i].dir = NULL;
        }
	}

	free(dirlist);
    dirlist = NULL;

    if(actioncanceled)
        return -10;

    return 1;
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

    char *filename = (char *) malloc(MAXPATHLEN);

    if(!filename)
        return -2;

    SubDirList *dirlist = (SubDirList *) malloc(sizeof(SubDirList));

    if(!dirlist) {
        free(filename);
        filename = NULL;
        return -3;
    }

    u32 dircount = 0;

    memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

    dir = diropen(src);
    if(dir == NULL) {
        free(dirlist);
        dirlist = NULL;
        free(filename);
        filename = NULL;
        return -1;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if(actioncanceled) {
            for(u32 i = 0; i < dircount; i++) {
                if(dirlist[i].dir) {
                    free(dirlist[i].dir);
                    dirlist[i].dir = NULL;
                }
            }
            free(dirlist);
            dirlist = NULL;
            free(filename);
            filename = NULL;
            dirclose(dir);
            return -10;
        }

        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                dirlist = (SubDirList*) realloc(dirlist, (dircount+1)* sizeof(SubDirList));

                if(!dirlist) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].dir) {
                            free(dirlist[i].dir);
                            dirlist[i].dir = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -4;
                }

                memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

                dirlist[dircount].dir = (char*) malloc(strlen(filename)+2);
                if(!dirlist[dircount].dir) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].dir) {
                            free(dirlist[i].dir);
                            dirlist[i].dir = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -5;
                }
                sprintf(dirlist[dircount].dir, "%s/", filename);
                dircount++;
            }
        } else {
            char currentname[strlen(src)+strlen(filename)+2];
            char destname[strlen(dest)+strlen(filename)+2];
            CreateSubfolder(dest);
            snprintf(currentname, sizeof(currentname), "%s%s", src, filename);
            snprintf(destname, sizeof(destname), "%s%s", dest, filename);
            CopyFile(currentname, destname);
            RemoveFile(currentname);
        }
	}
	dirclose(dir);
    free(filename);
    filename = NULL;

    for(u32 i = 0; i < dircount; i++) {
        char currentname[strlen(src)+strlen(dirlist[i].dir)+2];
        char destname[strlen(dest)+strlen(dirlist[i].dir)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", src, dirlist[i].dir);
        snprintf(destname, sizeof(destname), "%s%s", dest, dirlist[i].dir);
        CopyDirectory(currentname, destname);

        //done free entry memory now
        if(dirlist[i].dir) {
            free(dirlist[i].dir);
            dirlist[i].dir = NULL;
        }
	}

	free(dirlist);
    dirlist = NULL;

	int pos = strlen(src)-1;
	src[pos] = '\0';

    if(actioncanceled)
        return -10;

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
    int ret = 1;

    char filename[MAXPATHLEN];

    dir = diropen(dirpath);
    if(dir == NULL) {
        return -1;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if(actioncanceled) {
            dirclose(dir);
            return -10;
        }

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

    if(actioncanceled)
        return -10;

    if(remove(dirpath) != 0)
        return -1;

    return ret;
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

    char *filename = (char *) malloc(MAXPATHLEN);

    if(!filename)
        return;

    SubDirList *dirlist = (SubDirList *) malloc(sizeof(SubDirList));

    if(!dirlist) {
        free(filename);
        filename = NULL;
        return;
    }

    u32 dircount = 0;

    memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

    dir = diropen(folderpath);
    if(dir == NULL) {
        free(dirlist);
        dirlist = NULL;
        free(filename);
        filename = NULL;
        return;
    }

    while (dirnext(dir,filename,&st) == 0)
	{
        if(!sizegainrunning) {
            for(u32 i = 0; i < dircount; i++) {
                if(dirlist[i].dir) {
                    free(dirlist[i].dir);
                    dirlist[i].dir = NULL;
                }
            }
            free(dirlist);
            dirlist = NULL;
            dirclose(dir);
            free(filename);
            filename = NULL;
            return;
        }

        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                dirlist = (SubDirList*) realloc(dirlist, (dircount+1)* sizeof(SubDirList));

                if(!dirlist) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].dir) {
                            free(dirlist[i].dir);
                            dirlist[i].dir = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return;
                }

                memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

                dirlist[dircount].dir = (char*) malloc(strlen(filename)+2);
                if(!dirlist[dircount].dir) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].dir) {
                            free(dirlist[i].dir);
                            dirlist[i].dir = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return;
                }
                sprintf(dirlist[dircount].dir, "%s/", filename);
                dircount++;
            }
        } else {
            filecount = filecount + 1;
            foldersize = foldersize + st.st_size;
        }
	}
	dirclose(dir);
	free(filename);
    filename = NULL;

	for(u32 i = 0; i < dircount; i++) {
        char currentname[strlen(folderpath)+strlen(dirlist[i].dir)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", folderpath, dirlist[i].dir);
        GetFolderSize(currentname, foldersize, filecount);

        //done free entry memory now
        if(dirlist[i].dir) {
            free(dirlist[i].dir);
            dirlist[i].dir = NULL;
        }
	}

	free(dirlist);
    dirlist = NULL;
}
