 /****************************************************************************
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
 * for WiiXplorer 2010
 ***************************************************************************/
 #include <unistd.h>
#include "Prompts/PromptWindows.h"
#include "SoundOperations/gui_bgm.h"
#include "ImageOperations/ImageWrite.h"
#include "Prompts/DeviceMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "SettingsMenu.h"
#include "menu_settings.h"
#include "main.h"

/****************************************************************************
 * MenuFileExtensions
 ***************************************************************************/
int MenuFileExtensions()
{
	int menu = MENU_NONE;
	char entered[300];
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Video Files"));
	options.SetName(i++, tr("Audio Files"));
	options.SetName(i++, tr("Image Files"));
	options.SetName(i++, tr("Archive Files"));
	options.SetName(i++, tr("Homebrew Files"));
	options.SetName(i++, tr("Font Files"));
	options.SetName(i++, tr("Language Files"));
	options.SetName(i++, tr("Wii Binary Files"));
	options.SetName(i++, tr("PDF Files"));
	options.SetName(i++, tr("WiiXplorer Movies"));

	SettingsMenu * Menu = new SettingsMenu(tr("File Extensions Assignment"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

		if(Menu->GetMenu() != MENU_NONE)
		{
			menu = Menu->GetMenu();
		}
        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
        {
			menu = Taskbar::Instance()->GetMenu();
        }

		ret = Menu->GetClickedOption();

		switch (ret)
		{
			case 0:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetVideo());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetVideo(entered);
                }
                break;
			case 1:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetAudio());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetAudio(entered);
                }
                break;
			case 2:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetImage());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetImage(entered);
                }
                break;
			case 3:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetArchive());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetArchive(entered);
                }
                break;
			case 4:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetHomebrew());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetHomebrew(entered);
                }
                break;
			case 5:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetFont());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetFont(entered);
                }
                break;
			case 6:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetLanguageFiles());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetLanguageFiles(entered);
                }
                break;
			case 7:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetWiiBinary());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetWiiBinary(entered);
                }
                break;
			case 8:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetPDF());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetPDF(entered);
                }
                break;
			case 9:
                snprintf(entered, sizeof(entered), "%s", Settings.FileExtensions.GetWiiXplorerMovies());
                if(OnScreenKeyboard(entered, 299))
                {
                    Settings.FileExtensions.SetWiiXplorerMovies(entered);
                }
                break;
            default:
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++, Settings.FileExtensions.GetVideo());
            options.SetValue(i++, Settings.FileExtensions.GetAudio());
            options.SetValue(i++, Settings.FileExtensions.GetImage());
            options.SetValue(i++, Settings.FileExtensions.GetArchive());
            options.SetValue(i++, Settings.FileExtensions.GetHomebrew());
            options.SetValue(i++, Settings.FileExtensions.GetFont());
            options.SetValue(i++, Settings.FileExtensions.GetLanguageFiles());
            options.SetValue(i++, Settings.FileExtensions.GetWiiBinary());
            options.SetValue(i++, Settings.FileExtensions.GetPDF());
            options.SetValue(i++, Settings.FileExtensions.GetWiiXplorerMovies());
        }
	}

    delete Menu;
    Settings.Save();

	return menu;
}
