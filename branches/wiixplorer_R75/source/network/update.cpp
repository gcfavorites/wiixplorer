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
#include "Language/txt.h"

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
int CheckForUpdate()
{
	if(!IsNetworkInit()) {
		NetworkInitPrompt();
	}

    const char url[80] = ("http://pagesperso-orange.fr/skual/WiiXplorer/UpdateDol/rev.txt");
    const char URL[70] = ("http://pagesperso-orange.fr/skual/WiiXplorer/UpdateDol/");
	char dol[80];
	char rev[strlen(Settings.UpdatePath)+10];
	
	snprintf(rev, sizeof(rev), "%srev.txt", Settings.UpdatePath);
	snprintf(dol, sizeof(dol), "%sboot.dol", URL);

	CreateSubfolder(Settings.UpdatePath);

	int res = DownloadFileToPath(url, rev);
	if(res < 0)
	{
		RemoveFile(rev);
		return 0;
	}
	else
	{
		TXT l;
		int open = l.openRevTxtfile(rev);
		if (open == 0) {
			WindowPrompt(tr("ERROR"), 0, tr("OK"));
			RemoveFile(rev);
			return 0;
		}
		else {
			int currentrev = atoi(SvnRev());
			int revnumber = atoi(l.getRev(0));
			
			if(currentrev >= revnumber) {
				RemoveFile(rev);
				return 0;
			}

			char text[200];
			sprintf(text, tr("Update to Rev%i available"), revnumber);
			int choice = WindowPrompt(text, tr("Do you want to update now ?"), tr("Yes"), tr("No"));
			if(choice)
				UpdateApp(dol);
			RemoveFile(rev);
		}
	}
	return 1;
}

//will reverted to the trunk r77 way when the new wiixplorer sources will be available

