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
#include "Menus/Explorer.h"


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

int MenuGeneralSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("PDF Processing Zoom"));
	options.SetName(i++, tr("Keyboard Delete Delay"));
	options.SetName(i++, tr("Rumble"));
	options.SetName(i++, tr("Scrolling Speed"));
	options.SetName(i++, tr("Zip Compression Mode"));
	options.SetName(i++, tr("Copy Thread Priority"));
	options.SetName(i++, tr("Copy Thread Background Priority"));
	options.SetName(i++, tr("Show Partition Formatter"));
	options.SetName(i++, tr("Use Both USB Ports"));

	SettingsMenu * Menu = new SettingsMenu(tr("Explorer Settings"), &options, MENU_SETTINGS);

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
                char entered[150];
                snprintf(entered, sizeof(entered), "%0.2f", Settings.PDFLoadZoom);
                if(OnScreenKeyboard(entered, 149))
                {
					Settings.PDFLoadZoom = atof(entered);
					WindowPrompt(tr("Warning:"), tr("This option could mess up the pdf view."), tr("OK"));
                }
                break;
            case 1:
                snprintf(entered, sizeof(entered), "%i", Settings.KeyboardDeleteDelay);
                if(OnScreenKeyboard(entered, 149))
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
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++, "%0.2f", Settings.PDFLoadZoom);

            options.SetValue(i++, "%i", Settings.KeyboardDeleteDelay);

            if(Settings.Rumble) options.SetValue(i++, tr("ON"));
            else options.SetValue(i++, tr("OFF"));

            options.SetValue(i++, "%i", Settings.ScrollSpeed);

            options.SetValue(i++, "%i %s", Settings.CompressionLevel, CompressionSynonym(Settings.CompressionLevel));

            options.SetValue(i++, PrioritySynonym(Settings.CopyThreadPrio));

            options.SetValue(i++, PrioritySynonym(Settings.CopyThreadBackPrio));

            if(Settings.ShowFormatter) options.SetValue(i++, tr("ON"));
            else options.SetValue(i++, tr("OFF"));

            if(Settings.USBPort) options.SetValue(i++, tr("ON"));
            else options.SetValue(i++, tr("OFF"));
        }
	}

    delete Menu;

	return menu;
}
