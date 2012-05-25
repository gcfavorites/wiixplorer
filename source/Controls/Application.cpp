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
#include "Application.h"
#include "Menus/Explorer.h"
#include "DiskOperations/di2.h"
#include "SoundOperations/MusicPlayer.h"
#include "Memory/Resources.h"
#include "Controls/Taskbar.h"
#include "network/networkops.h"
#include "DeviceControls/DeviceHandler.hpp"
#include "TextOperations/FontSystem.h"
#include "FTPOperations/FTPServer.h"
#include "Prompts/HomeMenu.h"
#include "Prompts/ProgressWindow.h"
#include "Settings.h"
#include "VideoOperations/video.h"
#include "ThreadedTaskHandler.hpp"
#include "System/IosLoader.h"
#include "Memory/mem2.h"
#include "Settings.h"
#include "audio.h"
#include "input.h"
#include "sys.h"

Application *Application::instance = NULL;
bool Application::exitApplication = false;

Application::Application()
	: GuiFrame(screenwidth, screenheight) // screenwidth and height are defined in Video.h
	, DeleteQueue(NULL)
{
	GXColor ImgColor[4];
	ImgColor[0] = RGBATOGXCOLOR(Settings.BackgroundUL);
	ImgColor[1] = RGBATOGXCOLOR(Settings.BackgroundUR);
	ImgColor[2] = RGBATOGXCOLOR(Settings.BackgroundBR);
	ImgColor[3] = RGBATOGXCOLOR(Settings.BackgroundBL);

	bgImg = new GuiImage(screenwidth, screenheight, &ImgColor[0]);

	//! Setup WiiMote Pointers
	standardPointer[0] = Resources::GetImageData("player1_point.png");
	standardPointer[1] = Resources::GetImageData("player2_point.png");
	standardPointer[2] = Resources::GetImageData("player3_point.png");
	standardPointer[3] = Resources::GetImageData("player4_point.png");

	for (int i = 0; i < 4; i++)
	{
		grabPointer[i] = NULL;
		pointer[i] = standardPointer[i];
	}
}

Application::~Application()
{
	RemoveAll();
	delete bgImg;
	delete btnHome;

	for (int i = 0; i < 4; i++)
	{
		if(grabPointer[i])
			Resources::Remove(grabPointer[i]);
		Resources::Remove(standardPointer[i]);
	}

	LWP_MutexDestroy(m_mutex);
}

void Application::quit()
{
	// Fade out...
	for(int i = 0; i <= 255; i += 15)
	{
		Draw();
		GXColor fadeoutColor = (GXColor){0, 0, 0, i};
		Menu_DrawRectangle(0, 0, 100.0f, screenwidth, screenheight, &fadeoutColor, false, true);
		Menu_Render();
	}

	exitApplication = true;
}

void Application::hide()
{
	RemoveAll();
}

void Application::show()
{
	Append(bgImg);
	Append(MusicPlayer::Instance());
	Append(ProgressWindow::Instance());
	//! Append taskbar instance
	Append(Taskbar::Instance());
}

void Application::exec()
{
	while(!exitApplication)
	{
		updateEvents();
	}

	ExitApp();
}

