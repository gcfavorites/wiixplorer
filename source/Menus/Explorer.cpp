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
#include "Explorer.h"
#include "ArchiveOperations/ArchiveBrowser.h"
#include "FileOperations/ListFileBrowser.hpp"
#include "FileOperations/IconFileBrowser.hpp"
#include "FileOperations/fileops.h"
#include "FileStartUp/FileStartUp.h"
#include "Controls/Application.h"
#include "Controls/Clipboard.h"
#include "Controls/Taskbar.h"
#include "Memory/Resources.h"
#include "FileOperations/ProcessChoice.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "Settings.h"
#include "sys.h"

Explorer::Explorer(GuiWindow *p, const char *path)
	: GuiWindow(0, 0, p)
{
	this->Init();
	if(path)
		this->LoadPath(path);
}

Explorer::~Explorer()
{
	hide();

	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	RemoveAll();

	Resources::Remove(btnSoundClick);
	Resources::Remove(btnSoundOver);
	Resources::Remove(creditsImgData);
	Resources::Remove(Background);
	Resources::Remove(Address);
	Resources::Remove(Refresh);
	Resources::Remove(sdstorage);
	Resources::Remove(usbstorage);
	Resources::Remove(usbstorage_blue);
	Resources::Remove(networkstorage);
	Resources::Remove(ftpstorage);
	Resources::Remove(dvd_ImgData);

	delete BackgroundImg;
	delete creditsImg;
	delete deviceImg;
	delete AdressbarImg;
	delete RefreshImg;

	delete AdressText;

	delete RefreshBtn;
	delete CreditsBtn;
	delete deviceSwitchBtn;
	delete Adressbar;
	delete clickmenuBtn;
	delete BackInDirBtn;

	delete trigA;
	delete trigPlus;
	delete trigBackInDir;

	if(Credits)
		delete Credits;
	if(fileBrowser != curBrowser)
		delete fileBrowser;

	delete curBrowser;
	delete guiBrowser;
}

