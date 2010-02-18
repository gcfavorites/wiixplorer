 /****************************************************************************
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
 * Original Template of LibWiiGui
 * Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_bgm.h"
#include "libwiigui/gui_optionbrowser.h"
#include "Controls/Clipboard.h"
#include "network/ChangeLog.h"
#include "Menus/Explorer.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "filelist.h"
#include "FileOperations/filebrowser.h"
#include "FileOperations/fileops.h"
#include "devicemounter.h"
#include "FileStartUp/FileStartUp.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "network/networkops.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Language/LanguageBrowser.h"
#include "network/update.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "sys.h"

bool boothomebrew = false;
int curDevice = 0;

static bool firsttimestart = true;

extern u32 launchServer(char *drive, int status, bool stop) ;
extern int ServerFtpGui();

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
void ResumeGui()
{
	MainWindow::Instance()->ResumeGui();
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
void HaltGui()
{
	MainWindow::Instance()->HaltGui();
}

/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/
static int MenuBrowseDevice()
{
    if(firsttimestart  && Settings.MountMethod >= SMB1 && Settings.MountMethod <= SMB4 && !IsNetworkInit())
    {

        if(WaitSMBConnect() < 2)
            ShowError(tr("Could not connect to the network"));
        firsttimestart = false;
    }

	int menu = MENU_NONE;

    Explorer * Explorer_1 = new Explorer(curDevice);

    MainWindow::Instance()->Append(Explorer_1);
    ResumeGui();

    while(menu == MENU_NONE)
    {
	    usleep(THREAD_SLEEP);

        if(shutdown == 1)
            Sys_Shutdown();

        else if(reset == 1)
            Sys_Reboot();

        menu = Explorer_1->GetMenuChoice();

        if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();
    }

    delete Explorer_1;
    Explorer_1 = NULL;
	ResumeGui();

	return menu;
}

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
static int MenuSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options(7);
	options.SetName(i++, tr("Bootup Mount"));
	options.SetName(i++, tr("Language"));
	options.SetName(i++, tr("Music Volume"));
	options.SetName(i++, tr("Music Loop Mode"));
	options.SetName(i++, tr("Mount NTFS"));
	options.SetName(i++, tr("Customfont Path"));
	options.SetName(i++, tr("Network Settings"));

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size);
	GuiImageData btnOutline(button_png, button_png_size);
	GuiImageData btnOutlineOver(button_over_png, button_over_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -65);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(584, 248, &options);
	optionBrowser.SetPosition(30, 100);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	GuiImageData settingsimgData(settingsbtn_over_png, settingsbtn_over_png_size);
	GuiImage settingsimg(&settingsimgData);
	settingsimg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsimg.SetPosition(50, optionBrowser.GetTop()-35);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&optionBrowser);
	w.Append(&settingsimg);
	MainWindow::Instance()->Append(&w);
    w.SetEffect(EFFECT_FADE, 50);
	ResumeGui();

	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

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

		else if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_BROWSE_DEVICE;
		}

        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.MountMethod++;
				if(Settings.MountMethod >= MAXDEVICES)
                    Settings.MountMethod = 0;
                else if(Settings.MountMethod < SD)
                    Settings.MountMethod = 0;
				break;
			case 1:
				menu = MENU_LANGUAGE_BROWSE;
				break;
            case 2:
				Settings.MusicVolume += 10;
				if(Settings.MusicVolume > 100)
                    Settings.MusicVolume = 0;
                GuiBGM::Instance()->SetVolume(Settings.MusicVolume);
				break;
            case 3:
                Settings.BGMLoopMode++;
                if(Settings.BGMLoopMode >= MAX_LOOP_MODES)
                    Settings.BGMLoopMode = 0;
                GuiBGM::Instance()->SetLoop(Settings.BGMLoopMode);
				break;
            case 4:
				Settings.MountNTFS++;
				if(Settings.MountNTFS >= on_off_max)
                    Settings.MountNTFS = off;
				break;
            case 5:
                char entered[150];
                snprintf(entered, sizeof(entered), "%s", Settings.CustomFontPath);
                if(OnScreenKeyboard(entered, 149)) {
                    snprintf(Settings.CustomFontPath, sizeof(Settings.CustomFontPath), "%s", entered);
                    WindowPrompt(tr("Fontpath changed"), tr("Restart the app to load the new font."), tr("OK"));
                }
				break;
            case 6:
                menu = MENU_NETWORK_SETTINGS;
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++,DeviceName[Settings.MountMethod]);

            if(strcmp(Settings.LanguagePath, "") != 0)
            {
                if(strcmp(Settings.LanguagePath, "") == 0)
                {
                    options.SetValue(i++, tr("Standard"));
                }
                else if(Settings.LanguagePath[strlen(Settings.LanguagePath)-1] == '/')
                {
                    options.SetValue(i++, tr("Standard"));
                }
                else
                {
                    char * language = strrchr(Settings.LanguagePath, '/')+1;
                    options.SetValue(i++, "%s", language);
                }
            }
            else
                options.SetValue(i++, tr("App Default"));

            if (Settings.MusicVolume > 0)
                options.SetValue(i++, "%i", Settings.MusicVolume);
            else
                options.SetValue(i++, tr("OFF"));

            if (Settings.BGMLoopMode == ONCE) options.SetValue(i++,tr("Play Once"));
            else if (Settings.BGMLoopMode == LOOP) options.SetValue(i++,tr("Loop"));
            else if (Settings.BGMLoopMode == RANDOM_BGM) options.SetValue(i++,tr("Random"));
            else if (Settings.BGMLoopMode == DIR_LOOP) options.SetValue(i++,tr("Play Directory"));

            if (Settings.MountNTFS == on) options.SetValue(i++,tr("ON"));
            else if (Settings.MountNTFS == off) options.SetValue(i++,tr("OFF"));

            options.SetValue(i++, "%s", Settings.CustomFontPath);

            options.SetValue(i++, " ");
        }
	}

    w.SetEffect(EFFECT_FADE, -50);
	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	HaltGui();
	MainWindow::Instance()->Remove(&w);
	ResumeGui();

    Settings.Save();

	return menu;
}

/****************************************************************************
 * MenuSMBSettings
 ***************************************************************************/
