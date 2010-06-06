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
 * RarFile.cpp
 *
 * RarFile Class
 * for WiiXplorer 2009
 ***************************************************************************/
#include <ogcsys.h>
#include <string.h>

#include "Language/gettext.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PromptWindows.h"
#include "Tools/tools.h"
#include "RarFile.h"
#include "FileOperations/fileops.h"

extern bool actioncanceled;

RarFile::RarFile(const char *filepath)
{
    RarArc.Open(filepath);
    RarArc.SetExceptions(false);
    this->LoadList(filepath);
}

RarFile::~RarFile()
{
    ClearList();
    RarArc.Close();
}

bool RarFile::LoadList(const char * filepath)
{
    if (!RarArc.IsArchive(true))
    {
        RarArc.Close();
        return false;
    }

    if (!RarArc.IsOpened())
    {
        RarArc.Close();
        return false;
    }

    while(RarArc.ReadHeader() > 0)
    {
        int HeaderType=RarArc.GetHeaderType();
        if (HeaderType==ENDARC_HEAD)
            break;

        if(HeaderType == FILE_HEAD)
        {
            u32 UnpSize = 0;
            u32 FileSize = 0;
            if (!(RarArc.NewLhd.Flags & LHD_SPLIT_BEFORE))
            {
                UnpSize = (u32) RarArc.NewLhd.FullUnpSize;
            }
            FileSize = (u32) RarArc.NewLhd.FullPackSize;

            int strlength = strlen(RarArc.NewLhd.FileName)+1;
            ArchiveFileStruct * TempStruct = new ArchiveFileStruct;
            TempStruct->filename = new char[strlength];
            strcpy(TempStruct->filename, RarArc.NewLhd.FileName);
            TempStruct->length = UnpSize;
            TempStruct->comp_length = FileSize;
            TempStruct->isdir = RarArc.IsArcDir();
            TempStruct->fileindex = RarStructure.size();
            TempStruct->ModTime = (u64) RarArc.NewLhd.mtime.GetDos();
            TempStruct->archiveType = RAR;

            RarStructure.push_back(TempStruct);
        }
        RarArc.SeekToNext();
    }
    return true;
}

void RarFile::ClearList()
{
    for(u32 i = 0; i < RarStructure.size(); i++)
    {
        if(RarStructure.at(i)->filename != NULL)
        {
            delete [] RarStructure.at(i)->filename;
            RarStructure.at(i)->filename = NULL;
        }
        if(RarStructure.at(i) != NULL)
        {
            delete RarStructure.at(i);
            RarStructure.at(i) = NULL;
        }
    }

    RarStructure.clear();
}

ArchiveFileStruct * RarFile::GetFileStruct(int ind)
{
    if(ind > (int) RarStructure.size() || ind < 0)
        return NULL;

    return RarStructure.at(ind);
}

u32 RarFile::GetItemCount()
{
    return RarStructure.size();
}


bool RarFile::SeekFile(int ind)
{
    RarArc.Seek(0, SEEK_SET);

    while(RarArc.ReadHeader() > 0)
    {
        int HeaderType=RarArc.GetHeaderType();
        if (HeaderType==ENDARC_HEAD)
            break;

        if(HeaderType == FILE_HEAD && RarArc.NewLhd.FileName &&
           strcmp(RarStructure[ind]->filename, RarArc.NewLhd.FileName) == 0)
            return true;

        RarArc.SeekToNext();
    }

    return false;
}

bool RarFile::CheckPassword()
{
    if((RarArc.NewLhd.Flags & LHD_PASSWORD) && Password.length() == 0)
    {
        const char * Title = GetProgressTitle();
        StopProgress();
        int choice = WindowPrompt(tr("Password is needed."), tr("Please enter the password."), tr("OK"), tr("Cancel"));
        if(!choice)
            return false;

        char entered[150];
        memset(entered, 0, sizeof(entered));

        if(OnScreenKeyboard(entered, sizeof(entered)) == 0)
            return false;

        Password.assign(entered);
        StartProgress(Title);
    }

    return true;
}