void Explorer::Init()
{
	guiBrowser = NULL;
	curBrowser = NULL;
	Credits = NULL;

	fileBrowser = new FileBrowser();
	curBrowser = fileBrowser;

	SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	SetPosition(0, 50);

	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigPlus = new GuiTrigger();
	trigPlus->SetButtonOnlyTrigger(-1, WiiControls.ContextMenuButton | ClassicControls.ContextMenuButton << 16, GCControls.ContextMenuButton);
	trigBackInDir = new GuiTrigger();
	trigBackInDir->SetButtonOnlyTrigger(-1, WiiControls.UpInDirectory | ClassicControls.UpInDirectory << 16, GCControls.UpInDirectory);

	btnSoundClick = Resources::GetSound("button_click.wav");
	btnSoundOver = Resources::GetSound("button_over.wav");

	creditsImgData = Resources::GetImageData("WiiXplorer.png");
	Background = Resources::GetImageData("bg_browser.png");
	Address = Resources::GetImageData("addressbar_textbox.png");
	Refresh = Resources::GetImageData("refresh2.png");

	sdstorage = Resources::GetImageData("sdstorage.png");
	usbstorage = Resources::GetImageData("usbstorage.png");
	usbstorage_blue = Resources::GetImageData("usbstorage_blue.png");
	networkstorage = Resources::GetImageData("networkstorage.png");
	ftpstorage = Resources::GetImageData("ftpstorage.png");
	dvd_ImgData = Resources::GetImageData("dvdstorage.png");

	width = Background->GetWidth();
	height = Background->GetHeight();
	BackgroundImg = new GuiImage(Background);

	if(Settings.BrowserMode == ICONBROWSER)
	{
		guiBrowser = new IconFileBrowser(curBrowser, width, 252);
	}
	else
	{
		guiBrowser = new ListFileBrowser(curBrowser, width, 252);
	}

	guiBrowser->SetPosition(0, 53);
	guiBrowser->Clicked.connect(this, &Explorer::OnBrowserChanges);

	creditsImg = new GuiImage(creditsImgData);
	CreditsBtn = new GuiButton(creditsImgData->GetWidth(), creditsImgData->GetHeight());
	CreditsBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	CreditsBtn->SetPosition(guiBrowser->GetLeft()+235, guiBrowser->GetTop()+262);
	CreditsBtn->SetImage(creditsImg);
	CreditsBtn->SetSoundClick(btnSoundClick);
	CreditsBtn->SetSoundOver(btnSoundOver);
	CreditsBtn->SetTrigger(trigA);
	CreditsBtn->SetEffectGrow();
	CreditsBtn->Clicked.connect(this, &Explorer::OnButtonClick);

	deviceImg = new GuiImage(sdstorage);

	deviceSwitchBtn = new GuiButton(deviceImg->GetWidth(), deviceImg->GetHeight());
	deviceSwitchBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	deviceSwitchBtn->SetPosition(guiBrowser->GetLeft()+20, guiBrowser->GetTop()-38);
	deviceSwitchBtn->SetImage(deviceImg);
	deviceSwitchBtn->SetSoundClick(btnSoundClick);
	deviceSwitchBtn->SetSoundOver(btnSoundOver);
	deviceSwitchBtn->SetTrigger(trigA);
	deviceSwitchBtn->SetEffectGrow();
	deviceSwitchBtn->Clicked.connect(this, &Explorer::OnButtonClick);

	AdressText = new GuiText((char *) NULL, 20, (GXColor) {0, 0, 0, 255});
	AdressText->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	AdressText->SetPosition(18, 0);
	AdressText->SetMaxWidth(Address->GetWidth()-45-Refresh->GetWidth(), SCROLL_HORIZONTAL);
	AdressbarImg = new GuiImage(Address);
	Adressbar = new GuiButton(Address->GetWidth()-Refresh->GetWidth()-5, Address->GetHeight());
	Adressbar->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	Adressbar->SetPosition(guiBrowser->GetLeft()+62, guiBrowser->GetTop()-38);
	Adressbar->SetImage(AdressbarImg);
	Adressbar->SetLabel(AdressText);
	Adressbar->SetSoundClick(btnSoundClick);
	Adressbar->SetSoundOver(btnSoundOver);
	Adressbar->SetRumble(false);
	Adressbar->SetTrigger(trigA);
	Adressbar->Clicked.connect(this, &Explorer::OnButtonClick);

	RefreshImg = new GuiImage(Refresh);
	RefreshImg->SetScale(0.8);
	RefreshBtn = new GuiButton(Refresh->GetWidth(), Refresh->GetHeight());
	RefreshBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	RefreshBtn->SetPosition(Adressbar->GetLeft()+Adressbar->GetWidth()-5, Adressbar->GetTop()+4);
	RefreshBtn->SetImage(RefreshImg);
	RefreshBtn->SetSoundClick(btnSoundClick);
	RefreshBtn->SetSoundOver(btnSoundOver);
	RefreshBtn->SetTrigger(trigA);
	RefreshBtn->SetEffectGrow();
	RefreshBtn->Clicked.connect(this, &Explorer::OnButtonClick);

	clickmenuBtn = new GuiButton(guiBrowser->GetWidth(), guiBrowser->GetHeight());
	clickmenuBtn->SetPosition(guiBrowser->GetLeft(), guiBrowser->GetTop());
	clickmenuBtn->SetTrigger(trigPlus);
	clickmenuBtn->Clicked.connect(this, &Explorer::OnButtonClick);

	BackInDirBtn = new GuiButton(0, 0);
	BackInDirBtn->SetTrigger(trigBackInDir);
	BackInDirBtn->Clicked.connect(this, &Explorer::BackInDirectory);

	Append(BackgroundImg);
	Append(clickmenuBtn);
	Append(CreditsBtn);
	Append(Adressbar);
	Append(RefreshBtn);
	Append(deviceSwitchBtn);
	Append(guiBrowser);
	Append(BackInDirBtn);

	show();
}

