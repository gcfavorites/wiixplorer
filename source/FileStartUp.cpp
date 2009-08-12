#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "Prompts/PromptWindows.h"
#include "FileStartUp.h"
#include "TextViewer.h"
#include "ImageViewer.h"

int FileStartUp(const char *filepath)
{
    char *fileext = strrchr(filepath, '.');
	char *filename = strrchr(filepath, '/')+1;

    if(strcasecmp(fileext, ".dol") == 0 || strcasecmp(fileext, ".elf") == 0) {
        return BOOTHOMEBREW;
    }
    else if(strcasecmp(fileext, ".png") == 0 || strcasecmp(fileext, ".jpg") == 0) {
 // || strcasecmp(fileext, ".bmp") == 0 || strcasecmp(fileext, ".gif") == 0) {
		int choice = WindowPrompt(filename, "Do you want to open this file with ImageViewer?", "Yes", "No");
		if (choice)
			ImageViewer(filepath);
    }
    else if(strcasecmp(fileext, ".mp3") == 0) {
        //TODO
    }
    else if(strcasecmp(fileext, ".ogg") == 0) {
        //TODO
    }
    else if(strcasecmp(fileext, ".zip") == 0) {
        //TODO
    }
    else if(strcasecmp(fileext, ".rar") == 0) {
        //TODO
    }
    else {
        int choice = WindowPrompt(filename, "Do you want to open this file in TextViewer?", "Yes", "No");
        if(choice)
            TextViewer(filepath);
    }

    return 0;
}
