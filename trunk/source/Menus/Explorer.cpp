/***************************************************************************
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
 * Explorer.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include "Explorer.h"
#include "menu.h"
#include "Controls/MainWindow.h"
#include "Controls/Resources.h"
#include "FileOperations/ProcessChoice.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "FileStartUp/FileStartUp.h"
#include "Language/gettext.h"
#include "devicemounter.h"
#include "filebrowser.h"
#include "fileops.h"
#include "sys.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();
extern CLIPBOARD Clipboard;
extern bool boothomebrew;
extern u8 shutdown;
extern u8 reset;

Explorer::Explorer()
    :GuiWindow(0, 0)
{
	menu = MENU_NONE;
	currentDevice = 0;

    this->Setup();
}

Explorer::Explorer(int device)
    :GuiWindow(0, 0)
{
	menu = MENU_NONE;
	currentDevice = device;

    this->LoadDevice(device);
    this->Setup();
}

Explorer::Explorer(const char *path)
    :GuiWindow(0, 0)
{
	menu = MENU_NONE;
	currentDevice = 0;

    this->LoadPath(path);
    this->Setup();
}

Explorer::~Explorer()
{
    ResumeGui();
    SetEffect(EFFECT_FADE, -50);
    while(this->GetEffect() > 0) usleep(100);

    HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

	Resources::Remove(btnSoundClick);
	Resources::Remove(btnSoundOver);
	Resources::Remove(creditsImgData);
    Resources::Remove(Background);
    Resources::Remove(Address);
    Resources::Remove(sdstorage);
    Resources::Remove(usbstorage);
    Resources::Remove(networkstorage);

    delete BackgroundImg;
    delete creditsImg;
    delete deviceImg;
    delete AdressbarImg;

    delete AdressText;

    delete CreditsBtn;
    delete deviceSwitchBtn;
    delete Adressbar;
    delete clickmenuBtn;

    delete trigA;
    delete trigPlus;
    delete trigMinus;
}

void Explorer::Setup()
{
    Device_Menu = NULL;
    RightClick = NULL;
    CreditsPressed = false;

	SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	SetPosition(28, 50);

    trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigPlus = new GuiTrigger();
    trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
	trigMinus = new GuiTrigger();
    trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	btnSoundClick = Resources::GetSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	btnSoundOver = Resources::GetSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);

    creditsImgData = Resources::GetImageData(WiiXplorer_png, WiiXplorer_png_size);
	Background = Resources::GetImageData(bg_browser_png, bg_browser_png_size);
	Address = Resources::GetImageData(addressbar_textbox_png, addressbar_textbox_png_size);

	sdstorage = Resources::GetImageData(sdstorage_png, sdstorage_png_size);
	usbstorage = Resources::GetImageData(usbstorage_png, usbstorage_png_size);
	networkstorage = Resources::GetImageData(networkstorage_png, networkstorage_png_size);
//	isfsstorage = Resources::GetImageData(isfsstorage_png, isfsstorage_png_size);

    width = Background->GetWidth();
    height = Background->GetHeight();
	BackgroundImg = new GuiImage(Background);

	fileBrowser = new GuiFileBrowser(width, 252);
	fileBrowser->SetPosition(0, 53);

    creditsImg = new GuiImage(creditsImgData);
	CreditsBtn = new GuiButton(creditsImgData->GetWidth(), creditsImgData->GetHeight());
	CreditsBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	CreditsBtn->SetPosition(fileBrowser->GetLeft()+235, fileBrowser->GetTop()+262);
	CreditsBtn->SetImage(creditsImg);
	CreditsBtn->SetSoundClick(btnSoundClick);
	CreditsBtn->SetSoundOver(btnSoundOver);
	CreditsBtn->SetTrigger(trigA);
	CreditsBtn->SetEffectGrow();
    CreditsBtn->Clicked.connect(this, &Explorer::OnButtonClick);

	deviceImg = new GuiImage(sdstorage);

	if(currentDevice > SD && currentDevice < SMB1)
        deviceImg->SetImage(usbstorage);
    else if(currentDevice >= SMB1)
        deviceImg->SetImage(networkstorage);

	deviceSwitchBtn = new GuiButton(deviceImg->GetWidth(), deviceImg->GetHeight());
	deviceSwitchBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	deviceSwitchBtn->SetPosition(fileBrowser->GetLeft()+20, fileBrowser->GetTop()-38);
	deviceSwitchBtn->SetImage(deviceImg);
	deviceSwitchBtn->SetSoundClick(btnSoundClick);
	deviceSwitchBtn->SetSoundOver(btnSoundOver);
	deviceSwitchBtn->SetTrigger(trigA);
	deviceSwitchBtn->SetEffectGrow();
    deviceSwitchBtn->Clicked.connect(this, &Explorer::OnButtonClick);

	char currentdir[50];
    snprintf(currentdir, sizeof(currentdir), "%s%s", browser.rootdir, browser.dir);
    AdressText = new GuiText(currentdir, 20, (GXColor) {0, 0, 0, 255});
	AdressText->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	AdressText->SetPosition(18, 0);
	AdressText->SetMaxWidth(Address->GetWidth()-40, SCROLL_HORIZONTAL);
	AdressbarImg = new GuiImage(Address);
	Adressbar = new GuiButton(Address->GetWidth(), Address->GetHeight());
	Adressbar->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	Adressbar->SetPosition(fileBrowser->GetLeft()+62, fileBrowser->GetTop()-38);
	Adressbar->SetImage(AdressbarImg);
	Adressbar->SetLabel(AdressText);
    Adressbar->Clicked.connect(this, &Explorer::OnButtonClick);

	clickmenuBtn = new GuiButton(fileBrowser->GetWidth(), fileBrowser->GetHeight());
	clickmenuBtn->SetTrigger(trigPlus);
    clickmenuBtn->Clicked.connect(this, &Explorer::OnButtonClick);

	Append(BackgroundImg);
	Append(clickmenuBtn);
	Append(fileBrowser);
	Append(CreditsBtn);
	Append(Adressbar);
	Append(deviceSwitchBtn);

    SetEffect(EFFECT_FADE, 50);
}

int Explorer::LoadPath(const char * path)
{
	filecount = BrowsePath(path);
	if(filecount < 0)
	{
		int choice = WindowPrompt(tr("Error"),
		tr("Unable to load path."),
		tr("Retry"),
		tr("Close"));

		if(choice) {
            SDCard_Init();
            USBDevice_Init();
			return LoadPath(path);
		}
		else
			return -2;
	}
	return filecount;
}
int Explorer::LoadDevice(int device)
{
    filecount = BrowseDevice(device);
	if(filecount < 0)
	{
		int choice = WindowPrompt(tr("Error"),
		tr("Unable to load path."),
		tr("Retry"),
		tr("Close"));

		if(choice) {
            SDCard_Init();
            USBDevice_Init();
			return LoadDevice(device);
		}
		else
			return -2;
	}
	return filecount;
}

int Explorer::GetMenuChoice()
{
    CheckBrowserChanges();
    CheckDeviceMenu();
    CheckRightClick();

    ///* Temporary till the Credits is a class *//
    if(CreditsPressed)
    {
        CreditsPressed = false;
        CreditsWindow();
        MainWindow::Instance()->ChangeFocus(this);
    }
	return menu;
}

