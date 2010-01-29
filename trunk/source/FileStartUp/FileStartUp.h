#ifndef __FILESTARTUP_H
#define __FILESTARTUP_H

enum
{
    BOOTHOMEBREW = 1,
    ARCHIVE,
    TRIGGERUPDATE
};

int FileStartUp(const char * filepath);

#endif
