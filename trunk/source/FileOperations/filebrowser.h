/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.h
 *
 * Generic file routines - reading, writing, browsing
 ****************************************************************************/

#ifndef _FILEBROWSER_H_
#define _FILEBROWSER_H_

#include <unistd.h>
#include <gccore.h>
#include <sys/dir.h>

#define MAXJOLIET 255
#define MAXDISPLAY MAXPATHLEN

#include "Prompts/DeviceMenu.h"

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
	char isdir; // 0 - file, 1 - directory
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


class FileBrowser
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
