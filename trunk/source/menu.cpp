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
#include "SoundOperations/gui_bgm.h"
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
    char entered[150];

	OptionList options(9);
	options.SetName(i++, tr("Bootup Mount"));
	options.SetName(i++, tr("Language"));
	options.SetName(i++, tr("Music Volume"));
	options.SetName(i++, tr("Music Loop Mode"));
	options.SetName(i++, tr("Slideshow Delay"));
	options.SetName(i++, tr("Mount NTFS"));
	options.SetName(i++, tr("Network Settings"));
	options.SetName(i++, tr("App Path"));
	options.SetName(i++, tr("Customfont Path"));

	GuiSound btnSoundOver(button_over_wav, button_over_wav_size);
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

	GuiText titleTxt(tr("Settings"), 24, (GXColor){0, 0, 0, 255});
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
                Settings.SlideshowDelay += 5;
                if(Settings.SlideshowDelay > 60)
                    Settings.SlideshowDelay = 5;
				break;
			case 5:
				Settings.MountNTFS++;
				if(Settings.MountNTFS >= on_off_max)
                    Settings.MountNTFS = off;
				break;
            case 6:
                menu = MENU_NETWORK_SETTINGS;
				break;
            case 7:
                snprintf(entered, sizeof(entered), "%s", Settings.AppPath);
                if(OnScreenKeyboard(entered, 149)) {
					if (entered[strlen(entered)-1] != '/')
						strcat(entered, "/");
					snprintf(Settings.AppPath, sizeof(Settings.AppPath), "%s", entered);
                    WindowPrompt(tr("AppPath changed"), 0, tr("OK"));
                }
				break;
            case 8:
                snprintf(entered, sizeof(entered), "%s", Settings.CustomFontPath);
                if(OnScreenKeyboard(entered, 149)) {
                    snprintf(Settings.CustomFontPath, sizeof(Settings.CustomFontPath), "%s", entered);
                    WindowPrompt(tr("Fontpath changed"), tr("Restart the app to load the new font."), tr("OK"));
                }
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

			options.SetValue(i++, "%i", Settings.SlideshowDelay);

            if (Settings.MountNTFS == on) options.SetValue(i++,tr("ON"));
            else if (Settings.MountNTFS == off) options.SetValue(i++,tr("OFF"));

            options.SetValue(i++, " ");

            options.SetValue(i++, "%s", Settings.AppPath);

            options.SetValue(i++, "%s", Settings.CustomFontPath);
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

	GuiSound btnSoundOver(button_over_wav, button_over_wav_size);
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
				Settings.CurrentSMBUser++;
                if(Settings.CurrentSMBUser >= MAXSMBUSERS)
                    Settings.CurrentSMBUser = 0;
				break;
            case 1:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentSMBUser].Host);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentSMBUser].Host, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].Host), "%s", entered);
                }
                break;
            case 2:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentSMBUser].User);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentSMBUser].User, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].User), "%s", entered);
                }
                break;
            case 3:
                entered[0] = 0;
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentSMBUser].Password, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].Password), "%s", entered);
                }
                break;
            case 4:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentSMBUser].SMBName);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentSMBUser].SMBName, sizeof(Settings.SMBUser[Settings.CurrentSMBUser].SMBName), "%s", entered);
                }
                break;
            case 5:
                result = WindowPrompt(tr("Do you want to reconnect the SMB?"),0,tr("OK"),tr("Cancel"));
                if(result) {
                    SMB_Reconnect();
                }
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++,tr("User %i"), Settings.CurrentSMBUser+1);
            options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentSMBUser].Host);
            options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentSMBUser].User);
			if (strcmp(Settings.SMBUser[Settings.CurrentSMBUser].Password, "") != 0)
				options.SetValue(i++,"********");
			else
				options.SetValue(i++," ");
            options.SetValue(i++,"%s", Settings.SMBUser[Settings.CurrentSMBUser].SMBName);
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
 * MenuFTPSettings
 ***************************************************************************/
