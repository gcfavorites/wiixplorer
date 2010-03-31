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
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <malloc.h>

#include "filebrowser.h"
#include "Prompts/PromptWindows.h"
#include "menu.h"

/****************************************************************************
 * FileBrowser Class to parse directories on the fly
 ***************************************************************************/
FileBrowser::FileBrowser()
    :Browser()
{
    browserList = NULL;
    parsethread = LWP_THREAD_NULL;
    dirIter = NULL;
    parseHalt = true;
    exit_Requested = false;

    //!Reset and prepare browser
    ResetBrowser();
    //!Initialize Parsethread for browser
    InitParseThread();
}

/****************************************************************************
 * Destructor
 ***************************************************************************/
FileBrowser::~FileBrowser()
{
    ShutdownParseThread();
    dirclose(dirIter); // close directory
    dirIter = NULL;
    if(browserList != NULL)
	{
		free(browserList);
		browserList = NULL;
	}
}

/****************************************************************************
 * BrowsePath
 * Displays a list of files on the selected path
 ***************************************************************************/
int FileBrowser::BrowsePath(const char *path)
{
	int length = strlen(path)+1;

	char *device = strchr(path, ':');
	int position = device-path+1;


	if(!device || position < 0)
        return -1;

	snprintf(browser.rootdir, position+1, "%s", path);

    int n = 0;
	for(int i = position; i < length; i++)
	{
	    browser.dir[n] = path[i];
	    browser.dir[n+1] = '\0';
	    n++;
	}

	ParseDirectory(); // Parse root directory
	return browser.numEntries;
}

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected device
 ***************************************************************************/
int FileBrowser::BrowseDevice(int device)
{
    if(device < 0 || device >= MAXDEVICES)
        return -1;

	strcpy(browser.dir, "");
	sprintf(browser.rootdir, "%s:/", DeviceName[device]);

	ParseDirectory(); // Parse root directory
	return browser.numEntries;
}

/****************************************************************************
 * Enter the current selected directory
 ***************************************************************************/
int FileBrowser::EnterSelDir()
{
    int dirlength = strlen(browser.dir);
    int filelength = strlen(browserList[browser.selIndex].filename);
    if((dirlength+filelength+1) > MAXPATHLEN)
        return -1;

    if(dirlength == 0)
        sprintf(browser.dir, "%s", browserList[browser.selIndex].filename);
    else
        sprintf(browser.dir, "%s/%s", browser.dir, browserList[browser.selIndex].filename);

    return 1;
}

/****************************************************************************
 * Leave the current directory
 ***************************************************************************/
int FileBrowser::LeaveCurDir()
{
    char * ptr = strrchr(browser.dir, '/');
    if(ptr)
    {
        ptr[0] = '\0';
        return 1;
    }
    else
    {
        if(strlen(browser.dir) != 0)
        {
            strcpy(browser.dir, "");
            return 1;
        }
        else
        {
            //There is no upper directory
            return 1;
        }
    }
}

/****************************************************************************
 * UpdateDirName()
 * Update curent directory name for file browser
 ***************************************************************************/
int FileBrowser::UpdateDirName()
{
    if(strcmp(browserList[browser.selIndex].filename, "..") == 0)
    {
        return LeaveCurDir();
    }

    return EnterSelDir();
}

/****************************************************************************
 * SetPageIndex
 * not inline for later mutex purpose
 ***************************************************************************/
void FileBrowser::SetPageIndex(int ind)
{
    browser.pageIndex = ind;
}
/****************************************************************************
 * SetSelectedIndex
 * not inline for later mutex purpose
 ***************************************************************************/
void FileBrowser::SetSelectedIndex(int ind)
{
    browser.selIndex = ind;
}
/****************************************************************************
 * Get the current full path
 ***************************************************************************/
const char * FileBrowser::GetCurrentPath()
{
    snprintf(currentpath, sizeof(currentpath), "%s%s", browser.rootdir, browser.dir);

    return currentpath;
}
/****************************************************************************
 * Get the current full path with filename
 ***************************************************************************/
