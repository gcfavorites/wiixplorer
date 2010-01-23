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
#include "libwiigui/gui_optionbrowser.h"
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
#include "Language/gettext.h"
#include "Language/LanguageBrowser.h"
#include "network/update.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "sys.h"

CLIPBOARD Clipboard;
bool boothomebrew = false;

static int currentDevice = 0;
static bool firsttimestart = true;

extern u8 shutdown;
extern u8 reset;

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
    if(firsttimestart  && Settings.MountMethod >= SMB1 && Settings.MountMethod <= SMB4 &&
        Settings.AutoConnect == on && !IsNetworkInit())
    {

        WaitSMBConnect();
        firsttimestart = false;
        sleep(2);
    }
/*
	// populate initial directory listing
	if(BrowseDevice(currentDevice) <= 0)
	{
		int choice = WindowPrompt(tr("Error"),
		tr("Unable to load device."),
		tr("Retry"),
		tr("Next device"),
		tr("Change Settings"));

		if(choice == 1)
		{
            SDCard_Init();
            USBDevice_Init();
			return MENU_BROWSE_DEVICE;
		}
		else if(choice == 2)
		{
		    int retries = 2;
		    while(BrowseDevice(currentDevice) <= 0)
            {
                currentDevice++;
                if(currentDevice >= MAXDEVICES)
                {
                    currentDevice = 0;

                    if(retries == 0)
                    {
                        WindowPrompt(tr("ERROR"), tr("Can't load any device"), tr("OK"));
                        return MENU_BROWSE_DEVICE;
                    }
                    retries--;
                }
            }
		}
		else
			return MENU_SETTINGS;
	}
*/
	int menu = MENU_NONE;

    Explorer * Explorer_1 = new Explorer(currentDevice);

    MainWindow::Instance()->Append(Explorer_1);
    MainWindow::Instance()->ChangeFocus(Explorer_1);
    ResumeGui();

    while(menu == MENU_NONE)
    {
	    VIDEO_WaitVSync();

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
	if(Settings.AutoConnect == on)
        options.SetName(i++, tr("Reconnect SMB"));
    else
        options.SetName(i++, tr("Connect SMB"));

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

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
			menu = MENU_SETTINGS;
		}

        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.CurrentUser++;
                if(Settings.CurrentUser > MAXSMBUSERS-1)
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
	options.SetName(i++, tr("Auto Connect"));
	options.SetName(i++, tr("Music Volume"));
	options.SetName(i++, tr("Mount NTFS"));
	options.SetName(i++, tr("Customfont Path"));
	options.SetName(i++, tr("Update (App) Path"));
	options.SetName(i++, tr("SMB Settings"));

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

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

	GuiImageData settingsimgData(settingsbtn_over_png);
	GuiImage settingsimg(&settingsimgData);
	settingsimg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsimg.SetPosition(50, optionBrowser.GetTop()-35);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&updateBtn);
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

        else if(updateBtn.GetState() == STATE_CLICKED)
		{
            int res = CheckForUpdate();
            if(res == 0)
            {
                int choice = WindowPrompt(tr("No new updates available"), 0, tr("OK"), tr("Show Changelog"));
                if(choice == 0)
                {
                    ChangeLog Changelog;
                    Changelog.Show();
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
				Settings.AutoConnect++;
				if(Settings.AutoConnect >= on_off_max)
                    Settings.AutoConnect = off;
				break;
            case 3:
				Settings.MusicVolume += 10;
				if(Settings.MusicVolume > 100)
                    Settings.MusicVolume = 0;
                MainWindow::Instance()->ChangeVolume(Settings.MusicVolume);
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
                snprintf(entered, sizeof(entered), "%s", Settings.UpdatePath);
                if(OnScreenKeyboard(entered, 149)) {
                    snprintf(Settings.UpdatePath, sizeof(Settings.UpdatePath), "%s", entered);
                    WindowPrompt(tr("Update Path changed."), 0, tr("OK"));
                }
				break;
            case 7:
                menu = MENU_SMB_SETTINGS;
                break;
		}


        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++,DeviceName[Settings.MountMethod]);

            if(strcmp(Settings.LanguagePath, "") != 0) {
                char *language = strrchr(Settings.LanguagePath, '/')+1;
                options.SetValue(i++, "%s", language);
            } else
                options.SetValue(i++, tr("App Default"));

            if (Settings.AutoConnect == on) options.SetValue(i++,tr("ON"));
            else if (Settings.AutoConnect == off) options.SetValue(i++,tr("OFF"));

            if (Settings.MusicVolume > 0) options.SetValue(i++, "%i", Settings.MusicVolume);
            else options.SetValue(i++, tr("OFF"));

            if (Settings.MountNTFS == on) options.SetValue(i++,tr("ON"));
            else if (Settings.MountNTFS == off) options.SetValue(i++,tr("OFF"));

            options.SetValue(i++, "%s", Settings.CustomFontPath);

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
	currentDevice = Settings.MountMethod;

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
			case MENU_BROWSE_DEVICE:
				currentMenu = MenuBrowseDevice();
				break;
			case MENU_LANGUAGE_BROWSE:
				currentMenu = LanguageBrowser();
				break;
			default: // unrecognized menu
				currentMenu = MenuBrowseDevice();
				break;
		}
	}

	ResumeGui();

    Settings.Save();
	MainWindow::Instance()->DestroyInstance();
	ExitApp();
	ClearFontData();
	Resources::DestroyInstance();
	CloseSMBShare();
    NTFS_UnMount();
    SDCard_deInit();
    USBDevice_deInit();
    DiskDrive_deInit();
	DeInit_Network();

	WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

    if(boothomebrew)
        BootHomebrew(Clipboard.filepath);
}
