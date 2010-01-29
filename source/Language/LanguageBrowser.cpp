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
 * LanguageBrowser.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_optionbrowser.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindows.h"
#include "devicemounter.h"
#include "Language/gettext.h"
#include "DirList.h"
#include "main.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/****************************************************************************
 * LanguageBrowser
 ***************************************************************************/
int LanguageBrowser()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0, n = 0;

	char lang_path[300];
	snprintf(lang_path, sizeof(lang_path), "%sconfig/WiiXplorer/Languages/", Settings.BootDevice);

	DirList FileList(lang_path, ".lang");

	int filecount = FileList.GetFilecount();

	if(!filecount) {
        WindowPrompt(tr("No language files found."), 0, tr("OK"));
        return MENU_SETTINGS;
	}
	OptionList options(filecount-1);

	for(i = 0; i < filecount; i++)
	{
	    if(!FileList.IsDir(i))
	    {
            options.SetName(n, FileList.GetFilename(i));
            options.SetValue(n, " ");
            n++;
	    }
	}

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png, button_png_size);
	GuiImageData btnOutlineOver(button_over_png, button_over_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -90);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiText ConsoleDefaultBtnTxt(tr("Console Default"), 18, (GXColor){0, 0, 0, 255});
	GuiImage ConsoleDefaultBtnImg(&btnOutline);
	GuiButton ConsoleDefaultBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	ConsoleDefaultBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	ConsoleDefaultBtn.SetPosition(-50, -90);
	ConsoleDefaultBtn.SetLabel(&ConsoleDefaultBtnTxt);
	ConsoleDefaultBtn.SetImage(&ConsoleDefaultBtnImg);
	ConsoleDefaultBtn.SetSoundOver(&btnSoundOver);
	ConsoleDefaultBtn.SetTrigger(&trigA);
	ConsoleDefaultBtn.SetEffectGrow();

	GuiText AppDefaultBtnTxt(tr("App Default"), 18, (GXColor){0, 0, 0, 255});
	GuiImage AppDefaultBtnImg(&btnOutline);
	GuiButton AppDefaultBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	AppDefaultBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	AppDefaultBtn.SetPosition(0, -65);
	AppDefaultBtn.SetLabel(&AppDefaultBtnTxt);
	AppDefaultBtn.SetImage(&AppDefaultBtnImg);
	AppDefaultBtn.SetSoundOver(&btnSoundOver);
	AppDefaultBtn.SetTrigger(&trigA);
	AppDefaultBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(584, 248, &options);
	optionBrowser.SetPosition(30, 60);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	GuiImageData settingsimgData(settingsbtn_over_png, settingsbtn_over_png_size);
	GuiImage settingsimg(&settingsimgData);
	settingsimg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsimg.SetPosition(50, optionBrowser.GetTop()-35);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&ConsoleDefaultBtn);
	w.Append(&AppDefaultBtn);
	w.Append(&optionBrowser);
	w.Append(&settingsimg);
	MainWindow::Instance()->Append(&w);
    w.SetEffect(EFFECT_FADE, 50);
	ResumeGui();

	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

        if(shutdown == 1)
            Sys_Shutdown();

        else if(reset == 1)
            Sys_Reboot();

        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

		else if(backBtn.GetState() == STATE_CLICKED)
		{
            Settings.Save();
			menu = MENU_SETTINGS;
		}

		else if(ConsoleDefaultBtn.GetState() == STATE_CLICKED)
		{
		    int choice = WindowPrompt(tr("Console Default"), tr("Do you want to load console default language."), tr("Yes"), tr("Cancel"));
            if(choice)
            {
                if(Settings.LoadLanguage(NULL, CONSOLE_DEFAULT))
                    Settings.Save();

                menu = MENU_SETTINGS;
            }
		}

		else if(AppDefaultBtn.GetState() == STATE_CLICKED)
		{
		    int choice = WindowPrompt(tr("App Default"), tr("Do you want to load app default language ?"), tr("Yes"), tr("Cancel"));
            if(choice)
            {
                if(Settings.LoadLanguage(NULL, APP_DEFAULT))
                        Settings.Save();

                menu = MENU_SETTINGS;
            }
		}

		ret = optionBrowser.GetClickedOption();

		if(ret >= 0)
		{
		    int choice = WindowPrompt(FileList.GetFilename(ret), tr("Do you want to load this language ?"), tr("Yes"), tr("Cancel"));
            if(choice)
            {
                char filepath[200];
                snprintf(filepath, sizeof(filepath), "%s%s", FileList.GetFilepath(ret), FileList.GetFilename(ret));
                if(Settings.LoadLanguage(filepath))
                        Settings.Save();

                menu = MENU_SETTINGS;
            }
		}
	}

    w.SetEffect(EFFECT_FADE, -50);
	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	HaltGui();
	MainWindow::Instance()->Remove(&w);
	ResumeGui();

	return menu;
}
