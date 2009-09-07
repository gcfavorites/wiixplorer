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
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#include "libwiigui/gui.h"
#include "network/networkops.h"
#include "Prompts/PromptWindows.h"
#include "devicemounter.h"
#include "fileops.h"
#include "filebrowser.h"
#include "foldersize.h"
#include "menu.h"
#include "filelist.h"
#include "Language/gettext.h"
#include "sys.h"
#include "svnrev.h"

/*** Variables used only in this file ***/
static GuiText prTxt(NULL, 24, (GXColor){0, 0, 0, 255});
static GuiText speedTxt(NULL, 22, (GXColor){0, 0, 0, 255});
static GuiText sizeTxt(NULL, 22, (GXColor){0, 0, 0, 255});
static GuiText msgTxt(NULL, 22, (GXColor){0, 0, 0, 255});
static GuiImageData progressbar(progressbar_png);
static GuiImage progressbarImg(&progressbar);
static GuiImageData throbber(throbber_png);
static GuiImage throbberImg(&throbber);

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/****************************************************************************
 * OnScreenKeyboard by Tantric 2009
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
int OnScreenKeyboard(char * var, u16 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText okBtnTxt(tr("OK"), 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt(tr("Cancel"), 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		VIDEO_WaitVSync();

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return save;
}

/****************************************************************************
* WindowPrompt
*
* Displays a prompt window to user, with information, an error message, or
* presenting a user with a choice of up to 4 Buttons.
*
* Give him 1 Titel, 1 Subtitel and 4 Buttons
* If titel/subtitle or one of the buttons is not needed give him a 0 on that
* place.
***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label,
const char *btn2Label, const char *btn3Label,
const char *btn4Label)
{
    int choice = -1;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);


    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiImageData dialogBox(dialogue_box_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText titleTxt(title, 24, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,55);
    titleTxt.SetMaxWidth(400, DOTTED);

    GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);
    msgTxt.SetMaxWidth(430, DOTTED);

    GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn1Img(&btnOutline);
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn1.SetLabel(&btn1Txt);
    btn1.SetImage(&btn1Img);
    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetSoundClick(&btnClick);
    btn1.SetTrigger(&trigA);
    btn1.SetState(STATE_SELECTED);
    btn1.SetEffectGrow();

    GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn2Img(&btnOutline);
    GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn2.SetLabel(&btn2Txt);
    btn2.SetImage(&btn2Img);
    btn2.SetSoundOver(&btnSoundOver);
    btn2.SetSoundClick(&btnClick);
    if(!btn3Label && !btn4Label)
        btn2.SetTrigger(&trigB);
    btn2.SetTrigger(&trigA);
    btn2.SetEffectGrow();

    GuiText btn3Txt(btn3Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn3Img(&btnOutline);
    GuiButton btn3(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn3.SetLabel(&btn3Txt);
    btn3.SetImage(&btn3Img);
    btn3.SetSoundOver(&btnSoundOver);
    btn3.SetSoundClick(&btnClick);
    if(!btn4Label)
        btn3.SetTrigger(&trigB);
    btn3.SetTrigger(&trigA);
    btn3.SetEffectGrow();

    GuiText btn4Txt(btn4Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn4Img(&btnOutline);
    GuiButton btn4(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn4.SetLabel(&btn4Txt);
    btn4.SetImage(&btn4Img);
    btn4.SetSoundOver(&btnSoundOver);
    btn4.SetSoundClick(&btnClick);
    if(btn4Label)
        btn4.SetTrigger(&trigB);
    btn4.SetTrigger(&trigA);
    btn4.SetEffectGrow();

    if(btn2Label && !btn3Label && !btn4Label) {
        btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1.SetPosition(40, -50);
        btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2.SetPosition(-40, -50);
        btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn3.SetPosition(50, -65);
        btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn4.SetPosition(-50, -65);
    } else if(btn2Label && btn3Label && !btn4Label) {
        btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1.SetPosition(50, -120);
        btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2.SetPosition(-50, -120);
        btn3.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn3.SetPosition(0, -65);
        btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn4.SetPosition(-50, -65);
    } else if(btn2Label && btn3Label && btn4Label) {
        btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1.SetPosition(50, -120);
        btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2.SetPosition(-50, -120);
        btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn3.SetPosition(50, -65);
        btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn4.SetPosition(-50, -65);
    } else if(!btn2Label && btn3Label && btn4Label) {
        btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn1.SetPosition(0, -120);
        btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2.SetPosition(-50, -120);
        btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn3.SetPosition(50, -65);
        btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn4.SetPosition(-50, -65);
    } else {
        btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn1.SetPosition(0, -50);
        btn2.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn2.SetPosition(50, -120);
        btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn3.SetPosition(50, -65);
        btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn4.SetPosition(-50, -65);
    }

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);

    if(btn1Label)
        promptWindow.Append(&btn1);
    if(btn2Label)
        promptWindow.Append(&btn2);
    if(btn3Label)
        promptWindow.Append(&btn3);
    if(btn4Label)
        promptWindow.Append(&btn4);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

        if(btn1.GetState() == STATE_CLICKED) {
            choice = 1;
        }
        else if(btn2.GetState() == STATE_CLICKED) {
            if(!btn3Label)
                choice = 0;
            else
                choice = 2;
            }
        else if(btn3.GetState() == STATE_CLICKED) {
            if(!btn4Label)
                choice = 0;
            else
                choice = 3;
        }
        else if(btn4.GetState() == STATE_CLICKED) {
            choice = 0;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return choice;
}

/****************************************************************************
* Wait SMB Connection
***************************************************************************/
int WaitSMBConnect(void)
{
    int choice = -1;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImageData dialogBox(dialogue_box_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText titleTxt(tr("Please wait:"), 24, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,55);
    titleTxt.SetMaxWidth(430, DOTTED);

    GuiText msgTxt(tr("Network initialising..."), 22, (GXColor){0, 0, 0, 255});
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);
    msgTxt.SetMaxWidth(430, DOTTED);

    GuiText btn1Txt(tr("Cancel"), 22, (GXColor){0, 0, 0, 255});
    GuiImage btn1Img(&btnOutline);
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn1.SetLabel(&btn1Txt);
    btn1.SetImage(&btn1Img);
    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetSoundClick(&btnClick);
    btn1.SetTrigger(&trigA);
    btn1.SetState(STATE_SELECTED);
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn1.SetPosition(0, -50);
    btn1.SetEffectGrow();

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);
    promptWindow.Append(&btn1);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(IsNetworkInit()) {
            msgTxt.SetText(tr("SMB is connecting..."));

            for(int i = 0; i < 4; i++)
                if(IsSMB_Mounted(i))
                    choice = 1;
        }

        if(btn1.GetState() == STATE_CLICKED) {
            choice = -2;
            btn1.ResetState();
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return choice;
}

