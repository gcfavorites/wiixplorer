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
#include "Prompts/PromptWindows.h"
#include "network/networkops.h"

/****************************************************************************
 * MenuFTPServerSettings
 ***************************************************************************/
int MenuFTPServerSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Auto Start:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("FTP Port:"));

	SettingsMenu * Menu = new SettingsMenu(tr("FTP Server Settings"), &options, MENU_NETWORK_SETTINGS);

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
                Settings.FTPServer.AutoStart = (Settings.FTPServer.AutoStart+1) % 2;
                break;
            case 1:
                entered[0] = 0;
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPServer.Password, sizeof(Settings.FTPServer.Password), "%s", entered);
                }
                break;
            case 2:
                snprintf(entered, sizeof(entered), "%d", Settings.FTPServer.Port);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    Settings.FTPServer.Port = (u16) atoi(entered);
                }
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if(Settings.FTPServer.AutoStart == 1)
                options.SetValue(i++, tr("ON"));
            else
                options.SetValue(i++, tr("OFF"));

			if (strcmp(Settings.FTPServer.Password, "") != 0)
				options.SetValue(i++,"********");
			else
				options.SetValue(i++," ");

            options.SetValue(i++,"%i", Settings.FTPServer.Port);
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}
