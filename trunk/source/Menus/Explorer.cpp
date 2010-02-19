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
#include "Controls/Clipboard.h"
#include "Controls/Taskbar.h"
#include "Memory/Resources.h"
#include "FileOperations/ProcessChoice.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/Properties.h"
#include "Prompts/ArchiveProperties.h"
#include "FileStartUp/FileStartUp.h"
#include "devicemounter.h"
#include "FileOperations/filebrowser.h"
#include "FileOperations/fileops.h"
#include "sys.h"

/*** Extern variables ***/
extern bool boothomebrew;
extern int curDevice;

Explorer::Explorer()
    :GuiWindow(0, 0)
{
	menu = MENU_NONE;
	currentDevice = 0;

    Browser = new FileBrowser();

    this->Setup();
}

Explorer::Explorer(int device)
    :GuiWindow(0, 0)
{
	menu = MENU_NONE;
	currentDevice = device;

    Browser = new FileBrowser();

    this->Setup();
    this->LoadDevice(device);
}

Explorer::Explorer(const char *path)
    :GuiWindow(0, 0)
{
	menu = MENU_NONE;
	currentDevice = 0;

    Browser = new FileBrowser();

    this->Setup();
    this->LoadPath(path);
}

Explorer::~Explorer()
{
    MainWindow::Instance()->ResumeGui();
    SetEffect(EFFECT_FADE, -50);
    while(this->GetEffect() > 0) usleep(100);

    MainWindow::Instance()->HaltGui();
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
	Resources::Remove(ftpstorage);
    Resources::Remove(dvd_ImgData);

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

    if(Credits)
        delete Credits;
    if(Device_Menu)
        delete Device_Menu;
    if(RightClick)
        delete RightClick;

    delete fileBrowser;
    if(ArcBrowser)
        delete ArcBrowser;
    delete Browser;
}

