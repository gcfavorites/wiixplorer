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
 * ZipBrowser.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_optionbrowser.h"
#include "Controls/MainWindow.h"
#include "Prompts/PromptWindows.h"
#include "unzip/unzip.h"
#include "ArchiveOperations/ZipFile.h"
#include "FileOperations/fileops.h"
#include "Language/gettext.h"
#include "sys.h"

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

bool ZipBrowse(const char *filepath, const char *dest)
{
    bool exitwindow = false;
    bool result = false;

    ZipFile zipFile(filepath);

    result = zipFile.ExtractAll(dest);

    if(result != true)
        WindowPrompt(tr("Error"), tr("Failed to extract files."), tr("OK"));
    else
        WindowPrompt(tr("Files successfully extracted."), 0, tr("OK"));

    return result;     //!The rest is comming next

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
    promptWindow.Append(&Backbtn);

    HaltGui();
    MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->Append(&promptWindow);
    MainWindow::Instance()->ChangeFocus(&promptWindow);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 40);
    ResumeGui();

    while(promptWindow.GetEffect() > 0) usleep(100);

	while (!exitwindow)
	{
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();

		if (Backbtn.GetState() == STATE_CLICKED) {
			exitwindow = true;
			Backbtn.ResetState();
		}
	}

	HaltGui();
	MainWindow::Instance()->Remove(&promptWindow);
	MainWindow::Instance()->SetState(STATE_DEFAULT);
	ResumeGui();
}
