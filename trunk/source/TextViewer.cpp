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
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "menu.h"
#include "fileops.h"
#include "sys.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();


/****************************************************************************
* TextViewer
***************************************************************************/
void TextViewer(const char *filepath)
{
    bool exitwindow = false;
    int currentLine = 0;
    int linestoshow = 8;

    u8 *file = NULL;
    u64 filesize = 0;

    StartProgress("Loading file:");
    int ret = LoadFileToMem(filepath, &file, &filesize);
    StopProgress();

    if(ret == -1) {
        WindowPrompt("Error", "Can not open the file", "OK");
        return;
    }
    else if(ret == -2) {
        WindowPrompt("Error", "Not enough memory.", "OK");
        return;
    }
    else if(ret == -3) {
        WindowPrompt("Error", "Can not open the file", "OK");
        return;
    }
    else if(ret == -10) {
        WindowPrompt("Loading file:", "Action cancelled.", "OK");
        return;
    }

    //To check if enough memory available for strcpy in GuiText
    char *filetext = new char[filesize];
    if(!filetext) {
        WindowPrompt("Error", "Not enough memory.", "OK");
        return;
    }

    snprintf(filetext, filesize, "%s", (char *) file);

    free(file);
    file = NULL;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigUp;
    trigUp.SetButtonOnlyTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP, PAD_BUTTON_UP);
    GuiTrigger trigDown;
    trigDown.SetButtonOnlyTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN, PAD_BUTTON_DOWN);
    GuiTrigger trigLeft;
    trigLeft.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
    GuiTrigger trigRight;
    trigRight.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

    GuiImageData dialogBox(dialogue_box_png);
    GuiImage dialogBoxImg(&dialogBox);

    char *filename = strrchr(filepath, '/')+1;

    GuiText titleTxt(filename, 22, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,40);
    titleTxt.SetMaxWidth(430, GuiText::DOTTED);

    GuiText MainFileTxt(filetext, 18, (GXColor){0, 0, 0, 255});
    MainFileTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    MainFileTxt.SetPosition(30, 70);
    MainFileTxt.SetMaxWidth(400, GuiText::LONGTEXT);
    MainFileTxt.SetFirstLine(0);
    MainFileTxt.SetLinesToDraw(linestoshow);

    delete filetext;
    filetext = NULL;

    GuiImageData arrowUp(scrollbar_arrowup_png);
	GuiImage arrowUpImg(&arrowUp);
	GuiImageData arrowUpOver(scrollbar_arrowup_over_png);
	GuiImage arrowUpOverImg(&arrowUpOver);

	GuiButton arrowUpBtn(arrowUpImg.GetWidth(), arrowUpImg.GetHeight());
	arrowUpBtn.SetImage(&arrowUpImg);
	arrowUpBtn.SetImageOver(&arrowUpOverImg);
	arrowUpBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	arrowUpBtn.SetPosition(-20, 80);
    arrowUpBtn.SetTrigger(&trigUp);
    arrowUpBtn.SetTrigger(&trigA);
	arrowUpBtn.SetSoundOver(&btnSoundOver);
	arrowUpBtn.SetSoundClick(&btnClick);

	GuiImageData arrowDown(scrollbar_arrowdown_png);
    GuiImage arrowDownImg(&arrowDown);
	GuiImageData arrowDownOver(scrollbar_arrowdown_over_png);
	GuiImage arrowDownOverImg(&arrowDownOver);

	GuiButton arrowDownBtn(arrowDownImg.GetWidth(), arrowDownImg.GetHeight());
	arrowDownBtn.SetImage(&arrowDownImg);
	arrowDownBtn.SetImageOver(&arrowDownOverImg);
	arrowDownBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	arrowDownBtn.SetPosition(-20, -70);
    arrowDownBtn.SetTrigger(&trigDown);
    arrowDownBtn.SetTrigger(&trigA);
	arrowDownBtn.SetSoundOver(&btnSoundOver);
	arrowDownBtn.SetSoundClick(&btnClick);

	GuiButton arrowLeft(1, 1);
	arrowLeft.SetTrigger(&trigLeft);

	GuiButton arrowRight(1, 1);
	arrowRight.SetTrigger(&trigRight);

    GuiImage btn1Img(&arrowUp);
    btn1Img.SetAngle(45);
    GuiImage btn1ImgOver(&arrowUpOver);
    btn1ImgOver.SetAngle(45);
    GuiButton btn1(btn1Img.GetWidth(), btn1Img.GetHeight());
    btn1.SetImage(&btn1Img);
    btn1.SetImageOver(&btn1ImgOver);
    btn1.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    btn1.SetPosition(-20, 30);
    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetSoundClick(&btnClick);
    btn1.SetTrigger(&trigA);
    btn1.SetEffectGrow();

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&MainFileTxt);
    promptWindow.Append(&arrowUpBtn);
    promptWindow.Append(&arrowDownBtn);
    promptWindow.Append(&arrowLeft);
    promptWindow.Append(&arrowRight);
    promptWindow.Append(&btn1);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(!exitwindow)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();

        else if(btn1.GetState() == STATE_CLICKED) {
            exitwindow = true;
            btn1.ResetState();
        }

        else if(arrowUpBtn.GetState() == STATE_CLICKED) {

            currentLine--;
            if(currentLine < 0)
                currentLine = 0;
            MainFileTxt.SetFirstLine(currentLine);
            usleep(20000);
            int chan = arrowUpBtn.GetStateChan();
            if(!userInput[chan].wpad.btns_h)
                    arrowUpBtn.ResetState();
        }
        else if(arrowDownBtn.GetState() == STATE_CLICKED) {

            int totalLines = MainFileTxt.GetTotalLines();
            currentLine++;
            if(currentLine+linestoshow > totalLines)
                currentLine = totalLines-linestoshow;
            if(currentLine < 0)
                currentLine = 0;

            MainFileTxt.SetFirstLine(currentLine);
            usleep(20000);
            int chan = arrowDownBtn.GetStateChan();
            if(!userInput[chan].wpad.btns_h)
                arrowDownBtn.ResetState();
        }
        else if(arrowRight.GetState() == STATE_CLICKED) {

            int totalLines = MainFileTxt.GetTotalLines();
            currentLine += 8;
            if(currentLine+linestoshow > totalLines)
                currentLine = totalLines-linestoshow;
            if(currentLine < 0)
                currentLine = 0;

            MainFileTxt.SetFirstLine(currentLine);
            arrowRight.ResetState();
        }
        else if(arrowLeft.GetState() == STATE_CLICKED) {

            currentLine -= 8;
            if(currentLine < 0)
                currentLine = 0;

            MainFileTxt.SetFirstLine(currentLine);
            arrowLeft.ResetState();
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
}
