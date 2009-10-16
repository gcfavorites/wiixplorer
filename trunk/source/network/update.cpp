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
#include "Prompts/ProgressWindow.h"
#include "http.h"
#include "networkops.h"
#include "svnrev.h"
#include "main.h"
#include "URL_List.h"
#include "fileops.h"

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
    if(res < 0)
    {
        RemoveFile(dest);
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
int choice = 0;

int CheckForUpdate()
{
	if(!IsNetworkInit()) {
		choice = WindowPrompt(tr("No network connection"),tr("Do you want to connect?"),tr("Yes"),tr("Cancel"));
		if(choice == 1) {
			StartProgress(tr("Connecting..."), THROBBER);
			Initialize_Network();
			StopProgress();
			if (!IsNetworkInit()){
				WindowPrompt(tr("No network connection"), 0, tr("OK"));
			}
		}
		if (!IsNetworkInit()){
			return -1;
		}
	}

    int revnumber = 0;
    int currentrev = atoi(SvnRev());

    URL_List URLs("http://code.google.com/p/wiixplorer/downloads/list");

    int urlcount = URLs.GetURLCount();

    char *DownloadLink = NULL;

    for(int i = 0; i < urlcount; i++)
    {
        char *tmp = URLs.GetURL(i);
        if(tmp)
        {
            char *fileext = strrchr(tmp, '.');
            if(fileext)
            {
                if(strcasecmp(fileext, ".dol") == 0)
                {
                    DownloadLink = (char *) malloc(strlen(tmp)+1);
                    sprintf(DownloadLink, "%s", tmp);

                    char revtxt[80];
                    char *filename = strrchr(DownloadLink, '/')+2;
                    u8 n = 0;
                    for (n = 0; n < strlen(filename)-2; n++)
                        revtxt[n] = filename[n];
                    revtxt[n] = 0;
                    revnumber = atoi(revtxt);

                    if(revnumber > currentrev)
                        break;
                    else
                    {
                        if(DownloadLink)
                            free(DownloadLink);
                        DownloadLink = NULL;
                        revnumber = 0;
                    }
                }
            }
        }
    }

    if (revnumber > currentrev)
    {
        char text[200];
        sprintf(text, tr("Update to Rev%i available"), revnumber);
        int choice = WindowPrompt(text, tr("Do you want to update now ?"), tr("Yes"), tr("No"));
        if(choice)
            UpdateApp(DownloadLink);
    }

    if(DownloadLink)
        free(DownloadLink);
    DownloadLink = NULL;

    return revnumber;
}
