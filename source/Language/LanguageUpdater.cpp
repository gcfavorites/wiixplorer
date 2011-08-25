/****************************************************************************
 * Copyright (C) 2010
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"
#include "network/networkops.h"
#include "network/URL_List.h"
#include "menu.h"

static const char * UpdateURL = "http://wiixplorer.googlecode.com/svn/trunk/Languages/";

int UpdateLanguageFiles()
{
	if(!NetworkInitPrompt())
		return -1;

	char * ptr = strrchr(Settings.LanguagePath, '/');
	if(ptr && strlen(Settings.LanguagePath) > 0 && Settings.LanguagePath[strlen(Settings.LanguagePath)-1] != '/')
		ptr[1] = '\0';

	if(!CreateSubfolder(Settings.LanguagePath))
	{
		ShowError("%s", tr("Cannot create directory: "), Settings.LanguagePath);
		return -1;
	}

	URL_List LinkList(UpdateURL);

	if(LinkList.GetURLCount() <= 0)
	{
		ShowError(tr("No files found."));
		return -1;
	}

	StartProgress(tr("Downloading files..."));
	u32 done = 0;
	u32 FilesDownloaded = 0;

	for (int i = 0; i < LinkList.GetURLCount(); i++)
	{
//		if(actioncanceled)
//			continue;

		ShowProgress(done, (LinkList.GetURLCount()-1)*16*1024, LinkList.GetURL(i));

		char * fileext = strrchr(LinkList.GetURL(i), '.');
		if(!fileext)
			continue;

		if (strcasecmp(fileext, ".lang") != 0)
			continue;

		char fullURL[MAXPATHLEN];
		if(LinkList.IsFullURL(i))
			snprintf(fullURL, sizeof(fullURL), "%s", LinkList.GetURL(i));
		else
			snprintf(fullURL, sizeof(fullURL), "%s%s", UpdateURL, LinkList.GetURL(i));

		struct block file = downloadfile(fullURL);

		if (file.data && file.size > 0)
		{
			snprintf(fullURL, sizeof(fullURL), "%s%s", Settings.LanguagePath, LinkList.GetURL(i));
			FILE * filePtr = fopen(fullURL, "wb");
			if(filePtr)
			{
				fwrite(file.data, 1, file.size, filePtr);
				fclose(filePtr);
				done += file.size;
				FilesDownloaded++;
			}
		}

		if(file.data)
			free(file.data);
	}
	StopProgress();

	return FilesDownloaded;
}
