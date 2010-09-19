/***************************************************************************
 * Copyright (C) 2009
 * by r-win & Dimok
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
 * MainWindow.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include <unistd.h>

#include "MainWindow.h"
#include "SoundOperations/MusicPlayer.h"
#include "SoundOperations/SoundHandler.hpp"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PromptWindows.h"
#include "Launcher/Channels.h"
#include "Launcher/Applications.h"
#include "Memory/Resources.h"
#include "Controls/Taskbar.h"
#include "Menus/menu_ftpserver.h"
#include "Menus/menu_browsedevice.h"
#include "Menus/Settings/menu_settings.h"
#include "Language/LanguageBrowser.h"
#include "network/networkops.h"
#include "FTPOperations/FTPServer.h"
#include "FileOperations/filebrowser.h"
#include "Controls/IOHandler.hpp"
#include "filelist.h"
#include "Settings.h"
#include "menu.h"
#include "VideoOperations/video.h"
#include "sys.h"

MainWindow *MainWindow::instance = NULL;

MainWindow::MainWindow()
	: GuiWindow(screenwidth, screenheight) // screenwidth and height are defined in Video.h
{
	guihalt = true;
	exitApplication = false;
	guithread = LWP_THREAD_NULL;

	ThreadStack = (u8 *) memalign(32, 32768);

	//!Initialize main GUI handling thread
	LWP_CreateThread (&guithread, UpdateGUI, this, ThreadStack, 32768, LWP_PRIO_HIGHEST);

	//!Initialize the i/o hanlde thread
	IOHandler::Instance();

    //!FTP Server thread
    if(Settings.FTPServer.AutoStart)
        FTPServer::Instance();

    //!Initialize network thread if selected
    if(Settings.AutoConnect)
    {
        InitNetworkThread();
        ResumeNetworkThread();
    }

    GXColor ImgColor[4];
    ImgColor[0] = RGBATOGXCOLOR(Settings.BackgroundUL);
    ImgColor[1] = RGBATOGXCOLOR(Settings.BackgroundUR);
    ImgColor[2] = RGBATOGXCOLOR(Settings.BackgroundBR);
    ImgColor[3] = RGBATOGXCOLOR(Settings.BackgroundBL);

    bgImg = new GuiImage(screenwidth, screenheight, &ImgColor[0]);
	Append(bgImg);

	MusicPlayer::Instance()->SetVolume(Settings.MusicVolume);
	MusicPlayer::Instance()->SetLoop(Settings.BGMLoopMode);
	MusicPlayer::Instance()->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	MusicPlayer::Instance()->SetPosition(30, 230);
	Append(MusicPlayer::Instance());

	standardPointer[0] = new GuiImageData(player1_point_png, player1_point_png_size);
	standardPointer[1] = new GuiImageData(player2_point_png, player2_point_png_size);
	standardPointer[2] = new GuiImageData(player3_point_png, player3_point_png_size);
	standardPointer[3] = new GuiImageData(player4_point_png, player4_point_png_size);

	for (int i = 0; i < 4; i++)
	{
		grabPointer[i] = NULL;
		pointer[i] = standardPointer[i];
	}

	Append(Taskbar::Instance());
}

MainWindow::~MainWindow()
{
	Quit();
	RemoveAll();

	Taskbar::DestroyInstance();
	MusicPlayer::DestroyInstance();
	SoundHandler::DestroyInstance();
	Channels::DestroyInstance();
	IOHandler::DestroyInstance();
    ProgressWindow::DestroyInstance();

	delete bgImg;

	for (int i = 0; i < 4; i++)
	{
	    if(grabPointer[i])
            delete grabPointer[i];
		delete standardPointer[i];
	}
	if(ThreadStack)
        free(ThreadStack);
}

MainWindow * MainWindow::Instance()
{
	if (instance == NULL)
	{
		instance = new MainWindow();
	}
	return instance;
}

void MainWindow::DestroyInstance()
{
    if(instance)
    {
        delete instance;
    }
    instance = NULL;
}

void MainWindow::Quit()
{
	if (!exitApplication)
	{
		// Exit the application if this function is called...
		exitApplication = true;

		ResumeGui(); // Resume the gui, or the thread won't exit
		LWP_JoinThread(guithread, NULL);
		guithread = LWP_THREAD_NULL;

		// Fade out...
		for(int i = 0; i <= 255; i += 15)
		{
			Draw();
            GXColor fadeoutColor = (GXColor){0, 0, 0, i};
			Menu_DrawRectangle(0, 0, 100.0f, screenwidth, screenheight, &fadeoutColor, false, true);
			Menu_Render();
		}
	}
}

void MainWindow::Show()
{
    ResumeGui();
	int currentMenu = MENU_BROWSE_DEVICE;

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
            case MENU_GENERAL_SETTINGS:
                currentMenu = MenuGeneralSettings();
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
            case MENU_COLOR_SETTINGS:
                currentMenu = MenuColorSettings();
                break;
            default: // unrecognized menu
                currentMenu = MenuBrowseDevice();
                break;
        }
	}

	ResumeGui();

	ExitApp();
}

void MainWindow::SetGuiPriority(int prio)
{
    if(prio < 0 || prio > LWP_PRIO_HIGHEST)
        return;

	LWP_SetThreadPriority(guithread, prio);
}

void MainWindow::ResumeGui()
{
	guihalt = false;
	LWP_ResumeThread (guithread);
}

void MainWindow::HaltGui()
{
	guihalt = true;

	while(!LWP_ThreadIsSuspended (guithread))
		usleep(THREAD_SLEEP);
}

void *MainWindow::UpdateGUI(void *arg)
{
	((MainWindow *) arg)->InternalUpdateGUI();
	return NULL;
}

void MainWindow::InternalUpdateGUI()
{
	while (!exitApplication)
	{
		if(guihalt)
		{
			LWP_SuspendThread(guithread);
			usleep(THREAD_SLEEP);
			continue;
		}

        UpdatePads();
		Draw();

		for (int i = 3; i >= 0; i--)
		{
			if (userInput[i].wpad->ir.valid)
			{
				Menu_DrawImg(pointer[i]->GetImage(), pointer[i]->GetWidth(),
							 pointer[i]->GetHeight(), GX_TF_RGBA8,
							 userInput[i].wpad->ir.x-pointer[i]->GetWidth()/2,
							 userInput[i].wpad->ir.y-pointer[i]->GetHeight()/2, 100.0f,
							 userInput[i].wpad->ir.angle, 1, 1, 255, -100, screenwidth+100, -100, screenheight+100);
			}
		}

		Menu_Render();

		for (int i = 0; i < 4; i++)
		{
			Update(&userInput[i]);
		}
	}
}

void MainWindow::SetGrabPointer(int i)
{
	if (i < 0 || i > 3)
		return;

    if(!grabPointer[i])
    {
        if(i == 0)
            grabPointer[i] = new GuiImageData(player1_grab_png, player1_grab_png_size);
        else if(i == 1)
            grabPointer[i] = new GuiImageData(player2_grab_png, player2_grab_png_size);
        else if(i == 2)
            grabPointer[i] = new GuiImageData(player3_grab_png, player3_grab_png_size);
        else
            grabPointer[i] = new GuiImageData(player4_grab_png, player4_grab_png_size);
    }

	pointer[i] = grabPointer[i];
}

void MainWindow::ResetPointer(int i)
{
	if (i < 0 || i > 3)
		return;

	pointer[i] = standardPointer[i];

    if(grabPointer[i])
        delete grabPointer[i];
	grabPointer[i] = NULL;
}