const char * FileBrowser::GetCurrentSelectedFilepath()
{
    snprintf(currentpath, sizeof(currentpath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);

    return currentpath;
}

/****************************************************************************
 * Get the current item structure
 ***************************************************************************/
ItemStruct FileBrowser::GetCurrentItemStruct() const
{
    ItemStruct Item;
    memset(&Item, 0, sizeof(ItemStruct));

    snprintf(Item.itempath, sizeof(Item.itempath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
    Item.itemsize = browserList[browser.selIndex].length;
    Item.isdir = browserList[browser.selIndex].isdir;
    Item.itemindex = browser.selIndex;

    return Item;
}

/****************************************************************************
 * ResetBrowser()
 * Clears the file browser memory, and allocates one initial entry
 ***************************************************************************/
void FileBrowser::ResetBrowser()
{
	browser.numEntries = 0;
    browser.selIndex = 0;
    browser.pageIndex = 0;

	// Clear any existing values
	if(browserList != NULL)
	{
		free(browserList);
		browserList = NULL;
	}
	// set aside space for 1 entry
	browserList = (BROWSERENTRY *)malloc(sizeof(BROWSERENTRY));
	memset(browserList, 0, sizeof(BROWSERENTRY));
}

/****************************************************************************
 * FileSortCallback
 *
 * Quick sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/
static int FileSortCallback(const void *f1, const void *f2)
{
	/* Special case for implicit directories */
	if(((BROWSERENTRY *)f1)->filename[0] == '.' || ((BROWSERENTRY *)f2)->filename[0] == '.')
	{
		if(strcmp(((BROWSERENTRY *)f1)->filename, ".") == 0) { return -1; }
		if(strcmp(((BROWSERENTRY *)f2)->filename, ".") == 0) { return 1; }
		if(strcmp(((BROWSERENTRY *)f1)->filename, "..") == 0) { return -1; }
		if(strcmp(((BROWSERENTRY *)f2)->filename, "..") == 0) { return 1; }
	}

	/* If one is a file and one is a directory the directory is first. */
	if(((BROWSERENTRY *)f1)->isdir && !(((BROWSERENTRY *)f2)->isdir)) return -1;
	if(!(((BROWSERENTRY *)f1)->isdir) && ((BROWSERENTRY *)f2)->isdir) return 1;

	return stricmp(((BROWSERENTRY *)f1)->filename, ((BROWSERENTRY *)f2)->filename);
}

/****************************************************************************
 * ParseDirEntries
 *
 * Update current directory and set new entry list and entrynum
 ***************************************************************************/
bool FileBrowser::ParseDirEntries()
{
	if(!dirIter)
		return false;

	char filename[MAXPATHLEN];
	struct stat filestat;

	int i, res;

	for(i=0; i < 20; i++)
	{
		res = dirnext(dirIter,filename,&filestat);

		if(res != 0)
			break;

		if(strcmp(filename,".") == 0)
		{
			i--;
			continue;
		}

		BROWSERENTRY * newBrowserList = (BROWSERENTRY *)realloc(browserList, (browser.numEntries+i+1) * sizeof(BROWSERENTRY));

		if(!newBrowserList) // failed to allocate required memory
		{
			ResetBrowser();
			WindowPrompt(tr("Out of memory: too many files!"), 0 , tr("OK"));
			break;
		}
		else
		{
			browserList = newBrowserList;
		}

		memset(&(browserList[browser.numEntries+i]), 0, sizeof(BROWSERENTRY)); // clear the new entry

		strncpy(browserList[browser.numEntries+i].filename, filename, MAXJOLIET);
		browserList[browser.numEntries+i].length = filestat.st_size;
		browserList[browser.numEntries+i].isdir = (filestat.st_mode & S_IFDIR) ? true : false; // flag this as a dir

		if(browserList[browser.numEntries+i].isdir)
		{
			if(strcmp(filename, "..") == 0)
				sprintf(browserList[browser.numEntries+i].displayname, "..");
			else
				strncpy(browserList[browser.numEntries+i].displayname, browserList[browser.numEntries+i].filename, MAXJOLIET);
		}
		else
		{
		    strncpy(browserList[browser.numEntries+i].displayname, browserList[browser.numEntries+i].filename, MAXJOLIET);
        }
	}

	// Sort the file list
	if(i >= 0)
	{
		browser.numEntries += i;
		qsort(browserList, browser.numEntries, sizeof(BROWSERENTRY), FileSortCallback);
	}

	if(res != 0 || parseHalt)
	{
		dirclose(dirIter); // close directory
		dirIter = NULL;
		return false; // no more entries
	}
	return true; // more entries
}

/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
void FileBrowser::Refresh()
{
    ParseDirectory(false);
}

/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
int FileBrowser::ParseDirectory(bool ResetPosition)
{
	char fulldir[MAXPATHLEN];

	// halt parsing
	parseHalt = true;

	while(!LWP_ThreadIsSuspended(parsethread))
		usleep(THREAD_SLEEP);

	// reset browser
	if(dirIter)
	{
	    dirclose(dirIter);
        dirIter = NULL;
	}

    u32 currPageIndex = browser.pageIndex;
    u32 currSelIndex = browser.selIndex;

	ResetBrowser();

	if(!ResetPosition)
	{
        browser.pageIndex = currPageIndex;
        browser.selIndex = currSelIndex;
	}

	// open the directory
	sprintf(fulldir, "%s%s", browser.rootdir, browser.dir); // add device to path
	dirIter = diropen(fulldir);
	if(dirIter == NULL)
	{
	    // if we can't open the dir, try opening the root dir
        strcpy(browser.dir, "");
        sprintf(fulldir, "%s%s", browser.rootdir, browser.dir);
        dirIter = diropen(fulldir);

        if(dirIter == NULL)
            return -1;
	}

	parseHalt = false;
	ParseDirEntries(); // index first 20 entries
	LWP_ResumeThread(parsethread);

	return browser.numEntries;
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
int FileBrowser::BrowserChangeFolder()
{
	if(!UpdateDirName())
		return -1;

	ParseDirectory();

	return browser.numEntries;
}

/****************************************************************************
 * ParseThread callback function
 ***************************************************************************/
void * FileBrowser::UpdateThread(void *arg)
{
	((FileBrowser *) arg)->InternalThreadUpdate();
	return NULL;
}

/****************************************************************************
 * InternalThreadUpdate
 ***************************************************************************/
void FileBrowser::InternalThreadUpdate()
{
	while(!exit_Requested)
	{
		while(ParseDirEntries())
			usleep(THREAD_SLEEP);

		LWP_SuspendThread(parsethread);
	}
}

/****************************************************************************
 * InitParseThread
 ***************************************************************************/
void FileBrowser::InitParseThread()
{
	LWP_CreateThread(&parsethread, UpdateThread, this, NULL, 0, 60);
}

/****************************************************************************
 * InitParseThread
 ***************************************************************************/
void FileBrowser::ShutdownParseThread()
{
    if(parsethread != LWP_THREAD_NULL)
	{
        exit_Requested = true;
        LWP_ResumeThread(parsethread);
        LWP_JoinThread(parsethread, NULL);
        parsethread = LWP_THREAD_NULL;
	}
}
