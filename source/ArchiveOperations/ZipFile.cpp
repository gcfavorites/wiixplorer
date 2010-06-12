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
 * ZipFile.cpp
 *
 * ZipFile Class
 * for WiiXplorer 2009
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "ZipFile.h"
#include "FileOperations/fileops.h"

extern bool actioncanceled;

ZipFile::ZipFile(const char *filepath)
{
    File = unzOpen(filepath);
    if(File)
        this->LoadList();
}

ZipFile::~ZipFile()
{
    for(u32 i = 0; i < ZipStructure.size(); i++)
    {
        if(ZipStructure[i]->filename)
            delete [] ZipStructure[i]->filename;

        if(ZipStructure[i])
            delete ZipStructure[i];
    }

    ZipStructure.clear();

    unzClose(File);
}

bool ZipFile::LoadList()
{
    if(!File)
        return false;

    int ret = unzGoToFirstFile(File);
    if(ret != UNZ_OK)
        return false;

    char filename[1024];
    unz_file_info cur_file_info;
    RealArchiveItemCount = 0;

    do
    {
        if(unzGetCurrentFileInfo(File, &cur_file_info, filename, sizeof(filename), NULL, NULL, NULL, NULL) == UNZ_OK)
        {
            bool isDir = false;
            if(filename[strlen(filename)-1] == '/')
            {
                isDir = true;
                filename[strlen(filename)-1] = '\0';
            }

            int strlength = strlen(filename)+1;

            ArchiveFileStruct * CurArcFile = new ArchiveFileStruct;
            CurArcFile->filename = new char[strlength];
            strcpy(CurArcFile->filename, filename);
            CurArcFile->length = cur_file_info.uncompressed_size;
            CurArcFile->comp_length = cur_file_info.compressed_size;
            CurArcFile->isdir = isDir;
            CurArcFile->fileindex = RealArchiveItemCount;
            CurArcFile->ModTime = (u64) cur_file_info.dosDate;
            CurArcFile->archiveType = ZIP;

            ZipStructure.push_back(CurArcFile);
        }
        ++RealArchiveItemCount;
    }
    while(unzGoToNextFile(File) == UNZ_OK);

    PathControl();

    return true;
}

ArchiveFileStruct * ZipFile::GetFileStruct(int ind)
{
    if(ind < 0 || ind >= (int) ZipStructure.size())
        return NULL;

    return ZipStructure[ind];
}

bool ZipFile::SeekFile(int ind)
{
    if(ind < 0 || ind >= (int) ZipStructure.size())
        return false;

    int ret = unzGoToFirstFile(File);
    if(ret != UNZ_OK)
        return false;

    while(ind > 0)
    {
        if(unzGoToNextFile(File) != UNZ_OK)
            return false;

        --ind;
    }

    return true;
}

void ZipFile::CheckMissingPath(const char * path)
{
    if(!path)
        return;

    u32 i = 0;
    for(i = 0; i < ZipStructure.size(); i++)
    {
        if(strcasecmp(ZipStructure[i]->filename, path) == 0)
            break;
    }

    if(i == ZipStructure.size())
    {
        int strlength = strlen(path)+1;
        ArchiveFileStruct * CurArcFile = new ArchiveFileStruct;
        CurArcFile->filename = new char[strlength];
        strcpy(CurArcFile->filename, path);
        CurArcFile->length = 0;
        CurArcFile->comp_length = 0;
        CurArcFile->isdir = true;
        CurArcFile->fileindex = ZipStructure.size();
        CurArcFile->ModTime = 0;
        CurArcFile->archiveType = ZIP;

        ZipStructure.push_back(CurArcFile);
    }
}

void ZipFile::PathControl()
{
    char missingpath[1024];

    for(u32 n = 0; n < ZipStructure.size(); n++)
    {
        const char * filepath = ZipStructure[n]->filename;
        int strlength = strlen(filepath);

        for(int i = 0; i < strlength; i++)
        {
            if(filepath[i] == '/')
                CheckMissingPath(missingpath);

            missingpath[i] = filepath[i];
            missingpath[i+1] = '\0';
        }
    }
}