void Explorer::CheckBrowserChanges()
{
    //!This will be changed later
	if(fileBrowser->GetState() == STATE_CLICKED)
    {
            fileBrowser->ResetState();
            // check corresponding browser entry
            if(browserList[browser.selIndex].isdir)
            {
                if(BrowserChangeFolder())
                {
                    fileBrowser->ResetState();
                    fileBrowser->fileList[0]->SetState(STATE_SELECTED);
                    fileBrowser->TriggerUpdate();
                    AdressText->SetTextf("%s%s", browser.rootdir, browser.dir);
                } else {
                    menu = MENU_BROWSE_DEVICE;
                }
            } else {
                char filepath[MAXPATHLEN];
                snprintf(filepath, sizeof(filepath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                int result = FileStartUp(filepath);

                if(result == BOOTHOMEBREW) {
                    boothomebrew = true;
                    menu = MENU_EXIT;
                }
                else if(result == TRIGGERUPDATE) {
                    ParseDirectory();
                    fileBrowser->TriggerUpdate();
                }
            }
    }
}

void Explorer::CheckDeviceMenu()
{
    if(Device_Menu != NULL)
    {
        fileBrowser->DisableTriggerUpdate(true);

        deviceSwitchBtn->SetState(STATE_DISABLED);
        this->Append(Device_Menu);

        int device_choice = -1;
        while(device_choice == -1 && Device_Menu != NULL)
        {
            usleep(100);
            if(shutdown == 1)
                Sys_Shutdown();

            else if(reset == 1)
                Sys_Reboot();

            device_choice = Device_Menu->GetChoice();
        }
        delete Device_Menu;
        Device_Menu = NULL;

        if(device_choice >= 0)
        {
            LoadDevice(device_choice);
            ParseDirectory();
            fileBrowser->fileList[0]->SetState(STATE_SELECTED);
            fileBrowser->TriggerUpdate();
            AdressText->SetTextf("%s%s", browser.rootdir, browser.dir);
        }
        deviceSwitchBtn->SetState(STATE_DEFAULT);
        fileBrowser->DisableTriggerUpdate(false);
        MainWindow::Instance()->ChangeFocus(this);
    }
}

void Explorer::CheckRightClick()
{
    if(RightClick != NULL)
    {
        fileBrowser->DisableTriggerUpdate(true);

        clickmenuBtn->SetState(STATE_DISABLED);
        this->Append(RightClick);

        int RightClick_choice = -1;
        while(RightClick_choice == -1 && RightClick != NULL)
        {
            usleep(100);
            if(shutdown == 1)
                Sys_Shutdown();

            else if(reset == 1)
                Sys_Reboot();

            RightClick_choice = RightClick->GetChoice();
        }

        delete RightClick;
        RightClick = NULL;

        if(RightClick_choice >= 0)
        {
            ProcessChoice(RightClick_choice);
            ParseDirectory();
            fileBrowser->TriggerUpdate();
        }
        clickmenuBtn->SetState(STATE_DEFAULT);
        fileBrowser->DisableTriggerUpdate(false);
        MainWindow::Instance()->ChangeFocus(this);
    }
}

void Explorer::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == CreditsBtn)
    {
        CreditsPressed = true;
    }

    else if(sender == deviceSwitchBtn)
    {
        Device_Menu = new DeviceMenu(deviceSwitchBtn->GetLeft()-5-this->GetLeft(), deviceSwitchBtn->GetTop()+deviceSwitchBtn->GetHeight()-this->GetTop());
    }

    else if(sender == clickmenuBtn)
    {
        if(this->IsInside(p.x, p.y))
        {
            RightClick = new RightClickMenu(p.x, p.y);
        }
    }
}