void Application::updateEvents()
{
	if(shutdown) {
		Sys_Shutdown();
	}
	else if(reset) {
		Sys_Reboot();
	}

	//! first update the inputs
	UpdatePads();

	//! update the gui elements with new inputs
	for (int i = 0; i < 4; i++)
	{
		if(!updateOnlyElement.empty())
			updateOnlyElement.back()->Update(&userInput[i]);
		else
			Update(&userInput[i]);

		//! always update the home menu, everywhere
		btnHome->Update(&userInput[i]);
	}

	//! render everything
	Draw();

	//! render wii mote pointer always last and on top
	for (int i = 3; i >= 0; i--)
	{
		if (userInput[i].wpad->ir.valid)
		{
			Menu_DrawImg(pointer[i]->GetImage(), pointer[i]->GetWidth(),
						 pointer[i]->GetHeight(), GX_TF_RGBA8,
						 userInput[i].wpad->ir.x-pointer[i]->GetWidth()/2,
						 userInput[i].wpad->ir.y-pointer[i]->GetHeight()/2, 100.0f,
						 userInput[i].wpad->ir.angle, 1, 1, 255);
		}
	}

	Menu_Render();

	//! delete elements that were queued for delete after the rendering is done
	if(DeleteQueue)
	{
		LWP_MutexLock(m_mutex);
		while(DeleteQueue)
		{
			ElementList *tmp = DeleteQueue;
			DeleteQueue = DeleteQueue->next;
			delete tmp->element;
			delete tmp;
		}
		LWP_MutexUnlock(m_mutex);
	}
}

void Application::SetGrabPointer(int i)
{
	if (i < 0 || i > 3)
		return;

	if(!grabPointer[i])
	{
		if(i == 0)
			grabPointer[i] = Resources::GetImageData("player1_grab.png");
		else if(i == 1)
			grabPointer[i] = Resources::GetImageData("player2_grab.png");
		else if(i == 2)
			grabPointer[i] = Resources::GetImageData("player3_grab.png");
		else
			grabPointer[i] = Resources::GetImageData("player4_grab.png");
	}

	pointer[i] = grabPointer[i];
}

void Application::ResetPointer(int i)
{
	if (i < 0 || i > 3)
		return;

	pointer[i] = standardPointer[i];

	if(grabPointer[i])
		Resources::Remove(grabPointer[i]);
	grabPointer[i] = NULL;
}

void Application::PushForDelete(GuiElement *e)
{
	if(!e)
		return;

	LWP_MutexLock(m_mutex);

	if(!DeleteQueue)
	{
		DeleteQueue = new ElementList;
		DeleteQueue->element = e;
		DeleteQueue->next = 0;
		LWP_MutexUnlock(m_mutex);
		return;
	}

	ElementList *list = DeleteQueue;

	while(list->next)
		list = list->next;

	list->next = new ElementList;
	list->next->element = e;
	list->next->next = 0;

	LWP_MutexUnlock(m_mutex);
}

/***********************************************************
 * Main application initialization.
 *
 * Called only once at the beginning of the application.
 ***********************************************************/
