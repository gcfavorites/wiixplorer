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
 * PromptWindows.cpp
 *
 * All promptwindows
 * for WiiXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <unistd.h>

#include "Prompts/PromptWindow.h"
#include "filelist.h"
#include "Language/gettext.h"
#include "Controls/MainWindow.h"
#include "sys.h"

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;

/****************************************************************************
* PromptWindow Class
*
* Displays a prompt window to user, with information, an error message, or
* presenting a user with a choice of up to 4 Buttons.
*
* Give him 1 Titel, 1 Subtitel and 4 Buttons
* If titel/subtitle or one of the buttons is not needed give him a 0 on that
* place.
***************************************************************************/
PromptWindow::PromptWindow(const char *title, const char *msg,
                        const char *btn1Label, const char *btn2Label,
                        const char *btn3Label, const char *btn4Label)
	: GuiWindow(440, 270)
{
    choice = -1;

	btnClick = Resources::GetSound(button_click_pcm, button_click_pcm_size);
	btnSoundOver = Resources::GetSound(button_over_pcm, button_over_pcm_size);

	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    bgWindow = Resources::GetImageData(dialogue_box_png, dialogue_box_png_size);
    bgWindowImg = new GuiImage(bgWindow);
    width = bgWindow->GetWidth();
    height = bgWindow->GetHeight();

    btnOutline = Resources::GetImageData(button_png, button_png_size);

    titleTxt = new GuiText(title, 22, (GXColor){0, 0, 0, 255});
    titleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt->SetPosition(0,55);
    titleTxt->SetMaxWidth(this->GetWidth()-68, DOTTED);

    msgTxt = new GuiText(msg, 20, (GXColor){0, 0, 0, 255});
    msgTxt->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt->SetPosition(0,-40);
    msgTxt->SetMaxWidth(this->GetWidth()-68, WRAP);

    btn1Txt = new GuiText(btn1Label, 18, (GXColor){0, 0, 0, 255});
    btn1Img = new GuiImage(btnOutline);
    btn1 = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
    btn1->SetLabel(btn1Txt);
    btn1->SetImage(btn1Img);
    btn1->SetSoundOver(btnSoundOver);
    btn1->SetSoundClick(btnClick);
    btn1->SetTrigger(trigA);
    btn1->SetState(STATE_SELECTED);
    btn1->SetEffectGrow();
	btn1->Clicked.connect(this, &PromptWindow::OnButtonClick);

	btn2Txt = NULL;
	btn2Img = NULL;
	btn2 = NULL;

    if(btn2Label)
    {
        btn2Txt = new GuiText(btn2Label, 18, (GXColor){0, 0, 0, 255});
        btn2Img = new GuiImage(btnOutline);
        btn2 = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
        btn2->SetLabel(btn2Txt);
        btn2->SetImage(btn2Img);
        btn2->SetSoundOver(btnSoundOver);
        btn2->SetSoundClick(btnClick);
        if(!btn3Label && !btn4Label)
            btn2->SetTrigger(trigB);
        btn2->SetTrigger(trigA);
        btn2->SetEffectGrow();
        btn2->Clicked.connect(this, &PromptWindow::OnButtonClick);
    }

	btn3Txt = NULL;
	btn3Img = NULL;
	btn3 = NULL;

    if(btn3Label)
    {
        btn3Txt = new GuiText(btn3Label, 18, (GXColor){0, 0, 0, 255});
        btn3Img = new GuiImage(btnOutline);
        btn3 = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
        btn3->SetLabel(btn3Txt);
        btn3->SetImage(btn3Img);
        btn3->SetSoundOver(btnSoundOver);
        btn3->SetSoundClick(btnClick);
        if(!btn4Label)
            btn3->SetTrigger(trigB);
        btn3->SetTrigger(trigA);
        btn3->SetEffectGrow();
        btn3->Clicked.connect(this, &PromptWindow::OnButtonClick);
    }

	btn4Txt = NULL;
	btn4Img = NULL;
	btn4 = NULL;

    if(btn4Label)
    {
        btn4Txt = new GuiText(btn4Label, 22, (GXColor){0, 0, 0, 255});
        btn4Img = new GuiImage(btnOutline);
        btn4 = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
        btn4->SetLabel(btn4Txt);
        btn4->SetImage(btn4Img);
        btn4->SetSoundOver(btnSoundOver);
        btn4->SetSoundClick(btnClick);
        if(btn4Label)
            btn4->SetTrigger(trigB);
        btn4->SetTrigger(trigA);
        btn4->SetEffectGrow();
        btn4->Clicked.connect(this, &PromptWindow::OnButtonClick);
    }

    if(btn2Label && !btn3Label && !btn4Label) {
        btn1->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1->SetPosition(40, -50);
        btn2->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2->SetPosition(-40, -50);
    } else if(btn2Label && btn3Label && !btn4Label) {
        btn1->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1->SetPosition(50, -120);
        btn2->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2->SetPosition(-50, -120);
        btn3->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn3->SetPosition(0, -65);
    } else if(btn2Label && btn3Label && btn4Label) {
        btn1->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1->SetPosition(50, -120);
        btn2->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2->SetPosition(-50, -120);
        btn3->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn3->SetPosition(50, -65);
        btn4->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn4->SetPosition(-50, -65);
    } else if(!btn2Label && btn3Label && btn4Label) {
        btn1->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn1->SetPosition(0, -120);
        btn3->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn3->SetPosition(50, -65);
        btn4->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn4->SetPosition(-50, -65);
    } else {
        btn1->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn1->SetPosition(0, -50);
    }

    Append(bgWindowImg);
    Append(titleTxt);
    Append(msgTxt);

    if(btn1Label)
        Append(btn1);
    if(btn2Label)
        Append(btn2);
    if(btn3Label)
        Append(btn3);
    if(btn4Label)
        Append(btn4);

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    SetPosition(0,0);
    SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

    MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->SetDim(true);
    this->SetState(STATE_DEFAULT);
}

