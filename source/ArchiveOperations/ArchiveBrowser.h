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
 * ArchiveBrowser
 *
 * ArchiveBrowser Class
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef ARCHIVE_BROWSER_H_
#define ARCHIVE_BROWSER_H_

#include <gctypes.h>
#include <vector>
#include <algorithm>
#include "FileOperations/Browser.hpp"
#include "Archive.h"

class ArchiveBrowser : public Browser
{
    public:
        //!Constructor
        ArchiveBrowser(const char * filepath);
        ArchiveBrowser(Archive * archive);
        //!Destructor
        ~ArchiveBrowser();
        //!Extract item (folder or file) into a destination path
        int ExtractItem(int ind, const char * dest);
        int ExtractCurrentItem(const char * dest) { return ExtractItem(SelIndex, dest); };
        //!Extract a folder into a destination path
        int ExtractFolder(const char * foldername, const char * dest);
        //!Extract the all archive items into a destination path
        int ExtractAll(const char * dest);
        //!Get the current EntrieCount
        int GetEntrieCount() { return PathStructure.size(); };
        //!Get current pageindex
        int GetPageIndex() { return PageIndex; };
        //!Get current pageindex
        int GetSelIndex() { return SelIndex; };
        //!Set the pageindex
        void SetPageIndex(int ind);
        //!Set the SelectedIndex
        void SetSelectedIndex(int ind);
        //!Is the current ind item a directory
        bool IsDir(int ind);
        //!Get the item archive structure of ind file
        ArchiveFileStruct * GetItemStructure(int ind);
        ArchiveFileStruct * GetCurrentItemStructure() { return GetItemStructure(SelIndex); };
        //!Get the name to display in the explorer
        const char * GetItemDisplayname(int ind);
        //!Get the name to display in the explorer
        const char * GetCurrentDisplayname() { return GetItemDisplayname(SelIndex); };
        //!Change the folder
        int ChangeDirectory();
        //!Parse the archive directory into an archive structure
        int ParseArchiveDirectory(const char * ArcPath);
        //!Returns a ItemStruct for the file marker
        ItemStruct GetCurrentItemStruct() const;
        //!Get the current archive path
        const char * GetCurrentPath();
        //!Refresh current directory
        void Refresh();

        //!Clear the current PathStructure and free the memory
        void ClearList();
        //!Sort the current PathStructure
        void SortList();
    private:
        int EnterSelDir();
        int LeaveCurDir();
        void AddListEntrie(const char * filename, size_t length, size_t comp_length, bool isdir, u32 index, u64 modtime, u8 archiveType);
        bool InDirectoryTree(const char * Path, const char * itemFullFilename, bool firstpage);

        Archive * archive;
        bool ExternalArchive;
        int PageIndex;
        int SelIndex;
        u32 ItemNumber;
        char currentPath[MAXPATHLEN];
        char * displayPath;
        char * OrigArchiveFilename;

        std::vector<ArchiveFileStruct *> PathStructure;

        static bool FileSortCallback(const ArchiveFileStruct * f1, const ArchiveFileStruct * f2);
};

#endif //ARCHIVE_BROWSER_H_