/****************************************************************************
* RightClickMenu
***************************************************************************/
int RightClickMenu(int x, int y)
{
    int choice = -1;
    int numItems = 8;
    int buttonY = 0;

    GuiImageData dialogBox(clickmenu_png);
    GuiImage dialogBoxImg(&dialogBox);
    dialogBoxImg.SetPosition(-8, -dialogBox.GetHeight()/numItems/2);

    GuiImageData menu_select(menu_selection_png);

    if(screenwidth < x + dialogBox.GetWidth() + 10)
        x = screenwidth - dialogBox.GetWidth() - 10;

    if(screenheight < y + dialogBox.GetHeight() + 10)
        y = screenheight - dialogBox.GetHeight() - 10;

    GuiWindow promptWindow(dialogBox.GetWidth(), dialogBox.GetHeight());
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(x, y);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    GuiText cutTxt(tr("Cut"), 24, (GXColor){0, 0, 0, 255});
    GuiText cutTxtOver(tr("Cut"), 24, (GXColor){28, 32, 190, 255});
    cutTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    cutTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    GuiButton btnCut(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    btnCut.SetLabel(&cutTxt);
    btnCut.SetLabelOver(&cutTxtOver);
    btnCut.SetSoundClick(&btnClick);
    GuiImage btnCutMenuSelect(&menu_select);
    btnCut.SetImageOver(&btnCutMenuSelect);
    btnCut.SetTrigger(&trigA);
    btnCut.SetPosition(0,buttonY);
    btnCut.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText copyTxt(tr("Copy"), 24, (GXColor){0, 0, 0, 255});
    GuiText copyTxtOver(tr("Copy"), 24, (GXColor){28, 32, 190, 255});
    copyTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    copyTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    GuiButton Copybtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Copybtn.SetLabel(&copyTxt);
    Copybtn.SetLabelOver(&copyTxtOver);
    GuiImage CopybtnMenuSelect(&menu_select);
    Copybtn.SetImageOver(&CopybtnMenuSelect);
    Copybtn.SetSoundClick(&btnClick);
    Copybtn.SetTrigger(&trigA);
    Copybtn.SetPosition(0,buttonY);
    Copybtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText pasteTxt(tr("Paste"), 24, (GXColor){0, 0, 0, 255});
    GuiText PasteTxtOver(tr("Paste"), 24, (GXColor){28, 32, 190, 255});
    pasteTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PasteTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    GuiButton Pastebtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Pastebtn.SetLabel(&pasteTxt);
    Pastebtn.SetLabelOver(&PasteTxtOver);
    GuiImage PastebtnMenuSelect(&menu_select);
    Pastebtn.SetImageOver(&PastebtnMenuSelect);
    Pastebtn.SetSoundClick(&btnClick);
    Pastebtn.SetTrigger(&trigA);
    Pastebtn.SetPosition(0,buttonY);
    Pastebtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText RenameTxt(tr("Rename"), 24, (GXColor){0, 0, 0, 255});
    GuiText RenameTxtOver(tr("Rename"), 24, (GXColor){28, 32, 190, 255});
    RenameTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    RenameTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    GuiButton Renamebtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Renamebtn.SetLabel(&RenameTxt);
    Renamebtn.SetLabelOver(&RenameTxtOver);
    GuiImage RenamebtnMenuSelect(&menu_select);
    Renamebtn.SetImageOver(&RenamebtnMenuSelect);
    Renamebtn.SetSoundClick(&btnClick);
    Renamebtn.SetTrigger(&trigA);
    Renamebtn.SetPosition(0,buttonY);
    Renamebtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText DeleteTxt(tr("Delete"), 24, (GXColor){0, 0, 0, 255});
    GuiText DeleteTxtOver(tr("Delete"), 24, (GXColor){28, 32, 190, 255});
    DeleteTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    DeleteTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    GuiButton Deletebtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Deletebtn.SetLabel(&DeleteTxt);
    Deletebtn.SetLabelOver(&DeleteTxtOver);
    GuiImage DeletebtnMenuSelect(&menu_select);
    Deletebtn.SetImageOver(&DeletebtnMenuSelect);
    Deletebtn.SetSoundClick(&btnClick);
    Deletebtn.SetTrigger(&trigA);
    Deletebtn.SetPosition(0,buttonY);
    Deletebtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText NewFolderTxt(tr("New Folder"), 24, (GXColor){0, 0, 0, 255});
    GuiText NewFolderTxtOver(tr("New Folder"), 24, (GXColor){28, 32, 190, 255});
    NewFolderTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    NewFolderTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    GuiButton NewFolderbtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    NewFolderbtn.SetLabel(&NewFolderTxt);
    NewFolderbtn.SetLabelOver(&NewFolderTxtOver);
    GuiImage NewFolderbtnMenuSelect(&menu_select);
    NewFolderbtn.SetImageOver(&NewFolderbtnMenuSelect);
    NewFolderbtn.SetSoundClick(&btnClick);
    NewFolderbtn.SetTrigger(&trigA);
    NewFolderbtn.SetPosition(0,buttonY);
    NewFolderbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText PropertiesTxt(tr("Properties"), 24, (GXColor){0, 0, 0, 255});
    GuiText PropertiesTxtOver(tr("Properties"), 24, (GXColor){28, 32, 190, 255});
    PropertiesTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PropertiesTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    GuiButton Propertiesbtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Propertiesbtn.SetLabel(&PropertiesTxt);
    Propertiesbtn.SetLabelOver(&PropertiesTxtOver);
    GuiImage PropertiesbtnMenuSelect(&menu_select);
    Propertiesbtn.SetImageOver(&PropertiesbtnMenuSelect);
    Propertiesbtn.SetSoundClick(&btnClick);
    Propertiesbtn.SetTrigger(&trigA);
    Propertiesbtn.SetPosition(0,buttonY);
    Propertiesbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiButton NoBtn(screenwidth, screenheight);
    NoBtn.SetPosition(-x, -y);
    NoBtn.SetTrigger(&trigA);
    NoBtn.SetTrigger(&trigB);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&NoBtn);
    promptWindow.Append(&btnCut);
    promptWindow.Append(&Copybtn);
    promptWindow.Append(&Pastebtn);
    promptWindow.Append(&Renamebtn);
    promptWindow.Append(&Deletebtn);
    promptWindow.Append(&NewFolderbtn);
    promptWindow.Append(&Propertiesbtn);

    HaltGui();
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1) {
            Sys_Shutdown();
        }
        else if(reset == 1)
            Sys_Reboot();

        else if(btnCut.GetState() == STATE_CLICKED) {
            choice = CUT;
            break;
        }
        else if(Copybtn.GetState() == STATE_CLICKED) {
            choice = COPY;
            break;
        }
        else if(Pastebtn.GetState() == STATE_CLICKED) {
            choice = PASTE;
            break;
        }
        else if(Renamebtn.GetState() == STATE_CLICKED) {
            choice = RENAME;
            break;
        }
        else if(Deletebtn.GetState() == STATE_CLICKED) {
            choice = DELETE;
            break;
        }
        else if(NewFolderbtn.GetState() == STATE_CLICKED) {
            choice = NEWFOLDER;
            break;
        }
        else if(Propertiesbtn.GetState() == STATE_CLICKED) {
            choice = PROPERTIES;
            break;
        }
        else if(NoBtn.GetState() == STATE_CLICKED){
            choice = -2;
            break;
        }
    }

    HaltGui();
    mainWindow->Remove(&promptWindow);
    ResumeGui();

    return choice;
}

