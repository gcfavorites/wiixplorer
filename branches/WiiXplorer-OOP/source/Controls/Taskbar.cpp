/***************************************************************************
 * Copyright (C) 2009
 * by r-win
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
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#include <time.h>

#include "Taskbar.h"

extern u8 shutdown;

Taskbar *Taskbar::instance = NULL;

Taskbar::Taskbar()
	: GuiWindow(screenwidth, 32)
{
	taskbarImgData = new GuiImageData(taskbar_png);
	taskbarImg = new GuiImage(taskbarImgData);

	width = taskbarImg->GetWidth();
	height = taskbarImg->GetHeight();

	SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	SetPosition(4, -15);

	timeTxt = new GuiText("", 20, (GXColor) {40, 40, 40, 255});
	timeTxt->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	timeTxt->SetPosition(540, 0);
	timeTxt->SetFont(clock_ttf, clock_ttf_size);

	soundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	soundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	settingsBtn = new PictureButton(settingsbtn_png, settingsbtn_over_png, soundClick, soundOver);
	settingsBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	settingsBtn->SetPosition(87, 0);
	settingsBtn->SetTrigger(trigA);

	exitBtn = new PictureButton(power_png, power_over_png, soundClick, soundOver);
	exitBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	exitBtn->SetPosition(490, 0);
	exitBtn->SetTrigger(trigA);

	Append(taskbarImg);
	Append(timeTxt);
	Append(settingsBtn);
	Append(exitBtn);
}

Taskbar::~Taskbar()
{
	delete taskbarImg;
	delete taskbarImgData;

	delete timeTxt;
	delete settingsBtn;
	delete exitBtn;
	delete soundClick;
	delete soundOver;

	delete trigA;
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
    GuiWindow::Update(t);

    if(exitBtn->GetState() == STATE_CLICKED)
    {
        shutdown = 1;
        exitBtn->ResetState();
    }
    else if(settingsBtn->GetState() == STATE_CLICKED)
    {
        settingsBtn->ResetState();
    }
}
