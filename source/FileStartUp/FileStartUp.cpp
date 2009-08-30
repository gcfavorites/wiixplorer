#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "Prompts/PromptWindows.h"
#include "FileStartUp/FileStartUp.h"
#include "FileStartUp/TextViewer.h"
#include "FileStartUp/ImageViewer.h"
#include "FileStartUp/MusicLoader.h"
#include "gettext.h"

int FileStartUp(const char *filepath)
{
    char *fileext = strrchr(filepath, '.');
	char *filename = strrchr(filepath, '/')+1;

    if(strcasecmp(fileext, ".dol") == 0 || strcasecmp(fileext, ".elf") == 0) {
        return BOOTHOMEBREW;
    }
    else if(strcasecmp(fileext, ".png") == 0 || strcasecmp(fileext, ".jpg") == 0
			/*|| strcasecmp(fileext, ".bmp") == 0*/ || strcasecmp(fileext, ".gif") == 0) {
		int choice = WindowPrompt(filename, tr("Do you want to open this file with ImageViewer?"), tr("Yes"), tr("No"));
		if (choice)
			ImageViewer(filepath);
    }
    else if(strcasecmp(fileext, ".ogg") == 0 || strcasecmp(fileext, ".mp3") == 0) {
        int choice = WindowPrompt(filename, tr("Do you want to playback this file?"), tr("Yes"), tr("No"));
        if(choice)
            LoadMusic(filepath);
    }
    else if(strcasecmp(fileext, ".zip") == 0) {
        //TODO
    }
    else if(strcasecmp(fileext, ".rar") == 0) {
        //TODO
    }
    else {
        int choice = WindowPrompt(filename, tr("Do you want to open this file in TextViewer?"), tr("Yes"), tr("No"));
        if(choice)
            TextViewer(filepath);
    }

    return 0;
}
