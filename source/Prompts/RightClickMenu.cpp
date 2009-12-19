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

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

RightClickMenu::RightClickMenu(int x, int y)
    :GuiWindow(0, 0)
{
    choice = -1;
    numItems = 8;
    buttonY = 0;

    dialogBox = Resources::GetImageData(clickmenu_png, clickmenu_png_size);
    dialogBoxImg = new GuiImage(dialogBox);
    dialogBoxImg->SetPosition(-8, -dialogBox->GetHeight()/numItems/2);

    menu_select = Resources::GetImageData(menu_selection_png, menu_selection_png_size);

    if(screenwidth < x + dialogBox->GetWidth() + 10)
        x = screenwidth - dialogBox->GetWidth() - 10;

    if(screenheight < y + dialogBox->GetHeight() + 10)
        y = screenheight - dialogBox->GetHeight() - 10;

    width = dialogBox->GetWidth();
    height = dialogBox->GetHeight();

    SetPosition(x, y);

	btnClick = Resources::GetSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    cutTxt = new GuiText(tr("Cut"), 24, (GXColor){0, 0, 0, 255});
    cutTxtOver = new GuiText(tr("Cut"), 24, (GXColor){28, 32, 190, 255});
    cutTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    cutTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    btnCut = new GuiButton(width, height/numItems);
    btnCut->SetLabel(cutTxt);
    btnCut->SetLabelOver(cutTxtOver);
    btnCut->SetSoundClick(btnClick);
    btnCutMenuSelect = new GuiImage(menu_select);
    btnCut->SetImageOver(btnCutMenuSelect);
    btnCut->SetTrigger(trigA);
    btnCut->SetPosition(0,buttonY);
    btnCut->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    btnCut->Clicked.connect(this, &RightClickMenu::OnButtonClick);
    buttonY += height/numItems;

    copyTxt = new GuiText(tr("Copy"), 24, (GXColor){0, 0, 0, 255});
    copyTxtOver = new GuiText(tr("Copy"), 24, (GXColor){28, 32, 190, 255});
    copyTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    copyTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Copybtn = new GuiButton(width, height/numItems);
    Copybtn->SetLabel(copyTxt);
    Copybtn->SetLabelOver(copyTxtOver);
    CopybtnMenuSelect = new GuiImage(menu_select);
    Copybtn->SetImageOver(CopybtnMenuSelect);
    Copybtn->SetSoundClick(btnClick);
    Copybtn->SetTrigger(trigA);
    Copybtn->SetPosition(0,buttonY);
    Copybtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Copybtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);
    buttonY += height/numItems;

    pasteTxt = new GuiText(tr("Paste"), 24, (GXColor){0, 0, 0, 255});
    PasteTxtOver = new GuiText(tr("Paste"), 24, (GXColor){28, 32, 190, 255});
    pasteTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PasteTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Pastebtn = new GuiButton(width, height/numItems);
    Pastebtn->SetLabel(pasteTxt);
    Pastebtn->SetLabelOver(PasteTxtOver);
    PastebtnMenuSelect = new GuiImage(menu_select);
    Pastebtn->SetImageOver(PastebtnMenuSelect);
    Pastebtn->SetSoundClick(btnClick);
    Pastebtn->SetTrigger(trigA);
    Pastebtn->SetPosition(0,buttonY);
    Pastebtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Pastebtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);
    buttonY += height/numItems;

    RenameTxt = new GuiText(tr("Rename"), 24, (GXColor){0, 0, 0, 255});
    RenameTxtOver = new GuiText(tr("Rename"), 24, (GXColor){28, 32, 190, 255});
    RenameTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    RenameTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Renamebtn = new GuiButton(width, height/numItems);
    Renamebtn->SetLabel(RenameTxt);
    Renamebtn->SetLabelOver(RenameTxtOver);
    RenamebtnMenuSelect = new GuiImage(menu_select);
    Renamebtn->SetImageOver(RenamebtnMenuSelect);
    Renamebtn->SetSoundClick(btnClick);
    Renamebtn->SetTrigger(trigA);
    Renamebtn->SetPosition(0,buttonY);
    Renamebtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Renamebtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);
    buttonY += height/numItems;

    DeleteTxt = new GuiText(tr("Delete"), 24, (GXColor){0, 0, 0, 255});
    DeleteTxtOver = new GuiText(tr("Delete"), 24, (GXColor){28, 32, 190, 255});
    DeleteTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    DeleteTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Deletebtn = new GuiButton(width, height/numItems);
    Deletebtn->SetLabel(DeleteTxt);
    Deletebtn->SetLabelOver(DeleteTxtOver);
    DeletebtnMenuSelect = new GuiImage(menu_select);
    Deletebtn->SetImageOver(DeletebtnMenuSelect);
    Deletebtn->SetSoundClick(btnClick);
    Deletebtn->SetTrigger(trigA);
    Deletebtn->SetPosition(0,buttonY);
    Deletebtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Deletebtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);
    buttonY += height/numItems;

    NewFolderTxt = new GuiText(tr("New Folder"), 24, (GXColor){0, 0, 0, 255});
    NewFolderTxtOver = new GuiText(tr("New Folder"), 24, (GXColor){28, 32, 190, 255});
    NewFolderTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    NewFolderTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    NewFolderbtn = new GuiButton(width, height/numItems);
    NewFolderbtn->SetLabel(NewFolderTxt);
    NewFolderbtn->SetLabelOver(NewFolderTxtOver);
    NewFolderbtnMenuSelect = new GuiImage(menu_select);
    NewFolderbtn->SetImageOver(NewFolderbtnMenuSelect);
    NewFolderbtn->SetSoundClick(btnClick);
    NewFolderbtn->SetTrigger(trigA);
    NewFolderbtn->SetPosition(0,buttonY);
    NewFolderbtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    NewFolderbtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);
    buttonY += height/numItems;

    PropertiesTxt = new GuiText(tr("Properties"), 24, (GXColor){0, 0, 0, 255});
    PropertiesTxtOver = new GuiText(tr("Properties"), 24, (GXColor){28, 32, 190, 255});
    PropertiesTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PropertiesTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Propertiesbtn = new GuiButton(width, height/numItems);
    Propertiesbtn->SetLabel(PropertiesTxt);
    Propertiesbtn->SetLabelOver(PropertiesTxtOver);
    PropertiesbtnMenuSelect = new GuiImage(menu_select);
    Propertiesbtn->SetImageOver(PropertiesbtnMenuSelect);
    Propertiesbtn->SetSoundClick(btnClick);
    Propertiesbtn->SetTrigger(trigA);
    Propertiesbtn->SetPosition(0,buttonY);
    Propertiesbtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Propertiesbtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);
    buttonY += height/numItems;

    NoBtn = new GuiButton(screenwidth, screenheight);
    NoBtn->SetPosition(-x, -y);
    NoBtn->SetTrigger(trigA);
    NoBtn->SetTrigger(trigB);
    NoBtn->Clicked.connect(this, &RightClickMenu::OnButtonClick);

    Append(dialogBoxImg);
    Append(NoBtn);
    Append(btnCut);
    Append(Copybtn);
    Append(Pastebtn);
    Append(Renamebtn);
    Append(Deletebtn);
    Append(NewFolderbtn);
    Append(Propertiesbtn);
}

