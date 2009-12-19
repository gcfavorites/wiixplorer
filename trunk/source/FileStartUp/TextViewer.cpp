/***************************************************************************
 * Copyright (C) 2009
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
 * TextViewer.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "libwiigui/gui.h"
#include "Controls/MainWindow.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "menu.h"
#include "fileops.h"
#include "Language/gettext.h"
#include "sys.h"
#include "TextEditor.h"

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();


/****************************************************************************
* TextViewer (temporary menu)
***************************************************************************/
void TextViewer(const char *filepath)
{
    bool exitwindow = false;

    u8 *file = NULL;
    u64 filesize = 0;

    StartProgress(tr("Loading file:"));
    int ret = LoadFileToMem(filepath, &file, &filesize);
    StopProgress();

    if(ret == -1) {
        WindowPrompt(tr("Error"), tr("Can not open the file"), tr("OK"));
        return;
    }
    else if(ret == -2) {
        WindowPrompt(tr("Error"), tr("Not enough memory."), tr("OK"));
        return;
    }
    else if(ret == -3) {
        WindowPrompt(tr("Error"), tr("Can not open the file"), tr("OK"));
        return;
    }
    else if(ret == -10) {
        WindowPrompt(tr("Loading file:"), tr("Action cancelled."), tr("OK"));
        return;
    }

    //To check if enough memory available for strcpy in GuiText
    char *filetext = new char[filesize];
    if(!filetext) {
        WindowPrompt(tr("Error"), tr("Not enough memory."), tr("OK"));
        return;
    }

    snprintf(filetext, filesize, "%s", (char *) file);

    free(file);
    file = NULL;

    char *filename = strrchr(filepath, '/')+1;

    TextEditor * Editor = new TextEditor(filetext, 9, filename);
    Editor->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    Editor->SetPosition(0, 0);

    delete [] filetext;
    filetext = NULL;

    MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->Append(Editor);
    MainWindow::Instance()->ChangeFocus(Editor);

    while(!exitwindow)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();

        else if(Editor->GetState() == STATE_CLOSED)
            exitwindow = true;
    }

    delete Editor;
    Editor = NULL;

    MainWindow::Instance()->SetState(STATE_DEFAULT);
    ResumeGui();
}
