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

#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "fileops.h"
#include "filebrowser.h"

#define BLOCKSIZE               70*1024      //70KB


typedef struct {
    char    *name;
} SubDirList;

bool replaceall = false;
bool replacenone = false;

extern bool sizegainrunning;
extern bool actioncanceled;

/****************************************************************************
 * GetReplaceChoice
 *
 * Get the user choice if he wants to replace a file or not
 ***************************************************************************/
int GetReplaceChoice(const char * filename)
{
    const char * progressText = GetProgressTitle();
    StopProgress();

    int choice = WindowPrompt(fmt("%s %s", tr("File already exists:"), filename), tr("Do you want to replace this file?"), tr("Yes"), tr("No"), tr("Yes to all"), tr("No to all"));

    StartProgress(progressText);

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
 * FindFile
 *
 * Check if file is available in the given directory
 ***************************************************************************/
bool FindFile(const char * filename, const char * path)
{
    if(!filename || ! path)
        return false;

    DIR_ITER *dir = NULL;
    struct stat st;
    char currfilename[MAXPATHLEN];

    dir = diropen(path);
    if(!dir)
    {
        dirclose(dir);
        return false;
    }

    while (dirnext(dir,currfilename,&st) == 0)
    {
        if (strcasecmp(currfilename, filename) == 0)
        {
            dirclose(dir);
            return true;
        }
    }
    dirclose(dir);
    return false;
}

/****************************************************************************
 * SearchFile
 *
 * Search for a file recursive through all subdirectories
 ***************************************************************************/
bool SearchFile(const char * searchpath, const char * searched_filename, char * outfilepath)
{
    struct stat st;
    DIR_ITER *dir = NULL;
    bool result = false;

    char filename[MAXPATHLEN];
    char pathptr[strlen(searchpath)+2];
    snprintf(pathptr, sizeof(pathptr), "%s", searchpath);

    if(pathptr[strlen(pathptr)-1] == '/')
    {
        pathptr[strlen(pathptr)-1] = '\0';
    }

	char * notRoot = strrchr(pathptr, '/');
	if(!notRoot)
	{
	    strcat(pathptr, "/");
	}

    dir = diropen(pathptr);
    if(!dir)
        return false;

    while (dirnext(dir,filename,&st) == 0 && result == false)
	{
	    if(strcasecmp(filename, searched_filename) == 0)
	    {
	        if(outfilepath)
	        {
	            sprintf(outfilepath, "%s/%s", pathptr, filename);
	        }
	        result = true;
	    }
        else if((st.st_mode & S_IFDIR) != 0)
        {
            if(strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0)
            {
                char newpath[1024];
                snprintf(newpath, sizeof(newpath), "%s/%s", pathptr, filename);
                result = SearchFile(newpath, searched_filename, outfilepath);
            }
        }
	}
    dirclose(dir);

    return result;
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

    struct stat filestat;

    char dirnoslash[strlen(filepath)+2];
    snprintf(dirnoslash, sizeof(dirnoslash), "%s", filepath);

    if(dirnoslash[strlen(dirnoslash)-1] == '/')
        dirnoslash[strlen(dirnoslash)-1] = '\0';

	char * notRoot = strrchr(dirnoslash, '/');
	if(!notRoot)
	{
	    strcat(dirnoslash, "/");
	}

    if (stat(dirnoslash, &filestat) == 0)
        return true;

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
    int ret = -1;
    u64 filesize = FileSize(filepath);
	char * filename = strrchr(filepath, '/');
	if(filename)
        filename++;

    FILE *file = fopen(filepath, "rb");

    if (file == NULL)
    {
        *inbuffer = NULL;
        *size = 0;
        return -1;
    }

    u8 *buffer = (u8 *) malloc(filesize);
    if (buffer == NULL)
    {
        fclose(file);
        *inbuffer = NULL;
        *size = 0;
        return -2;
    }

    u64 done = 0;
    u32 blocksize = BLOCKSIZE;

    do
    {
        if(actioncanceled)
        {
            free(buffer);
            fclose(file);
            *inbuffer = NULL;
            *size = 0;
            return -10;
        }

        if(blocksize > filesize-done)
            blocksize = filesize-done;

        ShowProgress(done, filesize, filename);
        ret = fread(buffer+done, 1, blocksize, file);
        if(ret < 0)
        {
            free(buffer);
            fclose(file);
            *inbuffer = NULL;
            *size = 0;
            return -3;
        }
        else if(ret == 0)
        {
            //we are done
            break;
        }

        done += ret;

    } while(done < filesize);

    fclose(file);

    if (done != filesize)
    {
        free(buffer);
        *inbuffer = NULL;
        *size = 0;
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
int LoadFileToMemWithProgress(const char *progressText, const char *filepath, u8 **inbuffer, u64 *size)
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

    bool result  = false;

    char dirnoslash[strlen(fullpath)+1];
    strcpy(dirnoslash, fullpath);

    int pos = strlen(dirnoslash)-1;
    while(dirnoslash[pos] == '/')
    {
        dirnoslash[pos] = '\0';
        pos--;
    }

    if(CheckFile(dirnoslash))
    {
        return true;
    }
    else
    {
        char parentpath[strlen(dirnoslash)+2];
        strcpy(parentpath, dirnoslash);
        char * ptr = strrchr(parentpath, '/');

        if(!ptr)
        {
            //!Device root directory (must be with '/')
            strcat(parentpath, "/");
            struct stat filestat;
            if (stat(parentpath, &filestat) == 0)
                return true;

            return false;
        }

        ptr++;
        ptr[0] = '\0';

        result = CreateSubfolder(parentpath);
    }

    if(!result)
        return false;

    if (mkdir(dirnoslash, 0777) == -1)
    {
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
	u32 read = 1;
	u32 wrote = 1;
    u64 sizesrc = FileSize(src);

	char * filename = strrchr(src, '/');
	if(filename)
        filename++;

    bool fileexist = CheckFile(dest);

    if(fileexist == true)
    {
        int choice = -1;
        if(!replaceall && !replacenone)
            choice = GetReplaceChoice(filename);

        if(replacenone || choice == 2)
            return 1;

        if(strstr(dest, "ntfs") != NULL)
        {
            RemoveFile(dest);
        }
    }

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
        if(actioncanceled)
        {
            fclose(source);
            fclose(destination);
            free(buffer);
            RemoveFile((char *) dest);
            return -10;
        }

        if(blksize > sizesrc - done)
            blksize = sizesrc - done;

        //Display progress
        ShowProgress(done, sizesrc, filename);
        read = fread(buffer, 1, blksize, source);
        if(read < 0)
        {
            fclose(source);
            fclose(destination);
            free(buffer);
            RemoveFile((char *) dest);
            return -3;
        }

        wrote = fwrite(buffer, 1, read, destination);
        if(wrote < 0)
        {
            fclose(source);
            fclose(destination);
            free(buffer);
            RemoveFile((char *) dest);
            return -3;
        }

        done += wrote;

    } while (read > 0);

    free(buffer);
    fclose(source);
    fclose(destination);

    if(sizesrc != done)
        return -4;

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

    SubDirList *filelist = (SubDirList *) malloc(sizeof(SubDirList));

    if(!filelist) {
        free(dirlist);
        dirlist = NULL;
        free(filename);
        filename = NULL;
        return -3;
    }

    u32 filecount = 0;

    memset(&(filelist[filecount]), 0, sizeof(SubDirList));

    dir = diropen(src);
    if(dir == NULL) {
        free(dirlist);
        dirlist = NULL;
        free(filelist);
        filelist = NULL;
        free(filename);
        filename = NULL;
        return -1;
    }

    CreateSubfolder(dest);

    while (dirnext(dir,filename,&st) == 0)
	{
        if(actioncanceled) {
            for(u32 i = 0; i < dircount; i++) {
                if(dirlist[i].name) {
                    free(dirlist[i].name);
                    dirlist[i].name = NULL;
                }
            }
            for(u32 i = 0; i < filecount; i++) {
                if(filelist[i].name) {
                    free(filelist[i].name);
                    filelist[i].name = NULL;
                }
            }
            free(dirlist);
            dirlist = NULL;
            free(filelist);
            filelist = NULL;
            dirclose(dir);
            free(filename);
            filename = NULL;
            return -10;
        }

        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                SubDirList *newdirlist = (SubDirList*) realloc(dirlist, (dircount+1)* sizeof(SubDirList));

                if(!newdirlist) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    for(u32 i = 0; i < filecount; i++) {
                        if(filelist[i].name) {
                            free(filelist[i].name);
                            filelist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(newdirlist);
                    newdirlist = NULL;
                    free(filelist);
                    filelist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -4;

                } else
                    dirlist = newdirlist;

                memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

                dirlist[dircount].name = (char*) malloc(strlen(filename)+2);
                if(!dirlist[dircount].name) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    for(u32 i = 0; i < filecount; i++) {
                        if(filelist[i].name) {
                            free(filelist[i].name);
                            filelist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filelist);
                    filelist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -5;
                }
                sprintf(dirlist[dircount].name, "%s/", filename);
                dircount++;
            }
        } else {

            SubDirList *newfilelist = (SubDirList*) realloc(filelist, (filecount+1)* sizeof(SubDirList));

            if(!newfilelist) {
                for(u32 i = 0; i < dircount; i++) {
                    if(dirlist[i].name) {
                        free(dirlist[i].name);
                        dirlist[i].name = NULL;
                    }
                }
                for(u32 i = 0; i < filecount; i++) {
                    if(filelist[i].name) {
                        free(filelist[i].name);
                        filelist[i].name = NULL;
                    }
                }
                free(dirlist);
                dirlist = NULL;
                free(filelist);
                filelist = NULL;
                free(newfilelist);
                newfilelist = NULL;
                free(filename);
                filename = NULL;
                dirclose(dir);
                return -4;

            } else
                filelist = newfilelist;

            memset(&(filelist[filecount]), 0, sizeof(SubDirList));

            filelist[filecount].name = (char*) malloc(strlen(filename)+2);
            if(!filelist[filecount].name) {
                for(u32 i = 0; i < dircount; i++) {
                    if(dirlist[i].name) {
                        free(dirlist[i].name);
                        dirlist[i].name = NULL;
                    }
                }
                for(u32 i = 0; i < filecount; i++) {
                    if(filelist[i].name) {
                        free(filelist[i].name);
                        filelist[i].name = NULL;
                    }
                }
                free(dirlist);
                dirlist = NULL;
                free(filelist);
                filelist = NULL;
                free(filename);
                filename = NULL;
                dirclose(dir);
                return -5;
            }
            sprintf(filelist[filecount].name, "%s", filename);
            filecount++;
        }
	}
	dirclose(dir);
    free(filename);
    filename = NULL;

    for(u32 i = 0; i < filecount; i++) {
        char currentname[strlen(src)+strlen(filelist[i].name)+2];
        char destname[strlen(dest)+strlen(filelist[i].name)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", src, filelist[i].name);
        snprintf(destname, sizeof(destname), "%s%s", dest, filelist[i].name);
        CopyFile(currentname, destname);
        if(filelist[i].name) {
            free(filelist[i].name);
            filelist[i].name = NULL;
        }
    }

    if(filelist) {
        free(filelist);
        filelist = NULL;
    }

    for(u32 i = 0; i < dircount; i++) {
        char currentname[strlen(src)+strlen(dirlist[i].name)+2];
        char destname[strlen(dest)+strlen(dirlist[i].name)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", src, dirlist[i].name);
        snprintf(destname, sizeof(destname), "%s%s", dest, dirlist[i].name);
        CopyDirectory(currentname, destname);

        //done free entry memory now
        if(dirlist[i].name) {
            free(dirlist[i].name);
            dirlist[i].name = NULL;
        }
	}

    if(dirlist) {
        free(dirlist);
        dirlist = NULL;
    }

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
    bool samedevices = CompareDevices(src, dest);

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

    SubDirList *filelist = (SubDirList *) malloc(sizeof(SubDirList));

    if(!filelist) {
        free(dirlist);
        dirlist = NULL;
        free(filename);
        filename = NULL;
        return -3;
    }

    u32 filecount = 0;

    memset(&(filelist[filecount]), 0, sizeof(SubDirList));

    dir = diropen(src);
    if(dir == NULL) {
        free(dirlist);
        dirlist = NULL;
        free(filelist);
        filelist = NULL;
        free(filename);
        filename = NULL;
        return -1;
    }

    CreateSubfolder(dest);

    while (dirnext(dir,filename,&st) == 0)
	{
        if(actioncanceled) {
            for(u32 i = 0; i < dircount; i++) {
                if(dirlist[i].name) {
                    free(dirlist[i].name);
                    dirlist[i].name = NULL;
                }
            }
            for(u32 i = 0; i < filecount; i++) {
                if(filelist[i].name) {
                    free(filelist[i].name);
                    filelist[i].name = NULL;
                }
            }
            free(dirlist);
            dirlist = NULL;
            free(filelist);
            filelist = NULL;
            dirclose(dir);
            free(filename);
            filename = NULL;
            return -10;
        }

        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                SubDirList *newdirlist = (SubDirList*) realloc(dirlist, (dircount+1)* sizeof(SubDirList));

                if(!newdirlist) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    for(u32 i = 0; i < filecount; i++) {
                        if(filelist[i].name) {
                            free(filelist[i].name);
                            filelist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(newdirlist);
                    newdirlist = NULL;
                    free(filelist);
                    filelist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -4;

                } else
                    dirlist = newdirlist;

                memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

                dirlist[dircount].name = (char*) malloc(strlen(filename)+2);
                if(!dirlist[dircount].name) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    for(u32 i = 0; i < filecount; i++) {
                        if(filelist[i].name) {
                            free(filelist[i].name);
                            filelist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filelist);
                    filelist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -5;
                }
                sprintf(dirlist[dircount].name, "%s/", filename);
                dircount++;
            }
        } else {

            SubDirList *newfilelist = (SubDirList*) realloc(filelist, (filecount+1)* sizeof(SubDirList));

            if(!newfilelist) {
                for(u32 i = 0; i < dircount; i++) {
                    if(dirlist[i].name) {
                        free(dirlist[i].name);
                        dirlist[i].name = NULL;
                    }
                }
                for(u32 i = 0; i < filecount; i++) {
                    if(filelist[i].name) {
                        free(filelist[i].name);
                        filelist[i].name = NULL;
                    }
                }
                free(dirlist);
                dirlist = NULL;
                free(filelist);
                filelist = NULL;
                free(newfilelist);
                newfilelist = NULL;
                free(filename);
                filename = NULL;
                dirclose(dir);
                return -4;

            } else
                filelist = newfilelist;

            memset(&(filelist[filecount]), 0, sizeof(SubDirList));

            filelist[filecount].name = (char*) malloc(strlen(filename)+2);
            if(!filelist[filecount].name) {
                for(u32 i = 0; i < dircount; i++) {
                    if(dirlist[i].name) {
                        free(dirlist[i].name);
                        dirlist[i].name = NULL;
                    }
                }
                for(u32 i = 0; i < filecount; i++) {
                    if(filelist[i].name) {
                        free(filelist[i].name);
                        filelist[i].name = NULL;
                    }
                }
                free(dirlist);
                dirlist = NULL;
                free(filelist);
                filelist = NULL;
                free(filename);
                filename = NULL;
                dirclose(dir);
                return -5;
            }
            sprintf(filelist[filecount].name, "%s", filename);
            filecount++;
        }
	}
	dirclose(dir);
    free(filename);
    filename = NULL;

    for(u32 i = 0; i < filecount; i++)
    {
        char currentname[strlen(src)+strlen(filelist[i].name)+2];
        char destname[strlen(dest)+strlen(filelist[i].name)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", src, filelist[i].name);
        snprintf(destname, sizeof(destname), "%s%s", dest, filelist[i].name);
        MoveFile(currentname, destname);

        if(samedevices)
            ShowProgress(0, 1, filelist[i].name);

        if(filelist[i].name) {
            free(filelist[i].name);
            filelist[i].name = NULL;
        }
    }

    if(filelist) {
        free(filelist);
        filelist = NULL;
    }

    for(u32 i = 0; i < dircount; i++) {
        char currentname[strlen(src)+strlen(dirlist[i].name)+2];
        char destname[strlen(dest)+strlen(dirlist[i].name)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", src, dirlist[i].name);
        snprintf(destname, sizeof(destname), "%s%s", dest, dirlist[i].name);
        MoveDirectory(currentname, destname);

        //done free entry memory now
        if(dirlist[i].name) {
            free(dirlist[i].name);
            dirlist[i].name = NULL;
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
 * MoveFile
 *
 * Move a file from srcpath to destdir
 ***************************************************************************/
int MoveFile(const char *srcpath, char *destdir)
{
    if(CompareDevices(srcpath, destdir))
    {
        if(rename(srcpath, destdir) == 0)
            return 1;
        else
            return -1;
    }

    int res = CopyFile(srcpath, destdir);
    if(res < 0)
        return -1;

    if(RemoveFile(srcpath))
        return 1;

    return -1;
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

    if(dirpath[strlen(dirpath)-1] != '/')
        strcat(dirpath, "/");

    memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

    SubDirList *filelist = (SubDirList *) malloc(sizeof(SubDirList));

    if(!filelist) {
        free(dirlist);
        dirlist = NULL;
        free(filename);
        filename = NULL;
        return -3;
    }

    u32 filecount = 0;

    memset(&(filelist[filecount]), 0, sizeof(SubDirList));

    dir = diropen(dirpath);
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
                if(dirlist[i].name) {
                    free(dirlist[i].name);
                    dirlist[i].name = NULL;
                }
            }
            for(u32 i = 0; i < filecount; i++) {
                if(filelist[i].name) {
                    free(filelist[i].name);
                    filelist[i].name = NULL;
                }
            }
            free(dirlist);
            dirlist = NULL;
            free(filelist);
            filelist = NULL;
            dirclose(dir);
            free(filename);
            filename = NULL;
            return -10;
        }

        if((st.st_mode & S_IFDIR) != 0) {
            if(strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                SubDirList *newdirlist = (SubDirList*) realloc(dirlist, (dircount+1)* sizeof(SubDirList));

                if(!newdirlist) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    for(u32 i = 0; i < filecount; i++) {
                        if(filelist[i].name) {
                            free(filelist[i].name);
                            filelist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(newdirlist);
                    newdirlist = NULL;
                    free(filelist);
                    filelist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -4;

                } else
                    dirlist = newdirlist;

                memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

                dirlist[dircount].name = (char*) malloc(strlen(filename)+2);
                if(!dirlist[dircount].name) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    for(u32 i = 0; i < filecount; i++) {
                        if(filelist[i].name) {
                            free(filelist[i].name);
                            filelist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filelist);
                    filelist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return -5;
                }
                sprintf(dirlist[dircount].name, "%s/", filename);
                dircount++;
            }
        } else {

            SubDirList *newfilelist = (SubDirList*) realloc(filelist, (filecount+1)* sizeof(SubDirList));

            if(!newfilelist) {
                for(u32 i = 0; i < dircount; i++) {
                    if(dirlist[i].name) {
                        free(dirlist[i].name);
                        dirlist[i].name = NULL;
                    }
                }
                for(u32 i = 0; i < filecount; i++) {
                    if(filelist[i].name) {
                        free(filelist[i].name);
                        filelist[i].name = NULL;
                    }
                }
                free(dirlist);
                dirlist = NULL;
                free(filelist);
                filelist = NULL;
                free(newfilelist);
                newfilelist = NULL;
                free(filename);
                filename = NULL;
                dirclose(dir);
                return -4;

            } else
                filelist = newfilelist;

            memset(&(filelist[filecount]), 0, sizeof(SubDirList));

            filelist[filecount].name = (char*) malloc(strlen(filename)+2);
            if(!filelist[filecount].name) {
                for(u32 i = 0; i < dircount; i++) {
                    if(dirlist[i].name) {
                        free(dirlist[i].name);
                        dirlist[i].name = NULL;
                    }
                }
                for(u32 i = 0; i < filecount; i++) {
                    if(filelist[i].name) {
                        free(filelist[i].name);
                        filelist[i].name = NULL;
                    }
                }
                free(dirlist);
                dirlist = NULL;
                free(filelist);
                filelist = NULL;
                free(filename);
                filename = NULL;
                dirclose(dir);
                return -5;
            }
            sprintf(filelist[filecount].name, "%s", filename);
            filecount++;
        }
	}

	dirclose(dir);

    for(u32 i = 0; i < filecount; i++) {
        char fullpath[strlen(dirpath)+strlen(filelist[i].name)+2];
        snprintf(fullpath, sizeof(fullpath), "%s%s", dirpath, filelist[i].name);
        RemoveFile(fullpath);
        //Display Throbber rotating
        ShowProgress(0, 1, filelist[i].name);
        if(filelist[i].name) {
            free(filelist[i].name);
            filelist[i].name = NULL;
        }
    }
    if(filelist) {
        free(filelist);
        filelist = NULL;
    }

    for(u32 i = 0; i < dircount; i++) {
        char fullpath[strlen(dirpath)+strlen(dirlist[i].name)+2];
        snprintf(fullpath, sizeof(fullpath), "%s%s", dirpath, dirlist[i].name);
        RemoveDirectory(fullpath);
        if(dirlist[i].name) {
            free(dirlist[i].name);
            dirlist[i].name = NULL;
        }
    }

	int pos = strlen(dirpath)-1;
	dirpath[pos] = '\0';

    if(actioncanceled)
        return -10;

    if(remove(dirpath) != 0)
        return -1;

    return 1;
}

/****************************************************************************
 * RemoveFile
 *
 * Delete the file from a given filepath
 ***************************************************************************/
bool RemoveFile(const char * filepath)
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
                if(dirlist[i].name) {
                    free(dirlist[i].name);
                    dirlist[i].name = NULL;
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
                SubDirList *newdirlist = (SubDirList*) realloc(dirlist, (dircount+1)* sizeof(SubDirList));

                if(!newdirlist) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(newdirlist);
                    newdirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return;

                } else
                    dirlist = newdirlist;

                memset(&(dirlist[dircount]), 0, sizeof(SubDirList));

                dirlist[dircount].name = (char*) malloc(strlen(filename)+2);
                if(!dirlist[dircount].name) {
                    for(u32 i = 0; i < dircount; i++) {
                        if(dirlist[i].name) {
                            free(dirlist[i].name);
                            dirlist[i].name = NULL;
                        }
                    }
                    free(dirlist);
                    dirlist = NULL;
                    free(filename);
                    filename = NULL;
                    dirclose(dir);
                    return;
                }
                sprintf(dirlist[dircount].name, "%s/", filename);
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
        char currentname[strlen(folderpath)+strlen(dirlist[i].name)+2];
        snprintf(currentname, sizeof(currentname), "%s%s", folderpath, dirlist[i].name);
        GetFolderSize(currentname, foldersize, filecount);

        //done free entry memory now
        if(dirlist[i].name) {
            free(dirlist[i].name);
            dirlist[i].name = NULL;
        }
	}

	free(dirlist);
    dirlist = NULL;
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