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
#include "ColorSettingsMenu.h"
#include "Settings.h"

ColorSettingsMenu::ColorSettingsMenu(GuiFrame *r)
	: SettingsMenu(tr("Color Settings"), r)
{
	SetupOptions();
}

ColorSettingsMenu::~ColorSettingsMenu()
{
}


void ColorSettingsMenu::SetupOptions()
{
	int i = 0;

	options.SetName(i++, tr("Language"));
	options.SetName(i++, tr("Clock Mode"));
	options.SetName(i++, tr("General Settings"));
	options.SetName(i++, tr("Explorer Settings"));
	options.SetName(i++, tr("Boot Settings"));
	options.SetName(i++, tr("Image Settings"));
	options.SetName(i++, tr("Sound Settings"));
	options.SetName(i++, tr("Network Settings"));
	options.SetName(i++, tr("File Extensions"));
	options.SetName(i++, tr("Controls Settings"));
	options.SetName(i++, tr("Path Setup"));
	options.SetName(i++, tr("Color Settings"));

	SetOptionValues();
}

void ColorSettingsMenu::SetOptionValues()
{
	int i = 0;
}

void ColorSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option)
{

	SetOptionValues();
}
