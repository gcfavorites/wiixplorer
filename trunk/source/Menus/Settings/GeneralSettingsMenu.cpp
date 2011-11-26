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
#include "GeneralSettingsMenu.h"
#include "Prompts/PromptWindows.h"
#include "Settings.h"

static inline int NextPriority(int prio)
{
	switch(prio)
	{
		case 0:
			return 30;
		case 30:
			return 70;
		case 70:
			return 100;
		case 100:
			return 127;
		case 127:
			return 0;
		default:
			return 30;
	}
}

static inline const char * PrioritySynonym(int prio)
{
	switch(prio)
	{
		case 0:
			return tr("Idle");
		case 30:
			return tr("Low");
		case 70:
			return tr("Normal");
		case 100:
			return tr("High");
		case 127:
			return tr("Highest");
		default:
			return NULL;
	}
}

static inline const char * CompressionSynonym(int comp)
{
	switch(comp)
	{
		case -1:
			return tr("(Default)");
		case 0:
			return tr("(Store)");
		case 1:
			return tr("(Best speed)");
		case 9:
			return tr("(Best compression)");
		default:
			return "";
	}
}

GeneralSettingsMenu::GeneralSettingsMenu(GuiFrame *r)
	: SettingsMenu(tr("General Settings"), r)
{
	SetupOptions();
}

GeneralSettingsMenu::~GeneralSettingsMenu()
{
}

void GeneralSettingsMenu::SetupOptions()
{
	int i = 0;

	options.SetName(i++, tr("PDF Processing Zoom"));
	options.SetName(i++, tr("Keyboard Delete Delay"));
	options.SetName(i++, tr("Rumble"));
	options.SetName(i++, tr("Scrolling Speed"));
	options.SetName(i++, tr("Zip Compression Mode"));
	options.SetName(i++, tr("Copy Thread Priority"));
	options.SetName(i++, tr("Copy Thread Background Priority"));
	options.SetName(i++, tr("Show Partition Formatter"));
	options.SetName(i++, tr("Use Both USB Ports"));

	SetOptionValues();
}

void GeneralSettingsMenu::SetOptionValues()
{
	int i = 0;

	options.SetValue(i++, "%0.2f", Settings.PDFLoadZoom);

	options.SetValue(i++, "%i", Settings.KeyboardDeleteDelay);

	if(Settings.Rumble)
		options.SetValue(i++, tr("ON"));
	else
		options.SetValue(i++, tr("OFF"));

	options.SetValue(i++, "%i", Settings.ScrollSpeed);

	options.SetValue(i++, "%i %s", Settings.CompressionLevel, CompressionSynonym(Settings.CompressionLevel));

	options.SetValue(i++, PrioritySynonym(Settings.CopyThreadPrio));

	options.SetValue(i++, PrioritySynonym(Settings.CopyThreadBackPrio));

	if(Settings.ShowFormatter)
		options.SetValue(i++, tr("ON"));
	else
		options.SetValue(i++, tr("OFF"));

	if(Settings.USBPort)
		options.SetValue(i++, tr("ON"));
	else
		options.SetValue(i++, tr("OFF"));

}

void GeneralSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option)
{
	char entered[300];

	switch (option)
	{
		case 0:
			snprintf(entered, sizeof(entered), "%0.2f", Settings.PDFLoadZoom);
			if(OnScreenKeyboard(entered, sizeof(entered)))
			{
				Settings.PDFLoadZoom = atof(entered);
				WindowPrompt(tr("Warning:"), tr("This option could mess up the pdf view."), tr("OK"));
			}
			break;
		case 1:
			snprintf(entered, sizeof(entered), "%i", Settings.KeyboardDeleteDelay);
			if(OnScreenKeyboard(entered, sizeof(entered)))
			{
				Settings.KeyboardDeleteDelay = atoi(entered);
			}
			break;
		case 2:
			Settings.Rumble = (Settings.Rumble+1) % 2;
			break;
		case 3:
			Settings.ScrollSpeed = (Settings.ScrollSpeed+1) % 21;
			break;
		case 4:
			Settings.CompressionLevel++;
			if(Settings.CompressionLevel > 9)
				Settings.CompressionLevel = -1;
			break;
		case 5:
			Settings.CopyThreadPrio = NextPriority(Settings.CopyThreadPrio);
			break;
		case 6:
			Settings.CopyThreadBackPrio = NextPriority(Settings.CopyThreadBackPrio);
			break;
		case 7:
			Settings.ShowFormatter = (Settings.ShowFormatter+1) % 2;
			break;
		case 8:
			Settings.USBPort = 0;//(Settings.USBPort+1) % 2; //TODO: fix in ehci module
			break;
		default:
			break;
	}

	SetOptionValues();
}