void Application::init(void)
{
	InitGecko();		// Initialize stdout/stderr and gecko output
	MEM2_init(52);		// Initialize 52 MB (max is 53469152 bytes though)
	MagicPatches(1);	// We all love magic
	Settings.EntraceIOS = (u8) IOS_GetVersion();	// remember entrace IOS

	//! TODO: clean and split this up

	//!********************************************
	//! Main init routine
	//!********************************************
	//! setup default font for initialization process
	SetupDefaultFont(NULL);
	//! setup background image and class size as they might be changed in video init
	this->SetSize(screenwidth, screenheight);
	bgImg->SetSize(screenwidth, screenheight);
	//! Temporary prompt window to notify the user about the loading process
	PromptWindow window("Initializing...", "Mounting devices");
	window.SetParent(this);
	bool effectStop = false;
	while(!effectStop)
	{
		//! render last frame when effect is 0 too
		effectStop = window.GetEffect() == 0;
		//! render prompt
		bgImg->Draw();
		window.Draw();
		Menu_Render();
	}

	//! render prompt
	bgImg->Draw();
	window.Draw();
	Menu_Render();

	//! Initalise the SD/USB devices
	DeviceHandler::Instance()->MountSD();
	DeviceHandler::Instance()->MountAllUSB();

	//! try loading the settings files
	bool bSettingsLoaded = Settings.Load();

	if(Settings.EntraceIOS != Settings.BootIOS)
	{
		//! notify user that we are about to reload IOS
		window.SetMessage(fmt("Reloading into IOS %i", Settings.BootIOS));
		//! render prompt
		bgImg->Draw();
		window.Draw();
		Menu_Render();

		//! load IOS
		IosLoader::ReloadAppIos(Settings.BootIOS);

		//! if settings were not loaded before, reloading here again.
		if(!bSettingsLoaded)
			Settings.Load();
	}

	//! notify user about new action
	window.SetMessage("Loading resources");
	//! render prompt
	bgImg->Draw();
	window.Draw();
	Menu_Render();

	//! init rest of the devices
	SetupPads();		// Initialize input
	InitAudio();		// Initialize audio
	Sys_Init();			// Initialize shutdown/reset buttons
	DI2_Init();			//Init DVD Driver
	ISFS_Initialize();	// ISFS init
	DeviceHandler::Instance()->MountGCA();
	DeviceHandler::Instance()->MountGCB();
	DeviceHandler::Instance()->MountNAND();
	Settings.LoadLanguage(Settings.LanguagePath);
	SetupPDFFontPath(Settings.UpdatePath);
	SetupDefaultFont(Settings.CustomFontPath);

	//! Set UTF 8 locale
	setlocale(LC_CTYPE, "C-UTF-8");
	setlocale(LC_MESSAGES, "C-UTF-8");

	//! FTP Server thread
	if(Settings.FTPServer.AutoStart)
		FTPServer::Instance();

	//! Initialize network thread if selected
	if(Settings.AutoConnect)
	{
		InitNetworkThread();
		ResumeNetworkThread();
	}

	//! Initialize the task thread
	ThreadedTaskHandler::Instance();

	//!********************************************
	//! class internal things
	//!********************************************
	LWP_MutexInit(&m_mutex, true);

	//! resetup BG color
	GXColor *bgColor = GetBGColorPtr();
	bgColor[0] = RGBATOGXCOLOR(Settings.BackgroundUL);
	bgColor[1] = RGBATOGXCOLOR(Settings.BackgroundUR);
	bgColor[2] = RGBATOGXCOLOR(Settings.BackgroundBR);
	bgColor[3] = RGBATOGXCOLOR(Settings.BackgroundBL);

	//! Setup the music player
	MusicPlayer::Instance()->SetVolume(Settings.MusicVolume);
	MusicPlayer::Instance()->SetLoop(Settings.BGMLoopMode);
	MusicPlayer::Instance()->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	MusicPlayer::Instance()->SetPosition(30, 230);

	//! Fade window out
	window.SetEffect(EFFECT_FADE, -20);
	effectStop = false;
	while(!effectStop)
	{
		//! render last frame when effect is 0 too
		effectStop = window.GetEffect() == 0;
		//! render prompt
		bgImg->Draw();
		window.Draw();
		Menu_Render();
	}
	//! remove parant as otherwise prompt window would slide out
	window.SetParent(NULL);

	//! Set main thread prio very high as it is the render thread
	LWP_SetThreadPriority(LWP_GetSelf(), 120);

	//! setup the home menu button
	trigHome.SetButtonOnlyTrigger(-1, WiiControls.HomeButton | ClassicControls.HomeButton << 16, GCControls.HomeButton);

	btnHome = new GuiButton(0, 0);
	btnHome->SetTrigger(&trigHome);
	btnHome->Clicked.connect(this, &Application::OnHomeButtonClick);
}

void Application::OnHomeButtonClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p3 UNUSED)
{
	// disable home menu button clicks while we are inside the home menu
	btnHome->SetClickable(false);

	HomeMenu *homeMenu = new HomeMenu();
	homeMenu->DimBackground(true);
	homeMenu->Closing.connect(this, &Application::OnHomeMenuClosing);
	this->SetUpdateOnly(homeMenu);
	this->Append(homeMenu);
}

void Application::OnHomeMenuClosing(GuiFrame *menu UNUSED)
{
	btnHome->SetClickable(true);
}