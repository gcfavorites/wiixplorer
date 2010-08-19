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
 * update.cpp
 *
 * Update operations
 * for WiiXplorer 2009
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>

#include "Prompts/PromptWindows.h"
#include "FileOperations/fileops.h"
#include "FileDownloader.h"
#include "http.h"
#include "ChangeLog.h"
#include "networkops.h"
#include "update.h"
#include "svnrev.h"
#include "main.h"
#include "sys.h"
#include "HTML_Stream.h"

/****************************************************************************
 * UpdateApp from a given url. The dol is downloaded and overwrites the old one.
 ***************************************************************************/
int UpdateApp(const char *url)
{
    if(!url)
    {
        ShowError(tr("URL is empty."));
        return -1;
    }

    char dest[strlen(Settings.UpdatePath)+10];

    snprintf(dest, sizeof(dest), "%sboot.tmp", Settings.UpdatePath);

    CreateSubfolder(Settings.UpdatePath);

    int res = DownloadFileToPath(url, dest, false);
    if(res < 102400)
    {
        RemoveFile(dest);
        WindowPrompt(tr("Update failed"), tr("Could not download file."), tr("OK"));
        return -1;
    }
    else
    {
        char realdest[strlen(dest)+1];
        snprintf(realdest, sizeof(realdest), "%sboot.dol", Settings.UpdatePath);
        RemoveFile(realdest);
        rename(dest, realdest);
        if(Settings.UpdateMetaxml)
            UpdateMetaXml();
        if(Settings.UpdateIconpng)
            UpdateIconPNG();
    }

    return 1;
}

/****************************************************************************
 * Checking if an Update is available
 ***************************************************************************/
int CheckForUpdate()
{
	if(!IsNetworkInit() && !NetworkInitPrompt())
		return -1;

    int revnumber = 0;
    int currentrev = atoi(SvnRev());

    HTML_Stream HTML("http://code.google.com/p/wiixplorer/downloads/list");

    const char * HTML_Pos = NULL;
    char DownloadLink[300];
    memset(DownloadLink, 0, sizeof(DownloadLink));

    while(1)
    {
        HTML_Pos = HTML.FindStringEnd("href='");
        if(!HTML_Pos)
            break;

        char * tmpLink = HTML.CopyString("'");
        if(!tmpLink)
            continue;

        char *fileext = strrchr(tmpLink, '.');
        if(fileext)
        {
            if(strcasecmp(fileext, ".dol") == 0)
            {
                char revtxt[80];
                char *filename = strrchr(tmpLink, '/')+2;
                u8 n = 0;
                for (n = 0; n < strlen(filename)-2; n++)
                    revtxt[n] = filename[n];
                revtxt[n] = 0;
                int fileRev = atoi(revtxt);

                if(fileRev > revnumber)
                {
                    revnumber = fileRev;
                    snprintf(DownloadLink, sizeof(DownloadLink), "%s", tmpLink);
                }
            }
        }
        free(tmpLink);
    }

    if (revnumber > currentrev)
    {
        char text[100];
        sprintf(text, tr("Update to Rev%i available"), revnumber);
        int choice = 1;
        while(choice)
        {
            choice = WindowPrompt(text, tr("Do you want to update now ?"), tr("Yes"), tr("Show Changelog"), tr("Cancel"));

            if(choice == 1)
            {
                if(UpdateApp(DownloadLink) >= 0)
                {
                    int ret = WindowPrompt(tr("Update successfully finished"), tr("Do you want to reboot now ?"), tr("Yes"), tr("No"));
                    if(ret)
                        RebootApp();
                }
                break;
            }
            else if(choice == 2)
            {
                ChangeLog Changelog;
                Changelog.DownloadChangeLog(revnumber-5, revnumber);
                if(!Changelog.Show())
                    WindowPrompt(tr("Failed to get the Changelog."), 0, tr("OK"));
            }
        }
    }
    else
    {
        revnumber = 0;
    }

    return revnumber;
}

/****************************************************************************
 * Update the Meta.xml file
 ***************************************************************************/
bool UpdateMetaXml()
{
	if(!IsNetworkInit())
        Initialize_Network();

    struct block file = downloadfile("http://wiixplorer.googlecode.com/svn/trunk/HBC/meta.xml");
    if(!file.data)
        return false;

    CreateSubfolder(Settings.UpdatePath);

    char path[MAXPATHLEN];
    snprintf(path, sizeof(path), "%smeta.xml", Settings.UpdatePath);
    FILE * pFile = fopen(path, "wb");
    if(!pFile)
        return false;

    fwrite(file.data, 1, file.size, pFile);
    fclose(pFile);
    free(file.data);

    return true;
}

/****************************************************************************
 * Update the Icon.png file
 ***************************************************************************/
bool UpdateIconPNG()
{
	if(!IsNetworkInit())
        Initialize_Network();

    struct block file = downloadfile("http://wiixplorer.googlecode.com/svn/trunk/HBC/icon.png");
    if(!file.data)
        return false;

    CreateSubfolder(Settings.UpdatePath);

    char path[MAXPATHLEN];
    snprintf(path, sizeof(path), "%sicon.png", Settings.UpdatePath);
    FILE * pFile = fopen(path, "wb");
    if(!pFile)
        return false;

    fwrite(file.data, 1, file.size, pFile);
    fclose(pFile);
    free(file.data);

    return true;
}