static int MenuSMBSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options(6);
	options.SetName(i++, tr("User:"));
	options.SetName(i++, tr("Host:"));
	options.SetName(i++, tr("Username:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("SMB Name:"));
	options.SetName(i++, tr("Reconnect SMB"));

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size);
	GuiImageData btnOutline(button_png, button_png_size);
	GuiImageData btnOutlineOver(button_over_png, button_over_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -65);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(584, 248, &options);
	optionBrowser.SetPosition(30, 100);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	GuiText titleTxt(tr("SMB Settings"), 24, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, optionBrowser.GetTop()-35);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&optionBrowser);
	w.Append(&titleTxt);
	MainWindow::Instance()->Append(&w);
    w.SetEffect(EFFECT_FADE, 50);
	ResumeGui();

	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	while(menu == MENU_NONE)
	{
	    VIDEO_WaitVSync();

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

		else if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_NETWORK_SETTINGS;
		}

        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.CurrentUser++;
                if(Settings.CurrentUser >= MAXSMBUSERS)
                    Settings.CurrentUser = 0;
				break;
            case 1:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].Host);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentUser].Host, sizeof(Settings.SMBUser[Settings.CurrentUser].Host), "%s", entered);
                }
                break;
            case 2:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].User);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentUser].User, sizeof(Settings.SMBUser[Settings.CurrentUser].User), "%s", entered);
                }
                break;
            case 3:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].Password);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentUser].Password, sizeof(Settings.SMBUser[Settings.CurrentUser].Password), "%s", entered);
                }
                break;
            case 4:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].SMBName);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentUser].SMBName, sizeof(Settings.SMBUser[Settings.CurrentUser].SMBName), "%s", entered);
                }
                break;
            case 5:
                result = WindowPrompt(tr("Do you want to reconnect the SMB?"),0,tr("OK"),tr("Cancel"));
                if(result) {
                    CloseSMBShare();
                    sleep(1);
                    ConnectSMBShare();
                }
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++,tr("User %i"), Settings.CurrentUser+1);
            options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentUser].Host);
            options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentUser].User);
            options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentUser].Password);
            options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentUser].SMBName);
            options.SetValue(i++," ");
        }
	}

    w.SetEffect(EFFECT_FADE, -50);
	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	HaltGui();
	MainWindow::Instance()->Remove(&w);
	ResumeGui();

    Settings.Save();

	return menu;
}
/****************************************************************************
 * MenuUpdateSettings
 ***************************************************************************/
