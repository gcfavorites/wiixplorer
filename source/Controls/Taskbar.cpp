/***************************************************************************
 * Copyright (C) 2009
 * by r-win & Dimok
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
 * for WiiXplorer 2009
 ***************************************************************************/

#include <time.h>

#include "menu.h"
#include "Taskbar.h"
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"

Taskbar *Taskbar::instance = NULL;

Taskbar::Taskbar()
	: GuiWindow(0, 0)
{
    menu = MENU_NONE;
    triggerupdate = false;

	taskbarImgData = Resources::GetImageData(taskbar_png, taskbar_png_size);
	taskbarImg = new GuiImage(taskbarImgData);
	taskbarImg->SetWidescreen(true);

	width = taskbarImg->GetWidth();
	height = taskbarImg->GetHeight();

	timeTxt = new GuiText("", 20, (GXColor) {40, 40, 40, 255});
	timeTxt->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	timeTxt->SetPosition(517, 0);
	timeTxt->SetFont(clock_ttf, clock_ttf_size);

	soundClick = Resources::GetSound(button_click_pcm, button_click_pcm_size);
	soundOver = Resources::GetSound(button_over_pcm, button_over_pcm_size);
	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	settingsBtn = new PictureButton(settingsbtn_png, settingsbtn_png_size, settingsbtn_over_png, settingsbtn_over_png_size, soundClick, soundOver);
	settingsBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	settingsBtn->SetPosition(108, 0);
	settingsBtn->SetSelectable(false);
	settingsBtn->SetTrigger(trigA);
	settingsBtn->Clicked.connect(this, &Taskbar::OnSettingsClick);

	ftpBtn = new PictureButton(ftpbtn_png, ftpbtn_png_size, ftpbtn_over_png, ftpbtn_over_png_size, soundClick, soundOver);
	ftpBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	ftpBtn->SetPosition(108+72+8, 0);
	ftpBtn->SetSelectable(false);
	ftpBtn->SetTrigger(trigA);
	ftpBtn->Clicked.connect(this, &Taskbar::OnFtpClick);

	exitBtn = new PictureButton(power_png, power_png_size, power_over_png, power_over_png_size, soundClick, soundOver);
	exitBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	exitBtn->SetPosition(465, 0);
	exitBtn->SetSelectable(false);
	exitBtn->SetTrigger(trigA);
	exitBtn->Clicked.connect(this, &Taskbar::OnExitClick);

	Append(taskbarImg);
	Append(timeTxt);
	Append(settingsBtn);
	Append(ftpBtn);
	Append(exitBtn);

	SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	SetPosition(0, -15);
}

Taskbar::~Taskbar()
{
	settingsBtn->Clicked.disconnect(this);
	exitBtn->Clicked.disconnect(this);

	delete taskbarImg;
	Resources::Remove(taskbarImgData);

	delete timeTxt;
	delete settingsBtn;
	delete exitBtn;

	delete trigA;

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
		strftime(timetxt, sizeof(timetxt), "%H:%M:%S", timeinfo);

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
    return menu;
}

void Taskbar::OnExitClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();
	menu = MENU_EXIT;
}

void Taskbar::OnSettingsClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();
	menu = MENU_SETTINGS;
}


void Taskbar::OnFtpClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();
	menu = MENU_FTP;
}


