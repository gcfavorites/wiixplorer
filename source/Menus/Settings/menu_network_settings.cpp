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
#include "SettingsMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindows.h"
#include "network/networkops.h"
#include "network/ChangeLog.h"
#include "network/update.h"

/****************************************************************************
 * MenuNetworkSettings
 ***************************************************************************/
int MenuNetworkSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
    bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Auto Connect"));
	options.SetName(i++, tr("Update Meta.xml"));
	options.SetName(i++, tr("Update Icon.png"));
	options.SetName(i++, tr("SMB Settings"));
	options.SetName(i++, tr("FTP Client Settings"));
	options.SetName(i++, tr("FTP Server Settings"));

	SettingsMenu * Menu = new SettingsMenu(tr("Network Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

    GuiImageData * btnOutline = Resources::GetImageData(button_png, button_png_size);
	GuiSound * btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    SimpleGuiTrigger trigA(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	GuiText updateBtnTxt(tr("Update App"), 22, (GXColor){0, 0, 0, 255});
	GuiImage updateBtnImg(btnOutline);
	GuiButton updateBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	updateBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	updateBtn.SetPosition(100+btnOutline->GetWidth()/2, -65);
	updateBtn.SetLabel(&updateBtnTxt);
	updateBtn.SetImage(&updateBtnImg);
	updateBtn.SetSoundOver(btnSoundOver);
	updateBtn.SetTrigger(&trigA);
	updateBtn.SetEffectGrow();

	MainWindow::Instance()->Append(&updateBtn);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

		if(Menu->GetMenu() != MENU_NONE)
		{
			menu = Menu->GetMenu();
		}
        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
        {
			menu = Taskbar::Instance()->GetMenu();
        }
        else if(updateBtn.GetState() == STATE_CLICKED)
		{
            int res = CheckForUpdate();
            if(res == 0)
            {
                int choice = WindowPrompt(tr("No new updates available"), 0, tr("OK"), tr("Show Changelog"));
                if(choice == 0)
                {
                    ChangeLog Changelog;
                    if(!Changelog.Show())
                        WindowPrompt(tr("Failed to get the Changelog."), 0, tr("OK"));
                }
            }
		    updateBtn.ResetState();
		}

		ret = Menu->GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.AutoConnect++;
                if(Settings.AutoConnect > 1)
                    Settings.AutoConnect = 0;
				break;
			case 1:
				Settings.UpdateMetaxml++;
                if(Settings.UpdateMetaxml > 1)
                    Settings.UpdateMetaxml = 0;
				break;
			case 2:
				Settings.UpdateIconpng++;
                if(Settings.UpdateIconpng > 1)
                    Settings.UpdateIconpng = 0;
				break;
            case 3:
                menu = MENU_SMB_SETTINGS;
                break;
            case 4:
                menu = MENU_FTPCLIENT_SETTINGS;
                break;
            case 5:
                menu = MENU_FTPSERVER_SETTINGS;
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if(Settings.AutoConnect == 1) options.SetValue(i++, tr("ON"));
            else if(Settings.AutoConnect == 0) options.SetValue(i++, tr("OFF"));

            if(Settings.UpdateMetaxml == 1) options.SetValue(i++, tr("ON"));
            else if(Settings.UpdateMetaxml == 0) options.SetValue(i++, tr("OFF"));

            if(Settings.UpdateIconpng == 1) options.SetValue(i++, tr("ON"));
            else if(Settings.UpdateIconpng == 0) options.SetValue(i++, tr("OFF"));

            options.SetValue(i++, " ");
        }
	}

    delete Menu;

	MainWindow::Instance()->HaltGui();
	MainWindow::Instance()->Remove(&updateBtn);
	MainWindow::Instance()->ResumeGui();

	Resources::Remove(btnOutline);
	Resources::Remove(btnSoundOver);

    Settings.Save();

	return menu;
}
