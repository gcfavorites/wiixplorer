#ifndef __FILESTARTUP_H
#define __FILESTARTUP_H

enum
{
    BOOTHOMEBREW = 1,
    ARCHIVE,
    REFRESH_BROWSER,
    RELOAD_BROWSER
};

int FileStartUp(const char * filepath);

#endif
