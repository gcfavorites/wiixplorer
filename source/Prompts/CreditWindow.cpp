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
 * CreditWindow.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include "Language/gettext.h"
#include "Controls/MainWindow.h"
#include "CreditWindow.h"
#include "svnrev.h"


CreditWindow::CreditWindow()
    : GuiWindow(0, 0)
{
    choice = -1;

    dialogBox = Resources::GetImageData(bg_properties_png, bg_properties_png_size);
    dialogBoxImg = new GuiImage(dialogBox);

    width = dialogBox->GetWidth();
    height = dialogBox->GetHeight();

    SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

    trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    btnClick = Resources::GetSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    int i = 0;
    int y = 30;

    Entrie[i] = new GuiText(tr("Credits"), 28, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    Entrie[i]->SetPosition(0, y);
    i++;
    y += 60;

    Entrie[i] = new GuiText(tr("Coders:"), 24, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(20, y);
    i++;

    Entrie[i] = new GuiText("Dimok", 22, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(170, y);
    i++;
    y += 32;

    Entrie[i] = new GuiText("r-win", 22, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(170, y);
    i++;
    y += 40;

    Entrie[i] = new GuiText(tr("Designer:"), 24, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(20, y);
    i++;

    Entrie[i] = new GuiText("NeoRame", 22, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(170, y);
    i++;
    y += 60;

    Entrie[i] = new GuiText(tr("Special thanks to:"), 24, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(20,y);
    i++;
    y += 30;

    char text[80];
    snprintf(text, sizeof(text), "Dj_Skual %s", tr("and all the translators."));
    Entrie[i] = new GuiText(text, 22, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(20,y);
    i++;
    y += 30;

    snprintf(text, sizeof(text), "Tantric %s ", tr("for his great tool LibWiiGui."));
    Entrie[i] = new GuiText(text, 22, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(20,y);
    i++;
    y += 30;

    Entrie[i] = new GuiText(tr("The whole DevkitPro & libogc staff."), 22, (GXColor) {0, 0, 0, 255});
    Entrie[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Entrie[i]->SetPosition(20,y);

    arrowUp = Resources::GetImageData(scrollbar_arrowup_png, scrollbar_arrowup_png_size);
    arrowUpOver = Resources::GetImageData(scrollbar_arrowup_over_png, scrollbar_arrowup_over_png_size);
    arrowUpImg = new GuiImage(arrowUp);
    arrowUpImg->SetAngle(45);
    arrowUpImgOver = new GuiImage(arrowUpOver);
    arrowUpImgOver->SetAngle(45);
    Backbtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
    Backbtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    Backbtn->SetSoundClick(btnClick);
    Backbtn->SetImage(arrowUpImg);
    Backbtn->SetImageOver(arrowUpImgOver);
    Backbtn->SetPosition(-20, 20);
    Backbtn->SetEffectGrow();
    Backbtn->SetTrigger(trigA);
    Backbtn->SetTrigger(trigB);
    Backbtn->Clicked.connect(this, &CreditWindow::OnButtonClick);

    char Rev[50];
    sprintf(Rev, "Rev. %i", atoi(SvnRev()));

    RevNum = new GuiText(Rev, 22, (GXColor) {0, 0, 0, 255});
    RevNum->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    RevNum->SetPosition(20, 20);

    Append(dialogBoxImg);
    Append(RevNum);
    for(int i = 0; i < CREDITENTRIES; i++)
        Append(Entrie[i]);
    Append(Backbtn);

    MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->SetDim(true);
    this->SetState(STATE_DEFAULT);
    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 40);
}

CreditWindow::~CreditWindow()
{
    MainWindow::Instance()->ResumeGui();
    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 40);
    while(this->GetEffect() > 0) usleep(100);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

    Resources::Remove(btnClick);
    Resources::Remove(dialogBox);
    Resources::Remove(arrowUp);
    Resources::Remove(arrowUpOver);
    delete dialogBoxImg;
    delete arrowUpImg;
    delete arrowUpImgOver;

    delete Backbtn;

    delete RevNum;

    delete trigA;
    delete trigB;

    for(int i = 0; i < CREDITENTRIES; i++)
    {
        delete Entrie[i];
        Entrie[i] = NULL;
    }

    MainWindow::Instance()->SetState(STATE_DEFAULT);
    MainWindow::Instance()->SetDim(false);
    MainWindow::Instance()->ResumeGui();
}

int CreditWindow::GetChoice()
{
    return choice;
}

void CreditWindow::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == Backbtn)
    {
        choice = 1;
    }
}
