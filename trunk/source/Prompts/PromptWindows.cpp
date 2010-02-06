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
#include "main.h"
#include "filelist.h"
#include "Controls/MainWindow.h"
#include "sys.h"
#include "svnrev.h"

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

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size);
	GuiImageData btnOutline(button_png, button_png_size);
	GuiImageData btnOutlineOver(button_over_png, button_over_png_size);
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
		usleep(100);

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
            Sys_Reboot();

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

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
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

    InitNetworkThread();
    ResumeNetworkThread();

    while(choice == -1)
    {
        usleep(100);

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
            Sys_Reboot();

        choice = Prompt->GetChoice();

        if(IsNetworkInit())
        {
            Prompt->SetMessage(tr("SMB is connecting..."));

            for(int i = 0; i < 4; i++)
            {
                if(IsSMB_Mounted(i))
                {
                    choice = 2;
                    break;
                }
            }
        }
    }

    delete Prompt;
    Prompt = NULL;

    return choice;
}

void ShowCredits(CreditWindow *& Credits)
{
    if(!Credits)
        return;

    MainWindow::Instance()->SetDim(true);
    MainWindow::Instance()->Append(Credits);

    int credits_choice = -1;
    while(credits_choice < 0)
    {
        usleep(100);

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
            Sys_Reboot();

        credits_choice = Credits->GetChoice();
    }
    delete Credits;
    Credits = NULL;
}
