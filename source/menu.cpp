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
#include "menu.h"
#include "main.h"
#include "input.h"
#include "filelist.h"
#include "filebrowser.h"
#include "fileops.h"
#include "foldersize.h"
#include "fatmounter.h"
#include "FileStartUp.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "network/networkops.h"
#include "BootHomebrew/BootHomebrew.h"
#include "gettext.h"
#include "sys.h"

GuiWindow * mainWindow = NULL;
GuiSound * bgMusic = NULL;

static GuiImageData * pointer[4];
static GuiImage * bgImg = NULL;
static GuiImageData * bgImgData = NULL;
static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static CLIPBOARD Clipboard;
static int ExitRequested = 0;
static bool boothomebrew = false;
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
	guiHalt = false;
	LWP_ResumeThread (guithread);
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
	guiHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(guithread))
		usleep(THREAD_SLEEP);
}

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/

static void *
UpdateGUI (void *arg)
{
	int i;

	while(1)
	{
		if(guiHalt)
		{
		    #ifdef HW_RVL
		    for(i = 0; i < 4; i++) {
		        if(pointer[i]) {
                    delete pointer[i];
                    pointer[i] = NULL;
		        }
                pointer[i] = new GuiImageData(player1_point_png);
		    }
            #endif
			LWP_SuspendThread(guithread);
		}
		else
		{
			mainWindow->Draw();

			#ifdef HW_RVL
			for(i=3; i >= 0; i--) // so that player 1's cursor appears on top!
			{
				if(userInput[i].wpad.ir.valid)
					Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48,
						96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, 1, 1, 255);
				//DoRumble(i);
			}
			#endif

			Menu_Render();

			for(i=0; i < 4; i++)
				mainWindow->Update(&userInput[i]);

			if(ExitRequested)
			{
				for(i = 0; i < 255; i += 15)
				{
					mainWindow->Draw();
					Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, i},1);
					Menu_Render();
				}
                ExitRequested = 2;
                return NULL;
			}
		}
	}
	return NULL;
}

/****************************************************************************
 * InitThread
 *
 * Startup threads
 ***************************************************************************/
void InitThreads()
{
    //!Initialize main GUI handling thread
	LWP_CreateThread (&guithread, UpdateGUI, NULL, NULL, 0, 70);

	//!Initalize the progress thread
	InitProgressThread();
	StopProgress();

    //!Initialize network thread if selected
    InitNetworkThread();
    if(Settings.AutoConnect == on)
        ResumeNetworkThread();
    else
        HaltNetworkThread();

    //!Initialize GetSizeThread for Properties
    InitGetSizeThread();
    StopSizeGain();
}

/****************************************************************************
 * ExitGUIThread
 *
 * Shutdown GUI threads
 ***************************************************************************/
void ExitGUIThreads()
{
	ExitRequested = 1;
	LWP_JoinThread(guithread, NULL);
	guithread = LWP_THREAD_NULL;
}

