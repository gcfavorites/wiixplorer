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
#include "SettingsMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindows.h"

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
int MenuSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Language"));
	options.SetName(i++, tr("Clock Mode"));
	options.SetName(i++, tr("Explorer Settings"));
	options.SetName(i++, tr("Boot Settings"));
	options.SetName(i++, tr("Image Settings"));
	options.SetName(i++, tr("Sound Settings"));
	options.SetName(i++, tr("Network Settings"));
	options.SetName(i++, tr("File Extensions"));
	options.SetName(i++, tr("Controls Settings"));
	options.SetName(i++, tr("Path Setup"));
	options.SetName(i++, tr("Color Settings"));

	SettingsMenu * Menu = new SettingsMenu(tr("Settings"), &options, MENU_BROWSE_DEVICE);

	MainWindow::Instance()->Append(Menu);

    GuiImageData * btnOutline = Resources::GetImageData(button_png, button_png_size);
	GuiSound * btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    SimpleGuiTrigger trigA(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	GuiText resetBtnTxt(tr("Reset"), 22, (GXColor){0, 0, 0, 255});
	GuiImage resetBtnImg(btnOutline);
	GuiButton resetBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	resetBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	resetBtn.SetPosition(100+btnOutline->GetWidth()/2, -65);
	resetBtn.SetLabel(&resetBtnTxt);
	resetBtn.SetImage(&resetBtnImg);
	resetBtn.SetSoundOver(btnSoundOver);
	resetBtn.SetTrigger(&trigA);
	resetBtn.SetEffectGrow();

	MainWindow::Instance()->Append(&resetBtn);

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
        else if(resetBtn.GetState() == STATE_CLICKED)
        {
			resetBtn.ResetState();
			int choice = WindowPrompt(tr("Do you want to reset the settings?"), 0, tr("Yes"), tr("Cancel"));
			if(choice)
			{
                Settings.Reset();
                firstRun = true;
			}
        }

		ret = Menu->GetClickedOption();

		switch (ret)
		{
			case 0:
				menu = MENU_LANGUAGE_BROWSE;
				break;
            case 1:
				Settings.ClockMode = (Settings.ClockMode+1) % 2;
				break;
			case 2:
                menu = MENU_EXPLORER_SETTINGS;
				break;
			case 3:
                menu = MENU_BOOT_SETTINGS;
				break;
            case 4:
                menu = MENU_IMAGE_SETTINGS;
				break;
            case 5:
                menu = MENU_SOUND_SETTINGS;
				break;
            case 6:
                menu = MENU_NETWORK_SETTINGS;
				break;
            case 7:
                menu = MENU_FILE_EXTENSIONS;
				break;
            case 8:
                menu = MENU_CONTROLS_SETTINGS;
				break;
            case 9:
                menu = MENU_PATH_SETUP;
				break;
            case 10:
                menu = MENU_COLOR_SETTINGS;
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if(strcmp(Settings.LanguagePath, "") != 0)
            {
                if(strcmp(Settings.LanguagePath, "") == 0)
                {
                    options.SetValue(i++, tr("Standard"));
                }
                else if(Settings.LanguagePath[strlen(Settings.LanguagePath)-1] == '/')
                {
                    options.SetValue(i++, tr("Standard"));
                }
                else
                {
                    char * language = strrchr(Settings.LanguagePath, '/')+1;
                    options.SetValue(i++, "%s", language);
                }
            }
            else
                options.SetValue(i++, tr("App Default"));

            if (Settings.ClockMode == 1)
                options.SetValue(i++, tr("12H"));
            else
                options.SetValue(i++, tr("24H"));

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");
        }
	}
	MainWindow::Instance()->HaltGui();
	MainWindow::Instance()->Remove(&resetBtn);
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnOutline);
    delete Menu;
	MainWindow::Instance()->ResumeGui();

    Settings.Save();

	return menu;
}