void Explorer::show()
{
	SetEffect(EFFECT_FADE, 50);
	this->SetVisible(true);
}

void Explorer::hide()
{
	SetEffect(EFFECT_FADE, -50);
	while(this->GetEffect() > 0)
		Application::Instance()->updateEvents();

	this->SetVisible(false);
}

int Explorer::LoadPath(const char * path)
{
	int filecount = curBrowser->BrowsePath(path);
	if(filecount < 0)
	{
		int choice = WindowPrompt(tr("Error:"), tr("Unable to load path."), tr("Retry"), tr("Close"));
		if(choice)
			return LoadPath(path);
		else
			return -2;
	}

	curBrowser->ResetMarker();
	AdressText->SetText(curBrowser->GetCurrentPath());
	Settings.LastUsedPath.assign(curBrowser->GetCurrentPath());
	SetDeviceImage();
	return filecount;
}

void Explorer::SetDeviceImage()
{
	const char * currentroot = fileBrowser->GetRootDir();
	if(strncmp(currentroot, DeviceName[SD], 2) == 0)
	{
		deviceImg->SetImage(sdstorage);
	}
	else if(strncmp(currentroot, DeviceName[USB1], 3) == 0)
	{
		const char * FSName = DeviceHandler::PathToFSName(curBrowser->GetCurrentPath());

		if(FSName && strncmp(FSName, "NTFS", 4) != 0)
			deviceImg->SetImage(usbstorage);
		else
			deviceImg->SetImage(usbstorage_blue);
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

void Explorer::OnBrowserChanges(int index UNUSED)
{
	// check corresponding browser entry
	if(curBrowser->IsCurrentDir())
	{
		int result = curBrowser->ChangeDirectory();
		if(result > 0)
		{
			guiBrowser->SetSelected(0);
			curBrowser->SetPageIndex(0);
			curBrowser->ResetMarker();
			guiBrowser->TriggerUpdate();
			AdressText->SetText(curBrowser->GetCurrentPath());
			if(fileBrowser == curBrowser)
				Settings.LastUsedPath.assign(curBrowser->GetCurrentPath());
		}
		else if(result == CLOSE_ARCHIVE && fileBrowser != curBrowser)
		{
			delete curBrowser;
			curBrowser = fileBrowser;
			guiBrowser->SetBrowser(curBrowser);
			guiBrowser->TriggerUpdate();
			AdressText->SetTextf("%s", curBrowser->GetCurrentPath());
		}
		else
		{
			ShowError(tr("Can't browse that path."));
		}
	}
	else
	{
		char filepath[MAXPATHLEN];
		int result = 0;

		SetState(STATE_DISABLED);

		result = curBrowser->ExecuteFile(curBrowser->GetCurrentSelectedFilepath());

		SetState(STATE_DEFAULT);

		if(result == ARCHIVE)
		{
			if(fileBrowser != curBrowser)
				delete curBrowser;

			curBrowser = new ArchiveBrowser(filepath);
			guiBrowser->SetBrowser(curBrowser);
			AdressText->SetText(curBrowser->GetCurrentPath());
		}
		else if(result == REFRESH_BROWSER || result == RELOAD_BROWSER)
		{
			curBrowser->Refresh();
			guiBrowser->TriggerUpdate();
		}
	}
}

void Explorer::OnRightClick(PopUpMenu *menu, int item)
{
	Application::Instance()->Remove(menu);
	Application::Instance()->UpdateOnly(NULL);
	menu->Close();


	if(curBrowser != fileBrowser && item >= 0) //! Archive
	{
		ProcessArcChoice((ArchiveBrowser *) curBrowser, item, fileBrowser->GetCurrentPath());
		curBrowser->Refresh();
		guiBrowser->TriggerUpdate();
	}
	else if(item >= 0)  //! Real file browser
	{
		ProcessChoice(fileBrowser, item);
		if(item >= PASTE)
		{
			curBrowser->Refresh();
			guiBrowser->TriggerUpdate();
		}
	}
}

void Explorer::OnDeviceSelect(DeviceMenu *Device_Menu, int device)
{
	Application::Instance()->UpdateOnly(NULL);
	Device_Menu->Close();
	Device_Menu = NULL;

	if(device >= SD && device < MAXDEVICES)
	{
		if(curBrowser != fileBrowser)
		{
			delete curBrowser;
			curBrowser = fileBrowser;
			guiBrowser->SetBrowser(curBrowser);
		}
		LoadPath(fmt("%s:/", DeviceName[device]));
		guiBrowser->SetSelected(0);
		guiBrowser->TriggerUpdate();
		AdressText->SetText(curBrowser->GetCurrentPath());
		Settings.LastUsedPath.assign(fileBrowser->GetCurrentPath());
	}
}

void Explorer::OnCreditsClosing()
{
	Application::Instance()->UpdateOnly(NULL);
	Application::Instance()->PushForDelete(Credits);
	Credits = NULL;
}

void Explorer::OnButtonClick(GuiButton *sender, int pointer UNUSED, const POINT &p)
{
	if(sender == CreditsBtn)
	{
		Credits = new CreditWindow(this);
		Credits->DimBackground(true);
		Credits->Closing.connect(this, &Explorer::OnCreditsClosing);
		Application::Instance()->UpdateOnly(Credits);
	}

	else if(sender == deviceSwitchBtn)
	{
		DeviceMenu *Device_Menu = new DeviceMenu(deviceSwitchBtn->GetLeft()-5-this->GetLeft(), deviceSwitchBtn->GetTop()+deviceSwitchBtn->GetHeight()-this->GetTop(), this);
		Device_Menu->DeviceSelected.connect(this, &Explorer::OnDeviceSelect);
		Application::Instance()->UpdateOnly(Device_Menu);
	}

	else if(sender == clickmenuBtn && this->IsInside(p.x, p.y))
	{
		PopUpMenu *RightClick = NULL;
		if(curBrowser != fileBrowser) //! Archive
		{
			RightClick = new PopUpMenu(p.x, p.y);
			RightClick->AddItem(tr("Paste"));
			RightClick->AddItem(tr("Extract"));
			RightClick->AddItem(tr("Extract All"));
			RightClick->AddItem(tr("Properties"));
			RightClick->Finish();
		}
		else //! Real file browser
		{
			RightClick = new PopUpMenu(p.x, p.y);
			RightClick->AddItem(tr("Cut"));
			RightClick->AddItem(tr("Copy"));
			RightClick->AddItem(tr("Paste"));
			RightClick->AddItem(tr("Rename"));
			RightClick->AddItem(tr("Delete"));
			RightClick->AddItem(tr("New Folder"));
			RightClick->AddItem(tr("Add to zip"));
			RightClick->AddItem(tr("MD5 Check"));
			RightClick->AddItem(tr("Properties"));
			RightClick->Finish();
		}
		RightClick->ItemClicked.connect(this, &Explorer::OnRightClick);
		Application::Instance()->UpdateOnly(RightClick);
		Application::Instance()->Append(RightClick);
	}

	else if(sender == RefreshBtn)
	{
		curBrowser->Refresh();
		guiBrowser->TriggerUpdate();
	}

	else if(sender == Adressbar)
	{
		char entered[1024];
		strcpy(entered, curBrowser->GetCurrentPath());
		if(OnScreenKeyboard(entered, sizeof(entered)))
		{
			LoadPath(entered);
		}
	}
}

void Explorer::BackInDirectory(GuiButton *sender, int pointer UNUSED, const POINT &p UNUSED)
{
	curBrowser->BackInDirectory();
	guiBrowser->SetSelected(0);
	curBrowser->SetPageIndex(0);
	curBrowser->ResetMarker();
	curBrowser->Refresh();
	guiBrowser->TriggerUpdate();
	AdressText->SetText(curBrowser->GetCurrentPath());

	sender->ResetState();
}
