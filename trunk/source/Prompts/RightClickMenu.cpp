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
 * RightClickMenu.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include "RightClickMenu.h"
#include "Controls/MainWindow.h"
#include "Language/gettext.h"


RightClickMenu::RightClickMenu(int x, int y, const char *btn1Label, const char *btn2Label,
                                             const char *btn3Label, const char *btn4Label,
                                             const char *btn5Label, const char *btn6Label,
                                             const char *btn7Label)
    :GuiWindow(0, 0)
{
    choice = -1;
    numItems = 0;
    buttonX = 15;
    buttonY = 0;
    const int EntrieHeight = 33;

    const char * labelpnt[MAX_CLICKBUTTONS];

    for(int i = 0; i < MAX_CLICKBUTTONS; i++)
    {
        labelpnt[i] = NULL;
        btnMenuSelect[i] = NULL;
        ButtonTxt[i] = NULL;
        ButtonTxtOver[i] = NULL;
        Button[i] = NULL;
    }

    if(!btn1Label)
        return;

    if(btn1Label)
    {
        labelpnt[numItems] = btn1Label;
        numItems++;
    }
    if(btn2Label)
    {
        labelpnt[numItems] = btn2Label;
        numItems++;
    }
    if(btn3Label)
    {
        labelpnt[numItems] = btn3Label;
        numItems++;
    }
    if(btn4Label)
    {
        labelpnt[numItems] = btn4Label;
        numItems++;
    }
    if(btn5Label)
    {
        labelpnt[numItems] = btn5Label;
        numItems++;
    }
    if(btn6Label)
    {
        labelpnt[numItems] = btn6Label;
        numItems++;
    }
    if(btn7Label)
    {
        labelpnt[numItems] = btn7Label;
        numItems++;
    }


    ClickMenuMiddle = Resources::GetImageData(ClickMenuMiddle_png, ClickMenuMiddle_png_size);
    ClickMenuUpper = Resources::GetImageData(ClickMenuUpper_png, ClickMenuUpper_png_size);
    ClickMenuMiddleImg = new GuiImage(ClickMenuMiddle);
    ClickMenuUpperImg = new GuiImage(ClickMenuUpper);
    ClickMenuLowerImg = new GuiImage(ClickMenuUpper);
    ClickMenuLowerImg->SetAngle(180);

    menu_select = Resources::GetImageData(menu_selection_png, menu_selection_png_size);

	btnClick = Resources::GetSound(button_click_pcm, button_click_pcm_size);

	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    int middleheight = EntrieHeight*numItems;
    int tileCount = middleheight/4;

    ClickMenuMiddleImg->SetTileVertical(tileCount);
    ClickMenuMiddleImg->SetPosition(0, ClickMenuUpperImg->GetHeight());
    ClickMenuUpperImg->SetPosition(0, 0);
    ClickMenuLowerImg->SetPosition(1, ClickMenuUpperImg->GetHeight()+tileCount*4);

    width = ClickMenuUpperImg->GetWidth();
    height = ClickMenuUpperImg->GetHeight()+middleheight+ClickMenuLowerImg->GetHeight();

    if(screenwidth < x + width + 60)
        x = screenwidth - width - 60;

    if(screenheight < y + height + 60)
        y = screenheight - height - 60;

    NoBtn = new GuiButton(screenwidth, screenheight);
    NoBtn->SetPosition(-x, -y);
    NoBtn->SetTrigger(trigA);
    NoBtn->SetTrigger(trigB);
    NoBtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);

    Append(ClickMenuUpperImg);
    Append(ClickMenuMiddleImg);
    Append(ClickMenuLowerImg);
    Append(NoBtn);

    SetPosition(x, y);

    buttonY = ClickMenuUpperImg->GetHeight()+5;

    int maxTxtWidth = 0;

    for(int i = 0; i < numItems; i++)
    {
        ButtonTxt[i] = new GuiText(labelpnt[i], 24, (GXColor){0, 0, 0, 255});
        ButtonTxtOver[i] = new GuiText(labelpnt[i], 24, (GXColor){28, 32, 190, 255});
        ButtonTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        ButtonTxtOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        Button[i] = new GuiButton(width, height/numItems);
        Button[i]->SetLabel(ButtonTxt[i]);
        Button[i]->SetLabelOver(ButtonTxtOver[i]);
        Button[i]->SetSoundClick(btnClick);
        btnMenuSelect[i] = new GuiImage(menu_select);
        Button[i]->SetImageOver(btnMenuSelect[i]);
        Button[i]->SetTrigger(trigA);
        Button[i]->SetPosition(buttonX, buttonY);
        Button[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        Button[i]->Clicked.connect(this, &RightClickMenu::OnButtonClick);
        buttonY += EntrieHeight;

        int buttonTxtWidth = ButtonTxt[i]->GetTextWidth();
        int buttonTxtOverWidth = ButtonTxtOver[i]->GetTextWidth();

        if(maxTxtWidth < buttonTxtWidth)
            maxTxtWidth = buttonTxtWidth;
        if(maxTxtWidth < buttonTxtOverWidth)
            maxTxtWidth = buttonTxtOverWidth;

        Append(Button[i]);
    }

    float newscale = 1.0f;

    if(maxTxtWidth > (ClickMenuMiddleImg->GetWidth()-30))
        newscale = 1.0f * maxTxtWidth/(ClickMenuMiddleImg->GetWidth()-30);

    ClickMenuMiddleImg->SetScaleX(newscale);
    ClickMenuUpperImg->SetScaleX(newscale);
    ClickMenuLowerImg->SetScaleX(newscale);

    int NewPosX = ClickMenuMiddleImg->GetLeft()-GetLeft()+(newscale-1)*ClickMenuMiddleImg->GetWidth()/2;
    ClickMenuMiddleImg->SetPosition(NewPosX, ClickMenuMiddleImg->GetTop()-GetTop());
    ClickMenuUpperImg->SetPosition(NewPosX, ClickMenuUpperImg->GetTop()-GetTop());
    ClickMenuLowerImg->SetPosition(NewPosX, ClickMenuLowerImg->GetTop()-GetTop());
}

RightClickMenu::~RightClickMenu()
{
    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

	Resources::Remove(ClickMenuMiddle);
	Resources::Remove(ClickMenuUpper);
	Resources::Remove(menu_select);
	Resources::Remove(btnClick);

    for(int i = 0; i < MAX_CLICKBUTTONS; i++)
    {
        delete btnMenuSelect[i];
        delete Button[i];
        delete ButtonTxt[i];
        delete ButtonTxtOver[i];
    }

    delete ClickMenuMiddleImg;
    delete ClickMenuUpperImg;
    delete ClickMenuLowerImg;
    delete NoBtn;

    delete trigA;
    delete trigB;

    MainWindow::Instance()->ResumeGui();
}

int RightClickMenu::GetChoice()
{
    return choice;
}

void RightClickMenu::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();
    //TODO add the functions instead of the link
    if(sender == NoBtn)
    {
        choice = NoButton;
        return;
    }

    for(int i = 0; i < numItems; i++)
    {
        if(sender == Button[i])
        {
            choice = i;
            break;
        }
    }
}
