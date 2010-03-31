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
 * RarcFile.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"
#include "RarcFile.h"

extern bool actioncanceled;

RarcFile::RarcFile(const char *filepath)
    : WiiArchive(filepath)
{
    ParseFile();
}

RarcFile::RarcFile(const u8 * Buffer, u32 Size)
    : WiiArchive(Buffer, Size)
{
    ParseFile();
}

RarcFile::~RarcFile()
{
    CloseFile();
}

bool RarcFile::ParseFile()
{
    if(!FileBuffer && !File)
        return false;

    ClearList();

    ReadFile(&Header, sizeof(RarcHeader), 0);

    Yaz0_Header Yaz0_Head;
    memcpy(&Yaz0_Head, &Header, sizeof(Yaz0_Header));

    if(Yaz0_Head.magic == 'Yaz0')
    {
        if(!FileBuffer)
        {
            FileBuffer = (u8 *) malloc(FileSize);
            if(!FileBuffer)
            {
                CloseFile();
                return false;
            }
            ReadFile(FileBuffer, FileSize, 0);
            FromMem = true;
        }

        FileSize = Yaz0_Head.decompressed_size;

        u8 * buff = (u8 *) malloc(FileSize);
        if(!buff)
        {
            CloseFile();
            return false;
        }

        uncompressYaz0(FileBuffer, buff, FileSize);

        CloseFile();

        FromMem = true;
        FileBuffer = buff;

        return ParseRarcHeader();
    }
	else if(Header.magic == 'RARC')
	{
        return ParseRarcHeader();
	}

    //Unknown RarcFile
    CloseFile();

    return false;
}

bool RarcFile::ParseRarcHeader()
{
    ReadFile(&Header, sizeof(RarcHeader), 0);

    if(Header.magic != 'RARC')
    {
        CloseFile();
        return false;
    }

    FileSize = Header.size;

    RarcNode RootNode;

    ReadFile(&RootNode, sizeof(RarcNode), sizeof(RarcHeader));

    ItemIndex = 0;

    string ItemPath;
    ParseNode(&RootNode, ItemPath);

    return true;
}

void RarcFile::ParseNode(RarcNode * Node, string & parentDirectory)
{
    u32 StringOffset = Header.stringTableOffset+0x20;
    u32 DataOffset = Header.dataStartOffset+0x20;
    u32 CurrOffset = Header.fileEntriesOffset+0x20+Node->firstFileEntryOffset*sizeof(RarcFileEntry);

    //I love recursion... :P
    string parent_dir = parentDirectory;
    string ItemName;
    GetFilename(StringOffset+Node->filenameOffset, ItemName);

    if(parent_dir.size() == 0)
    {
        parent_dir = ItemName;
    }
    else
    {
        //It's just awesome...
        parent_dir.assign(fmt("%s/%s", parentDirectory.c_str(), ItemName.c_str()));
    }

    ItemName.clear();

    AddListEntrie(parent_dir.c_str(), 0, 0, true, ItemIndex++, 0, ArcArch);
    BufferOffset.push_back(0);

    for(u16 i = 0; i < Node->numFileEntries; i++)
    {
        RarcFileEntry FileEntry;
        ReadFile(&FileEntry, sizeof(RarcFileEntry), CurrOffset);

        GetFilename(StringOffset+FileEntry.filenameOffset, ItemName);

        u32 filelength = FileEntry.dataSize;

        /* It's a dir... */
        if(FileEntry.id == 0xFFFF)
        {
            if(strcmp(ItemName.c_str(), ".") != 0 && strcmp(ItemName.c_str(), "..") != 0)
            {
                RarcNode DirNode;
                ReadFile(&DirNode, sizeof(RarcNode), sizeof(RarcHeader)+sizeof(RarcNode)*FileEntry.dataOffset);
                ParseNode(&DirNode, parent_dir);
            }
        }
        /* It's a file... */
        else
        {
            AddListEntrie(fmt("%s/%s", parent_dir.c_str(), ItemName.c_str()), filelength, filelength, false, ItemIndex++, 0, ArcArch);
            BufferOffset.push_back(DataOffset+FileEntry.dataOffset);
        }

        ItemName.clear();
        CurrOffset += sizeof(RarcFileEntry);
    }
}

void RarcFile::GetFilename(int offset, string & Filename)
{
    int n = -1;
    char Char = 0;

    do
    {
        n++;
        ReadFile(&Char, 1, offset+n);
        Filename.push_back(Char);
    }
    while((Char != 0) && (offset+n < (int) FileSize));
}
