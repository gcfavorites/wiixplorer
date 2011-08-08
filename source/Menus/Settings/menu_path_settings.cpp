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
#include "TextOperations/FontSystem.h"
#include "Prompts/PromptWindows.h"
#include "Menus/menu_get_path.hpp"

using namespace std;

int MenuPathSetup()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	int choice = -1;
	bool firstRun = true;
	char entered[150];

	OptionList options;
	options.SetName(i++, tr("Update (App) Path"));
	options.SetName(i++, tr("Apps Path"));
	options.SetName(i++, tr("WiiMC Path"));
	options.SetName(i++, tr("MPlayerCE Path"));
	options.SetName(i++, tr("Customfont Path"));
	options.SetName(i++, tr("Screenshot Path"));
	options.SetName(i++, tr("Temporary Path"));
	options.SetName(i++, tr("URL List Path"));
	options.SetName(i++, tr("Delete Temp Path on Exit"));

	SettingsMenu * Menu = new SettingsMenu(tr("Path Setup"), &options, MENU_SETTINGS);

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
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					Menu->SetState(STATE_DISABLED);
					string Path;
					menu = MenuGetPath(Path);
					Menu->SetState(STATE_DEFAULT);

					if(Path.length() > 0)
					{
						if (Path[Path.length()-1] != '/')
							Path.append("/");
						snprintf(Settings.UpdatePath, sizeof(Settings.UpdatePath), "%s", Path.c_str());
					}
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.UpdatePath);
					if(OnScreenKeyboard(entered, 149)) {
						if (entered[strlen(entered)-1] != '/')
							strcat(entered, "/");
						snprintf(Settings.UpdatePath, sizeof(Settings.UpdatePath), "%s", entered);
						WindowPrompt(tr("Update Path changed."), 0, tr("OK"));
					}
				}
				break;
			case 1:
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					Menu->SetState(STATE_DISABLED);
					string Path;
					menu = MenuGetPath(Path);
					Menu->SetState(STATE_DEFAULT);

					if(Path.length() > 0)
					{
						if (Path[Path.length()-1] != '/')
							Path.append("/");
						snprintf(Settings.AppPath, sizeof(Settings.AppPath), "%s", Path.c_str());
					}
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.AppPath);
					if(OnScreenKeyboard(entered, 149))
					{
						if (entered[strlen(entered)-1] != '/')
							strcat(entered, "/");
						snprintf(Settings.AppPath, sizeof(Settings.AppPath), "%s", entered);
						WindowPrompt(tr("AppPath changed"), 0, tr("OK"));
					}
				}
				break;
			case 2:
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					Menu->SetState(STATE_DISABLED);
					string Path;
					menu = MenuGetPath(Path);
					Menu->SetState(STATE_DEFAULT);

					if(Path.length() > 0)
					{
						if (Path[Path.length()-1] != '/')
							Path.append("/");

						Path.append("boot.dol");

						snprintf(Settings.WiiMCPath, sizeof(Settings.WiiMCPath), "%s", Path.c_str());
					}
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.WiiMCPath);
					if(OnScreenKeyboard(entered, 149))
					{
						if (entered[strlen(entered)-1] != '/')
							strcat(entered, "/");
						if(strstr(entered, "boot.dol") == 0)
							strcat(entered, "boot.dol");
						snprintf(Settings.WiiMCPath, sizeof(Settings.WiiMCPath), "%s", entered);
						WindowPrompt(tr("WiiMC path changed"), 0, tr("OK"));
					}
				}
				break;
			case 3:
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					Menu->SetState(STATE_DISABLED);
					string Path;
					menu = MenuGetPath(Path);
					Menu->SetState(STATE_DEFAULT);

					if(Path.length() > 0)
					{
						if (Path[Path.length()-1] != '/')
							Path.append("/");

						Path.append("boot.dol");

						snprintf(Settings.MPlayerPath, sizeof(Settings.MPlayerPath), "%s", Path.c_str());
					}
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.MPlayerPath);
					if(OnScreenKeyboard(entered, 149))
					{
						if (entered[strlen(entered)-1] != '/')
							strcat(entered, "/");
						if(strstr(entered, "boot.dol") == 0)
							strcat(entered, "boot.dol");
						snprintf(Settings.MPlayerPath, sizeof(Settings.MPlayerPath), "%s", entered);
						WindowPrompt(tr("MPlayerCE path changed"), 0, tr("OK"));
					}
				}
				break;
			case 4:
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					WindowPrompt(tr("Notice:"), tr("Browsing for font files must be done in the main browser."), tr("OK"));
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.CustomFontPath);
					if(OnScreenKeyboard(entered, 149))
					{
						snprintf(Settings.CustomFontPath, sizeof(Settings.CustomFontPath), "%s", entered);
						bool result = SetupDefaultFont(Settings.CustomFontPath);
						if(result)
							WindowPrompt(tr("Fontpath changed."), tr("The new font is loaded."), tr("OK"));
						else
							WindowPrompt(tr("Fontpath changed."), tr("The new font could not be loaded."), tr("OK"));
					}
				}
				break;
			case 5:
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					Menu->SetState(STATE_DISABLED);
					string Path;
					menu = MenuGetPath(Path);
					Menu->SetState(STATE_DEFAULT);

					if(Path.length() > 0)
					{
						if (Path[Path.length()-1] != '/')
							Path.append("/");
						snprintf(Settings.ScreenshotPath, sizeof(Settings.ScreenshotPath), "%s", Path.c_str());
					}
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.ScreenshotPath);
					if(OnScreenKeyboard(entered, 149))
					{
						if (entered[strlen(entered)-1] != '/')
							strcat(entered, "/");
						snprintf(Settings.ScreenshotPath, sizeof(Settings.ScreenshotPath), "%s", entered);
					}
				}
				break;
			case 6:
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					Menu->SetState(STATE_DISABLED);
					string Path;
					menu = MenuGetPath(Path);
					Menu->SetState(STATE_DEFAULT);

					if(Path.length() > 0)
					{
						if (Path[Path.length()-1] != '/')
							Path.append("/");
						snprintf(Settings.TempPath, sizeof(Settings.TempPath), "%s", Path.c_str());
					}
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.TempPath);
					if(OnScreenKeyboard(entered, 149))
					{
						if (entered[strlen(entered)-1] != '/')
							strcat(entered, "/");
						snprintf(Settings.TempPath, sizeof(Settings.TempPath), "%s", entered);
					}
				}
				break;
			case 7:
				choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
				if(choice == 1)
				{
					Menu->SetState(STATE_DISABLED);
					string Path;
					menu = MenuGetPath(Path);
					Menu->SetState(STATE_DEFAULT);

					if(Path.length() > 0)
					{
						if (Path[Path.length()-1] != '/')
							Path.append("/");
						snprintf(Settings.LinkListPath, sizeof(Settings.LinkListPath), "%sURL_List.xml", Path.c_str());
					}
				}
				else if(choice == 2)
				{
					snprintf(entered, sizeof(entered), "%s", Settings.LinkListPath);
					if(OnScreenKeyboard(entered, 149))
					{
						if (entered[strlen(entered)-1] == '/')
							strcat(entered, "URL_List.xml");
						else if(entered[strlen(entered)-1] != 'l' ||
								entered[strlen(entered)-1] != 'm' ||
								entered[strlen(entered)-1] != 'x')
							strcat(entered, ".xml");

						snprintf(Settings.LinkListPath, sizeof(Settings.LinkListPath), "%s", entered);
						WindowPrompt(tr("Update Path changed."), 0, tr("OK"));
					}
				}
				break;
			case 8:
				Settings.DeleteTempPath = (Settings.DeleteTempPath+1) % 2;
				break;
			default:
				break;
		}

		if(firstRun || ret >= 0)
		{
			i = 0;
			firstRun = false;

			options.SetValue(i++, "%s", Settings.UpdatePath);

			options.SetValue(i++, "%s", Settings.AppPath);

			options.SetValue(i++, "%s", Settings.WiiMCPath);

			options.SetValue(i++, "%s", Settings.MPlayerPath);

			options.SetValue(i++, "%s", Settings.CustomFontPath);

			options.SetValue(i++, "%s", Settings.ScreenshotPath);

			options.SetValue(i++, "%s", Settings.TempPath);

			options.SetValue(i++, "%s", Settings.LinkListPath);

			if(Settings.DeleteTempPath == 1) options.SetValue(i++, tr("ON"));
			else options.SetValue(i++, tr("OFF"));
		}
	}

	delete Menu;

	Settings.Save();

	return menu;
}
