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
 * for WiiXplorer 2009
 ***************************************************************************/
#include <unistd.h>
#include "SettingsMenu.h"
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"
#include "sys.h"

SettingsMenu::SettingsMenu(const char * title, OptionList * opts, int returnTo)
    : GuiWindow(screenwidth, screenheight)
{
    menu = MENU_NONE;
    Options = opts;
    returnToMenu = returnTo;

	btnSoundClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    btnOutline = Resources::GetImageData(button_png, button_png_size);
    btnOutlineOver = Resources::GetImageData(button_over_png, button_over_png_size);

    trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	backBtnTxt = new GuiText(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	backBtnImg = new GuiImage(btnOutline);
	backBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	backBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn->SetPosition(50, -65);
	backBtn->SetLabel(backBtnTxt);
	backBtn->SetImage(backBtnImg);
	backBtn->SetSoundClick(btnSoundOver);
	backBtn->SetSoundOver(btnSoundOver);
	backBtn->SetTrigger(trigA);
	backBtn->SetTrigger(trigB);
	backBtn->SetEffectGrow();
    backBtn->Clicked.connect(this, &SettingsMenu::OnButtonClick);

	optionBrowser = new GuiOptionBrowser(584, 248, Options);
	optionBrowser->SetPosition(30, 100);
	optionBrowser->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	titleTxt = new GuiText(title, 24, (GXColor){0, 0, 0, 255});
	titleTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt->SetPosition(50, optionBrowser->GetTop()-35);

	Append(backBtn);
	Append(optionBrowser);
	Append(titleTxt);

    SetEffect(EFFECT_FADE, 50);
}

SettingsMenu::~SettingsMenu()
{
    MainWindow::Instance()->ResumeGui();

    SetEffect(EFFECT_FADE, -50);

    while(this->GetEffect() > 0)
        usleep(100);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();
    MainWindow::Instance()->ResumeGui();

	Resources::Remove(btnSoundClick);
	Resources::Remove(btnSoundOver);

    Resources::Remove(btnOutline);
    Resources::Remove(btnOutlineOver);

    delete backBtnImg;

    delete backBtn;

    delete titleTxt;
    delete backBtnTxt;

    delete optionBrowser;

    delete trigA;
    delete trigB;
}

int SettingsMenu::GetClickedOption()
{
    return optionBrowser->GetClickedOption();
}

int SettingsMenu::GetMenu()
{
    if(shutdown == 1)
    {
        Settings.Save();
        Sys_Shutdown();
    }

    else if(reset == 1)
    {
        Settings.Save();
        Sys_Reboot();
    }

    return menu;
}

void SettingsMenu::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == backBtn)
    {
        menu = returnToMenu;
    }
}

