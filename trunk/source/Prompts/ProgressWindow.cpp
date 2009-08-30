 /****************************************************************************
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
 * ProgressWindow
 * Wii-FileXplorer 2009
 *
 * ProgressWindow.cpp
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libwiigui/gui.h"
#include "Prompts/ProgressWindow.h"
#include "filebrowser.h"
#include "menu.h"
#include "fileops.h"
#include "gettext.h"
#include "sys.h"

/*** Variables used only in this file ***/
static lwp_t    progressthread = LWP_THREAD_NULL;
static char     progressTitle[100];
static char     progressMsg[200];
static char     progressSizeLeft[80];
static u32      progressSpeed = 0;
static short    showProgress = 0;
static u64      progressDone = 0;
static u64      progressTotal = 100;
static time_t   start = 0;

/*** Variables used outside of this file ***/
bool            actioncanceled = false;

/*** Extern variables ***/
extern GuiWindow *mainWindow;
extern u8 reset;
extern u8 shutdown;

/*** Extern functions ***/
extern void     ResumeGui();
extern void     HaltGui();

/****************************************************************************
 * UpdateProgressValues
 *
 * Update the time, size and speed values of the current progress
 ***************************************************************************/
void UpdateProgressValues()
{
    if(showProgress != PROGRESSBAR)
        return;

    //Elapsed time
    u32 elapsed = time(0) - start;
    if(elapsed < 1)
        elapsed = 1;

    //Calculate speed in KB/s
    progressSpeed = 0;
    if(progressDone)
        progressSpeed = progressDone/(elapsed * KBSIZE);

    if(progressTotal > KBSIZE && progressTotal < MBSIZE)
        snprintf(progressSizeLeft, sizeof(progressSizeLeft), "%0.2fKB/%0.2fKB", progressDone/KBSIZE, progressTotal/KBSIZE);
    else if(progressTotal > MBSIZE && progressTotal < GBSIZE)
        snprintf(progressSizeLeft, sizeof(progressSizeLeft), "%0.2fMB/%0.2fMB", progressDone/MBSIZE, progressTotal/MBSIZE);
    else if(progressTotal > GBSIZE)
        snprintf(progressSizeLeft, sizeof(progressSizeLeft), "%0.2fGB/%0.2fGB", progressDone/GBSIZE, progressTotal/GBSIZE);
    else
        snprintf(progressSizeLeft, sizeof(progressSizeLeft), "%LiB/%LiB", progressDone, progressTotal);
}

