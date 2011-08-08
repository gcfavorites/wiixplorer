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
#include "Controls/Application.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindows.h"
#include "network/networkops.h"

/****************************************************************************
 * MenuFTPClientSettings
 ***************************************************************************/
int MenuFTPClientSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
	char entered[150];
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Client:"));
	options.SetName(i++, tr("Host:"));
	options.SetName(i++, tr("Username:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("FTP Port:"));
	options.SetName(i++, tr("FTP Path:"));
	options.SetName(i++, tr("Passive Mode:"));
	options.SetName(i++, tr("Reconnect FTP"));
	options.SetName(i++, tr("Disconnect Client"));

	SettingsMenu * Menu = new SettingsMenu(tr("FTP Client Settings"), &options, MENU_NETWORK_SETTINGS);

	Application::Instance()->Append(Menu);

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(Menu->GetMenu() != MENU_NONE)
		{
			menu = Menu->GetMenu();
		}
//		else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
//		{
//			menu = Taskbar::Instance()->GetMenu();
//		}

		ret = Menu->GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.CurrentFTPUser++;
				if(Settings.CurrentFTPUser >= MAXFTPUSERS)
					Settings.CurrentFTPUser = 0;
				break;
			case 1:
				snprintf(entered, sizeof(entered), "%s", Settings.FTPUser[Settings.CurrentFTPUser].Host);
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.FTPUser[Settings.CurrentFTPUser].Host, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].Host), "%s", entered);
				}
				break;
			case 2:
				snprintf(entered, sizeof(entered), "%s", Settings.FTPUser[Settings.CurrentFTPUser].User);
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.FTPUser[Settings.CurrentFTPUser].User, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].User), "%s", entered);
				}
				break;
			case 3:
				entered[0] = 0;
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.FTPUser[Settings.CurrentFTPUser].Password, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].Password), "%s", entered);
				}
				break;
			case 4:
				snprintf(entered, sizeof(entered), "%i", Settings.FTPUser[Settings.CurrentFTPUser].Port);
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					Settings.FTPUser[Settings.CurrentFTPUser].Port = (u16) atoi(entered);
				}
				break;
			case 5:
				snprintf(entered, sizeof(entered), "%s", Settings.FTPUser[Settings.CurrentFTPUser].FTPPath);
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.FTPUser[Settings.CurrentFTPUser].FTPPath, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].FTPPath), "%s", entered);
				}
				break;
			case 6:
				Settings.FTPUser[Settings.CurrentFTPUser].Passive++;
				if(Settings.FTPUser[Settings.CurrentFTPUser].Passive > 1)
					Settings.FTPUser[Settings.CurrentFTPUser].Passive = 0;
				break;
			case 7:
				result = WindowPrompt(tr("Do you want to reconnect to the FTP client?"),0,tr("OK"),tr("Cancel"));
				if(result) {
					 CloseFTP();
					 sleep(1);
					 if(ConnectFTP())
						WindowPrompt(tr("FTP Client reconnected successfull."), 0, tr("OK"));
					else
						ShowError(tr("Reconnect failed. No client connected."));
				}
				break;
			case 8:
				result = WindowPrompt(tr("Do you want to disconnect the FTP client?"),0,tr("OK"),tr("Cancel"));
				if(result) {
					 CloseFTP(Settings.CurrentFTPUser);
				}
				break;
		}

		if(firstRun || ret >= 0)
		{
			i = 0;
			firstRun = false;

			options.SetValue(i++,tr("Client %i"), Settings.CurrentFTPUser+1);
			options.SetValue(i++,"%s", Settings.FTPUser[Settings.CurrentFTPUser].Host);
			options.SetValue(i++,"%s", Settings.FTPUser[Settings.CurrentFTPUser].User);
			if (strcmp(Settings.FTPUser[Settings.CurrentFTPUser].Password, "") != 0)
				options.SetValue(i++,"********");
			else
				options.SetValue(i++," ");
			options.SetValue(i++,"%i", Settings.FTPUser[Settings.CurrentFTPUser].Port);
			options.SetValue(i++,"%s", Settings.FTPUser[Settings.CurrentFTPUser].FTPPath);
			if (Settings.FTPUser[Settings.CurrentFTPUser].Passive == 1) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPUser[Settings.CurrentFTPUser].Passive == 0) options.SetValue(i++,tr("OFF"));
			options.SetValue(i++," ");
			options.SetValue(i++," ");
		}
	}

	delete Menu;

	Settings.Save();

	return menu;
}
