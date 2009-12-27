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
 * PromptWindows.cpp
 *
 * All promptwindows
 * for WiiXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "libwiigui/gui_keyboard.h"
#include "network/networkops.h"
#include "Prompts/PromptWindows.h"
#include "devicemounter.h"
#include "FileOperations/fileops.h"
#include "FileOperations/filebrowser.h"
#include "menu.h"
#include "filelist.h"
#include "Language/gettext.h"
#include "Controls/MainWindow.h"
#include "sys.h"
#include "svnrev.h"

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/****************************************************************************
 * OnScreenKeyboard by Tantric 2009
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
int OnScreenKeyboard(char * var, u16 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText okBtnTxt(tr("OK"), 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt(tr("Cancel"), 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->SetDim(true);
	MainWindow::Instance()->Append(&keyboard);
	MainWindow::Instance()->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		VIDEO_WaitVSync();

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	HaltGui();
	MainWindow::Instance()->Remove(&keyboard);
    MainWindow::Instance()->SetDim(false);
	MainWindow::Instance()->SetState(STATE_DEFAULT);
	ResumeGui();
	return save;
}

/****************************************************************************
* WindowPrompt
*
* Shortcut function
***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label,
const char *btn2Label, const char *btn3Label,
const char *btn4Label)
{
    int choice = -1;

    PromptWindow * Prompt = new PromptWindow(title, msg, btn1Label, btn2Label, btn3Label, btn4Label);

    MainWindow::Instance()->Append(Prompt);

    while(choice == -1)
    {
        usleep(100);

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();

        choice = Prompt->GetChoice();
    }

    delete Prompt;
    Prompt = NULL;

    return choice;
}

/****************************************************************************
* Wait SMB Connection
***************************************************************************/
int WaitSMBConnect(void)
{
    int choice = -1;

    PromptWindow * Prompt = NULL;
    Prompt = new PromptWindow(tr("Please wait:"), tr("Network initialising..."), tr("Cancel"));

    MainWindow::Instance()->Append(Prompt);

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(IsNetworkInit())
        {
            Prompt->SetMessage(tr("SMB is connecting..."));

            for(int i = 0; i < 4; i++)
                if(IsSMB_Mounted(i))
                    choice = 1;
        }

        choice = Prompt->GetChoice();
    }

    delete Prompt;
    Prompt = NULL;

    return choice;
}


/****************************************************************************
* RightMouseClicked connection for RightClickMenu
***************************************************************************/
void RightMouseClicked(GuiElement *sender, int pointer, POINT p)
{
    RightClickMenu * ClickMenu = NULL;
    ClickMenu = new RightClickMenu(p.x, p.y);

    MainWindow::Instance()->Append(ClickMenu);

    while(ClickMenu->GetChoice() == -1)
    {
        usleep(100);

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();
    }

    delete ClickMenu;
    ClickMenu = NULL;

    ResumeGui();
}

/****************************************************************************
* CreditsWindow
***************************************************************************/
void CreditsWindow(void)
{
    GuiImageData dialogBox(bg_properties_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiWindow promptWindow(dialogBox.GetWidth(), dialogBox.GetHeight());
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(100, 100);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    int numEntries = 9;
    int i = 0;
    int y = 30;

    GuiText * txt[numEntries];

    txt[i] = new GuiText(tr("Credits"), 28, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    txt[i]->SetPosition(0, y);
    i++;
    y += 60;

    txt[i] = new GuiText(tr("Coders:"), 24, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20, y);
    i++;

    txt[i] = new GuiText("Dimok", 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(170, y);
    i++;
    y += 32;

    txt[i] = new GuiText("r-win", 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(170, y);
    i++;
    y += 40;

    txt[i] = new GuiText(tr("Designer:"), 24, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20, y);
    i++;

    txt[i] = new GuiText("NeoRame", 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(170, y);
    i++;
    y += 60;

    txt[i] = new GuiText(tr("Special thanks to:"), 24, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20,y);
    i++;
    y += 30;

    char text[80];
    snprintf(text, sizeof(text), "Tantric %s ", tr("for his great tool LibWiiGui."));
    txt[i] = new GuiText(text, 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20,y);
    i++;
    y += 30;

    txt[i] = new GuiText(tr("The whole DevkitPro & libogc staff."), 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20,y);

    GuiImageData arrowUp(scrollbar_arrowup_png);
    GuiImageData arrowUpOver(scrollbar_arrowup_over_png);
    GuiImage arrowUpImg(&arrowUp);
    arrowUpImg.SetAngle(45);
    GuiImage arrowUpImgOver(&arrowUpOver);
    arrowUpImgOver.SetAngle(45);
    GuiButton Backbtn(arrowUpImg.GetWidth(), arrowUpImg.GetHeight());
    Backbtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    Backbtn.SetSoundClick(&btnClick);
    Backbtn.SetImage(&arrowUpImg);
    Backbtn.SetImageOver(&arrowUpImgOver);
    Backbtn.SetPosition(-20, 20);
    Backbtn.SetEffectGrow();
    Backbtn.SetTrigger(&trigA);
    Backbtn.SetTrigger(&trigB);

    char Rev[50];
    sprintf(Rev, "Rev. %i", atoi(SvnRev()));

    GuiText RevNum(Rev, 22, (GXColor) {0, 0, 0, 255});
    RevNum.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    RevNum.SetPosition(20, 20);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&RevNum);
    for(int i = 0; i < numEntries; i++)
        promptWindow.Append(txt[i]);
    promptWindow.Append(&Backbtn);

    HaltGui();
    MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->SetDim(true);
    MainWindow::Instance()->Append(&promptWindow);
    MainWindow::Instance()->ChangeFocus(&promptWindow);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 40);
    ResumeGui();

    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    while(1)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();

        else if(Backbtn.GetState() == STATE_CLICKED) {
            break;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 40);

    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    MainWindow::Instance()->Remove(&promptWindow);

    for(int i = 0; i < numEntries; i++) {
        delete txt[i];
        txt[i] = NULL;
    }

    MainWindow::Instance()->SetState(STATE_DEFAULT);
    MainWindow::Instance()->SetDim(false);
    ResumeGui();
}
