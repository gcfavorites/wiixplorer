/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.cpp
 *
 * Generic file routines - reading, writing, browsing
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <malloc.h>

#include "filebrowser.h"
#include "Prompts/PromptWindows.h"
#include "Language/gettext.h"
#include "menu.h"

/****************************************************************************
 * FileBrowser Class to parse directories on the fly
 ***************************************************************************/
FileBrowser::FileBrowser()
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
	sprintf(browser.dir, "/");
	switch(device)
	{
	    case SD:
            sprintf(browser.rootdir, "sd:");
            break;
	    case USB:
            sprintf(browser.rootdir, "usb:");
            break;
	    case NTFS0:
            sprintf(browser.rootdir, "ntfs0:");
            break;
	    case NTFS1:
            sprintf(browser.rootdir, "ntfs1:");
            break;
	    case NTFS2:
            sprintf(browser.rootdir, "ntfs2:");
            break;
	    case NTFS3:
            sprintf(browser.rootdir, "ntfs3:");
            break;
	    case NTFS4:
            sprintf(browser.rootdir, "ntfs4:");
            break;
	    case SMB1:
            sprintf(browser.rootdir, "smb1:");
            break;
	    case SMB2:
            sprintf(browser.rootdir, "smb2:");
            break;
	    case SMB3:
            sprintf(browser.rootdir, "smb3:");
            break;
	    case SMB4:
            sprintf(browser.rootdir, "smb4:");
            break;
//		case ISFS:
//			sprintf(browser.rootdir, "isfs:");
//			break;
//		case NAND:
//			sprintf(browser.rootdir, "nand:");
//			break;
	}
	ParseDirectory(); // Parse root directory
	return browser.numEntries;
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
 * UpdateDirName()
 * Update curent directory name for file browser
 ***************************************************************************/
int FileBrowser::UpdateDirName()
{
	int size=0;
	char * test;
	char temp[1024];

	/* current directory doesn't change */
	if (strcmp(browserList[browser.selIndex].filename,".") == 0)
	{
		return 0;
	}
	/* go up to parent directory */
	else if (strcmp(browserList[browser.selIndex].filename,"..") == 0)
	{
		/* determine last subdirectory namelength */
		sprintf(temp,"%s",browser.dir);
		test = strtok(temp,"/");
		while (test != NULL)
		{
			size = strlen(test);
			test = strtok(NULL,"/");
		}

		/* remove last subdirectory name */
		size = strlen(browser.dir) - size - 1;
		browser.dir[size] = 0;

		return 1;
	}
	/* Open a directory */
	else
	{
		/* test new directory namelength */
		if ((strlen(browser.dir)+1+strlen(browserList[browser.selIndex].filename)) < MAXPATHLEN)
		{
			/* update current directory name */
			sprintf(browser.dir, "%s/%s",browser.dir, browserList[browser.selIndex].filename);
			return 1;
		}
		else
		{
			return -1;
		}
	}
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
		browserList[browser.numEntries+i].isdir = (filestat.st_mode & _IFDIR) == 0 ? 0 : 1; // flag this as a dir

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
int FileBrowser::ParseDirectory()
{
	char fulldir[MAXPATHLEN];

	// halt parsing
	parseHalt = true;

	while(!LWP_ThreadIsSuspended(parsethread))
		usleep(THREAD_SLEEP);

	// reset browser
	dirIter = NULL;
	ResetBrowser();

	// open the directory
	sprintf(fulldir, "%s%s", browser.rootdir, browser.dir); // add device to path
	dirIter = diropen(fulldir);
	if(dirIter == NULL)
	{
	    // if we can't open the dir, try opening the root dir
        sprintf(browser.dir,"/");
        sprintf(fulldir, "%s%s", browser.rootdir, browser.dir);
        dirIter = diropen(fulldir);

        if(dirIter == NULL)
            return -1;
	}

	parseHalt = false;
	ParseDirEntries(); // index first 20 entries
	LWP_ResumeThread(parsethread); // index remaining entries

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
