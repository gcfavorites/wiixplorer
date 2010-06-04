/***************************************************************************
 * Copyright (C) 2009
 * by r-win & Dimok
 *
 * Copyright (C) 2010
 * by dude
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
 * Taskbar.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <fat.h>
#include <time.h>

#include "menu.h"
#include "Taskbar.h"
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/PopUpMenu.h"
#include "Launcher/Applications.h"
#include "Launcher/Channels.h"
#include "SoundOperations/MusicPlayer.h"
#include "devicemounter.h"
#include "sys.h"

#include "Prompts/HomeMenu.h"

Taskbar *Taskbar::instance = NULL;

Taskbar::Taskbar()
	: GuiWindow(0, 0)
{
    menu = MENU_NONE;
    triggerupdate = true;

	taskbarImgData = Resources::GetImageData(taskbar_png, taskbar_png_size);
	taskbarImg = new GuiImage(taskbarImgData);
	taskbarImg->SetWidescreen(true);

	width = taskbarImg->GetWidth();
	height = taskbarImg->GetHeight();

	timeTxt = new GuiText("", 20, (GXColor) {40, 40, 40, 255});
	timeTxt->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	timeTxt->SetPosition(517, -1);
	timeTxt->SetFont(clock_ttf, clock_ttf_size);

	soundClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	soundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigHome = new GuiTrigger();
	trigHome->SetButtonOnlyTrigger(-1, WiiControls.HomeButton | ClassicControls.HomeButton << 16, GCControls.HomeButton);

	startBtn = new PictureButton(start_png, start_png_size, start_over_png, start_over_png_size, soundClick, soundOver);
	startBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	startBtn->SetPosition(58, -2);
	startBtn->SetSelectable(false);
	startBtn->SetTrigger(trigA);

	homeBtn = new GuiButton(0, 0);
	homeBtn->SetSelectable(false);
	homeBtn->SetTrigger(trigHome);

	HeadPhonesData = Resources::GetImageData(player_icon_png, player_icon_png_size);
	HeadPhonesImg = new GuiImage(HeadPhonesData);
	Musicplayer = new GuiButton(HeadPhonesData->GetWidth(), HeadPhonesData->GetHeight());
	Musicplayer->SetImage(HeadPhonesImg);
	Musicplayer->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	Musicplayer->SetTrigger(trigA);
	Musicplayer->SetPosition(470, 0);
	Musicplayer->SetEffectGrow();

	Append(homeBtn);
	Append(taskbarImg);
	Append(startBtn);
	Append(Musicplayer);
	Append(timeTxt);

	SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	SetPosition(0, -15);
}

Taskbar::~Taskbar()
{
    RemoveAll();

	delete taskbarImg;
	delete HeadPhonesImg;
	Resources::Remove(taskbarImgData);
	Resources::Remove(HeadPhonesData);

	delete homeBtn;
	delete startBtn;
	delete Musicplayer;
	delete timeTxt;

	delete trigA;
	delete trigHome;

	Resources::Remove(soundClick);
	Resources::Remove(soundOver);
}

Taskbar * Taskbar::Instance()
{
	if (instance == NULL)
	{
		instance = new Taskbar();
	}
	return instance;
}

void Taskbar::DestroyInstance()
{
	delete instance;
	instance = NULL;
}

void Taskbar::SetState(int s, int c)
{
}

void Taskbar::SetDim(bool d)
{
}

void Taskbar::Draw()
{
	if(frameCount % 60 == 0) //! Update time value every sec
	{
		char timetxt[20];
		time_t currenttime = time(0);
		struct tm * timeinfo = localtime(&currenttime);

		if(Settings.ClockMode == 0)
            strftime(timetxt, sizeof(timetxt), "%H:%M:%S", timeinfo);
        else
            strftime(timetxt, sizeof(timetxt), "%I:%M:%S", timeinfo);

		timeTxt->SetText(timetxt);
	}
	GuiWindow::Draw();
}

void Taskbar::Update(GuiTrigger * t)
{
	if(_elements.size() == 0 || state == STATE_DISABLED || !triggerupdate)
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->Update(t); }
		catch (const std::exception& e) { }
	}
}

void Taskbar::ResetState()
{
    menu = MENU_NONE;
}

void Taskbar::SetMenu(int m)
{
    menu = m;
}

int Taskbar::GetMenu()
{
	if (startBtn->GetState() == STATE_CLICKED)
	{
		menu = CheckStartMenu();
	}
	else if (homeBtn->GetState() == STATE_CLICKED)
	{
		menu = CheckHomeButton();
	}
	else if(Musicplayer->GetState() == STATE_CLICKED)
	{
	    ShowMusicPlayer();
		Musicplayer->ResetState();
	}

    return menu;
}

void Taskbar::ShowMusicPlayer()
{
	SetState(STATE_DISABLED);
	MainWindow::Instance()->SetState(STATE_DISABLED);
	MainWindow::Instance()->SetDim(true);
    MusicPlayer::Instance()->Show();
	MainWindow::Instance()->SetDim(false);
	SetState(STATE_DEFAULT);
	MainWindow::Instance()->SetState(STATE_DEFAULT);
}

int Taskbar::CheckHomeButton()
{
	HomeMenu *hm = new HomeMenu();

	SetState(STATE_DISABLED);
	MainWindow::Instance()->Append(hm);

	int choice = -1;
	while (choice == -1)
	{
		VIDEO_WaitVSync();

		if(shutdown)
			Sys_Shutdown();
		else if(reset)
			Sys_Reboot();

		choice = hm->GetChoice();
	}

	delete hm;

	SetState(STATE_DEFAULT);
	homeBtn->ResetState();

	return menu;
}

int Taskbar::CheckStartMenu()
{
	PopUpMenu *StartMenu = new PopUpMenu(45, 164);

	if (!StartMenu)
		return menu;

	StartMenu->AddItem(tr("Apps"), apps_png, apps_png_size, true);
	StartMenu->AddItem(tr("Channels"), channels_png, channels_png_size, true);
	StartMenu->AddItem(tr("Settings"), settings_png, settings_png_size);
	StartMenu->AddItem(tr("FTP Server"), ftpserver_png, ftpserver_png_size);
	StartMenu->AddItem(tr("Reload"), refresh_png, refresh_png_size);
	StartMenu->AddItem(tr("Restart"), system_restart_png, system_restart_png_size);
	StartMenu->AddItem(tr("Exit"), system_log_out_png, system_log_out_png_size);

	StartMenu->Finish();

	menuWidth = StartMenu->GetWidth();

	SetState(STATE_DISABLED);
	MainWindow::Instance()->SetState(STATE_DISABLED);
	MainWindow::Instance()->Append(StartMenu);

	int choice = -1;
	while (choice == -1)
	{
		usleep(100);

		if (shutdown)
			Sys_Shutdown();
		else if (reset)
			Sys_Reboot();

		choice = StartMenu->GetChoice();

		if (choice == APPS)
		{
			CheckAppsMenu();
			choice = -1;
		}
		else if (choice == CHANNELS)
		{
			CheckChannelsMenu();
			choice = -1;
		}
	}

	delete StartMenu;

	SetState(STATE_DEFAULT);
	MainWindow::Instance()->SetState(STATE_DEFAULT);
	startBtn->ResetState();

	if (choice == SETTINGS)
	{
		menu = MENU_SETTINGS;
	}
	else if (choice == FTPSERVER)
	{
		menu = MENU_FTP;
	}
	else if (choice == RELOAD)
	{
		if (WindowPrompt(tr("Do you want to remount the devices?"), 0, tr("Yes"), tr("Cancel")))
		{
			SDCard_deInit();
            SDGeckoA_deInit();
            SDGeckoB_deInit();
			NTFS_UnMount();
			//don't need to shutdown the usb device
			fatUnmount("usb:/");
			SDCard_Init();
			SDGeckoA_Init();
			SDGeckoB_Init();
			USBDevice_Init();
			NTFS_Mount();
			Applications::Instance()->Reload();
		}
	}
	else if (choice == RESTART)
	{
		if (WindowPrompt(tr("Do you want to reboot WiiXplorer?"), 0, tr("Yes"), tr("Cancel")))
		{
			RebootApp();
		}
	}
	else if (choice == EXIT)
	{
		if (WindowPrompt(tr("Do you want to exit WiiXplorer?"), 0, tr("Yes"), tr("Cancel")))
		{
			menu = MENU_EXIT;
		}
	}

	return menu;
}

void Taskbar::CheckAppsMenu()
{
	int choice = -1;
	PopUpMenu *AppsMenu = new PopUpMenu(menuWidth+30, 100);

	if (!AppsMenu)
		return;

	int count = Applications::Instance()->Count();

	if (count > 0)
	{
		for (int i = 0; i < count; i++)
			AppsMenu->AddItem(Applications::Instance()->GetName(i));

		AppsMenu->Finish();

		MainWindow::Instance()->Append(AppsMenu);

		while (choice == -1)
		{
			usleep(100);

			if (shutdown)
				Sys_Shutdown();
			else if (reset)
				Sys_Reboot();

			choice = AppsMenu->GetChoice();
		}
	}

	delete AppsMenu;

	if (choice >= 0 && WindowPrompt(tr("Do you want to start the app?"), Applications::Instance()->GetName(choice), tr("Yes"), tr("Cancel")))
	{
		Applications::Instance()->Launch(choice);
	}
}

void Taskbar::CheckChannelsMenu()
{
	int choice = -1;
	PopUpMenu *ChannelsMenu = new PopUpMenu(menuWidth+30, 100);

	if (!ChannelsMenu)
		return;

	int count = Channels::Instance()->Count();

	if (count > 0)
	{
		for (int i = 0; i < count; i++)
			ChannelsMenu->AddItem(Channels::Instance()->GetName(i));

		ChannelsMenu->Finish();

		MainWindow::Instance()->Append(ChannelsMenu);

		while (choice == -1)
		{
			usleep(100);

			if (shutdown)
				Sys_Shutdown();
			else if (reset)
				Sys_Reboot();

			choice = ChannelsMenu->GetChoice();
		}
	}

	delete ChannelsMenu;

	if (choice >= 0 && WindowPrompt(tr("Do you want to start the channel?"), Channels::Instance()->GetName(choice), tr("Yes"), tr("Cancel")))
	{
		Channels::Instance()->Launch(choice);
	}
}
