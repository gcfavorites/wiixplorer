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
#include <unistd.h>
#include <gctypes.h>
#include <network.h>
#include "FTPServerMenu.h"
#include "FTPServer.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/DeviceMenu.h"
#include "network/networkops.h"
#include "Tools/gxprintf.h"
#include "Memory/Resources.h"
#include "ftpii/ftp.h"
#include "ftpii/net.h"
#include "ftpii/virtualpath.h"
#include "devicemounter.h"
#include "menu.h"
#include "sys.h"

FTPServerMenu::FTPServerMenu()
    : GuiWindow(0, 0)
{
	menu = MENU_NONE;

    trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	btnSoundClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    btnOutline = Resources::GetImageData(button_png, button_png_size);
    btnOutlineOver = Resources::GetImageData(button_over_png, button_over_png_size);
    network_icon = Resources::GetImageData(network_png, network_png_size);
    bgImgData = Resources::GetImageData(bg_browser_png, bg_browser_png_size);

    width = bgImgData->GetWidth();
    height = bgImgData->GetHeight()+100;

	bgImg = new GuiImage(bgImgData);
	Append(bgImg);

	networkImg = new GuiImage(network_icon);
	networkImg->SetPosition(30, 16);
	Append(networkImg);

	IPText = new GuiText(fmt("Server IP: %s    Port: %d", GetNetworkIP(), Settings.FTPServer.Port), 20, (GXColor){0, 0, 0, 255});
    IPText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    IPText->SetPosition(65, 25);
    Append(IPText);

	backBtnTxt = new GuiText(tr("Go Back"), 20, (GXColor){0, 0, 0, 255});
	backBtnImg = new GuiImage(btnOutline);
	backBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	backBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn->SetPosition(20, -85);
	backBtn->SetLabel(backBtnTxt);
	backBtn->SetImage(backBtnImg);
	backBtn->SetSoundClick(btnSoundClick);
	backBtn->SetSoundOver(btnSoundOver);
	backBtn->SetTrigger(trigA);
	backBtn->SetEffectGrow();
    backBtn->Clicked.connect(this, &FTPServerMenu::OnButtonClick);
	Append(backBtn);

	MainFTPBtnTxt = new GuiText(tr("Startup FTP"), 20, (GXColor){0, 0, 0, 255});
	MainFTPBtnImg = new GuiImage(btnOutline);
	MainFTPBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	MainFTPBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	MainFTPBtn->SetPosition(-20, -85);
	MainFTPBtn->SetLabel(MainFTPBtnTxt);
	MainFTPBtn->SetImage(MainFTPBtnImg);
	MainFTPBtn->SetSoundClick(btnSoundClick);
	MainFTPBtn->SetSoundOver(btnSoundOver);
	MainFTPBtn->SetTrigger(trigA);
	MainFTPBtn->SetEffectGrow();
    MainFTPBtn->Clicked.connect(this, &FTPServerMenu::OnButtonClick);
	Append(MainFTPBtn);

    Console = new GXConsole(bgImgData->GetWidth()-50, 250);
    Console->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Console->SetPosition(20, 65);
	Append(Console);

	SetGXConsole(Console);

    SetEffect(EFFECT_FADE, 50);

    if(FTPServer::Instance()->IsRunning())
    {
        gxprintf("%s %d.\n", tr("FTP Server is running and listening on port"), Settings.FTPServer.Port);
        MainFTPBtnTxt->SetText(tr("Shutdown FTP"));
    }
    else
        gxprintf("%s %d.\n", tr("Press Startup FTP to start the server on port"), Settings.FTPServer.Port);
}

FTPServerMenu::~FTPServerMenu()
{
    MainWindow::Instance()->ResumeGui();

    SetEffect(EFFECT_FADE, -50);

    while(this->GetEffect() > 0)
        usleep(100);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();
	SetGXConsole(NULL);

    MainWindow::Instance()->ResumeGui();

    Resources::Remove(btnSoundClick);
    Resources::Remove(btnSoundOver);

    Resources::Remove(btnOutline);
    Resources::Remove(btnOutlineOver);
    Resources::Remove(network_icon);
    Resources::Remove(bgImgData);

    delete bgImg;
    delete backBtnImg;
    delete MainFTPBtnImg;
    delete networkImg;

    delete IPText;
    delete backBtnTxt;
    delete MainFTPBtnTxt;

    delete backBtn;
    delete MainFTPBtn;

    delete trigA;

    delete Console;
}

int FTPServerMenu::GetMenu()
{
    if(shutdown)
        Sys_Shutdown();
    else if(reset)
        Sys_Reboot();

    else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
    {
        menu = Taskbar::Instance()->GetMenu();
    }

    return menu;
}

void FTPServerMenu::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == backBtn)
    {
        menu = MENU_BROWSE_DEVICE;
    }
    else if(sender == MainFTPBtn)
    {
        if(FTPServer::Instance()->IsRunning())
        {
            MainFTPBtnTxt->SetText(tr("Startup FTP"));
            FTPServer::Instance()->ShutdownFTP();
        }
        else
        {
            MainFTPBtnTxt->SetText(tr("Shutdown FTP"));
            FTPServer::Instance()->StartupFTP();
        }
    }
}
