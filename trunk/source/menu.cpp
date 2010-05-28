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
 * for WiiXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SoundOperations/MusicPlayer.h"
#include "Controls/Clipboard.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Language/LanguageBrowser.h"
#include "Menus/menu_ftpserver.h"
#include "Menus/menu_settings.h"
#include "Menus/menu_browsedevice.h"
#include "Menus/menu_controls_settings.h"
#include "Menus/menu_file_extensions.h"
#include "menu.h"
#include "sys.h"

bool boothomebrew = false;
int curDevice = 0;

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
 * MainMenu
 ***************************************************************************/
void MainMenu(int menu)
{
	int currentMenu = menu;

	curDevice = Settings.MountMethod;

	while(currentMenu != MENU_EXIT)
	{
	    Taskbar::Instance()->ResetState();

        switch(currentMenu)
        {
            case MENU_BROWSE_DEVICE:
                currentMenu = MenuBrowseDevice();
                break;
            case MENU_SETTINGS:
                currentMenu = MenuSettings();
                break;
            case MENU_LANGUAGE_BROWSE:
                currentMenu = LanguageBrowser();
                break;
            case MENU_NETWORK_SETTINGS:
                currentMenu = MenuNetworkSettings();
                break;
            case MENU_FTP:
                currentMenu = MenuFTPServer();
                break;
            case MENU_EXPLORER_SETTINGS:
                currentMenu = MenuExplorerSettings();
                break;
            case MENU_IMAGE_SETTINGS:
                currentMenu = MenuImageSettings();
                break;
            case MENU_SOUND_SETTINGS:
                currentMenu = MenuSoundSettings();
                break;
            case MENU_BOOT_SETTINGS:
                currentMenu = MenuBootSettings();
                break;
            case MENU_PATH_SETUP:
                currentMenu = MenuPathSetup();
                break;
            case MENU_SMB_SETTINGS:
                currentMenu = MenuSMBSettings();
                break;
            case MENU_FTPCLIENT_SETTINGS:
                currentMenu = MenuFTPClientSettings();
                break;
            case MENU_FTPSERVER_SETTINGS:
                currentMenu = MenuFTPServerSettings();
                break;
            case MENU_CONTROLS_SETTINGS:
                currentMenu = MenuControlsSettings();
                break;
            case MENU_FILE_EXTENSIONS:
                currentMenu = MenuFileExtensions();
                break;
            default: // unrecognized menu
                currentMenu = MenuBrowseDevice();
                break;
        }
	}

	ResumeGui();

	ExitApp();

    if(boothomebrew)
        BootHomebrew();
}