/****************************************************************************
* Properties
***************************************************************************/
int Properties(const char * filename, const char * filepath, int folder, float filesize)
{
    int choice = -1, stats = -1;
    u64 oldfoldersize = 0.0;
    char temp[MAXPATHLEN];
    struct stat filestat;
    snprintf(temp, sizeof(temp), "%s%s", filepath, filename);
    stats = stat(temp, &filestat);

    if(folder) {
        snprintf(temp, sizeof(temp), "%s%s/", filepath, filename);
        StartGetFolderSizeThread(temp);
    }

    GuiImageData dialogBox(bg_properties_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiWindow promptWindow(dialogBox.GetWidth(), dialogBox.GetHeight());
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(100, 100);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    GuiText TitleTxt(filename, 24, (GXColor){0, 0, 0, 255});
    TitleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    TitleTxt.SetPosition(0, 50);
    TitleTxt.SetMaxWidth(dialogBox.GetWidth()-20, DOTTED);

    GuiImageData titleData(icon_folder_png);
    GuiImage TitleImg(&titleData);
    TitleImg.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    TitleImg.SetPosition(-(TitleTxt.GetTextWidth()/2+titleData.GetWidth())+10, 30);

    sprintf(temp, tr("Filepath:  %s"), filepath);
    GuiText filepathTxt(temp, 22, (GXColor){0, 0, 0, 255});
    filepathTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filepathTxt.SetPosition(20, 100);
    filepathTxt.SetMaxWidth(dialogBox.GetWidth()-30, DOTTED);

    GuiText filecountTxt(tr("Files:"), 22, (GXColor){0, 0, 0, 255});
    filecountTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filecountTxt.SetPosition(20, 130);

    GuiText filecountTxtVal(tr("1"), 22, (GXColor){0, 0, 0, 255});
    filecountTxtVal.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filecountTxtVal.SetPosition(200, 130);

    if(filesize > KBSIZE && filesize < MBSIZE)
        sprintf(temp, "%0.2fKB", filesize/KBSIZE);
    else if(filesize > MBSIZE && filesize < GBSIZE)
        sprintf(temp, "%0.2fMB", filesize/MBSIZE);
    else if(filesize > GBSIZE)
        sprintf(temp, "%0.2fGB", filesize/GBSIZE);
    else
        sprintf(temp, "%0.2fB", filesize);

    GuiText filesizeTxt(tr("Size:"), 22, (GXColor){0, 0, 0, 255});
    filesizeTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filesizeTxt.SetPosition(20, 170);

    GuiText filesizeTxtVal(temp, 22, (GXColor){0, 0, 0, 255});
    filesizeTxtVal.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filesizeTxtVal.SetPosition(200, 170);

    char temp2[30];
    char * pch;
    if(folder) {
        snprintf(temp2, sizeof(temp2), tr("Folder"));
        TitleTxt.SetMaxWidth(dialogBox.GetWidth()-55, DOTTED);
    } else {
        snprintf(temp, sizeof(temp), "%s", filename);
        pch = strrchr(temp, '.')+1;
        snprintf(temp2, sizeof(temp2), "%s", pch);
    }

    GuiText filetypeTxt(tr("Filetype:"), 22, (GXColor){0, 0, 0, 255});
    filetypeTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filetypeTxt.SetPosition(20, 200);

    GuiText filetypeTxtVal(temp2, 22, (GXColor){0, 0, 0, 255});
    filetypeTxtVal.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filetypeTxtVal.SetPosition(200, 200);

    GuiText last_accessTxt(tr("Last access:"), 20, (GXColor){0, 0, 0, 255});
    last_accessTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_accessTxt.SetPosition(20, 230);

    strftime(temp, sizeof(temp), "%H:%M  %d %b %Y", localtime(&filestat.st_atime));
    GuiText last_accessTxtVal(temp, 20, (GXColor){0, 0, 0, 255});
    last_accessTxtVal.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_accessTxtVal.SetPosition(200, 230);

    GuiText last_modifTxt(tr("Last modified:"), 20, (GXColor){0, 0, 0, 255});
    last_modifTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_modifTxt.SetPosition(20, 260);

    strftime(temp, sizeof(temp), "%H:%M  %d %b %Y", localtime(&filestat.st_mtime));
    GuiText last_modifTxtVal(temp, 20, (GXColor){0, 0, 0, 255});
    last_modifTxtVal.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_modifTxtVal.SetPosition(200, 260);

    GuiText last_changeTxt(tr("Last status change:"), 20, (GXColor){0, 0, 0, 255});
    last_changeTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_changeTxt.SetPosition(20, 290);

    strftime(temp, sizeof(temp), "%H:%M  %d %b %Y", localtime(&filestat.st_ctime));
    GuiText last_changeTxtVal(temp, 20, (GXColor){0, 0, 0, 255});
    last_changeTxtVal.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_changeTxtVal.SetPosition(200, 290);

    GuiImageData arrowUp(scrollbar_arrowup_png);
    GuiImageData arrowUpOver(scrollbar_arrowup_over_png);
    GuiImage arrowUpImg(&arrowUp);
    arrowUpImg.SetAngle(45);
    GuiImage arrowUpImgOver(&arrowUpOver);
    arrowUpImgOver.SetAngle(45);
    GuiButton Backbtn(arrowUpImg.GetWidth(), arrowUpImg.GetHeight());
    Backbtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    Backbtn.SetSoundClick(&btnClick);
    Backbtn.SetImage(&arrowUpImg);
    Backbtn.SetImageOver(&arrowUpImgOver);
    Backbtn.SetPosition(-20, 20);
    Backbtn.SetEffectGrow();
    Backbtn.SetTrigger(&trigA);
    Backbtn.SetTrigger(&trigB);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&TitleTxt);
    if(folder)
        promptWindow.Append(&TitleImg);
    promptWindow.Append(&filepathTxt);
    promptWindow.Append(&filecountTxt);
    promptWindow.Append(&filecountTxtVal);
    promptWindow.Append(&filesizeTxt);
    promptWindow.Append(&filesizeTxtVal);
    promptWindow.Append(&filetypeTxt);
    promptWindow.Append(&filetypeTxtVal);
    promptWindow.Append(&last_accessTxt);
    promptWindow.Append(&last_accessTxtVal);
    promptWindow.Append(&last_modifTxt);
    promptWindow.Append(&last_modifTxtVal);
    promptWindow.Append(&last_changeTxt);
    promptWindow.Append(&last_changeTxtVal);
    promptWindow.Append(&Backbtn);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 40);
    ResumeGui();

    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();

        else if(Backbtn.GetState() == STATE_CLICKED) {
            choice = -3;
            break;
        }

        if(folder) {
            u64 foldersize = GetCurrentFolderSize();
            u32 filecount = GetFilecount();
            if(foldersize != oldfoldersize) {
				char sizetext[10];
				char filecounttext[20];
                if(foldersize > KBSIZE && foldersize < MBSIZE)
                    snprintf(sizetext, sizeof(sizetext), "%0.2fKB", foldersize/KBSIZE);
                else if(foldersize > MBSIZE && foldersize < GBSIZE)
                    snprintf(sizetext, sizeof(sizetext), "%0.2fMB", foldersize/MBSIZE);
                else if(foldersize > GBSIZE)
                    snprintf(sizetext, sizeof(sizetext), "%0.2fGB", foldersize/GBSIZE);
                else
                    snprintf(sizetext, sizeof(sizetext), "%LiB", foldersize);
                filesizeTxtVal.SetText(sizetext);
                snprintf(filecounttext, sizeof(filecounttext), "%i", filecount);
                filecountTxtVal.SetText(filecounttext);
            }
            oldfoldersize = foldersize;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 40);

    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    if(folder)
        StopSizeGain();

    return choice;
}