/****************************************************************************
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
void ProgressWindow()
{
	if(!showProgress)
        return;

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData progressbarOutline(progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 15);

	GuiImageData progressbarEmpty(progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 15);
	progressbarEmptyImg.SetTile(100);

    GuiImageData progressbar(progressbar_png);
    GuiImage progressbarImg(&progressbar);
    progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 15);

    GuiImageData throbber(throbber_png);
    GuiImage throbberImg(&throbber);
	throbberImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	throbberImg.SetPosition(0, 25);

	GuiText titleTxt(progressTitle, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,50);
	titleTxt.SetMaxWidth(430, DOTTED);

    GuiText msgTxt(NULL, 22, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,110);
	msgTxt.SetMaxWidth(430, DOTTED);

    GuiText prTxt(NULL, 26, (GXColor){0, 0, 0, 255});
	prTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	prTxt.SetPosition(200, 15);

	GuiText prsTxt("%", 26, (GXColor){0, 0, 0, 255});
	prsTxt.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	prsTxt.SetPosition(-188, 15);

    GuiText speedTxt(NULL, 24, (GXColor){0, 0, 0, 255});
    speedTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	speedTxt.SetPosition(350, 205);

    GuiText sizeTxt(NULL, 24, (GXColor){0, 0, 0, 255});
    sizeTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	sizeTxt.SetPosition(50, 205);

    GuiImageData btnOutline(small_button_png);
	GuiImage buttonImg(&btnOutline);
    GuiText AbortTxt(tr("Cancel"), 22, (GXColor){0, 0, 0, 255});
	GuiButton AbortBtn(buttonImg.GetWidth(), buttonImg.GetHeight());
	AbortBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	AbortBtn.SetPosition(0, -48);
	AbortBtn.SetLabel(&AbortTxt);
	AbortBtn.SetImage(&buttonImg);
	AbortBtn.SetTrigger(&trigA);
	AbortBtn.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
    promptWindow.Append(&AbortBtn);
    if(showProgress == PROGRESSBAR) {
        promptWindow.Append(&progressbarEmptyImg);
        promptWindow.Append(&progressbarImg);
        promptWindow.Append(&progressbarOutlineImg);
        promptWindow.Append(&prTxt);
        promptWindow.Append(&prsTxt);
        promptWindow.Append(&speedTxt);
        promptWindow.Append(&sizeTxt);
	} else {
	    promptWindow.Append(&throbberImg);
	}

	HaltGui();
	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

    while(promptWindow.GetEffect() > 0) usleep(100);

    int angle = 0;

	while(showProgress)
	{
	    VIDEO_WaitVSync();

	    UpdateProgressValues();

        msgTxt.SetText(progressMsg);

        if(showProgress == PROGRESSBAR) {
            progressbarImg.SetTile(100*progressDone/progressTotal);
            prTxt.SetTextf("%0.2f", (float) (100.0f*progressDone/progressTotal));
            speedTxt.SetTextf("%iKB/s", progressSpeed);
            sizeTxt.SetTextf("%s", progressSizeLeft);
        } else {
            angle += 5;
            if(angle > 360)
                angle = 0 + (angle - 360);
            throbberImg.SetAngle(angle);
        }

        if(shutdown == 1) {
            actioncanceled = true;
            sleep(1);
            Sys_Shutdown();
        }
        else if(reset == 1) {
            actioncanceled = true;
            sleep(1);
            Sys_Reboot();
        }

        else if(AbortBtn.GetState() == STATE_CLICKED) {
            actioncanceled = true;
            AbortBtn.ResetState();
        }
	}

	promptWindow.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(100);

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	actioncanceled = false;
}
/****************************************************************************
 * ProgressThread
  ***************************************************************************/

static void * ProgressThread(void *arg)
{
	while(1)
	{
		if(!showProgress)
			LWP_SuspendThread(progressthread);

        ProgressWindow();

        usleep(THREAD_SLEEP);
	}
	return NULL;
}

/****************************************************************************
 * StopProgress
 ***************************************************************************/
void StopProgress()
{
	showProgress = 0;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(progressthread))
		usleep(THREAD_SLEEP);
}

/****************************************************************************
 * StartProgress
 ***************************************************************************/
void StartProgress(const char *title, int progressmode)
{
    strncpy(progressTitle, title, sizeof(progressTitle));

	showProgress = progressmode;

	LWP_ResumeThread(progressthread);
}

/****************************************************************************
 * ShowProgress
 *
 * Callbackfunction for updating the progress values
 ***************************************************************************/
void ShowProgress(u64 done, u64 total, const char *msg)
{
    if(total <= 0)
		return;

	if(done > total)
		done = total;

    //progress start for this file
    if(!done) {
        start = time(0);
        //set new filename
        if(msg)
            strncpy(progressMsg, msg, sizeof(progressMsg));
        else
            sprintf(progressMsg, " ");  //this shouldn't happen but in case it clears msg
    }

    progressDone = done;
    progressTotal = total;
}

/****************************************************************************
 * InitProgressThread
 *
 * Startup Progressthread in idle prio
 ***************************************************************************/
void InitProgressThread() {
	LWP_CreateThread(&progressthread, ProgressThread, NULL, NULL, 0, 70);
}

/****************************************************************************
 * ExitProgressThread
 *
 * Shutdown Progressthread
 ***************************************************************************/
void ExitProgressThread() {
	LWP_JoinThread(progressthread, NULL);
	progressthread = LWP_THREAD_NULL;
}