void Explorer::Setup()
{
    Device_Menu = NULL;
    RightClick = NULL;
    ArcBrowser = NULL;
    Credits = NULL;

	SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	SetPosition(28, 50);

    trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigPlus = new GuiTrigger();
    trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
	trigMinus = new GuiTrigger();
    trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	btnSoundClick = Resources::GetSound(button_click_pcm, button_click_pcm_size);
	btnSoundOver = Resources::GetSound(button_over_pcm, button_over_pcm_size);

    creditsImgData = Resources::GetImageData(WiiXplorer_png, WiiXplorer_png_size);
	Background = Resources::GetImageData(bg_browser_png, bg_browser_png_size);
	Address = Resources::GetImageData(addressbar_textbox_png, addressbar_textbox_png_size);

	sdstorage = Resources::GetImageData(sdstorage_png, sdstorage_png_size);
	usbstorage = Resources::GetImageData(usbstorage_png, usbstorage_png_size);
	networkstorage = Resources::GetImageData(networkstorage_png, networkstorage_png_size);
	ftpstorage = Resources::GetImageData(ftpstorage_png, ftpstorage_png_size);
    dvd_ImgData = Resources::GetImageData(dvdstorage_png, dvdstorage_png_size);

    width = Background->GetWidth();
    height = Background->GetHeight();
	BackgroundImg = new GuiImage(Background);

	fileBrowser = new GuiFileBrowser(Browser, width, 252);
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

	deviceSwitchBtn = new GuiButton(deviceImg->GetWidth(), deviceImg->GetHeight());
	deviceSwitchBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	deviceSwitchBtn->SetPosition(fileBrowser->GetLeft()+20, fileBrowser->GetTop()-38);
	deviceSwitchBtn->SetImage(deviceImg);
	deviceSwitchBtn->SetSoundClick(btnSoundClick);
	deviceSwitchBtn->SetSoundOver(btnSoundOver);
	deviceSwitchBtn->SetTrigger(trigA);
	deviceSwitchBtn->SetEffectGrow();
    deviceSwitchBtn->Clicked.connect(this, &Explorer::OnButtonClick);

    AdressText = new GuiText((char*) NULL, 20, (GXColor) {0, 0, 0, 255});
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
	filecount = Browser->BrowsePath(path);
	if(filecount < 0)
	{
		int choice = WindowPrompt(tr("Error:"),
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

	Browser->ResetMarker();
    AdressText->SetText(Browser->GetCurrentPath());
	SetDeviceImage();
	return filecount;
}
int Explorer::LoadDevice(int device)
{
    filecount = Browser->BrowseDevice(device);
	if(filecount < 0)
	{
		int choice = WindowPrompt(tr("Error:"),
		tr("Unable to load the device."),
		tr("Retry"),
		tr("Close"));

		if(choice)
		{
            SDCard_Init();
            USBDevice_Init();
			return LoadDevice(device);
		}
		else
			return -2;
	}
	Browser->ResetMarker();

    AdressText->SetText(Browser->GetCurrentPath());
	SetDeviceImage();
	currentDevice = device;
	curDevice = device;

	return filecount;
}

void Explorer::SetDeviceImage()
{
    const char * currentroot = Browser->GetRootDir();
    if(strncmp(currentroot, DeviceName[SD], 2) == 0)
    {
        deviceImg->SetImage(sdstorage);
    }
    else if(strncmp(currentroot, DeviceName[USB], 3) == 0 ||
            strncmp(currentroot, DeviceName[NTFS0], 4) == 0)
    {
        deviceImg->SetImage(usbstorage);
    }
    else if(strncmp(currentroot, DeviceName[SMB1], 3) == 0)
    {
        deviceImg->SetImage(networkstorage);
    }
    else if(strncmp(currentroot, DeviceName[FTP1], 3) == 0)
    {
        deviceImg->SetImage(ftpstorage);
    }
	else if(strncmp(currentroot, DeviceName[DVD], 3) == 0)
    {
        deviceImg->SetImage(dvd_ImgData);
    }
}

int Explorer::GetMenuChoice()
{
    CheckBrowserChanges();
    CheckDeviceMenu();
    CheckRightClick();
    ShowCredits(Credits);

	return menu;
}

void Explorer::CheckBrowserChanges()
{
    //!This will be changed later
	if(fileBrowser->GetState() == STATE_CLICKED)
    {
        fileBrowser->ResetState();
        if(ArcBrowser)
        {
            ArchiveChanges();
        }
        // check corresponding browser entry
        else if(Browser->IsCurrentDir())
        {
            if(Browser->BrowserChangeFolder())
            {
                fileBrowser->fileList[0]->SetState(STATE_SELECTED);
                fileBrowser->TriggerUpdate();
                Browser->ResetMarker();
                AdressText->SetText(Browser->GetCurrentPath());
            }
            else
            {
                menu = MENU_BROWSE_DEVICE;
            }
        }
        else
        {
            char filepath[MAXPATHLEN];
            snprintf(filepath, sizeof(filepath), "%s", Browser->GetCurrentSelectedFilepath());

            SetState(STATE_DISABLED);
            fileBrowser->DisableTriggerUpdate(true);
            Taskbar::Instance()->SetTriggerUpdate(false);
            int result = FileStartUp(filepath);
            SetState(STATE_DEFAULT);
            fileBrowser->DisableTriggerUpdate(false);
            Taskbar::Instance()->SetTriggerUpdate(true);
            if(result == BOOTHOMEBREW)
            {
                boothomebrew = true;
				ItemStruct Item;
				memset(&Item, 0, sizeof(ItemStruct));

                snprintf(Item.itempath, sizeof(Item.itempath), "%s", filepath);
				Clipboard::Instance()->AddItem(&Item);
                menu = MENU_EXIT;
            }
            else if(result == ARCHIVE)
            {
                ArcBrowser = new ArchiveBrowser(filepath);
                fileBrowser->SetBrowser(ArcBrowser);
                AdressText->SetTextf("%s", ArcBrowser->GetCurrentPath());
            }
            else if(result == TRIGGERUPDATE)
            {
                Browser->ParseDirectory();
                fileBrowser->TriggerUpdate();
            }
            else if(result == RELOADBROWSER)
            {
                menu = MENU_BROWSE_DEVICE;
            }
        }
    }
}

void Explorer::CheckDeviceMenu()
{
    if(Device_Menu != NULL)
    {
        SetState(STATE_DISABLED);
        fileBrowser->DisableTriggerUpdate(true);
        Taskbar::Instance()->SetTriggerUpdate(false);
        Append(Device_Menu);

        int device_choice = -1;
        while(device_choice == -1 && Device_Menu != NULL)
        {
            usleep(100);

            if(shutdown)
                Sys_Shutdown();
            else if(reset)
                Sys_Reboot();

            device_choice = Device_Menu->GetChoice();
        }
        delete Device_Menu;
        Device_Menu = NULL;

        if(device_choice >= 0)
        {
            if(ArcBrowser)
            {
                fileBrowser->SetBrowser(Browser);
                delete ArcBrowser;
                ArcBrowser = NULL;
            }
            LoadDevice(device_choice);
            Browser->ParseDirectory();
            fileBrowser->fileList[0]->SetState(STATE_SELECTED);
            fileBrowser->TriggerUpdate();
            AdressText->SetTextf("%s", Browser->GetCurrentPath());
        }
        SetState(STATE_DEFAULT);
        fileBrowser->DisableTriggerUpdate(false);
        Taskbar::Instance()->SetTriggerUpdate(true);
    }
}

void Explorer::CheckRightClick()
{
    if(RightClick != NULL)
    {
        SetState(STATE_DISABLED);
        fileBrowser->DisableTriggerUpdate(true);
        Taskbar::Instance()->SetTriggerUpdate(false);
        Append(RightClick);

        int RightClick_choice = -1;
        while(RightClick_choice == -1 && RightClick != NULL)
        {
            usleep(100);

            if(shutdown)
                Sys_Shutdown();
            else if(reset)
                Sys_Reboot();

            RightClick_choice = RightClick->GetChoice();
        }

        delete RightClick;
        RightClick = NULL;

        if(ArcBrowser)
        {
            if(RightClick_choice == ArcProperties)
            {
                ArchiveProperties * Prompt = new ArchiveProperties(ArcBrowser->GetCurrentItemStructure());
                Prompt->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
                MainWindow::Instance()->SetDim(true);
                MainWindow::Instance()->Append(Prompt);

                while(Prompt->GetChoice() == -1) usleep(THREAD_SLEEP);

                delete Prompt;
                Prompt = NULL;
                MainWindow::Instance()->SetDim(false);
            }
            else if(RightClick_choice >= 0)
            {
                ProcessArcChoice(ArcBrowser, RightClick_choice, Browser->GetCurrentPath());
            }
        }

        else if(!ArcBrowser && RightClick_choice >= 0)
        {
            if(RightClick_choice == PROPERTIES)
            {
                Properties * Prompt = new Properties(Browser->GetCurrentSelectedFilepath());
                Prompt->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
                MainWindow::Instance()->SetDim(true);
                MainWindow::Instance()->Append(Prompt);

                while(Prompt->GetChoice() == -1) usleep(THREAD_SLEEP);

                delete Prompt;
                Prompt = NULL;
                MainWindow::Instance()->SetDim(false);
            }
            else if(RightClick_choice >= 0)
            {
                ProcessChoice(Browser, RightClick_choice);
                if(RightClick_choice >= PASTE)
                {
                    Browser->ParseDirectory();
                    fileBrowser->TriggerUpdate();
                }
            }
        }
        this->SetState(STATE_DEFAULT);
        fileBrowser->DisableTriggerUpdate(false);
        Taskbar::Instance()->SetTriggerUpdate(true);
    }
}

void Explorer::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == CreditsBtn)
    {
        Credits = new CreditWindow();
    }

    else if(sender == deviceSwitchBtn)
    {
        Device_Menu = new DeviceMenu(deviceSwitchBtn->GetLeft()-5-this->GetLeft(), deviceSwitchBtn->GetTop()+deviceSwitchBtn->GetHeight()-this->GetTop());
    }

    else if(sender == clickmenuBtn)
    {
        if(this->IsInside(p.x, p.y))
        {
            if(ArcBrowser)
            {
                RightClick = new RightClickMenu(p.x, p.y,
                                                tr("Open"), tr("Extract"), tr("Extract All"), tr("Properties"));
            }
            else
            {
                RightClick = new RightClickMenu(p.x, p.y,
                                                tr("Cut"), tr("Copy"), tr("Paste"),
                                                tr("Rename"), tr("Delete"), tr("NewFolder"),
                                                tr("Properties"));
            }
        }
    }
}

void Explorer::ArchiveChanges()
{
    if(!ArcBrowser)
        return;

    //Change archive path
    if(ArcBrowser->IsCurrentDir())
    {
        int result = ArcBrowser->ChangeDirectory();
        if(result > 0)
        {
            fileBrowser->fileList[0]->SetState(STATE_SELECTED);
            fileBrowser->TriggerUpdate();
            AdressText->SetTextf("%s", ArcBrowser->GetCurrentPath());
        }
        else if(result == 0)
        {
            //leave Archive
            fileBrowser->SetBrowser(Browser);
            delete ArcBrowser;
            ArcBrowser = NULL;
            fileBrowser->TriggerUpdate();
            AdressText->SetTextf("%s", Browser->GetCurrentPath());
        }
        else
        {
            //error Accured closing Archive
            fileBrowser->SetBrowser(Browser);
            delete ArcBrowser;
            ArcBrowser = NULL;
            fileBrowser->TriggerUpdate();
            AdressText->SetTextf("%s", Browser->GetCurrentPath());
        }
    }
    else
    {
        //TODO fileoperations inside archive
    }
}

void Explorer::SetState(int s)
{
	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->SetState(s); }
		catch (const std::exception& e) { }
	}
}
