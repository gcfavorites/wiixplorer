#include <gctypes.h>
#include <string.h>

#include "FileStartUp.h"

int FileStartUp(const char *filepath)
{
    char *fileext = strrchr(filepath, '.');

    if(strcasecmp(fileext, ".dol") == 0 || strcasecmp(fileext, ".elf") == 0) {
        return BOOTHOMEBREW;
    }
    else if(strcasecmp(fileext, ".png") == 0) {
        //TODO
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
        //TODO
    }

    return 0;

}
