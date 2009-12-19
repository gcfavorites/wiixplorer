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
#include "filelist.h"
#include "Settings.h"
#include "menu.h"
#include "video.h"
#include "sys.h"

#include "Controls/Resources.h"
#include "Controls/Taskbar.h"

extern Settings Settings;
extern u8 reset;

MainWindow *MainWindow::instance = NULL;

MainWindow::MainWindow()
	: GuiWindow(screenwidth, screenheight) // screenwidth and height are defined in Video.h
{
	guihalt = true;
	exitApplication = false;
	guithread = LWP_THREAD_NULL;

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

	bgImgData = Resources::GetImageData(background_png, background_png_size);
    bgImg = new GuiImage(bgImgData);
	Append(bgImg);

	bgMusic = Resources::GetSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(Settings.MusicVolume);
	bgMusic->SetLoop(1);
	bgMusic->Play(); // startup music

	#ifdef HW_RVL
	pointer[0] = Resources::GetImageData(player1_point_png, player1_point_png_size);
	pointer[1] = Resources::GetImageData(player2_point_png, player2_point_png_size);
	pointer[2] = Resources::GetImageData(player3_point_png, player3_point_png_size);
	pointer[3] = Resources::GetImageData(player4_point_png, player4_point_png_size);
	#endif

	Append(Taskbar::Instance());

    Taskbar::Instance()->SetFocus(0);
}

MainWindow::~MainWindow()
{
	Quit();

	RemoveAll();

	Taskbar::Instance()->DestroyInstance();

	delete bgImg;

	Resources::Remove(bgImgData);

	bgMusic->Stop();
	Resources::Remove(bgMusic);

	for (int i = 0; i < 4; i++) {
		Resources::Remove(pointer[i]);
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
//				delete *itr;
			}
		}
		windows.clear();

		reset = 1;
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

//! THIS AND THE FOLLOWING WILL BE MOVED SOON
void MainWindow::ChangeVolume(int v)
{
    bgMusic->SetVolume(v);
}

void MainWindow::LoadMusic(const u8 * file, u32 filesize, int mode)
{
    ///Stop and remove old music
    if(bgMusic) {
        delete bgMusic;
        bgMusic = NULL;
    }

    bgMusic = new GuiSound(file, filesize, mode);
    bgMusic->SetVolume(Settings.MusicVolume);
    bgMusic->SetLoop(1);
    bgMusic->Play(); /// startup music
}
