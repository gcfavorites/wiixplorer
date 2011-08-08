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
#include "Menus/Explorer.h"

int MenuExplorerSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Browser Mode"));
	options.SetName(i++, tr("Hide System Files/Folders"));

	SettingsMenu * Menu = new SettingsMenu(tr("Explorer Settings"), &options, MENU_SETTINGS);

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
				Settings.BrowserMode = (Settings.BrowserMode+1) % 2;
				break;
			case 1:
				Settings.HideSystemFiles = (Settings.HideSystemFiles+1) % 2;
				break;
		}

		if(firstRun || ret >= 0)
		{
			i = 0;
			firstRun = false;

			if (Settings.BrowserMode == ICONBROWSER)
				options.SetValue(i++, tr("Icon Mode"));
			else
				options.SetValue(i++, tr("List Mode"));

			if(Settings.HideSystemFiles) options.SetValue(i++, tr("ON"));
			else options.SetValue(i++, tr("OFF"));
		}
	}

	delete Menu;

	return menu;
}