static int MenuFTPSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options(8);
	options.SetName(i++, tr("User:"));
	options.SetName(i++, tr("Host:"));
	options.SetName(i++, tr("Username:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("FTP Port:"));
	options.SetName(i++, tr("FTP Path:"));
	options.SetName(i++, tr("Passive Mode:"));
	options.SetName(i++, tr("Reconnect FTP"));

	GuiSound btnSoundOver(button_over_wav, button_over_wav_size);
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

	GuiText titleTxt(tr("FTP Client Settings"), 24, (GXColor){0, 0, 0, 255});
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
				Settings.CurrentFTPUser++;
                if(Settings.CurrentFTPUser >= MAXFTPUSERS)
                    Settings.CurrentFTPUser = 0;
				break;
            case 1:
				snprintf(entered, sizeof(entered), "%s", Settings.FTPUser[Settings.CurrentFTPUser].Host);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPUser[Settings.CurrentFTPUser].Host, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].Host), "%s", entered);
                }
                break;
            case 2:
				snprintf(entered, sizeof(entered), "%s", Settings.FTPUser[Settings.CurrentFTPUser].User);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPUser[Settings.CurrentFTPUser].User, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].User), "%s", entered);
                }
                break;
            case 3:
                entered[0] = 0;
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPUser[Settings.CurrentFTPUser].Password, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].Password), "%s", entered);
                }
                break;
            case 4:
                snprintf(entered, sizeof(entered), "%i", Settings.FTPUser[Settings.CurrentFTPUser].Port);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    Settings.FTPUser[Settings.CurrentFTPUser].Port = (u16) atoi(entered);
                }
                break;
            case 5:
                snprintf(entered, sizeof(entered), "%s", Settings.FTPUser[Settings.CurrentFTPUser].FTPPath);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPUser[Settings.CurrentFTPUser].FTPPath, sizeof(Settings.FTPUser[Settings.CurrentFTPUser].FTPPath), "%s", entered);
                }
                break;
            case 6:
				Settings.FTPUser[Settings.CurrentFTPUser].Passive++;
				if(Settings.FTPUser[Settings.CurrentFTPUser].Passive >= on_off_max)
                    Settings.FTPUser[Settings.CurrentFTPUser].Passive = off;
				break;
			case 7:
                result = WindowPrompt(tr("Do you want to reconnect to FTP server?"),0,tr("OK"),tr("Cancel"));
                if(result) {
                     CloseFTP();
                     sleep(1);
                     ConnectFTP();
                }
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++,tr("User %i"), Settings.CurrentFTPUser+1);
            options.SetValue(i++,"%s", Settings.FTPUser[Settings.CurrentFTPUser].Host);
            options.SetValue(i++,"%s", Settings.FTPUser[Settings.CurrentFTPUser].User);
			if (strcmp(Settings.FTPUser[Settings.CurrentFTPUser].Password, "") != 0)
				options.SetValue(i++,"********");
			else
				options.SetValue(i++," ");
            options.SetValue(i++,"%i", Settings.FTPUser[Settings.CurrentFTPUser].Port);
            options.SetValue(i++,"%s", Settings.FTPUser[Settings.CurrentFTPUser].FTPPath);
			if (Settings.FTPUser[Settings.CurrentFTPUser].Passive == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPUser[Settings.CurrentFTPUser].Passive == off) options.SetValue(i++,tr("OFF"));
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
 * MenuFTPServerSettings
 ***************************************************************************/
