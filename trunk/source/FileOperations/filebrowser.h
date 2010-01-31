/****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * Original Filebrowser by Tantric for libwiigui
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
 * FileBrowser
 *
 * Directory parsing Class
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef _FILEBROWSER_H_
#define _FILEBROWSER_H_

#include <unistd.h>
#include <gccore.h>
#include <sys/dir.h>

#define MAXJOLIET 255
#define MAXDISPLAY MAXPATHLEN

#include "Prompts/DeviceMenu.h"
#include "Browser.hpp"

typedef struct
{
	char dir[MAXPATHLEN]; // directory path of browserList
	char rootdir[10]; // directory path of browserList
	int numEntries; // # of entries in browserList
	int selIndex; // currently selected index of browserList
	int pageIndex; // starting index of browserList page display
} BROWSERINFO;

typedef struct
{
	u64 offset; // DVD offset
	u64 length; // file length in 64 bytes for sizes higher than 4GB
	bool isdir; // 0 - file, 1 - directory
	char filename[MAXJOLIET + 1]; // full filename
	char displayname[MAXDISPLAY + 1]; // name for browser display
} BROWSERENTRY;

typedef struct
{
	char filepath[MAXPATHLEN];
	char filename[MAXJOLIET + 1];
	u64  filesize;
	bool isdir;
	bool cutted;
} CLIPBOARD;

class FileBrowser : public Browser
{
    public:
        FileBrowser();
        ~FileBrowser();
        int BrowsePath(const char *path);
        int BrowseDevice(int device);
        void SetPageIndex(int ind);
        void SetSelectedIndex(int ind);
        int GetEntrieCount() { return browser.numEntries; };
        int GetPageIndex() { return browser.pageIndex; };
        bool IsDir(int ind) { return browserList[ind].isdir; };
        bool IsCurrentDir() { return browserList[browser.selIndex].isdir; };
        u64 GetFilesize(int ind) { return browserList[ind].length; };
        u64 GetCurrentFilesize() { return browserList[browser.selIndex].length; };
        const char * GetItemDisplayname(int ind) { return browserList[ind].displayname; };
        const char * GetItemFilename(int ind) { return browserList[ind].filename; };
        const char * GetCurrentFilename() { return browserList[browser.selIndex].filename; };
        const char * GetRootDir() { return browser.rootdir; };
        const char * GetDir() { return browser.dir; };
        const char * GetCurrentPath();
        const char * GetCurrentSelectedFilepath();
        void ResetBrowser();
        int UpdateDirName();
        int ParseDirectory();
        int BrowserChangeFolder();
    private:
        int EnterSelDir();
        int LeaveCurDir();
        void InitParseThread();
        void ShutdownParseThread();
        bool ParseDirEntries();
		static void * UpdateThread(void *arg);
		void InternalThreadUpdate();

		BROWSERINFO browser;
        BROWSERENTRY * browserList;
        // folder parsing thread
        lwp_t parsethread;
        bool parseHalt;
        bool exit_Requested;
        DIR_ITER *dirIter;
        char currentpath[MAXPATHLEN];
};

#endif
