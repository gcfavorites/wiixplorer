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
#include "Prompts/PromptWindows.h"
#include "Settings.h"
#include "sys.h"

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

void BootSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option UNUSED)
{
	if(option == 0)
	{
		char entered[50];
		snprintf(entered, sizeof(entered), "%i", Settings.BootIOS);
		int result = OnScreenKeyboard(entered, sizeof(entered));
		if(result)
		{
			int ios = LIMIT(atoi(entered), 1, 255);
			
			if(!FindTitle(((u64) 0x00000001 << 32ULL) | ios))
			{
				if(WindowPrompt(fmt(tr("Could not find IOS %i"), ios), tr("Are you sure you have that IOS installed?"), tr("Yes"), tr("Cancel")))
					Settings.BootIOS = ios;
			}
			else
				Settings.BootIOS = ios;
		}
	}

	SetOptionValues();
}