/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/
static int MenuBrowseDevice()
{
	int i, choice = -1, counter = 0;
	char currentdir[50];
    time_t currenttime = time(0);
    struct tm * timeinfo = localtime(&currenttime);

    if(firsttimestart && Settings.MountMethod > USB &&
        Settings.AutoConnect == on && !IsNetworkInit()) {

        WaitSMBConnect();
        firsttimestart = false;
        sleep(2);
    }

	// populate initial directory listing
	if(BrowseDevice(Settings.MountMethod) <= 0)
	{
		int choice = WindowPrompt(tr("Error"),
		tr("Unable to load device."),
		tr("Retry"),
		tr("Change Settings"));

		if(choice) {
            SDCard_Init();
            USBDevice_Init();
			return MENU_BROWSE_DEVICE;
		}
		else
			return MENU_SETTINGS;
	}

	int menu = MENU_NONE;

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
    GuiTrigger trigMinus;
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);

	GuiFileBrowser fileBrowser(584, 248);
	fileBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	fileBrowser.SetPosition(28, 100);

	GuiImageData settingsImgData(settingsbtn_png);
	GuiImage settingsImg(&settingsImgData);
	GuiImageData settingsImgOverData(settingsbtn_over_png);
	GuiImage settingsImgOver(&settingsImgOverData);

	GuiButton SettingsBtn(settingsImg.GetWidth(), settingsImg.GetHeight());
	SettingsBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	SettingsBtn.SetPosition(87, 0);
	SettingsBtn.SetImage(&settingsImg);
	SettingsBtn.SetImageOver(&settingsImgOver);
	SettingsBtn.SetSoundClick(&btnSoundClick);
	SettingsBtn.SetSoundOver(&btnSoundOver);
	SettingsBtn.SetTrigger(&trigA);
	SettingsBtn.SetEffectGrow();

    GuiImageData ExitBtnImgData(power_png);
	GuiImage ExitBtnImg(&ExitBtnImgData);
    GuiImageData ExitBtnImgOverData(power_over_png);
	GuiImage ExitBtnImgOver(&ExitBtnImgOverData);
	GuiButton ExitBtn(ExitBtnImg.GetWidth(), ExitBtnImg.GetHeight());
	ExitBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	ExitBtn.SetPosition(490, 0);
	ExitBtn.SetImage(&ExitBtnImg);
	ExitBtn.SetImageOver(&ExitBtnImgOver);
	ExitBtn.SetSoundClick(&btnSoundClick);
	ExitBtn.SetSoundOver(&btnSoundOver);
	ExitBtn.SetTrigger(&trigA);
	ExitBtn.SetEffectGrow();

	GuiImageData sdstorage(sdstorage_png);
	GuiImageData usbstorage(usbstorage_png);
	GuiImageData networkstorage(networkstorage_png);
	GuiImage deviceImg(&sdstorage);

	if(Settings.MountMethod == 1)
        deviceImg.SetImage(&usbstorage);
    else if(Settings.MountMethod == 2)
        deviceImg.SetImage(&networkstorage);

	GuiButton deviceSwitchBtn(deviceImg.GetWidth(), deviceImg.GetHeight());
	deviceSwitchBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	deviceSwitchBtn.SetPosition(48, fileBrowser.GetTop()-38);
	deviceSwitchBtn.SetImage(&deviceImg);
	deviceSwitchBtn.SetSoundClick(&btnSoundClick);
	deviceSwitchBtn.SetSoundOver(&btnSoundOver);
	deviceSwitchBtn.SetTrigger(&trigA);
	deviceSwitchBtn.SetEffectGrow();

	GuiImageData Address(addressbar_textbox_png);
    snprintf(currentdir, sizeof(currentdir), "%s%s", browser.rootdir, browser.dir);
	GuiText AdressText(currentdir, 20, (GXColor) {0, 0, 0, 255});
	AdressText.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	AdressText.SetPosition(18, 0);
	AdressText.SetMaxWidth(Address.GetWidth()-40, GuiText::SCROLL);
	GuiImage AdressbarImg(&Address);
	GuiButton Adressbar(Address.GetWidth(), Address.GetHeight());
	Adressbar.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	Adressbar.SetPosition(110, fileBrowser.GetTop()-38);
	Adressbar.SetImage(&AdressbarImg);
	Adressbar.SetLabel(&AdressText);

	char timetxt[20];
	strftime(timetxt, sizeof(timetxt), "%H:%M:%S", timeinfo);

	GuiText TimeTxt(timetxt, 20, (GXColor) {40, 40, 40, 255});
	TimeTxt.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	TimeTxt.SetPosition(-26, 0);

	GuiImageData taskbarImgData(taskbar_png);
	GuiImage taskbarImg(&taskbarImgData);
	GuiWindow TaskBar(taskbarImg.GetWidth(), taskbarImg.GetHeight());
	TaskBar.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	TaskBar.SetPosition(4, -5);

	GuiButton clickmenuBtn(screenwidth, screenheight);
	clickmenuBtn.SetTrigger(&trigPlus);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	TaskBar.Append(&taskbarImg);
	TaskBar.Append(&SettingsBtn);
	TaskBar.Append(&ExitBtn);
	TaskBar.Append(&TimeTxt);
	w.Append(&clickmenuBtn);
	w.Append(&fileBrowser);
	w.Append(&Adressbar);
	w.Append(&deviceSwitchBtn);
	w.Append(&TaskBar);
	mainWindow->Append(&w);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

        counter++;

        if(counter > 50) {
            currenttime = time(0);
            timeinfo = localtime(&currenttime);
            strftime(timetxt, sizeof(timetxt), "%H:%M:%S", timeinfo);
            TimeTxt.SetText(timetxt);
            counter = 0;
        }

		for(i=0; i<PAGESIZE; i++)
		{
			if(fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();
				// check corresponding browser entry
				if(browserList[browser.selIndex].isdir)
				{
					if(BrowserChangeFolder())
					{
						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();
                        AdressText.SetTextf("%s%s", browser.rootdir, browser.dir);
					} else {
						menu = MENU_BROWSE_DEVICE;
						break;
					}
				} else {
				    char filepath[MAXPATHLEN];
					snprintf(filepath, sizeof(filepath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                    int result = FileStartUp(filepath);

                    if(result == BOOTHOMEBREW) {
                        choice = WindowPrompt(tr("Do you want to boot:"), browserList[browser.selIndex].filename, tr("Yes"), tr("No"));
                        if(choice == 1) {
                            menu = MENU_EXIT;
                            strncpy(Clipboard.filepath, filepath, sizeof(Clipboard.filepath));
                            boothomebrew = true;
                        }
                    }
				}
			}
		}

        if(SettingsBtn.GetState() == STATE_CLICKED)
			menu = MENU_SETTINGS;

        else if(ExitBtn.GetState() == STATE_CLICKED)
			menu = MENU_EXIT;

        else if(deviceSwitchBtn.GetState() == STATE_CLICKED) {

            Settings.MountMethod++;
            //Skip device if not connected
            if(Settings.MountMethod == USB && !USBDevice_Inserted())
                Settings.MountMethod++;
            while(Settings.MountMethod > USB && !IsSMB_Mounted(Settings.MountMethod-2)
                    && Settings.MountMethod < MAXDEVICES)
                Settings.MountMethod++;

            if(Settings.MountMethod >= MAXDEVICES)
                Settings.MountMethod = 0;

            if(Settings.MountMethod == SD && !SDCard_Inserted())
                Settings.MountMethod++;

            menu = MENU_BROWSE_DEVICE;
            deviceSwitchBtn.ResetState();
        }

		else if(clickmenuBtn.GetState() == STATE_CLICKED) {

		    int x = 0, y = 0;
            if(userInput[clickmenuBtn.GetStateChan()].wpad.ir.valid) {
                x = userInput[clickmenuBtn.GetStateChan()].wpad.ir.x;
                y = userInput[clickmenuBtn.GetStateChan()].wpad.ir.y;
            }

            if(fileBrowser.IsInside(x, y)) {

            fileBrowser.DisableTriggerUpdate(true);
            choice = RightClickMenu(x, y);

            if(strcmp(browserList[browser.selIndex].filename,"..") != 0) {

            if(choice == CUT) {
                if(browserList[browser.selIndex].isdir)
                    choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Cut directory?"), tr("Yes"), tr("Cancel"));
                else
                    choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Cut file?"), tr("Yes"), tr("Cancel"));
                if(choice == 1) {
                    sprintf(Clipboard.filepath, "%s%s", browser.rootdir, browser.dir);
                    sprintf(Clipboard.filename, "%s", browserList[browser.selIndex].filename);
                    if(browserList[browser.selIndex].isdir)
                        Clipboard.isdir = true;
                    else
                        Clipboard.isdir = false;
                    Clipboard.cutted = true;
                }
            }

            else if(choice == COPY) {
                if(browserList[browser.selIndex].isdir)
                    choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Copy directory?"), tr("Yes"), tr("Cancel"));
                else
                    choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Copy file?"), tr("Yes"), tr("Cancel"));
                if(choice == 1) {
                    sprintf(Clipboard.filepath, "%s%s", browser.rootdir, browser.dir);
                    sprintf(Clipboard.filename, "%s", browserList[browser.selIndex].filename);
                    if(browserList[browser.selIndex].isdir)
                        Clipboard.isdir = true;
                    else
                        Clipboard.isdir = false;
                    Clipboard.cutted = false;
                }
            }

            else if(choice == RENAME) {
                char srcpath[MAXPATHLEN];
                char destdir[MAXPATHLEN];
                snprintf(srcpath, sizeof(srcpath), "%s/%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                char entered[43];
                snprintf(entered, sizeof(entered), "%s", browserList[browser.selIndex].filename);
                int result = OnScreenKeyboard(entered, 43);
                if(result == 1) {
                    snprintf(destdir, sizeof(destdir), "%s%s/%s", browser.rootdir, browser.dir, entered);
                    int ret = rename(srcpath, destdir);
                    if(ret != 0)
                        WindowPrompt(tr("Failed renaming file"), tr("Name already exists."), tr("OK"));
                    ParseDirectory();
                    fileBrowser.TriggerUpdate();
                }
            }

            else if(choice == DELETE) {
                if(browserList[browser.selIndex].isdir) {
                    char currentpath[MAXPATHLEN];
                    snprintf(currentpath, sizeof(currentpath), "%s%s/%s/", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                    choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Delete directory and its content?"), tr("Yes"), tr("Cancel"));
                    if(choice == 1) {
                        StartProgress(tr("Deleting files:"), THROBBER);
                        int res = RemoveDirectory(currentpath);
                        StopProgress();
                        if(res == -10)
                            WindowPrompt(tr("Deleting files:"), tr("Action cancelled."), tr("OK"));
                        else if(res < 0)
                            WindowPrompt(tr("Error"), tr("Directory couldn't be deleted."), tr("OK"));
                        else
                            WindowPrompt(tr("Directory successfully deleted."), 0, tr("OK"));
                        ParseDirectory();
                        fileBrowser.TriggerUpdate();
                    }
                } else {
                    char currentpath[MAXPATHLEN];
                    snprintf(currentpath, sizeof(currentpath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                    choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Delete this file?"), tr("Yes"), tr("Cancel"));
                    if(choice == 1) {
                        if(RemoveFile(currentpath) == false) {
                            WindowPrompt(tr("Error"), tr("File couldn't be deleted."), tr("OK"));
                        }
                        ParseDirectory();
                        fileBrowser.TriggerUpdate();
                    }
                }
            }

            else if(choice == PROPERTIES) {
                char currentitem[MAXPATHLEN];
                snprintf(currentitem, sizeof(currentitem), "%s%s/", browser.rootdir, browser.dir);
                Properties(browserList[browser.selIndex].filename, currentitem, browserList[browser.selIndex].isdir, (float) browserList[browser.selIndex].length);
            }

            } else if(choice >= 0 && choice != PASTE && choice != NEWFOLDER)
                WindowPrompt(tr("You cant use this operation on:"), tr("Directory .."), tr("OK"));

            if(choice == PASTE) {
                choice = WindowPrompt(Clipboard.filename, tr("Paste into current directory?"), tr("Yes"), tr("Cancel"));
                if(choice == 1) {
                    char srcpath[MAXPATHLEN];
                    char destdir[MAXPATHLEN];
                    if(Clipboard.isdir == true) {
                        snprintf(srcpath, sizeof(srcpath), "%s/%s/", Clipboard.filepath, Clipboard.filename);
                        snprintf(destdir, sizeof(destdir), "%s%s/%s/", browser.rootdir, browser.dir, Clipboard.filename);
                        int res = 0;
                        if(Clipboard.cutted == false) {
                            StartProgress(tr("Copying files:"));
                            res = CopyDirectory(srcpath, destdir);
                            StopProgress();
                        } else {
                            if(strcmp(srcpath, destdir) != 0) {
                                StartProgress(tr("Moving files:"));
                                res = MoveDirectory(srcpath, destdir);
                                StopProgress();
                            } else {
                                WindowPrompt(tr("Error:"), tr("You can not cut into the directory itself.") , tr("OK"));
                                res =  -1;
                            }
                        }

                        if(res == -10)
                            WindowPrompt(tr("Transfering files:"), tr("Action cancelled."), tr("OK"));
                        else if(res < 0)
                            WindowPrompt(tr("An error occured."), tr("Failed copying files."), tr("OK"));
                        else {
                            if(Clipboard.cutted == false)
                                WindowPrompt(tr("Directory successfully copied."), 0, tr("OK"));
                            else {
                                WindowPrompt(tr("Directory successfully moved."), 0, tr("OK"));
                            }
                        }
                    } else {
                        snprintf(srcpath, sizeof(srcpath), "%s/%s", Clipboard.filepath, Clipboard.filename);
                        int ret = CheckFile(srcpath);
                        if(ret == false)
                            WindowPrompt(tr("File does not exist anymore!"), 0, tr("OK"));
                        else {
                            snprintf(destdir, sizeof(destdir), "%s%s/%s", browser.rootdir, browser.dir, Clipboard.filename);
                            if(strcmp(srcpath, destdir) != 0) {
                                StartProgress(tr("Copying file:"));
                                int res = CopyFile(srcpath, destdir);
                                StopProgress();
                                if(res < 0)
                                    WindowPrompt(tr("ERROR"), tr("Failed copying file."), tr("OK"));
                                else {
                                    if(Clipboard.cutted == false)
                                        WindowPrompt(tr("File successfully copied."), 0, tr("OK"));
                                    else {
                                        if(RemoveFile(srcpath) == false)
                                            WindowPrompt(tr("Error"), tr("File couldn't be deleted."), tr("OK"));
                                        else
                                            WindowPrompt(tr("File successfully moved."), 0, tr("OK"));
                                    }
                                }
                            } else {
                                WindowPrompt(tr("Error:"), tr("You cannot read/write from/to the same file."), tr("OK"));
                            }
                        }
                    }
                    ParseDirectory();
                }
                    fileBrowser.TriggerUpdate();
            }

            else if(choice == NEWFOLDER) {
                char entered[43];
                snprintf(entered, sizeof(entered), tr("New Folder"));
                int result = OnScreenKeyboard(entered, 42);
                if(result == 1) {
                    char currentpath[MAXPATHLEN];
                    snprintf(currentpath, sizeof(currentpath), "%s%s/%s/", browser.rootdir, browser.dir, entered);
                    bool ret = CreateSubfolder(currentpath);
                    if(ret == false)
                        WindowPrompt(tr("Error:"), tr("Unable to create folder."), tr("OK"));
                    ParseDirectory();
                    fileBrowser.TriggerUpdate();
                }
            }

            fileBrowser.DisableTriggerUpdate(false);
            fileBrowser.SetFocus(1);
		}
            clickmenuBtn.ResetState();
        } else {
            clickmenuBtn.ResetState();
        }
	}
	HaltGui();
	mainWindow->Remove(&w);
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
    char entered[43];

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
	backBtn.SetPosition(100, -35);
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
	mainWindow->Append(&w);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		VIDEO_WaitVSync();

        if(Settings.CurrentUser > MAXSMBUSERS-1)
            Settings.CurrentUser = 0;

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

		options.SetValue(0,tr("User %i"), Settings.CurrentUser+1);
		options.SetValue(1,"%s", Settings.SMBUser[Settings.CurrentUser].Host);
		options.SetValue(2,"%s", Settings.SMBUser[Settings.CurrentUser].User);
		options.SetValue(3,"%s", Settings.SMBUser[Settings.CurrentUser].Password);
		options.SetValue(4,"%s", Settings.SMBUser[Settings.CurrentUser].SMBName);
		options.SetValue(5," ");

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.CurrentUser++;
				break;
            case 1:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].Host);
                result = OnScreenKeyboard(entered, 42);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentUser].Host, sizeof(Settings.SMBUser[Settings.CurrentUser].Host), "%s", entered);
                }
                break;
            case 2:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].User);
                result = OnScreenKeyboard(entered, 42);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentUser].User, sizeof(Settings.SMBUser[Settings.CurrentUser].User), "%s", entered);
                }
                break;
            case 3:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].Password);
                result = OnScreenKeyboard(entered, 42);
                if(result) {
                    snprintf(Settings.SMBUser[Settings.CurrentUser].Password, sizeof(Settings.SMBUser[Settings.CurrentUser].Password), "%s", entered);
                }
                break;
            case 4:
                snprintf(entered, sizeof(entered), "%s", Settings.SMBUser[Settings.CurrentUser].SMBName);
                result = OnScreenKeyboard(entered, 42);
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

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
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

	OptionList options(5);
	options.SetName(i++, tr("Mount Method"));
	options.SetName(i++, tr("Language"));
	options.SetName(i++, tr("Auto Connect"));
	options.SetName(i++, tr("Music Volume"));
	options.SetName(i++, tr("SMB Settings"));

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(100, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(584, 248, &options);
	optionBrowser.SetPosition(30, 100);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	GuiImageData settingsimgData(settingsbtn_over_png);
	GuiImage settingsimg(&settingsimgData);
	settingsimg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsimg.SetPosition(50, optionBrowser.GetTop()-35);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&optionBrowser);
	w.Append(&settingsimg);
	mainWindow->Append(&w);
	ResumeGui();

	while(menu == MENU_NONE)
	{
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

        i = 0;

		if (Settings.MountMethod == SD) options.SetValue(i++,tr("SD"));
		else if (Settings.MountMethod == USB) options.SetValue(i++,tr("USB"));
		else if (Settings.MountMethod == SMB1) options.SetValue(i++, tr("SMB1"));
		else if (Settings.MountMethod == SMB2) options.SetValue(i++, tr("SMB2"));
		else if (Settings.MountMethod == SMB3) options.SetValue(i++, tr("SMB3"));
		else if (Settings.MountMethod == SMB4) options.SetValue(i++, tr("SMB4"));

		if (Settings.Language == APP_DEFAULT) options.SetValue(i++,tr("App Default"));
		else if (Settings.Language == CONSOLE_DEFAULT) options.SetValue(i++,tr("Console Default"));
		else if (Settings.Language == JAPANESE) options.SetValue(i++,tr("Japanese"));
		else if (Settings.Language == ENGLISH) options.SetValue(i++,tr("English"));
		else if (Settings.Language == GERMAN) options.SetValue(i++,tr("German"));
		else if (Settings.Language == FRENCH) options.SetValue(i++,tr("French"));
		else if (Settings.Language == SPANISH) options.SetValue(i++,tr("Spanish"));
		else if (Settings.Language == ITALIAN) options.SetValue(i++,tr("Italian"));
		else if (Settings.Language == DUTCH) options.SetValue(i++,tr("Dutch"));
		else if (Settings.Language == S_CHINESE) options.SetValue(i++,tr("S. Chinese"));
		else if (Settings.Language == T_CHINESE) options.SetValue(i++,tr("T. Chinese"));
		else if (Settings.Language == KOREAN) options.SetValue(i++,tr("Korean"));

		if (Settings.AutoConnect == on) options.SetValue(i++,tr("ON"));
		else if (Settings.AutoConnect == off) options.SetValue(i++,tr("OFF"));

		if (Settings.MusicVolume > 0) options.SetValue(i++, "%i", Settings.MusicVolume);
		else options.SetValue(i++, tr("OFF"));

        options.SetValue(i++, " ");

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
				Settings.Language++;
				if(Settings.Language >= MAX_LANGUAGE)
                    Settings.Language = 0;
                Settings.LoadLanguage(Settings.Language);
                menu = MENU_SETTINGS;   //reload window so the translations take effect
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
                bgMusic->SetVolume(Settings.MusicVolume);
				break;
            case 4:
                menu = MENU_SMB_SETTINGS;
                break;
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
		    if(SDCard_Inserted())
                Settings.Save();
			menu = MENU_BROWSE_DEVICE;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	ResumeGui();

	return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void MainMenu(int menu)
{
	int currentMenu = menu;

	#ifdef HW_RVL
	pointer[0] = new GuiImageData(player1_point_png);
	pointer[1] = new GuiImageData(player2_point_png);
	pointer[2] = new GuiImageData(player3_point_png);
	pointer[3] = new GuiImageData(player4_point_png);
	#endif

	mainWindow = new GuiWindow(screenwidth, screenheight);

    bgImgData = new GuiImageData(background_png);

    bgImg = new GuiImage(bgImgData);
	mainWindow->Append(bgImg);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	ResumeGui();

	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(Settings.MusicVolume);
	bgMusic->SetLoop(1);
	bgMusic->Play(); // startup music

	while(currentMenu != MENU_EXIT)
	{
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
			default: // unrecognized menu
				currentMenu = MenuBrowseDevice();
				break;
		}
	}

	ResumeGui();

	bgMusic->Stop();
	delete bgMusic;
	ExitApp();

	while(ExitRequested != 2) usleep(THREAD_SLEEP);

	delete bgImg;
	delete bgImgData;
	delete mainWindow;
	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];

    if(boothomebrew)
        BootHomebrew(Clipboard.filepath);

	CloseSMBShare();
    SDCard_deInit();
    USBDevice_deInit();

    //last point in programm to make sure the allocated memory is freed
	Sys_BackToLoader();
}
