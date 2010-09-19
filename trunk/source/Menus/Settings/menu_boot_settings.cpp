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
#include "SettingsMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/DeviceMenu.h"
#include "Launcher/NandTitle.h"

int MenuBootSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	NandTitle Titles;
	Titles.Get();

	OptionList options;
	options.SetName(i++, tr("Boot IOS"));

	SettingsMenu * Menu = new SettingsMenu(tr("Boot Settings"), &options, MENU_SETTINGS);

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

		if(ret >= 0)
		{
            switch(Settings.BootIOS)
            {
                case 58:
                    if(Titles.IndexOf(TITLE_ID(1, 202)) >= 0)
                    {
                        Settings.BootIOS = 202;
                        break;
                    }
                case 202:
                    if(Titles.IndexOf(TITLE_ID(1, 222)) >= 0)
                    {
                        Settings.BootIOS = 222;
                        break;
                    }
                case 222:
                    if(Titles.IndexOf(TITLE_ID(1, 223)) >= 0)
                    {
                        Settings.BootIOS = 223;
                        break;
                    }
                case 223:
                    if(Titles.IndexOf(TITLE_ID(1, 36)) >= 0)
                    {
                        Settings.BootIOS = 36;
                        break;
                    }
                case 36:
                    if(Titles.IndexOf(TITLE_ID(1, 60)) >= 0)
                    {
                        Settings.BootIOS = 60;
                        break;
                    }
                case 60:
                    if(Titles.IndexOf(TITLE_ID(1, 61)) >= 0)
                    {
                        Settings.BootIOS = 61;
                        break;
                    }
                case 61:
                    if(Titles.IndexOf(TITLE_ID(1, 58)) >= 0)
                    {
                        Settings.BootIOS = 58;
                        break;
                    }
                default:
                    break;
            }
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++, "%i", Settings.BootIOS);
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}
