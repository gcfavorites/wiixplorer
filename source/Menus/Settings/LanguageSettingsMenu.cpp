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
#include "LanguageSettingsMenu.h"
#include "Language/LanguageUpdater.h"
#include "Prompts/PromptWindows.h"
#include "Settings.h"

LanguageSettingsMenu::LanguageSettingsMenu(GuiFrame *r)
	: SettingsMenu(tr("Language Settings"), r)
{
	FileList = NULL;

	resetBtnTxt = new GuiText(tr("Default"), 22, (GXColor){0, 0, 0, 255});
	resetBtnImg = new GuiImage(btnOutline);
	resetBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	resetBtn->SetAlignment(ALIGN_CENTER | ALIGN_BOTTOM);
	resetBtn->SetPosition(100+btnOutline->GetWidth()/2, -65);
	resetBtn->SetLabel(resetBtnTxt);
	resetBtn->SetImage(resetBtnImg);
	resetBtn->SetSoundOver(btnSoundOver);
	resetBtn->SetTrigger(trigA);
	resetBtn->SetEffectGrow();
	resetBtn->Clicked.connect(this, &LanguageSettingsMenu::OnResetButtonClick);
	Append(resetBtn);

	downloadImgData = Resources::GetImageData("ftpstorage.png");
	downloadBtnImg = new GuiImage(downloadImgData);
	downloadBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	downloadBtn->SetAlignment(ALIGN_CENTER | ALIGN_TOP);
	downloadBtn->SetPosition(optionBrowser->GetWidth()/2+30, optionBrowser->GetTop()-40);
	downloadBtn->SetImage(downloadBtnImg);
	downloadBtn->SetSoundOver(btnSoundOver);
	downloadBtn->SetTrigger(trigA);
	downloadBtn->SetEffectGrow();
	downloadBtn->Clicked.connect(this, &LanguageSettingsMenu::OnDownloadButtonClick);
	Append(downloadBtn);

	SetupOptions();
}

LanguageSettingsMenu::~LanguageSettingsMenu()
{
	Remove(downloadBtn);
	delete downloadBtnImg;
	delete downloadBtn;

	Remove(resetBtn);
	delete resetBtnTxt;
	delete resetBtnImg;
	delete resetBtn;

	delete FileList;

	Resources::Remove(downloadImgData);
}

void LanguageSettingsMenu::SetupOptions()
{
	options.ClearList();

	char langpath[150];
	snprintf(langpath, sizeof(langpath), "%s", Settings.LanguagePath);
	if(langpath[strlen(langpath)-1] != '/')
	{
		char * ptr = strrchr(langpath, '/');
		if(ptr)
			ptr[1] = '\0';
	}

	if(FileList)
		delete FileList;

	FileList = new DirList(langpath, ".lang");
	int filecount = FileList->GetFilecount();

	for(int i = 0; i < filecount; i++)
	{
		options.SetName(i, FileList->GetFilename(i));
		options.SetValue(i, " ");
	}
}

void LanguageSettingsMenu::OnDownloadButtonClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED)
{
	int choice = WindowPrompt(0, tr("Do you want to download new language files?"), tr("Yes"), tr("Cancel"));
	if(choice)
	{
		int result = UpdateLanguageFiles();
		if(result > 0)
		{
			WindowPrompt(0, fmt(tr("Downloaded %d files."), result), tr("OK"));
			SetupOptions();
		}
	}
}

void LanguageSettingsMenu::OnResetButtonClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED)
{
	int choice = WindowPrompt(tr("Language Settings"), tr("Do you want to load the default language?"), tr("Console Default"), tr("App Default"), tr("Cancel"));
	if(choice == 1)
	{
		Settings.LoadLanguage(NULL, CONSOLE_DEFAULT);
		CloseMenu();
	}
	else if(choice == 2)
	{
		Settings.LoadLanguage(NULL, APP_DEFAULT);
		CloseMenu();
	}
}

void LanguageSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option)
{
	int choice = WindowPrompt(FileList->GetFilename(option), tr("Do you want to load this language ?"), tr("Yes"), tr("Cancel"));
	if(choice)
	{
		if(Settings.LoadLanguage(FileList->GetFilepath(option)))
		{
			CloseMenu();
		}
		else
			ShowError(tr("Failed loading the file."));
	}
}
