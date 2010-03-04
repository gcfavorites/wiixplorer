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
#include "libwiigui/gui_bgm.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PromptWindows.h"
#include "network/networkops.h"
#include "FileOperations/filebrowser.h"
#include "filelist.h"
#include "Settings.h"
#include "menu.h"
#include "video.h"
#include "sys.h"

#include "Memory/Resources.h"
#include "Controls/Taskbar.h"

MainWindow *MainWindow::instance = NULL;

MainWindow::MainWindow()
	: GuiWindow(screenwidth, screenheight) // screenwidth and height are defined in Video.h
{
	guihalt = true;
	exitApplication = false;
	guithread = LWP_THREAD_NULL;

	//!Initialize main GUI handling thread
	LWP_CreateThread (&guithread, UpdateGUI, this, NULL, 0, LWP_PRIO_HIGHEST);

	//!Initalize the progress thread
	InitProgressThread();
	StopProgress();

    //!Initialize network thread if selected
    if(Settings.AutoConnect)
    {
        InitNetworkThread();
        ResumeNetworkThread();
    }

	bgImgData = Resources::GetImageData(background_png, background_png_size);
    bgImg = new GuiImage(bgImgData);
	Append(bgImg);

	if(GuiBGM::Instance()->Load(Settings.MusicPath, true))
	{
	    GuiBGM::Instance()->ParsePath(Settings.MusicPath);
	}
	GuiBGM::Instance()->SetVolume(Settings.MusicVolume);
	GuiBGM::Instance()->SetLoop(Settings.BGMLoopMode);
	GuiBGM::Instance()->Play(); // startup music

	standardPointer[0] = new GuiImageData(player1_point_png, player1_point_png_size);
	standardPointer[1] = new GuiImageData(player2_point_png, player2_point_png_size);
	standardPointer[2] = new GuiImageData(player3_point_png, player3_point_png_size);
	standardPointer[3] = new GuiImageData(player4_point_png, player4_point_png_size);

	grabPointer[0] = new GuiImageData(player1_grab_png, player1_grab_png_size);
	grabPointer[1] = new GuiImageData(player2_grab_png, player2_grab_png_size);
	grabPointer[2] = new GuiImageData(player3_grab_png, player3_grab_png_size);
	grabPointer[3] = new GuiImageData(player4_grab_png, player4_grab_png_size);

	for (int i = 0; i < 4; i++)
		pointer[i] = standardPointer[i];

	Append(Taskbar::Instance());
}

MainWindow::~MainWindow()
{
	Quit();

	RemoveAll();

	Taskbar::Instance()->DestroyInstance();

	delete bgImg;

	Resources::Remove(bgImgData);

	GuiBGM::Instance()->DestroyInstance();

	for (int i = 0; i < 4; i++)
	{
		delete grabPointer[i];
		delete standardPointer[i];
	}
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

void MainWindow::Append(GuiElement *e)
{
    GuiWindow::Append(e);
}

void MainWindow::Append(GuiWindow *w)
{
    GuiWindow::Append(w);
}

void MainWindow::Quit()
{
	if (!exitApplication)
	{
	    //shutdown progress thread
	    ExitProgressThread();

		// Exit the application if this function is called...
		exitApplication = true;

		ResumeGui(); // Resume the gui, or the thread won't exit
		LWP_JoinThread(guithread, NULL);
		guithread = LWP_THREAD_NULL;

		// Fade out...
		for(int i = 0; i <= 255; i += 15)
		{
			Draw();
			Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, i},1);
			Menu_Render();
		}

		// Close all windows
		for (std::list<GuiWindow *>::iterator itr = windows.begin(); itr != windows.end(); itr++)
		{
			if (*itr != NULL) {
				Remove(*itr);
			}
		}
		windows.clear();
	}
}

void MainWindow::Show()
{
    ResumeGui();
    MainMenu(MENU_BROWSE_DEVICE);
}

void MainWindow::AddWindow(GuiWindow *window)
{
	Append(window);
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

		#ifdef HW_RVL
		for (int i = 3; i >= 0; i--)
		{
			if (userInput[i].wpad->ir.valid)
			{
				Menu_DrawImg(userInput[i].wpad->ir.x-pointer[i]->GetWidth()/2,
							 userInput[i].wpad->ir.y-pointer[i]->GetHeight()/2,
							 pointer[i]->GetWidth(), pointer[i]->GetHeight(),
							 pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255);
			}
		}
		#endif

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

	pointer[i] = grabPointer[i];
}

void MainWindow::ResetPointer(int i)
{
	if (i < 0 || i > 3)
		return;

	pointer[i] = standardPointer[i];
}
