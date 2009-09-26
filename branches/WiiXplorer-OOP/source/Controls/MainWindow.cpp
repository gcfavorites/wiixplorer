/***************************************************************************
 * Copyright (C) 2009
 * by r-win
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
#include "Prompts/ProgressWindow.h"
#include "network/networkops.h"
#include "filebrowser.h"
#include "foldersize.h"
#include "Taskbar.h"
#include "filelist.h"
#include "Settings.h"
#include "menu.h"
#include "video.h"
#include "sys.h"

extern Settings Settings;

MainWindow *MainWindow::instance = NULL;

MainWindow::MainWindow()
	: GuiWindow(screenwidth, screenheight) // screenwidth and height are defined in Video.h
{
	exitApplication = false;

	bgImgData = new GuiImageData(background_png);
    bgImg = new GuiImage(bgImgData);
	Append(bgImg);

	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(Settings.MusicVolume);
	bgMusic->SetLoop(1);
	bgMusic->Play(); // startup music

	Append(Taskbar::Instance());

	#ifdef HW_RVL
	pointer[0] = new GuiImageData(player1_point_png);
	pointer[1] = new GuiImageData(player2_point_png);
	pointer[2] = new GuiImageData(player3_point_png);
	pointer[3] = new GuiImageData(player4_point_png);
	#endif

	//!Initialize main GUI handling thread
	LWP_CreateThread (&guithread, UpdateGUI, this, NULL, 0, 70);

	//!Initalize the progress thread
	InitProgressThread();
	StopProgress();

    //!Initialize network thread if selected
    InitNetworkThread();
    if(Settings.AutoConnect == on)
        ResumeNetworkThread();
    else
        HaltNetworkThread();

    //!Initialize GetSizeThread for Properties
    InitGetSizeThread();
    StopSizeGain();

    //!Initialize Parsethread for browser
    InitParseThread();
}

MainWindow::~MainWindow()
{
	bgMusic->Stop();
	delete bgMusic;

	delete bgImg;
	delete bgImgData;

	#ifdef HW_RVL
	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];
	#endif
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
        instance->Quit();
        delete instance;
    }
    Taskbar::DestroyInstance();
    instance = NULL;
}

void MainWindow::Quit()
{
	// Exit the application if this function is called...
	exitApplication = true;

	LWP_JoinThread(guithread, NULL);
	guithread = LWP_THREAD_NULL;

	for(int i = 0; i < 255; i += 15)
	{
		Draw();
		Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, i},1);
		Menu_Render();
	}
}

void MainWindow::Show()
{
    ResumeGui();
    MainMenu();
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
	int i;

	while (!exitApplication)
	{
		if(guihalt)
		{
			LWP_SuspendThread(guithread);
			usleep(THREAD_SLEEP);
			continue;
		}

		Draw();

		#ifdef HW_RVL
		for (i = 3; i >= 0; i--)
		{
			if (userInput[i].wpad.ir.valid)
			{
				Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48,
					96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, 1, 1, 255);
			}
		}
		#endif

		Menu_Render();

		for(i=0; i < 4; i++)
		{
			Update(&userInput[i]);
		}
	}
}