PromptWindow::~PromptWindow()
{
    MainWindow::Instance()->ResumeGui();
    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(this->GetEffect() > 0) usleep(100);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

    if(btn1Img)
        delete btn1Img;
    if(btn2Img)
        delete btn2Img;
    if(btn3Img)
        delete btn3Img;
    if(btn4Img)
        delete btn4Img;

    delete bgWindowImg;

	Resources::Remove(btnOutline);
	Resources::Remove(bgWindow);

	Resources::Remove(btnSoundOver);
	Resources::Remove(btnClick);

    delete titleTxt;
    delete msgTxt;

    if(btn1Txt)
        delete btn1Txt;
    if(btn2Txt)
        delete btn2Txt;
    if(btn3Txt)
        delete btn3Txt;
    if(btn4Txt)
        delete btn4Txt;

    if(btn1)
        delete btn1;
    if(btn2)
        delete btn2;
    if(btn3)
        delete btn3;
    if(btn4)
        delete btn4;

    delete trigA;
    delete trigB;

    MainWindow::Instance()->SetState(STATE_DEFAULT);
    MainWindow::Instance()->SetDim(false);
    MainWindow::Instance()->ResumeGui();
}

int PromptWindow::GetChoice()
{
    return choice;
}

void PromptWindow::SetTitle(const char *title)
{
    titleTxt->SetText(title);
}

void PromptWindow::SetMessage(const char *msg)
{
    msgTxt->SetText(msg);
}

void PromptWindow::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == btn1)
        choice = 1;
    else if(sender == btn2 && !btn3 && !btn4)
        choice = 0;
    else if(sender == btn2 && (btn3 || btn4))
        choice = 2;
    else if(sender == btn3 && !btn4)
        choice = 0;
    else if(sender == btn3 && btn4)
        choice = 3;
    else
        choice = 0;
}
