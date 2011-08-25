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
#include "BootSettingsMenu.h"
#include "Settings.h"

BootSettingsMenu::BootSettingsMenu(GuiFrame *r)
	: SettingsMenu(tr("Boot Settings"), r)
{
	SetupOptions();
}

BootSettingsMenu::~BootSettingsMenu()
{
}

void BootSettingsMenu::SetupOptions()
{
	int i = 0;

	options.SetName(i++, tr("Boot IOS"));

	SetOptionValues();
}

void BootSettingsMenu::SetOptionValues()
{
	int i = 0;

	options.SetValue(i++, "%i", Settings.BootIOS);
}

void BootSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option)
{
	switch(option)
	{
		case 0:
		{
			switch(Settings.BootIOS)
			{
				case 58:
					Settings.BootIOS = 202;
					break;
				case 202:
					Settings.BootIOS = 222;
					break;
				case 222:
					Settings.BootIOS = 223;
					break;
				case 223:
					Settings.BootIOS = 36;
					break;
				case 36:
					Settings.BootIOS = 60;
					break;
				case 60:
					Settings.BootIOS = 61;
					break;
				case 61:
					Settings.BootIOS = 58;
					break;
				default:
					break;
			}
		}
		default:
			break;
	}

	SetOptionValues();
}
