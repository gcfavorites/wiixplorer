/****************************************************************************
 * FolderSizeThread
 * WiiXplorer 2009
 * by dimok
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fileops.h"

/*** Variables used only in this file ***/
static lwp_t foldersizethread = LWP_THREAD_NULL;
static char folderpath[1024];
static u64 foldersize = 0;
static u32 filecount = 0;
static bool running = false;

/****************************************************************************
 * GetCurrentFolderSize
 ***************************************************************************/
u64 GetCurrentFolderSize()
{
    return foldersize;
}
/****************************************************************************
 * GetFilenumber
 ***************************************************************************/
u32 GetFilecount()
{
    return filecount;
}

/****************************************************************************
 * GetSize
 ***************************************************************************/
static void GetSize()
{
    if(!running)
        return;

    GetFolderSize(folderpath, &foldersize, &filecount);

    running = false;
}

/****************************************************************************
 * FolderSizeThread
  ***************************************************************************/

static void * FolderSizeThread(void *arg)
{
	while(1)
	{
		if(!running)
			LWP_SuspendThread(foldersizethread);

        GetSize();

        usleep(100);
	}
	return NULL;
}
/****************************************************************************
 * StartGetFolderSizeThread
 *
 * Callbackfunction for updating the size value
 ***************************************************************************/
void StartGetFolderSizeThread(const char * path)
{
    strncpy(folderpath, path, sizeof(folderpath));
    foldersize = 0;
    filecount = 0;
    running = true;
    LWP_ResumeThread(foldersizethread);
}

/****************************************************************************
 * InitGetSizeThread
 *
 * Startup FolderSizeThread in idle prio
 ***************************************************************************/
void InitGetSizeThread() {
	LWP_CreateThread(&foldersizethread, FolderSizeThread, NULL, NULL, 0, 0);
}

/****************************************************************************
 * ExitGetSizeThread
 *
 * Shutdown FolderSizeThread
 ***************************************************************************/
void ExitGetSizeThread() {
	LWP_JoinThread(foldersizethread, NULL);
	foldersizethread = LWP_THREAD_NULL;
}
