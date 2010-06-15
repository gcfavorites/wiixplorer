/****************************************************************************
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
#include <gctypes.h>
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Explorer.h"
#include "menu.h"
#include "sys.h"

extern std::string LastUsedPath;

int MenuGetPath(std::string & Path)
{
    int menu = MENU_NONE;
    bool ExitRequested = false;

	Explorer * Explorer_1 = NULL;

	GuiSound * btnSoundClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	GuiSound * btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    GuiImageData * btnOutline = Resources::GetImageData(button_png, button_png_size);
    GuiImageData * btnOutlineOver = Resources::GetImageData(button_over_png, button_over_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);

	GuiText doneBtnTxt(tr("Done"), 20, (GXColor){0, 0, 0, 255});
	GuiImage doneBtnImg(btnOutline);
	GuiButton doneBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	doneBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	doneBtn.SetPosition(100+btnOutline->GetWidth()/2, -65);
	doneBtn.SetLabel(&doneBtnTxt);
	doneBtn.SetImage(&doneBtnImg);
	doneBtn.SetSoundClick(btnSoundOver);
	doneBtn.SetSoundOver(btnSoundOver);
	doneBtn.SetTrigger(&trigA);
	doneBtn.SetEffectGrow();

	GuiText CancelBtnTxt(tr("Cancel"), 20, (GXColor){0, 0, 0, 255});
	GuiImage CancelBtnImg(btnOutline);
	GuiButton CancelBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	CancelBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	CancelBtn.SetPosition(-105-btnOutline->GetWidth()/2, -65);
	CancelBtn.SetLabel(&CancelBtnTxt);
	CancelBtn.SetImage(&CancelBtnImg);
	CancelBtn.SetSoundOver(btnSoundOver);
	CancelBtn.SetTrigger(&trigA);
	CancelBtn.SetTrigger(&trigB);
	CancelBtn.SetEffectGrow();

    Explorer_1 = new Explorer(Settings.MountMethod);
    Explorer_1->SetFilter(FILTER_FILES);

    if(LastUsedPath.length() == 0)
        Explorer_1->LoadDevice(Settings.MountMethod);
    else
        Explorer_1->LoadPath(LastUsedPath.c_str());

    MainWindow::Instance()->HaltGui();
    MainWindow::Instance()->Append(Explorer_1);
    MainWindow::Instance()->Append(&doneBtn);
    MainWindow::Instance()->Append(&CancelBtn);
    MainWindow::Instance()->ResumeGui();

    while(!ExitRequested && menu == MENU_NONE)
    {
	    usleep(THREAD_SLEEP);

        if(shutdown)
            Sys_Shutdown();

        else if(reset)
            Sys_Reboot();

        else if(doneBtn.GetState() == STATE_CLICKED)
        {
            ExitRequested = true;
            Path = Explorer_1->GetCurrectPath();
            doneBtn.ResetState();
        }
        else if(CancelBtn.GetState() == STATE_CLICKED)
        {
            ExitRequested = true;
            doneBtn.ResetState();
        }

        menu = Explorer_1->GetMenuChoice();

        if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();
    }

    MainWindow::Instance()->HaltGui();
    MainWindow::Instance()->Remove(&doneBtn);
    MainWindow::Instance()->Remove(&CancelBtn);
    Resources::Remove(btnOutline);
    Resources::Remove(btnOutlineOver);
    Resources::Remove(btnSoundClick);
    Resources::Remove(btnSoundOver);
    delete Explorer_1;
    Explorer_1 = NULL;
    MainWindow::Instance()->ResumeGui();

	return menu;
}
