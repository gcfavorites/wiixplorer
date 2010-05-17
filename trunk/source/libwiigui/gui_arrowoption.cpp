/***************************************************************************
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
#include "Memory/Resources.h"
#include "gui_arrowoption.h"
#include "main.h"

GuiArrowOption::GuiArrowOption()
    : GuiWindow(440, 338)
{
	btnClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    ArrowImgData = Resources::GetImageData(arrow_right_png, arrow_right_png_size);
    trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
}

GuiArrowOption::~GuiArrowOption()
{
    ClearList();

    Resources::Remove(btnClick);
    Resources::Remove(btnSoundOver);
    Resources::Remove(ArrowImgData);

    delete trigA;
}

void GuiArrowOption::AddOption(const char * name, int PositionX, int PositionY)
{
    int Center = PositionX;

    GuiText * OptName = new GuiText(name, 16, (GXColor){0, 0, 0, 255});
    OptName->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    OptName->SetPosition(Center-OptName->GetTextWidth()/2, PositionY);

    GuiText * OptText = new GuiText(" ", 16, (GXColor){0, 0, 0, 255});
    OptText->SetPosition(Center-OptText->GetTextWidth()/2, PositionY+30);
    OptText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

    GuiButton * OptBtn = new GuiButton(OptName->GetTextWidth(), 18);
    OptBtn->SetSoundOver(btnSoundOver);
    OptBtn->SetSoundClick(btnClick);
    OptBtn->SetTrigger(trigA);
    OptBtn->SetPosition(Center-OptText->GetTextWidth()/2, PositionY+30);
    OptBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    OptBtn->Clicked.connect(this, &GuiArrowOption::OnButtonClick);

    GuiImage * OptImgLeft = new GuiImage(ArrowImgData);
    OptImgLeft->SetAngle(180);
	GuiButton * OptBtnLeft = new GuiButton(OptImgLeft->GetWidth(), OptImgLeft->GetHeight());
    OptBtnLeft->SetImage(OptImgLeft);
    OptBtnLeft->SetSoundOver(btnSoundOver);
    OptBtnLeft->SetSoundClick(btnClick);
    OptBtnLeft->SetTrigger(trigA);
    OptBtnLeft->SetEffectGrow();
    OptBtnLeft->SetPosition(Center-(OptText->GetTextWidth()/2+10), PositionY+30);
    OptBtnLeft->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    OptBtnLeft->Clicked.connect(this, &GuiArrowOption::OnLeftButtonClick);

    GuiImage * OptImgRight = new GuiImage(ArrowImgData);
	GuiButton * OptBtnRight = new GuiButton(OptImgRight->GetWidth(), OptImgRight->GetHeight());
    OptBtnRight->SetImage(OptImgRight);
    OptBtnRight->SetSoundOver(btnSoundOver);
    OptBtnRight->SetSoundClick(btnClick);
    OptBtnRight->SetTrigger(trigA);
    OptBtnRight->SetEffectGrow();
    OptBtnRight->SetPosition(Center+(OptText->GetTextWidth()/2+10), PositionY+30);
    OptBtnRight->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    OptBtnRight->Clicked.connect(this, &GuiArrowOption::OnRightButtonClick);

    Append(OptName);
    Append(OptText);
    Append(OptBtn);
    Append(OptBtnLeft);
    Append(OptBtnRight);

    OptionsName.push_back(OptName);
    OptionsText.push_back(OptText);
    OptionsBtn.push_back(OptBtn);
    OptionsImgLeft.push_back(OptImgLeft);
    OptionsBtnLeft.push_back(OptBtnLeft);
    OptionsImgRight.push_back(OptImgRight);
    OptionsBtnRight.push_back(OptBtnRight);
}

void GuiArrowOption::SetOptionName(int i, const char * text)
{
    if(i < 0 || i >= (int) OptionsName.size())
        return;

    int PositionY = OptionsName.at(i)->GetTop();
    int OldCenter = OptionsName.at(i)->GetLeft()+OptionsName.at(i)->GetTextWidth()/2;

    OptionsName.at(i)->SetTextf(text);
    OptionsName.at(i)->SetPosition(OldCenter-OptionsName.at(i)->GetTextWidth()/2, PositionY);
}

void GuiArrowOption::SetOptionValue(int i, const char * text)
{
    if(i < 0 || i >= (int) OptionsText.size())
        return;

    int PositionY = OptionsText.at(i)->GetTop()-GetTop();
    int Center = OptionsText.at(i)->GetLeft()+OptionsText.at(i)->GetTextWidth()/2-GetLeft();

    OptionsText.at(i)->SetTextf(text);

    int textWidth = OptionsText.at(i)->GetTextWidth();

    OptionsText.at(i)->SetPosition(Center-textWidth/2, PositionY);

    OptionsBtn.at(i)->SetSize(textWidth, 18);
    OptionsBtn.at(i)->SetPosition(Center-textWidth/2, PositionY);
    OptionsBtnLeft.at(i)->SetPosition(Center-textWidth/2-OptionsBtnLeft.at(i)->GetWidth()-10, PositionY);
    OptionsBtnRight.at(i)->SetPosition(Center+textWidth/2+10, PositionY);
}

void GuiArrowOption::RemoveOption(int i)
{
    if(i < 0 || i >= (int) OptionsBtn.size())
        return;

    Remove(OptionsName.at(i));
    delete OptionsName.at(i);
    Remove(OptionsText.at(i));
    delete OptionsText.at(i);
    Remove(OptionsBtn.at(i));
    delete OptionsBtn.at(i);
    Remove(OptionsImgLeft.at(i));
    delete OptionsImgLeft.at(i);
    Remove(OptionsBtnLeft.at(i));
    delete OptionsBtnLeft.at(i);
    Remove(OptionsImgRight.at(i));
    delete OptionsImgRight.at(i);
    Remove(OptionsBtnRight.at(i));
    delete OptionsBtnRight.at(i);

    OptionsName.erase(OptionsName.begin()+i);
    OptionsText.erase(OptionsText.begin()+i);
    OptionsBtn.erase(OptionsBtn.begin()+i);
    OptionsImgLeft.erase(OptionsImgLeft.begin()+i);
    OptionsBtnLeft.erase(OptionsBtnLeft.begin()+i);
    OptionsImgRight.erase(OptionsImgRight.begin()+i);
    OptionsBtnRight.erase(OptionsBtnRight.begin()+i);
}

int GuiArrowOption::GetOptionCount()
{
    return OptionsBtn.size();
}

void GuiArrowOption::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    ClickedButton(sender, pointer, p);
}

void GuiArrowOption::OnRightButtonClick(GuiElement *sender, int pointer, POINT p)
{
    ClickedRight(sender, pointer, p);
}

void GuiArrowOption::OnLeftButtonClick(GuiElement *sender, int pointer, POINT p)
{
    ClickedLeft(sender, pointer, p);
}

GuiButton * GuiArrowOption::GetButton(int i)
{
    if(i < 0 || i >= (int) OptionsBtn.size())
        return NULL;

    return OptionsBtn.at(i);
}

GuiButton * GuiArrowOption::GetButtonLeft(int i)
{
    if(i < 0 || i >= (int) OptionsBtnLeft.size())
        return NULL;

    return OptionsBtnLeft.at(i);
}

GuiButton * GuiArrowOption::GetButtonRight(int i)
{
    if(i < 0 || i >= (int) OptionsBtnRight.size())
        return NULL;

    return OptionsBtnRight.at(i);
}

void GuiArrowOption::ClearList()
{
    RemoveAll();

    for(u32 i = 0; i < OptionsBtn.size(); i++)
    {
        delete OptionsName.at(i);
        delete OptionsText.at(i);
        delete OptionsBtn.at(i);
        delete OptionsImgLeft.at(i);
        delete OptionsBtnLeft.at(i);
        delete OptionsImgRight.at(i);
        delete OptionsBtnRight.at(i);
    }

    OptionsName.clear();
    OptionsText.clear();
    OptionsBtn.clear();
    OptionsImgLeft.clear();
    OptionsBtnLeft.clear();
    OptionsImgRight.clear();
    OptionsBtnRight.clear();
}