/****************************************************************************
* CreditsWindow
***************************************************************************/
void CreditsWindow(void)
{
    GuiImageData dialogBox(bg_properties_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiWindow promptWindow(dialogBox.GetWidth(), dialogBox.GetHeight());
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(100, 100);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    int numEntries = 9;
    int i = 0;
    int y = 30;

    GuiText * txt[numEntries];

    txt[i] = new GuiText(tr("Credits"), 28, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    txt[i]->SetPosition(0, y);
    i++;
    y += 60;

    txt[i] = new GuiText(tr("Coders:"), 24, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20, y);
    i++;

    txt[i] = new GuiText("Dimok", 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(170, y);
    i++;
    y += 32;

    txt[i] = new GuiText("r-win", 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(170, y);
    i++;
    y += 40;

    txt[i] = new GuiText(tr("Designer:"), 24, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20, y);
    i++;

    txt[i] = new GuiText("NeoRame", 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(170, y);
    i++;
    y += 60;

    txt[i] = new GuiText(tr("Special thanks to:"), 24, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20,y);
    i++;
    y += 30;

    char text[80];
    snprintf(text, sizeof(text), "Tantric %s ", tr("for his great tool LibWiiGui."));
    txt[i] = new GuiText(text, 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20,y);
    i++;
    y += 30;

    txt[i] = new GuiText(tr("The whole DevkitPro & libogc staff."), 22, (GXColor) {0, 0, 0, 255});
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(20,y);

    GuiImageData arrowUp(scrollbar_arrowup_png);
    GuiImageData arrowUpOver(scrollbar_arrowup_over_png);
    GuiImage arrowUpImg(&arrowUp);
    arrowUpImg.SetAngle(45);
    GuiImage arrowUpImgOver(&arrowUpOver);
    arrowUpImgOver.SetAngle(45);
    GuiButton Backbtn(arrowUpImg.GetWidth(), arrowUpImg.GetHeight());
    Backbtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    Backbtn.SetSoundClick(&btnClick);
    Backbtn.SetImage(&arrowUpImg);
    Backbtn.SetImageOver(&arrowUpImgOver);
    Backbtn.SetPosition(-20, 20);
    Backbtn.SetEffectGrow();
    Backbtn.SetTrigger(&trigA);
    Backbtn.SetTrigger(&trigB);

    char Rev[50];
    sprintf(Rev, "Rev. %i", atoi(SvnRev()));

    GuiText RevNum(Rev, 22, (GXColor) {0, 0, 0, 255});
    RevNum.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    RevNum.SetPosition(20, 20);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&RevNum);
    for(int i = 0; i < numEntries; i++)
        promptWindow.Append(txt[i]);
    promptWindow.Append(&Backbtn);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 40);
    ResumeGui();

    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    while(1)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();

        else if(Backbtn.GetState() == STATE_CLICKED) {
            break;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 40);

    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&promptWindow);

    for(int i = 0; i < numEntries; i++) {
        delete txt[i];
        txt[i] = NULL;
    }

    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
}

/****************************************************************************
* DeviceMenu
***************************************************************************/
int DeviceMenu(void)
{
    int choice = -1, cnt = 0, deviceCount = 0;

    //! Menu images
    GuiImageData device_choose_right_Data(device_choose_right_png);
    GuiImage rightImg(&device_choose_right_Data);
    GuiImageData device_choose_left_Data(device_choose_left_png);
    GuiImage leftImg(&device_choose_left_Data);
    GuiImageData device_choose_center_Data(device_choose_center_png);
    GuiImage centerImg(&device_choose_center_Data);

    //! Device images
    GuiImageData sd_ImgData(sdstorage_png);
    GuiImageData usb_ImgData(usbstorage_png);
    GuiImageData smb_ImgData(networkstorage_png);
//	GuiImageData isfs_ImgData(isfsstorage_png);
//	GuiImageData nand_ImgData(isfsstorage_png);

    GuiImageData menu_select(menu_selection_png);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    GuiButton *deviceBtn[MAXDEVICES];
    GuiImage *deviceImgs[MAXDEVICES];
    GuiImage *deviceImgOver[MAXDEVICES];
    GuiText *deviceText[MAXDEVICES];

    int deviceSelection[MAXDEVICES];

    for(int i = 0; i < MAXDEVICES; i++)
    {
        deviceBtn[i] = NULL;
        deviceImgs[i] = NULL;
        deviceImgOver[i] = NULL;
        deviceText[i] = NULL;
        deviceSelection[i] = -1;
    }

    int PositionX = 0;
    int FontSize = 17;

    if(SDCard_Inserted())
    {
        deviceText[deviceCount] = new GuiText("sd", FontSize, (GXColor){0, 0, 0, 255});
        deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        deviceText[deviceCount]->SetPosition(0, 2);
        deviceImgs[deviceCount] = new GuiImage(&sd_ImgData);
        deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        deviceImgOver[deviceCount] = new GuiImage(&menu_select);
        deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
        deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
        deviceBtn[deviceCount]->SetSoundClick(&btnClick);
        deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
        deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
        deviceBtn[deviceCount]->SetTrigger(&trigA);
        deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        deviceBtn[deviceCount]->SetPosition(PositionX+leftImg.GetWidth(), 9);
        PositionX += deviceImgs[deviceCount]->GetWidth()+10;

        deviceSelection[deviceCount] = SD;

        deviceCount++;
    }

    if(USBDevice_Inserted())
    {
        deviceText[deviceCount] = new GuiText("usb", FontSize, (GXColor){0, 0, 0, 255});
        deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        deviceText[deviceCount]->SetPosition(0, 2);
        deviceImgs[deviceCount] = new GuiImage(&usb_ImgData);
        deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        deviceImgOver[deviceCount] = new GuiImage(&menu_select);
        deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
        deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
        deviceBtn[deviceCount]->SetSoundClick(&btnClick);
        deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
        deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
        deviceBtn[deviceCount]->SetTrigger(&trigA);
        deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        deviceBtn[deviceCount]->SetPosition(PositionX+leftImg.GetWidth(), 9);
        PositionX += deviceImgs[deviceCount]->GetWidth()+10;

        deviceSelection[deviceCount] = USB;

        deviceCount++;
    }

	char text[50];
    for(int i = 0; i < NTFS_GetMountCount(); i++)
    {
        sprintf(text, "%s", NTFS_GetMountName(i));
        deviceText[deviceCount] = new GuiText(text, FontSize, (GXColor){0, 0, 0, 255});
        deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        deviceText[deviceCount]->SetPosition(0, 2);
        deviceImgs[deviceCount] = new GuiImage(&usb_ImgData);
        deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        deviceImgOver[deviceCount] = new GuiImage(&menu_select);
        deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
        deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
        deviceBtn[deviceCount]->SetSoundClick(&btnClick);
        deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
        deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
        deviceBtn[deviceCount]->SetTrigger(&trigA);
        deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        deviceBtn[deviceCount]->SetPosition(PositionX+leftImg.GetWidth(), 9);
        PositionX += deviceImgs[deviceCount]->GetWidth()+10;

        deviceSelection[deviceCount] = NTFS0+i;

        deviceCount++;
    }

    for(int i = 0; i < 4; i++)
    {
        if(IsSMB_Mounted(i))
        {
            sprintf(text, "smb%i", i+1);
            deviceText[deviceCount] = new GuiText(text, FontSize, (GXColor){0, 0, 0, 255});
            deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            deviceText[deviceCount]->SetPosition(0, 2);
            deviceImgs[deviceCount] = new GuiImage(&smb_ImgData);
            deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            deviceImgOver[deviceCount] = new GuiImage(&menu_select);
            deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
            deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
            deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
            deviceBtn[deviceCount]->SetSoundClick(&btnClick);
            deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
            deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
            deviceBtn[deviceCount]->SetTrigger(&trigA);
            deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            deviceBtn[deviceCount]->SetPosition(PositionX+leftImg.GetWidth(), 9);
            PositionX += deviceImgs[deviceCount]->GetWidth()+10;

            deviceSelection[deviceCount] = SMB1+i;

            deviceCount++;
        }
    }
/*	
	deviceText[deviceCount] = new GuiText("nand", FontSize, (GXColor){0, 0, 0, 255});
	deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	deviceText[deviceCount]->SetPosition(0, 2);
	deviceImgs[deviceCount] = new GuiImage(&nand_ImgData);
	deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	deviceImgOver[deviceCount] = new GuiImage(&menu_select);
	deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
	deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
	deviceBtn[deviceCount]->SetSoundClick(&btnClick);
	deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
	deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
	deviceBtn[deviceCount]->SetTrigger(&trigA);
	deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	deviceBtn[deviceCount]->SetPosition(PositionX+leftImg.GetWidth(), 9);
	PositionX += deviceImgs[deviceCount]->GetWidth()+10;
	deviceSelection[deviceCount] = NAND;
	deviceCount++;
*/
    if(!deviceCount)
        return -5;

    //! Set image position and tile
    int tile = PositionX/centerImg.GetWidth();

    leftImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    leftImg.SetPosition(0, 0);

    centerImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    centerImg.SetPosition(leftImg.GetLeft()+leftImg.GetWidth(), 0);
    centerImg.SetTile(tile);

    rightImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    rightImg.SetPosition(leftImg.GetWidth() + tile * centerImg.GetWidth(), 0);

    int width = leftImg.GetWidth() + tile * centerImg.GetWidth() + rightImg.GetWidth();

    GuiWindow promptWindow(width, leftImg.GetHeight());
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(45, 110);

    GuiButton NoBtn(screenwidth, screenheight);
    NoBtn.SetPosition(-promptWindow.GetLeft(), -promptWindow.GetTop());
    NoBtn.SetTrigger(&trigA);
    NoBtn.SetTrigger(&trigB);

    promptWindow.Append(&NoBtn);
    promptWindow.Append(&leftImg);
    promptWindow.Append(&centerImg);
    promptWindow.Append(&rightImg);
    for(int i = 0; i < deviceCount; i++)
        promptWindow.Append(deviceBtn[i]);

    HaltGui();
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    promptWindow.SetEffect(EFFECT_FADE, 25);
    ResumeGui();

    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(NoBtn.GetState() == STATE_CLICKED){
            choice = -2;
            NoBtn.ResetState();
        }

        for(cnt = 0; cnt < deviceCount; cnt++)
        {
            if(deviceBtn[cnt]->GetState() == STATE_CLICKED)
            {
                choice = deviceSelection[cnt];
                deviceBtn[cnt]->ResetState();
            }
        }
    }

    promptWindow.SetEffect(EFFECT_FADE, -35);
    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    for(int i = 0; i < MAXDEVICES; i++)
    {
        if(deviceBtn[i]) {
            delete deviceBtn[i];
            deviceBtn[i] = NULL;
        }
        if(deviceImgs[i]) {
            delete deviceImgs[i];
            deviceImgs[i] = NULL;
        }
        if(deviceImgOver[i]) {
            delete deviceImgOver[i];
            deviceImgOver[i] = NULL;
        }
        if(deviceText[i]) {
            delete deviceText[i];
            deviceText[i] = NULL;
        }
    }
    ResumeGui();

    return choice;
}
