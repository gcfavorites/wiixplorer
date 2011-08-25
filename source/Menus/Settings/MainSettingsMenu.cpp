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
#include "MainSettingsMenu.h"
#include "Controls/Application.h"
#include "Prompts/PromptWindows.h"
#include "GeneralSettingsMenu.h"
#include "PathSettingsMenu.h"
#include "LanguageSettingsMenu.h"
#include "BootSettingsMenu.h"
#include "ControlsSettingsMenu.h"
#include "ExplorerSettingsMenu.h"
#include "FileExtSettingsMenu.h"
#include "NetworkSettingsMenu.h"
#include "SoundSettingsMenu.h"
#include "ImageSettingsMenu.h"
#include "Settings.h"

MainSettingsMenu::MainSettingsMenu(GuiFrame *r)
	: SettingsMenu(tr("Settings"), r)
{
	resetBtnTxt = new GuiText(tr("Reset"), 22, (GXColor){0, 0, 0, 255});
	resetBtnImg = new GuiImage(btnOutline);
	resetBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	resetBtn->SetAlignment(ALIGN_CENTER | ALIGN_BOTTOM);
	resetBtn->SetPosition(100+btnOutline->GetWidth()/2, -65);
	resetBtn->SetLabel(resetBtnTxt);
	resetBtn->SetImage(resetBtnImg);
	resetBtn->SetSoundOver(btnSoundOver);
	resetBtn->SetTrigger(trigA);
	resetBtn->SetEffectGrow();
	resetBtn->Clicked.connect(this, &MainSettingsMenu::OnResetButtonClick);
	Append(resetBtn);

	SetupOptions();
}

MainSettingsMenu::~MainSettingsMenu()
{
	Remove(resetBtn);
	delete resetBtnTxt;
	delete resetBtnImg;
	delete resetBtn;
}

void MainSettingsMenu::OnResetButtonClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED)
{
	int choice = WindowPrompt(tr("Do you want to reset the settings?"), 0, tr("Yes"), tr("Cancel"));
	if(choice)
	{
		Settings.Reset();
		SetOptionValues();
	}
}

void MainSettingsMenu::SetupOptions()
{
	int i = 0;

	options.SetName(i++, tr("Language"));
	options.SetName(i++, tr("Clock Mode"));
	options.SetName(i++, tr("General Settings"));
	options.SetName(i++, tr("Explorer Settings"));
	options.SetName(i++, tr("Boot Settings"));
	options.SetName(i++, tr("Image Settings"));
	options.SetName(i++, tr("Sound Settings"));
	options.SetName(i++, tr("Network Settings"));
	options.SetName(i++, tr("File Extensions"));
	options.SetName(i++, tr("Controls Settings"));
	options.SetName(i++, tr("Path Setup"));
	options.SetName(i++, tr("Color Settings"));

	SetOptionValues();
}

void MainSettingsMenu::SetOptionValues()
{
	int i = 0;

	if(strcmp(Settings.LanguagePath, "") != 0)
	{
		if(strcmp(Settings.LanguagePath, "") == 0)
		{
			options.SetValue(i++, tr("Standard"));
		}
		else if(Settings.LanguagePath[strlen(Settings.LanguagePath)-1] == '/')
		{
			options.SetValue(i++, tr("Standard"));
		}
		else
		{
			char * language = strrchr(Settings.LanguagePath, '/')+1;
			options.SetValue(i++, "%s", language);
		}
	}
	else
		options.SetValue(i++, tr("App Default"));

	if (Settings.ClockMode == 1)
		options.SetValue(i++, tr("12H"));
	else
		options.SetValue(i++, tr("24H"));

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");

	options.SetValue(i++, " ");
}

void MainSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option)
{
	switch(option)
	{
		case 0:
		{
			this->hide();

			LanguageSettingsMenu *menu = new LanguageSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 1:
		{
			Settings.ClockMode = (Settings.ClockMode+1) % 2;
			break;
		}
		case 2:
		{
			this->hide();

			GeneralSettingsMenu *menu = new GeneralSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 3:
		{
			this->hide();

			ExplorerSettingsMenu *menu = new ExplorerSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 4:
		{
			this->hide();

			BootSettingsMenu *menu = new BootSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 5:
		{
			this->hide();

			ImageSettingsMenu *menu = new ImageSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 6:
		{
			this->hide();

			SoundSettingsMenu *menu = new SoundSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 7:
		{
			this->hide();

			NetworkSettingsMenu *menu = new NetworkSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 8:
		{
			this->hide();

			FileExtSettingsMenu *menu = new FileExtSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 9:
		{
			this->hide();

			ControlsSettingsMenu *menu = new ControlsSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 10:
		{
			this->hide();

			PathSettingsMenu *menu = new PathSettingsMenu(this);
			Application::Instance()->Append(menu);
			break;
		}
		case 11:
			//menu = MENU_COLOR_SETTINGS;
			break;
		default:
			break;
	}

	SetOptionValues();
}
