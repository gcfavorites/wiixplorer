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
    CUT,
    COPY,
    PASTE,
    RENAME,
    DELETE,
    NEWFOLDER,
    PROPERTIES
};

int OnScreenKeyboard(char * var, u16 maxlen);
int WindowPrompt(const char *title, const char *msg = NULL, const char *btn1Label = NULL,
                    const char *btn2Label = NULL, const char *btn3Label = NULL,
                    const char *btn4Label = NULL);
int RightClickMenu(int x, int y);
int Properties(const char * filename, const char * filepath, int folder, float filesize = 0.0);

#endif