static int MenuFTPServerSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options(12);
	options.SetName(i++, tr("User:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("FTP Port:"));
	options.SetName(i++, tr("FTP Data Port:"));
	options.SetName(i++, tr("FTP Path:"));
	options.SetName(i++, tr("Enable Zip Mode:"));
	options.SetName(i++, tr("Enable read file:"));
	options.SetName(i++, tr("Enable list file:"));
	options.SetName(i++, tr("Enable Write file:"));
	options.SetName(i++, tr("Enable Delete file:"));
	options.SetName(i++, tr("Enable Create dir:"));
	options.SetName(i++, tr("Enable Delete dir:"));


	GuiSound btnSoundOver(button_over_wav, button_over_wav_size);
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

	GuiText titleTxt(tr("FTP Server Settings"), 24, (GXColor){0, 0, 0, 255});
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
				snprintf(entered, sizeof(entered), "%s", Settings.FTPServerUser.UserName);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPServerUser.UserName, sizeof(Settings.FTPServerUser.UserName), "%s", entered);
                }
                break;
            case 1:
                entered[0] = 0;
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPServerUser.Password, sizeof(Settings.FTPServerUser.Password), "%s", entered);
                }
                break;
            case 2:
                snprintf(entered, sizeof(entered), "%i", Settings.FTPServerUser.Port);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    Settings.FTPServerUser.Port = (u16) atoi(entered);
                }
                break;
            case 3:
                snprintf(entered, sizeof(entered), "%i", Settings.FTPServerUser.DataPort);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    Settings.FTPServerUser.DataPort = (u16) atoi(entered);
                }
                break;
            case 4:
                snprintf(entered, sizeof(entered), "%s", Settings.FTPServerUser.FTPPath);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPServerUser.FTPPath, sizeof(Settings.FTPServerUser.FTPPath), "%s", entered);
                }
                break;
            case 5:
				Settings.FTPServerUser.ZipMode++;
				if(Settings.FTPServerUser.ZipMode >= on_off_max)
                    Settings.FTPServerUser.ZipMode = off;
				break;
            case 6:
				Settings.FTPServerUser.EnableReadFile++;
				if(Settings.FTPServerUser.EnableReadFile >= on_off_max)
                    Settings.FTPServerUser.EnableReadFile = off;
            case 7:
				Settings.FTPServerUser.EnableListFile++;
				if(Settings.FTPServerUser.EnableListFile >= on_off_max)
                    Settings.FTPServerUser.EnableListFile = off;
            case 8:
				Settings.FTPServerUser.EnableWriteFile++;
				if(Settings.FTPServerUser.EnableWriteFile >= on_off_max)
                    Settings.FTPServerUser.EnableWriteFile = off;
				break;
            case 9:
				Settings.FTPServerUser.EnableDeleteFile++;
				if(Settings.FTPServerUser.EnableDeleteFile >= on_off_max)
                    Settings.FTPServerUser.EnableDeleteFile = off;
				break;
            case 10:
				Settings.FTPServerUser.EnableCreateDir++;
				if(Settings.FTPServerUser.EnableCreateDir >= on_off_max)
                    Settings.FTPServerUser.EnableCreateDir = off;
				break;
           case 11:
				Settings.FTPServerUser.EnableDeleteDir++;
				if(Settings.FTPServerUser.EnableDeleteDir >= on_off_max)
                    Settings.FTPServerUser.EnableDeleteDir = off;
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++,"%s", Settings.FTPServerUser.UserName);
			if (strcmp(Settings.FTPServerUser.Password, "") != 0)
				options.SetValue(i++,"********");
			else
				options.SetValue(i++," ");
            options.SetValue(i++,"%i", Settings.FTPServerUser.Port);
			options.SetValue(i++,"%i", Settings.FTPServerUser.DataPort);
            options.SetValue(i++,"%s", Settings.FTPServerUser.FTPPath);
			if (Settings.FTPServerUser.ZipMode == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPServerUser.ZipMode == off) options.SetValue(i++,tr("OFF"));
			if (Settings.FTPServerUser.EnableReadFile == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPServerUser.EnableReadFile == off) options.SetValue(i++,tr("OFF"));
			if (Settings.FTPServerUser.EnableListFile == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPServerUser.EnableListFile == off) options.SetValue(i++,tr("OFF"));
			if (Settings.FTPServerUser.EnableWriteFile == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPServerUser.EnableWriteFile == off) options.SetValue(i++,tr("OFF"));
			if (Settings.FTPServerUser.EnableDeleteFile == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPServerUser.EnableDeleteFile == off) options.SetValue(i++,tr("OFF"));
			if (Settings.FTPServerUser.EnableCreateDir == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPServerUser.EnableCreateDir == off) options.SetValue(i++,tr("OFF"));
			if (Settings.FTPServerUser.EnableDeleteDir == on) options.SetValue(i++,tr("ON"));
			else if (Settings.FTPServerUser.EnableDeleteDir == off) options.SetValue(i++,tr("OFF"));

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
 * MenuNetworkSettings
 ***************************************************************************/
static int MenuNetworkSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options(7);
	options.SetName(i++, tr("Auto Connect"));
	options.SetName(i++, tr("Update Meta.xml"));
	options.SetName(i++, tr("Update Icon.png"));
	options.SetName(i++, tr("Update (App) Path"));
	options.SetName(i++, tr("SMB Settings"));
	options.SetName(i++, tr("FTP Client Settings"));
	options.SetName(i++, tr("FTP Server Settings"));

	GuiSound btnSoundOver(button_over_wav, button_over_wav_size);
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

	GuiText titleTxt(tr("Network Settings"), 24, (GXColor){0, 0, 0, 255});
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
            case 5:
                menu = MENU_FTP_SETTINGS;
                break;
            case 6:
                menu = MENU_FTPSERVER_SETTINGS;
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
			case MENU_FTP_SETTINGS:
				currentMenu = MenuFTPSettings();
				break;
			case MENU_FTPSERVER_SETTINGS:
				currentMenu = MenuFTPServerSettings();
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

	char argument[MAXPATHLEN];
	snprintf(argument, sizeof(argument), "%s", Clipboard::Instance()->GetItemPath(Clipboard::Instance()->GetItemcount()-1));

	ExitApp();

    if(boothomebrew)
        BootHomebrew(argument);
}
