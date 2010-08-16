/***************************************************************************
 * Copyright (C) 2010
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "libwiigui/gui_keyboard.h"
#include "network/networkops.h"
#include "Prompts/PromptWindows.h"
#include "Controls/Taskbar.h"
#include "devicemounter.h"
#include "FileOperations/fileops.h"
#include "FileOperations/filebrowser.h"
#include "menu.h"
#include "main.h"
#include "filelist.h"
#include "Controls/MainWindow.h"
#include "Tools/StringTools.h"
#include "TextOperations/wstring.hpp"
#include "sys.h"
#include "svnrev.h"

int OnScreenKeyboard(char * var, u16 maxlen)
{
    wString Converter;
    Converter.resize(maxlen+2);

    char2wchar_t(var, &Converter[0]);

	int save = OnScreenKeyboard(&Converter[0], maxlen);

	if(save)
	{
	    snprintf(var, maxlen, "%s", Converter.toUTF8().c_str());
	}

	return save;
}

int OnScreenKeyboard(wchar_t * var, u16 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound * btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
	GuiImageData * btnOutline = Resources::GetImageData(button_png, button_png_size);
	GuiImageData * btnOutlineOver = Resources::GetImageData(button_over_png, button_over_png_size);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	GuiText okBtnTxt(tr("OK"), 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(btnOutline);
	GuiImage okBtnImgOver(btnOutlineOver);
	GuiButton okBtn(btnOutline->GetWidth(), btnOutline->GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(btnSoundOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt(tr("Cancel"), 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(btnOutline);
	GuiImage cancelBtnImgOver(btnOutlineOver);
	GuiButton cancelBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	MainWindow::Instance()->HaltGui();
	int oldState = MainWindow::Instance()->GetState();
	bool oldDim = MainWindow::Instance()->IsDimmed();
	MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->SetDim(true);
	MainWindow::Instance()->Append(&keyboard);
	MainWindow::Instance()->ResumeGui();

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
		wcsncpy(var, keyboard.GetString(), maxlen);
	}

	MainWindow::Instance()->HaltGui();
	MainWindow::Instance()->Remove(&keyboard);
    MainWindow::Instance()->SetDim(oldDim);
	MainWindow::Instance()->SetState(oldState);
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnOutline);
	Resources::Remove(btnOutlineOver);
	MainWindow::Instance()->ResumeGui();
	return save;
}

/****************************************************************************
* NetworkInitPrompt
***************************************************************************/
bool NetworkInitPrompt()
{
    if(IsNetworkInit())
        return true;

    PromptWindow * Prompt = new PromptWindow(tr("Network initialising..."), tr("Please wait..."));
    MainWindow::Instance()->Append(Prompt);

    if(!Settings.AutoConnect)
    {
        InitNetworkThread();
        ResumeNetworkThread();
    }

    while(Prompt->GetChoice() == -1 && !IsNetworkInit())
    {
        usleep(100);

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
            Sys_Reboot();
    }

    delete Prompt;
    Prompt = NULL;

    if(!IsNetworkInit())
    {
        ShowError(tr("No network connection."));
        return false;
    }

    return true;
}

/****************************************************************************
* WindowPrompt
*
* Shortcut function
***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label,
const char *btn2Label, const char *btn3Label,
const char *btn4Label, bool resetstate)
{
    int choice = -1;

    PromptWindow * Prompt = new PromptWindow(title, msg, btn1Label, btn2Label, btn3Label, btn4Label, resetstate);

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
    static bool firsttimestart = true;

    if(!firsttimestart)
        return 1;

    firsttimestart = false;
    int choice = -1;

    if(Settings.LastUsedPath.compare(0, 3, "smb") != 0 && Settings.LastUsedPath.compare(0, 3, "ftp") != 0)
        return 1;

    PromptWindow * Prompt = new PromptWindow(tr("Please wait:"), tr("Network initialising..."), tr("Cancel"));

    if(!Settings.AutoConnect)
    {
        InitNetworkThread();
        ResumeNetworkThread();
    }

    time_t timer1 = 0;

    MainWindow::Instance()->Append(Prompt);

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

            if(timer1 == 0)
                timer1 = time(0);

            if(time(0) - timer1 > 5)
                choice = -2;

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

    Taskbar::Instance()->SetTriggerUpdate(false);
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

    Taskbar::Instance()->SetTriggerUpdate(true);
}
