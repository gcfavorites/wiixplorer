#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>

#include "Prompts/PromptWindows.h"
#include "BootHomebrew/BootHomebrew.h"
#include "FileStartUp/FileStartUp.h"
#include "TextOperations/TextViewer.h"
#include "FileStartUp/ImageViewer.h"
#include "FileStartUp/MusicLoader.h"
#include "FileStartUp/ZipBrowser.h"
#include "Language/gettext.h"
#include "FileOperations/fileops.h"

int FileStartUp(const char *filepath)
{
    char *fileext = strrchr(filepath, '.');
	char *filename = strrchr(filepath, '/')+1;

	if(!fileext)
        goto loadtext;      //!to avoid crash with strcasecmp & fileext == NULL

    if(strcasecmp(fileext, ".dol") == 0 || strcasecmp(fileext, ".elf") == 0) {
        int choice = WindowPrompt(tr("Do you want to boot:"), filename, tr("Yes"), tr("No"));
        if(choice)
        {
             u8 *buffer = NULL;
             u64 filesize = 0;
             int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &buffer, &filesize);
             if(ret < 0)
                 return 0;
             ret = CopyHomebrewMemory(buffer, 0, filesize);
             if(buffer) {
                 free(buffer);
                 buffer = NULL;
             }
             if(ret >= 0)
                 return BOOTHOMEBREW;
        }

        return 0;
    }
    else if(strcasecmp(fileext, ".png") == 0 || strcasecmp(fileext, ".jpg") == 0
			|| strcasecmp(fileext, ".bmp") == 0 || strcasecmp(fileext, ".gif") == 0
			|| strcasecmp(fileext, ".tga") == 0) {
		int choice = WindowPrompt(filename, tr("Do you want to open this file with ImageViewer?"), tr("Yes"), tr("No"));
		if (choice)
			ImageViewer(filepath);
    }
    else if(strcasecmp(fileext, ".ogg") == 0 || strcasecmp(fileext, ".mp3") == 0) {
        int choice = WindowPrompt(filename, tr("Do you want to playback this file?"), tr("Yes"), tr("No"));
        if(choice)
            LoadMusic(filepath);
    }
    else if(strcasecmp(fileext, ".zip") == 0)
    {
        int choice = WindowPrompt(filename, tr("Enter a directory where to extract the files to."), tr("OK"), tr("Cancel"));
        if(choice) {
            char entered[151];
            bool result = false;
            int position = fileext-filepath;
            snprintf(entered, position+1, "%s", filepath);
            if(OnScreenKeyboard(entered, 150))
                result = ZipBrowse(filepath, entered);

            return TRIGGERUPDATE;
        }
    }
    else if(strcasecmp(fileext, ".rar") == 0) {
        //TODO
    }
    else {
        loadtext:

        int choice = WindowPrompt(filename, tr("Do you want to open this file in TextViewer?"), tr("Yes"), tr("No"));
        if(choice)
            TextViewer(filepath);
    }

    return 0;
}