void RarFile::UnstoreFile(ComprDataIO &DataIO, int64 DestUnpSize)
{
  Array<byte> Buffer(0x10000);
  while (1)
  {
    uint Code=DataIO.UnpRead(&Buffer[0],Buffer.Size());
    if (Code==0 || (int)Code==-1)
      break;
    Code=Code<DestUnpSize ? Code:(uint)DestUnpSize;
    DataIO.UnpWrite(&Buffer[0],Code);
    if (DestUnpSize>=0)
      DestUnpSize-=Code;
  }
}

int RarFile::InternalExtractFile(const char * outpath, bool withpath)
{
    ComprDataIO DataIO;
    Unpack Unp(&DataIO);
    Unp.Init(NULL);

    char filepath[MAXPATHLEN];
    char * Realfilename = strrchr(RarArc.NewLhd.FileName, '/');
    if(!Realfilename)
        Realfilename = RarArc.NewLhd.FileName;
    else
        Realfilename++;

    if(withpath)
        snprintf(filepath, sizeof(filepath), "%s/%s", outpath, RarArc.NewLhd.FileName);
    else
        snprintf(filepath, sizeof(filepath), "%s/%s", outpath, Realfilename);

    if(RarArc.IsArcDir())
    {
        CreateSubfolder(filepath);
        return 1;
    }

    char * temppath = strdup(filepath);
    char * pointer = strrchr(temppath, '/');
    if(pointer)
    {
        pointer++;
        pointer[0] = '\0';
    }

    CreateSubfolder(temppath);

    free(temppath);
    temppath = NULL;

    if(!CheckPassword())
        return -1;

    File CurFile;
    if(!CurFile.Create(filepath))
    {
        StopProgress();
        ShowError("File not created.");
        return false;
    }

    DataIO.UnpVolume = false;
    DataIO.UnpArcSize = RarArc.NewLhd.FullPackSize;
    DataIO.CurUnpRead=0;
    DataIO.CurUnpWrite=0;
    DataIO.UnpFileCRC=RarArc.OldFormat ? 0 : 0xffffffff;
    DataIO.PackedCRC=0xffffffff;
    DataIO.SetEncryption(
    (RarArc.NewLhd.Flags & LHD_PASSWORD) ? RarArc.NewLhd.UnpVer:0, Password.c_str(),
    (RarArc.NewLhd.Flags & LHD_SALT) ? RarArc.NewLhd.Salt:NULL,false,
    RarArc.NewLhd.UnpVer>=36);
    DataIO.SetPackedSizeToRead(RarArc.NewLhd.FullPackSize);
    DataIO.SetFiles(&RarArc,&CurFile);
    DataIO.SetTestMode(false);
    DataIO.SetSkipUnpCRC(true);

    //! Start always Progresstimer
    ShowProgress(0, RarArc.NewLhd.FullPackSize, Realfilename);

    if(actioncanceled)
        return -10;

    if (RarArc.NewLhd.Method == 0x30)
    {
        UnstoreFile(DataIO,RarArc.NewLhd.FullUnpSize);
    }
    else
    {
        Unp.SetDestSize(RarArc.NewLhd.FullUnpSize);

        if (RarArc.NewLhd.UnpVer <= 15)
            Unp.DoUnpack(15, false);
        else
            Unp.DoUnpack(RarArc.NewLhd.UnpVer,(RarArc.NewLhd.Flags & LHD_SOLID)!=0);
    }

    CurFile.Close();

    return 1;
}

int RarFile::ExtractFile(int fileindex, const char * outpath, bool withpath)
{
    if(fileindex < 0 || fileindex >= (int) RarStructure.size())
    {
        StopProgress();
        return -1;
    }

    if(!SeekFile(fileindex))
    {
        StopProgress();
        return -1;
    }

	return InternalExtractFile(outpath, withpath);
}

int RarFile::ExtractAll(const char * destpath)
{
    //! This is faster than looping and using ExtractFile for each item
    StartProgress(tr("Extracting files..."));
    RarArc.Seek(0, SEEK_SET);

    while(RarArc.ReadHeader() > 0)
    {
        int HeaderType=RarArc.GetHeaderType();
        if (HeaderType==ENDARC_HEAD)
            break;

        if(HeaderType == FILE_HEAD)
        {
            int ret = InternalExtractFile(destpath, true);
            if(ret < 0)
            {
                StopProgress();
                return ret;
            }
        }
        RarArc.SeekToNext();
    }

    StopProgress();

	return 1;
}
