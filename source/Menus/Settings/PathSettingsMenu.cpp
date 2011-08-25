/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "PathSettingsMenu.h"
#include "Prompts/PromptWindows.h"
#include "Settings.h"

PathSettingsMenu::PathSettingsMenu(GuiFrame *r)
	: SettingsMenu(tr("Path Settings"), r)
{
	SetupOptions();
}

PathSettingsMenu::~PathSettingsMenu()
{
}

void PathSettingsMenu::SetupOptions()
{
	int i = 0;

	options.SetName(i++, tr("Update (App) Path"));
	options.SetName(i++, tr("Apps Path"));
	options.SetName(i++, tr("WiiMC Path"));
	options.SetName(i++, tr("MPlayerCE Path"));
	options.SetName(i++, tr("Customfont Path"));
	options.SetName(i++, tr("Screenshot Path"));
	options.SetName(i++, tr("Temporary Path"));
	options.SetName(i++, tr("URL List Path"));
	options.SetName(i++, tr("Delete Temp Path on Exit"));
}

void PathSettingsMenu::SetOptionValues()
{
	int i = 0;

	options.SetValue(i++, "%s", Settings.UpdatePath);

	options.SetValue(i++, "%s", Settings.AppPath);

	options.SetValue(i++, "%s", Settings.WiiMCPath);

	options.SetValue(i++, "%s", Settings.MPlayerPath);

	options.SetValue(i++, "%s", Settings.CustomFontPath);

	options.SetValue(i++, "%s", Settings.ScreenshotPath);

	options.SetValue(i++, "%s", Settings.TempPath);

	options.SetValue(i++, "%s", Settings.LinkListPath);

	if(Settings.DeleteTempPath)
		options.SetValue(i++, tr("ON"));
	else
		options.SetValue(i++, tr("OFF"));
}

void PathSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option)
{
	switch (option)
	{
		case 0:
			ChangePath(Settings.UpdatePath, sizeof(Settings.UpdatePath));
			break;
		case 1:
			ChangePath(Settings.AppPath, sizeof(Settings.AppPath));
			break;
		case 2:
			ChangePath(Settings.WiiMCPath, sizeof(Settings.WiiMCPath));
			break;
		case 3:
			ChangePath(Settings.MPlayerPath, sizeof(Settings.MPlayerPath));
			break;
		case 4:
			ChangePath(Settings.CustomFontPath, sizeof(Settings.CustomFontPath));
			break;
		case 5:
			ChangePath(Settings.ScreenshotPath, sizeof(Settings.ScreenshotPath));
			break;
		case 6:
			ChangePath(Settings.TempPath, sizeof(Settings.TempPath));
			break;
		case 7:
			ChangePath(Settings.LinkListPath, sizeof(Settings.LinkListPath));
			break;
		case 8:
			Settings.DeleteTempPath = (Settings.DeleteTempPath+1) % 2;
			break;
		default:
			break;
	}

	SetOptionValues();
}

void PathSettingsMenu::ChangePath(char *setPath, int size)
{
	char entered[300];

	int choice = WindowPrompt(tr("How do you want to change the path?"), 0, tr("Browse"), tr("Enter"), tr("Cancel"));
	if(choice == 1)
	{
		std::string Path;
		//TODO: menu = MenuGetPath(Path);

		if(Path.length() > 0)
		{
			if (Path[Path.length()-1] != '/')
				Path.append("/");
			snprintf(setPath, size, "%s", Path.c_str());
		}
	}
	else if(choice == 2)
	{
		snprintf(entered, sizeof(entered), "%s", setPath);
		if(OnScreenKeyboard(entered, sizeof(entered)))
		{
			if (entered[strlen(entered)-1] != '/')
				strcat(entered, "/");
			snprintf(setPath, size, "%s", entered);
			WindowPrompt(tr("Path changed."), 0, tr("OK"));
		}
	}
}
