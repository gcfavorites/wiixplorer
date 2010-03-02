/****************************************************************************
 * ProgressWindow
 * WiiXplorer 2009
 * by dimok
 *
 * ProgressWindow.h
 ***************************************************************************/

#ifndef _PROGRESSWINDOW_H_
#define _PROGRESSWINDOW_H_

enum
{
    COPYFILE,
    COPYDIR,
    MOVEDIR,
    DELETEFILE,
    DELETEDIR
};

enum
{
    PROGRESSBAR = 1,
    THROBBER = 2
};

void InitProgressThread();
void ExitProgressThread();
void ShowProgress(u64 done, u64 total, const char *filename);
void StartProgress(const char *title, int display = PROGRESSBAR);
void StopProgress();
const char * GetProgressTitle();

#endif
