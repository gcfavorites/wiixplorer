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
#include "Memory/Resources.h"
#include "Prompts/PromptWindows.h"
#include "LanguageUpdater.h"
#include "DirList.h"
#include "main.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"

/****************************************************************************
 * LanguageBrowser
 ***************************************************************************/
int LanguageBrowser()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;

    char langpath[150];
    snprintf(langpath, sizeof(langpath), "%s", Settings.LanguagePath);
    if(langpath[strlen(langpath)-1] != '/')
    {
        char * ptr = strrchr(langpath, '/');
        if(ptr)
        {
            ptr++;
            ptr[0] = '\0';
        }
    }

	DirList FileList(langpath, ".lang");

	int filecount = FileList.GetFilecount();

	if(!filecount)
	{
        int choice = WindowPrompt(tr("No language files found."), tr("Change Language Path?"), tr("Change"), tr("Continue"));
        if(choice)
        {
            char entered[150];
            snprintf(entered, sizeof(entered), "%s", langpath);
            if(OnScreenKeyboard(entered, 149))
            {
                snprintf(Settings.LanguagePath, sizeof(Settings.LanguagePath), "%s", entered);
                if(Settings.LanguagePath[strlen(Settings.LanguagePath)-1] != '/' &&
                   strncasecmp(&Settings.LanguagePath[strlen(Settings.LanguagePath)-5], ".lang", 5) != 0)
                {
                    strncat(Settings.LanguagePath, "/", sizeof(Settings.LanguagePath));
                }

                WindowPrompt(tr("Language Path changed."), 0, tr("OK"));
                return MENU_LANGUAGE_BROWSE;
            }
        }
	}
	OptionList options;

	for(i = 0; i < filecount; i++)
	{
        options.SetName(i, FileList.GetFilename(i));
        options.SetValue(i, " ");
	}

	GuiSound * btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
	GuiImageData * btnOutline = Resources::GetImageData(button_png, button_png_size);
	GuiImageData * btnNetworkData = Resources::GetImageData(ftpstorage_png, ftpstorage_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(btnOutline);
	GuiButton backBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	backBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	backBtn.SetPosition(-105-btnOutline->GetWidth()/2, -65);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiText DefaultBtnTxt(tr("Default"), 18, (GXColor){0, 0, 0, 255});
	GuiImage DefaultBtnImg(btnOutline);
	GuiButton DefaultBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	DefaultBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	DefaultBtn.SetPosition(100+btnOutline->GetWidth()/2, -65);
	DefaultBtn.SetLabel(&DefaultBtnTxt);
	DefaultBtn.SetImage(&DefaultBtnImg);
	DefaultBtn.SetSoundOver(btnSoundOver);
	DefaultBtn.SetTrigger(&trigA);
	DefaultBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(584, 248, &options);
	optionBrowser.SetPosition(0, 100);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	GuiText titleTxt(tr("Language Settings"), 24, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(-optionBrowser.GetWidth()/2+titleTxt.GetTextWidth()/2+20, optionBrowser.GetTop()-35);

	GuiImage DownloadBtnImg(btnNetworkData);
	GuiButton DownloadBtn(btnNetworkData->GetWidth(), btnNetworkData->GetHeight());
	DownloadBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	DownloadBtn.SetPosition(optionBrowser.GetWidth()/2-60, optionBrowser.GetTop()-40);
	DownloadBtn.SetImage(&DownloadBtnImg);
	DownloadBtn.SetSoundOver(btnSoundOver);
	DownloadBtn.SetTrigger(&trigA);
	DownloadBtn.SetEffectGrow();

	MainWindow::Instance()->HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&DefaultBtn);
	w.Append(&optionBrowser);
	w.Append(&DownloadBtn);
	w.Append(&titleTxt);
	MainWindow::Instance()->Append(&w);
    w.SetEffect(EFFECT_FADE, 50);
	MainWindow::Instance()->ResumeGui();

	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
            Sys_Reboot();

        if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

		else if(backBtn.GetState() == STATE_CLICKED)
		{
            Settings.Save();
			menu = MENU_SETTINGS;
		}

		else if(DefaultBtn.GetState() == STATE_CLICKED)
		{
			DefaultBtn.ResetState();
			int choice = WindowPrompt(tr("Updating/Downloading Languagefiles."), tr("Do you want to load the default language."), tr("Console Default"), tr("App Default"), tr("Cancel"));
            if(choice)
            {
                if(choice == 1)
                {
                    if(Settings.LoadLanguage(NULL, CONSOLE_DEFAULT))
                        Settings.Save();
                }

                if(choice == 2)
                {
                    if(Settings.LoadLanguage(NULL, APP_DEFAULT))
                        Settings.Save();
                }

                menu = MENU_SETTINGS;
            }
		}

		else if(DownloadBtn.GetState() == STATE_CLICKED)
		{
			DownloadBtn.ResetState();
		    int choice = WindowPrompt(0, tr("Do you want to download new language files?"), tr("Yes"), tr("Cancel"));
            if(choice)
            {
				int result = UpdateLanguageFiles();
                if(result > 0)
                {
                    WindowPrompt(0, fmt(tr("Downloaded %d files."), result), tr("OK"));
                    menu = MENU_LANGUAGE_BROWSE;
                }
            }
		}

		ret = optionBrowser.GetClickedOption();

		if(ret >= 0)
		{
		    int choice = WindowPrompt(FileList.GetFilename(ret), tr("Do you want to load this language ?"), tr("Yes"), tr("Cancel"));
            if(choice)
            {
                if(Settings.LoadLanguage(FileList.GetFilepath(ret)))
                        Settings.Save();

                menu = MENU_SETTINGS;
            }
		}
	}

    w.SetEffect(EFFECT_FADE, -50);
	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	MainWindow::Instance()->HaltGui();
	MainWindow::Instance()->Remove(&w);
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnOutline);
	Resources::Remove(btnNetworkData);
	MainWindow::Instance()->ResumeGui();

	return menu;
}
