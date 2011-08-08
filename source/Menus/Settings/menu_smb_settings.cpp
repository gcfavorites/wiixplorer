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
 * MenuSMBSettings
 ***************************************************************************/
int MenuSMBSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
	char entered[150];
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Share:"));
	options.SetName(i++, tr("Host:"));
	options.SetName(i++, tr("Username:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("SMB Name:"));
	options.SetName(i++, tr("Reconnect SMB"));
	options.SetName(i++, tr("Disconnect SMB"));

	SettingsMenu * Menu = new SettingsMenu(tr("SMB Settings"), &options, MENU_NETWORK_SETTINGS);

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
				Settings.CurrentSMBUser++;
				if(Settings.CurrentSMBUser >= MAXSMBUSERS)
					Settings.CurrentSMBUser = 0;
				break;
			case 1:
				snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentSMBUser].Host);
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.SMBUser[Settings.CurrentSMBUser].Host, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].Host), "%s", entered);
				}
				break;
			case 2:
				snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentSMBUser].User);
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.SMBUser[Settings.CurrentSMBUser].User, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].User), "%s", entered);
				}
				break;
			case 3:
				entered[0] = 0;
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.SMBUser[Settings.CurrentSMBUser].Password, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].Password), "%s", entered);
				}
				break;
			case 4:
				snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentSMBUser].SMBName);
				result = OnScreenKeyboard(entered, 149);
				if(result) {
					snprintf(Settings.SMBUser[Settings.CurrentSMBUser].SMBName, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].SMBName), "%s", entered);
				}
				break;
			case 5:
				result = WindowPrompt(tr("Do you want to reconnect this SMB?"),0,tr("OK"),tr("Cancel"));
				if(result)
				{
					if(SMB_Reconnect())
						WindowPrompt(tr("SMB reconnected successfull."), 0, tr("OK"));
					else
						ShowError(tr("Reconnect failed. No share connected."));
				}
				break;
			case 6:
				result = WindowPrompt(tr("Do you want to disconnect this SMB?"),0,tr("OK"),tr("Cancel"));
				if(result) {
					CloseSMBShare(Settings.CurrentSMBUser);
				}
				break;
		}

		if(firstRun || ret >= 0)
		{
			i = 0;
			firstRun = false;

			options.SetValue(i++,tr("Share %i"), Settings.CurrentSMBUser+1);
			options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentSMBUser].Host);
			options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentSMBUser].User);
			if (strcmp(Settings.SMBUser[Settings.CurrentSMBUser].Password, "") != 0)
				options.SetValue(i++,"********");
			else
				options.SetValue(i++," ");
			options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentSMBUser].SMBName);
			options.SetValue(i++," ");
			options.SetValue(i++," ");
		}
	}

	delete Menu;

	Settings.Save();

	return menu;
}
