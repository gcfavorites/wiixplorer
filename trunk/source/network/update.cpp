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
 * for Wii-Xplorer 2009
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "Language/gettext.h"
#include "Prompts/PromptWindows.h"
#include "http.h"
#include "networkops.h"
#include "svnrev.h"
#include "main.h"
#include "URL_List.h"
#include "FileOperations/fileops.h"

/****************************************************************************
 * UpdateApp from a given url. The dol is downloaded and overwrites the old one.
 ***************************************************************************/
int UpdateApp(const char *url)
{
    if(!url)
    {
        WindowPrompt(tr("ERROR"), tr("URL is empty."), tr("OK"));
        return -1;
    }

    char dest[strlen(Settings.UpdatePath)+10];

    snprintf(dest, sizeof(dest), "%sboot.tmp", Settings.UpdatePath);

    CreateSubfolder(Settings.UpdatePath);

    int res = DownloadFileToPath(url, dest);
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
        WindowPrompt(tr("Update successfully finished"), tr("It is recommended to restart app now."), tr("OK"));
    }

    return 1;
}

/****************************************************************************
 * Checking if an Update is available
 ***************************************************************************/
int CheckForUpdate()
{
    if (!IsNetworkInit())
    {
        WindowPrompt(tr("No network connection"), 0, tr("OK"));
        return -1;
    }

    int revnumber = 0;
    int currentrev = atoi(SvnRev());

    struct block file = downloadfile("http://code.google.com/p/wiixplorer/downloads/list");

    u32 cnt = 0;

    while(cnt < file.size)
    {
        if(htmlstringcompare(file.data, "Revision ", cnt) == 0)
        {
            char temp[MAXPATHLEN];
            copyhtmlsting((const char *) file.data, temp, ", ", cnt);

            int temprev = atoi(temp);

            if(temprev > revnumber)
                revnumber = temprev;
        }
        cnt++;
    }

    free(file.data);

    if (revnumber > currentrev)
    {
        char text[200];
        sprintf(text, tr("Update to Rev%i available"), revnumber);
        int choice = WindowPrompt(text, tr("Do you want to update now ?"), tr("Yes"), tr("No"));
        if(choice)
            UpdateApp("http://wiixplorer.googlecode.com/files/boot.dol");
    }
    else
        return 0;

    return revnumber;
}
