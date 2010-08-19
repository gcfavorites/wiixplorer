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
#include <time.h>

#include "menu.h"
#include "Taskbar.h"
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PopUpMenu.h"
#include "Launcher/Applications.h"
#include "Launcher/Channels.h"
#include "Launcher/OperaBooter.hpp"
#include "network/networkops.h"
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
    WifiData = NULL;
    WifiImg = NULL;

	taskbarImgData = Resources::GetImageData(taskbar_png, taskbar_png_size);
	taskbarImg = new GuiImage(taskbarImgData);

	width = taskbarImg->GetWidth();
	height = taskbarImg->GetHeight();

	timeTxt = new GuiText("", 20, (GXColor) {40, 40, 40, 255});
	timeTxt->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	timeTxt->SetPosition(width-82, -1);
	timeTxt->SetFont(clock_ttf, clock_ttf_size);

	soundClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	soundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigHome = new GuiTrigger();
	trigHome->SetButtonOnlyTrigger(-1, WiiControls.HomeButton | ClassicControls.HomeButton << 16, GCControls.HomeButton);

	startBtn = new PictureButton(start_png, start_png_size, start_over_png, start_over_png_size, soundClick, soundOver);
	startBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	startBtn->SetPosition(23, -2);
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
	Musicplayer->SetPosition(458, 0);
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

	if(WifiData)
        Resources::Remove(WifiData);
	if(WifiImg)
        delete WifiImg;

	delete homeBtn;
	delete startBtn;
	delete Musicplayer;
	delete timeTxt;

	delete trigA;
	delete trigHome;

	Resources::Remove(soundClick);
	Resources::Remove(soundOver);

	for(u32 i = 0; i < Tasks.size(); i++)
	{
	    delete Tasks[i];
	}
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
    if(instance)
        delete instance;
	instance = NULL;
}

void Taskbar::SetState(int s, int c)
{
}

void Taskbar::SetDim(bool d)
{
}

void Taskbar::AddTask(Task * t)
{
    t->SetPosition(105+Tasks.size()*100, 0);
    t->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Tasks.push_back(t);
	Append(t);
}

void Taskbar::RemoveTask(Task * t)
{
    for(u32 i = 0; i < Tasks.size(); i++)
    {
        if(Tasks[i] == t)
        {
            Remove(Tasks[i]);
            Tasks.erase(Tasks.begin()+i);
            TasksDeleteQueue.push(Tasks[i]);
            break;
        }
    }
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
        MusicPlayer::Instance()->Show();
		Musicplayer->ResetState();
	}
	else if(WifiImg == NULL && IsNetworkInit())
	{
	    WifiData = Resources::GetImageData(network_wireless_png, network_wireless_png_size);
        WifiImg = new GuiImage(WifiData);
        WifiImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        WifiImg->SetPosition(418, 0);
        Append(WifiImg);
	}
	else if(TasksDeleteQueue.size() > 0)
	{
        while(!TasksDeleteQueue.empty())
        {
            delete TasksDeleteQueue.front();
            TasksDeleteQueue.pop();
        }
	}

    return menu;
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
	PopUpMenu *StartMenu = new PopUpMenu(screenwidth/2-width/2-2, 134);

	if (!StartMenu)
		return menu;

	StartMenu->AddItem(tr("Apps"), apps_png, apps_png_size, true);
	StartMenu->AddItem(tr("Channels"), channels_png, channels_png_size, true);
	StartMenu->AddItem(tr("URL List"), opera_icon_png, opera_icon_png_size, true);
	StartMenu->AddItem(tr("Settings"), settings_png, settings_png_size);
	StartMenu->AddItem(tr("FTP Server"), network_png, network_png_size);
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
		else if (choice == URLS)
		{
			OpenLinksMenu();
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
			NTFS_UnMount();
			//don't need to shutdown the devices
            SDCard_deInit();
            USBDevice_deInit();
            SDGeckoA_deInit();
            SDGeckoB_deInit();
			SDCard_Init();
			SDGeckoA_Init();
			SDGeckoB_Init();
			USBDevice_Init();
			NTFS_Mount();
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
	PopUpMenu *AppsMenu = new PopUpMenu(menuWidth+screenwidth/2-width/2-15, 140);

    Applications Apps(Settings.AppPath);
	int count = Apps.Count();

	if (count > 0)
	{
		for (int i = 0; i < count; i++)
			AppsMenu->AddItem(Apps.GetName(i));

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

	if (choice >= 0 && WindowPrompt(tr("Do you want to start the app?"), Apps.GetName(choice), tr("Yes"), tr("Cancel")))
	{
		Apps.Launch(choice);
	}
}

void Taskbar::CheckChannelsMenu()
{
	int choice = -1;
	PopUpMenu *ChannelsMenu = new PopUpMenu(menuWidth+screenwidth/2-width/2-15, 170);

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

void Taskbar::OpenLinksMenu()
{
	int choice = -1;
	PopUpMenu * LinksMenu = new PopUpMenu(menuWidth+screenwidth/2-width/2-15, 200);

    OperaBooter Booter(Settings.LinkListPath);

    LinksMenu->AddItem(tr("Add Link"));

    for (int i = 0; i < Booter.GetCount(); i++)
    {
        const char * name = Booter.GetName(i);
        if(name)
            LinksMenu->AddItem(name);
    }

    LinksMenu->Finish();

    MainWindow::Instance()->Append(LinksMenu);

    while (choice == -1)
    {
        usleep(100);

        if (shutdown)
            Sys_Shutdown();
        else if (reset)
            Sys_Reboot();

        choice = LinksMenu->GetChoice();
    }

	delete LinksMenu;

	if(choice == 0)
        Booter.AddLink();

	else if (choice > 0)
	{
	    int res = WindowPrompt(tr("How should this URL be opened?"), Booter.GetLink(choice-1), tr("Internet Channel"), tr("Download Link"), tr("Remove Link"), tr("Cancel"));
	    if(res == 1)
            Booter.Launch(choice-1);
        else if(res == 2)
            Booter.DownloadFile(choice-1);
        else if(res == 3)
            Booter.RemoveLink(choice-1);
	}
}