static int MenuNetworkSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options(5);
	options.SetName(i++, tr("Auto Connect"));
	options.SetName(i++, tr("Update Meta.xml"));
	options.SetName(i++, tr("Update Icon.png"));
	options.SetName(i++, tr("Update (App) Path"));
	options.SetName(i++, tr("SMB Settings"));

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size);
	GuiImageData btnOutline(button_png, button_png_size);
	GuiImageData btnOutlineOver(button_over_png, button_over_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -65);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiText updateBtnTxt(tr("Update App"), 22, (GXColor){0, 0, 0, 255});
	GuiImage updateBtnImg(&btnOutline);
	GuiButton updateBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	updateBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	updateBtn.SetPosition(-50, -65);
	updateBtn.SetLabel(&updateBtnTxt);
	updateBtn.SetImage(&updateBtnImg);
	updateBtn.SetSoundOver(&btnSoundOver);
	updateBtn.SetTrigger(&trigA);
	updateBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(584, 248, &options);
	optionBrowser.SetPosition(30, 100);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	GuiText titleTxt(tr("Update Settings"), 24, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, optionBrowser.GetTop()-35);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&updateBtn);
	w.Append(&optionBrowser);
	w.Append(&titleTxt);
	MainWindow::Instance()->Append(&w);
    w.SetEffect(EFFECT_FADE, 50);
	ResumeGui();

	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	while(menu == MENU_NONE)
	{
	    VIDEO_WaitVSync();

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

		else if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS;
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

        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

		ret = optionBrowser.GetClickedOption();

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
                snprintf(entered, sizeof(entered), "%s", Settings.UpdatePath);
                if(OnScreenKeyboard(entered, 149)) {
                    snprintf(Settings.UpdatePath, sizeof(Settings.UpdatePath), "%s", entered);
                    WindowPrompt(tr("Update Path changed."), 0, tr("OK"));
                }
				break;
            case 4:
                menu = MENU_SMB_SETTINGS;
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if(Settings.AutoConnect == on) options.SetValue(i++, tr("ON"));
            else if(Settings.AutoConnect == off) options.SetValue(i++, tr("OFF"));

            if(Settings.UpdateMetaxml == on) options.SetValue(i++, tr("ON"));
            else if(Settings.UpdateMetaxml == off) options.SetValue(i++, tr("OFF"));

            if(Settings.UpdateIconpng == on) options.SetValue(i++, tr("ON"));
            else if(Settings.UpdateIconpng == off) options.SetValue(i++, tr("OFF"));

            options.SetValue(i++, "%s", Settings.UpdatePath);

            options.SetValue(i++, " ");
        }
	}

    w.SetEffect(EFFECT_FADE, -50);
	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	HaltGui();
	MainWindow::Instance()->Remove(&w);
	ResumeGui();

    Settings.Save();

	return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void MainMenu(int menu)
{
	int currentMenu = menu;

	curDevice = Settings.MountMethod;

	while(currentMenu != MENU_EXIT)
	{
	    Taskbar::Instance()->ResetState();

		switch (currentMenu)
		{
			case MENU_SETTINGS:
				currentMenu = MenuSettings();
				break;
			case MENU_SMB_SETTINGS:
				currentMenu = MenuSMBSettings();
				break;
			case MENU_NETWORK_SETTINGS:
				currentMenu = MenuNetworkSettings();
				break;
			case MENU_BROWSE_DEVICE:
				currentMenu = MenuBrowseDevice();
				break;
			case MENU_LANGUAGE_BROWSE:
				currentMenu = LanguageBrowser();
				break;
			case MENU_FTP:
				currentMenu = ServerFtpGui();
				break;
			default: // unrecognized menu
				currentMenu = MenuBrowseDevice();
				break;
		}
	}

	ResumeGui();

	ExitApp();

    if(boothomebrew)
        BootHomebrew(Clipboard::Instance()->GetItemPath(Clipboard::Instance()->GetItemcount()-1));
}
