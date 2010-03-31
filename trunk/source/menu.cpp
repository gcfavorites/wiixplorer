 /****************************************************************************
 * Copyright (C) 2009
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
 * Original Template of LibWiiGui
 * Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libwiigui/gui.h"
#include "SoundOperations/gui_bgm.h"
#include "Controls/Clipboard.h"
#include "network/ChangeLog.h"
#include "Menus/Explorer.h"
#include "Menus/menu_settings.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "filelist.h"
#include "Prompts/PromptWindows.h"
#include "network/networkops.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "sys.h"

bool boothomebrew = false;
int curDevice = 0;

static bool firsttimestart = true;

extern u32 launchServer(char *drive, int status, bool stop) ;
extern int ServerFtpGui();

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
void ResumeGui()
{
	MainWindow::Instance()->ResumeGui();
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
void HaltGui()
{
	MainWindow::Instance()->HaltGui();
}

/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/
static int MenuBrowseDevice()
{
    if(firsttimestart  && Settings.MountMethod >= SMB1 && Settings.MountMethod <= SMB4 && !IsNetworkInit())
    {
        if(WaitSMBConnect() < 2)
            ShowError(tr("Could not connect to the network"));
        firsttimestart = false;
    }

	int menu = MENU_NONE;

    Explorer * Explorer_1 = new Explorer(curDevice);

    MainWindow::Instance()->Append(Explorer_1);
    ResumeGui();

    while(menu == MENU_NONE)
    {
	    usleep(THREAD_SLEEP);

        if(shutdown == 1)
            Sys_Shutdown();

        else if(reset == 1)
            Sys_Reboot();

        menu = Explorer_1->GetMenuChoice();

        if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();
    }

    delete Explorer_1;
    Explorer_1 = NULL;
	ResumeGui();

	return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void MainMenu(int menu)
{
	int currentMenu = menu;

	curDevice = Settings.MountMethod;

	while(currentMenu != MENU_EXIT)
	{
	    Taskbar::Instance()->ResetState();

		switch (currentMenu)
		{
			case MENU_BROWSE_DEVICE:
				currentMenu = MenuBrowseDevice();
				break;
			case MENU_SETTINGS:
				currentMenu = MenuSettings();
				break;
			case MENU_NETWORK_SETTINGS:
				currentMenu = MenuNetworkSettings();
				break;
			case MENU_FTP:
				currentMenu = ServerFtpGui();
				break;
			default: // unrecognized menu
				currentMenu = MenuBrowseDevice();
				break;
		}
	}

	ResumeGui();

	char argument[MAXPATHLEN];
	snprintf(argument, sizeof(argument), "%s", Clipboard::Instance()->GetItemPath(Clipboard::Instance()->GetItemcount()-1));

	ExitApp();

    if(boothomebrew)
        BootHomebrew(argument);
}
