/****************************************************************************
 * PromptWindows
 * Wiixplorer 2009
 * dimok
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _PROMPTWINDOWS_H_
#define _PROMPTWINDOWS_H_

enum
{
    COPYFILE,
    COPYDIR,
    DELETEFILE,
    DELETEDIR
};

enum
{
    PROGRESSBAR,
    THROBBER
};

void ShowProgress(u32 done, u32 total, char * filename, int progressmode = PROGRESSBAR);
int ProgressWindow(const char *title, char * source, char *dest, int process, int mode = PROGRESSBAR);
void OnScreenKeyboard(char * var, u16 maxlen);
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, const char *btn3Label, const char *btn4Label);

#endif