RightClickMenu::~RightClickMenu()
{
    HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

	Resources::Remove(dialogBox);
	Resources::Remove(menu_select);
	Resources::Remove(btnClick);

    delete dialogBoxImg;
    delete btnCutMenuSelect;
    delete CopybtnMenuSelect;
    delete PastebtnMenuSelect;
    delete RenamebtnMenuSelect;
    delete DeletebtnMenuSelect;
    delete NewFolderbtnMenuSelect;
    delete PropertiesbtnMenuSelect;

    delete cutTxt;
    delete cutTxtOver;
    delete copyTxt;
    delete copyTxtOver;
    delete pasteTxt;
    delete PasteTxtOver;
    delete RenameTxt;
    delete RenameTxtOver;
    delete DeleteTxt;
    delete DeleteTxtOver;
    delete NewFolderTxt;
    delete NewFolderTxtOver;
    delete PropertiesTxt;
    delete PropertiesTxtOver;

    delete btnCut;
    delete Copybtn;
    delete Pastebtn;
    delete Renamebtn;
    delete Deletebtn;
    delete NewFolderbtn;
    delete Propertiesbtn;
    delete NoBtn;

    delete trigA;
    delete trigB;

    ResumeGui();
}

int RightClickMenu::GetChoice()
{
    return choice;
}

void RightClickMenu::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();
    //TODO add the functions instead of the link
    if(sender == btnCut)
        choice = CUT;
    else if(sender == Copybtn)
        choice = COPY;
    else if(sender == Pastebtn)
        choice = PASTE;
    else if(sender == Renamebtn)
        choice = RENAME;
    else if(sender == Deletebtn)
        choice = DELETE;
    else if(sender == NewFolderbtn)
        choice = NEWFOLDER;
    else if(sender == Propertiesbtn)
        choice = PROPERTIES;
    else if(sender == NoBtn)
        choice = -10;
}