int ZipFile::ExtractFile(int ind, const char *dest, bool withpath)
{
    if(!File)
        return -1;

    if(!SeekFile(ind) && ind < RealArchiveItemCount)
        return -1;

    ArchiveFileStruct * CurArcFile = GetFileStruct(ind);

    u32 done = 0;

	char * RealFilename = strrchr(CurArcFile->filename, '/');
	if(RealFilename)
        RealFilename += 1;
    else
        RealFilename = CurArcFile->filename;

	char writepath[MAXPATHLEN];
	if(withpath)
        snprintf(writepath, sizeof(writepath), "%s/%s", dest, CurArcFile->filename);
    else
        snprintf(writepath, sizeof(writepath), "%s/%s", dest, RealFilename);

	u32 filesize = CurArcFile->length;

    if(CurArcFile->isdir)
    {
        strncat(writepath, "/", sizeof(writepath));
        CreateSubfolder(writepath);
        return 1;
    }

    int ret = unzOpenCurrentFile(File);

    if(ret != UNZ_OK)
        return -2;

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

    u32 blocksize = 1024*50;
    void *buffer = malloc(blocksize);

    FILE *pfile = fopen(writepath, "wb");
    if(!pfile)
    {
        unzCloseCurrentFile(File);
        StopProgress();
        free(buffer);
        fclose(pfile);
        WindowPrompt(tr("Could not extract file:"), CurArcFile->filename, "OK");
        return -3;
    }

    do
    {
        if(actioncanceled)
        {
            usleep(20000);
            free(buffer);
            fclose(pfile);
            unzCloseCurrentFile(File);
            StopProgress();
            return -10;
        }

        ShowProgress(done, filesize, RealFilename);

        if(filesize - done < blocksize)
            blocksize = filesize - done;

        ret = unzReadCurrentFile(File, buffer, blocksize);
        if(ret < 0)
        {
            free(buffer);
            fclose(pfile);
            unzCloseCurrentFile(File);
            StopProgress();
            return -4;
        }

        fwrite(buffer, 1, blocksize, pfile);

        done += ret;

    } while(done < filesize);

    fclose(pfile);
    unzCloseCurrentFile(File);

    free(buffer);

    return 1;
}

int ZipFile::ExtractAll(const char *dest)
{
    if(!File)
        return -1;

    bool Stop = false;

    u32 blocksize = 1024*50;
    void *buffer = malloc(blocksize);

    if(!buffer)
        return -5;

    char writepath[MAXPATHLEN];
    char filename[MAXPATHLEN];
    memset(writepath, 0, sizeof(writepath));
    memset(filename, 0, sizeof(filename));

    unz_file_info cur_file_info;

    int ret = unzGoToFirstFile(File);
    if(ret != UNZ_OK)
    {
        free(buffer);
        return -6;
    }

    StartProgress(tr("Extracting files..."));

    while(!Stop)
    {
        if(unzGetCurrentFileInfo(File, &cur_file_info, filename, sizeof(filename), NULL, NULL, NULL, NULL) != UNZ_OK)
        {
            Stop = true;
        }

        if(!Stop && filename[strlen(filename)-1] != '/')
        {
            u64 uncompressed_size = cur_file_info.uncompressed_size;

            u64 done = 0;
            char *pointer = NULL;

            ret = unzOpenCurrentFile(File);

            snprintf(writepath, sizeof(writepath), "%s/%s", dest, filename);

            pointer = strrchr(writepath, '/');
            int position = pointer-writepath+2;

            char temppath[strlen(writepath)];
            snprintf(temppath, position, "%s", writepath);

            CreateSubfolder(temppath);

            if(ret == UNZ_OK)
            {
                FILE *pfile = fopen(writepath, "wb");
                if(!pfile)
                {
                    free(buffer);
                    fclose(pfile);
                    unzCloseCurrentFile(File);
                    StopProgress();
                    return -8;
                }

                do
                {
                    if(actioncanceled) {
                        usleep(20000);
                        free(buffer);
                        fclose(pfile);
                        unzCloseCurrentFile(File);
                        StopProgress();
                        return -10;
                    }

                    ShowProgress(done, uncompressed_size, pointer+1);

                    if(uncompressed_size - done < blocksize)
                        blocksize = uncompressed_size - done;

                    ret = unzReadCurrentFile(File, buffer, blocksize);

                    if(ret == 0)
                        break;

                    fwrite(buffer, 1, blocksize, pfile);

                    done += ret;

                } while(done < uncompressed_size);

                fclose(pfile);
                unzCloseCurrentFile(File);
            }
        }
        if(unzGoToNextFile(File) != UNZ_OK)
        {
            Stop = true;
        }
    }

    free(buffer);
    buffer = NULL;

    StopProgress();

    return 1;
}