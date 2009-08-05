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
#include "sys.h"

GuiWindow * mainWindow = NULL;

static GuiSound * bgMusic = NULL;
static GuiImageData * pointer[4];
static GuiImage * bgImg = NULL;
static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static CLIPBOARD Clipboard;
static int ExitRequested = 0;
static bool boothomebrew = false;

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
	int i, choice = -1;
	char currentdir[50];

	// populate initial directory listing
	if(BrowseDevice(Settings.MountMethod) <= 0)
	{
		int choice = WindowPrompt("Error",
		"Unable to load device.",
		"Retry",
		"Change Settings");

		if(choice) {
			return MENU_BROWSE_DEVICE;
		}
		else
			return MENU_SETTINGS;
	}

	int menu = MENU_NONE;

	GuiText titleTxt("Browse Files", 28, (GXColor){0, 0, 0, 230});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(70,20);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
    GuiTrigger trigMinus;
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 100);

	GuiImageData btnOutline(button_png);
	GuiText SettingsBtnTxt("Settings", 24, (GXColor){0, 0, 0, 255});
	GuiImage SettingsBtnImg(&btnOutline);
	GuiButton SettingsBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	SettingsBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	SettingsBtn.SetPosition(370, -35);
	SettingsBtn.SetLabel(&SettingsBtnTxt);
	SettingsBtn.SetImage(&SettingsBtnImg);
	SettingsBtn.SetSoundClick(&btnSoundClick);
	SettingsBtn.SetSoundOver(&btnSoundOver);
	SettingsBtn.SetTrigger(&trigA);
	SettingsBtn.SetEffectGrow();

	GuiText ExitBtnTxt("Exit", 24, (GXColor){0, 0, 0, 255});
	GuiImage ExitBtnImg(&btnOutline);
	GuiButton ExitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	ExitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	ExitBtn.SetPosition(100, -35);
	ExitBtn.SetLabel(&ExitBtnTxt);
	ExitBtn.SetImage(&ExitBtnImg);
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
	deviceSwitchBtn.SetPosition(60, fileBrowser.GetTop()-45);
	deviceSwitchBtn.SetImage(&deviceImg);
	deviceSwitchBtn.SetSoundClick(&btnSoundClick);
	deviceSwitchBtn.SetSoundOver(&btnSoundOver);
	deviceSwitchBtn.SetTrigger(&trigA);
	deviceSwitchBtn.SetEffectGrow();

	GuiImageData Address(addressbar_textbox_png);
    snprintf(currentdir, sizeof(currentdir), "%s%s", browser.rootdir, browser.dir);
	GuiText AdressText(currentdir, 20, (GXColor) {0, 0, 0, 255});
	AdressText.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	AdressText.SetPosition(20, 0);
	AdressText.SetMaxWidth(Address.GetWidth()-40, GuiText::SCROLL);
	GuiImage AdressbarImg(&Address);
	GuiButton Adressbar(Address.GetWidth(), Address.GetHeight());
	Adressbar.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	Adressbar.SetPosition(120, fileBrowser.GetTop()-45);
	Adressbar.SetImage(&AdressbarImg);
	Adressbar.SetLabel(&AdressText);

	GuiButton clickmenuBtn(screenwidth, screenheight);
	clickmenuBtn.SetTrigger(&trigPlus);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&clickmenuBtn);
	w.Append(&SettingsBtn);
	w.Append(&ExitBtn);
	w.Append(&titleTxt);
	w.Append(&fileBrowser);
	w.Append(&Adressbar);
	w.Append(&deviceSwitchBtn);
	mainWindow->Append(&w);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

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
                        choice = WindowPrompt("Do you want to boot:", browserList[browser.selIndex].filename, "Yes", "No");
                        menu = MENU_EXIT;
                        strncpy(Clipboard.filepath, filepath, sizeof(Clipboard.filepath));
                        boothomebrew = true;
                    }
				}
			}
		}

        if(SettingsBtn.GetState() == STATE_CLICKED)
			menu = MENU_SETTINGS;

        else if(ExitBtn.GetState() == STATE_CLICKED)
			menu = MENU_EXIT;

        else if(deviceSwitchBtn.GetState() == STATE_CLICKED) {
            Settings.MountMethod = (Settings.MountMethod+1) % 3;
            menu = MENU_BROWSE_DEVICE;
            deviceSwitchBtn.ResetState();
        }

		else if(clickmenuBtn.GetState() == STATE_CLICKED) {

		    int x = 0, y = 0;
            if(userInput[clickmenuBtn.GetStateChan()].wpad.ir.valid) {
                x = userInput[clickmenuBtn.GetStateChan()].wpad.ir.x;
                y = userInput[clickmenuBtn.GetStateChan()].wpad.ir.y;
            }

            fileBrowser.DisableTriggerUpdate(true);
            choice = RightClickMenu(x, y);

            if(strcmp(browserList[browser.selIndex].filename,"..") != 0
                && fileBrowser.IsInside(x, y)) {

            if(choice == CUT) {
                if(browserList[browser.selIndex].isdir)
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Cut directory?", "Yes", "Cancel");
                else
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Cut file?", "Yes", "Cancel");
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
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Copy directory?", "Yes", "Cancel");
                else
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Copy file?", "Yes", "Cancel");
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
                        WindowPrompt("Failed renaming file", "Failname already exists.", "OK");
                    ParseDirectory();
                    fileBrowser.TriggerUpdate();
                }
            }

            else if(choice == DELETE) {
                if(browserList[browser.selIndex].isdir) {
                    char currentpath[MAXPATHLEN];
                    snprintf(currentpath, sizeof(currentpath), "%s%s/%s/", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Delete directory and its content?", "Yes", "Cancel");
                    if(choice == 1) {
                        StartProgress("Deleting files:", THROBBER);
                        int res = RemoveDirectory(currentpath);
                        StopProgress();
                        if(res < 0)
                            WindowPrompt("Error", "Directory couldn't be deleted.", "OK");
                        else
                            WindowPrompt("Directory successfully deleted.", 0, "OK");
                        ParseDirectory();
                        fileBrowser.TriggerUpdate();
                    }
                } else {
                    char currentpath[MAXPATHLEN];
                    snprintf(currentpath, sizeof(currentpath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Delete this file?", "Yes", "Cancel");
                    if(choice == 1) {
                        if(RemoveFile(currentpath) == false) {
                            WindowPrompt("Error", "File couldn't be deleted.", "OK");
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

            } else
                WindowPrompt("You cant use this operation on:", "Directory ..", "OK");

            if(choice == PASTE) {
                //if(Settings.MountMethod != SMB) {
                choice = WindowPrompt(Clipboard.filename, "Paste into current directory?", "Yes", "Cancel");
                if(choice == 1) {
                    char srcpath[MAXPATHLEN];
                    char destdir[MAXPATHLEN];
                    if(Clipboard.isdir == true) {
                        snprintf(srcpath, sizeof(srcpath), "%s/%s/", Clipboard.filepath, Clipboard.filename);
                        snprintf(destdir, sizeof(destdir), "%s%s/%s/", browser.rootdir, browser.dir, Clipboard.filename);
                        int res = 0;
                        if(Clipboard.cutted == false) {
                            StartProgress("Copying files:");
                            res = CopyDirectory(srcpath, destdir);
                            StopProgress();
                        } else {
                            StartProgress("Moving files:");
                            res = MoveDirectory(srcpath, destdir);
                            StopProgress();
                        }
                        if(res < 0)
                            WindowPrompt("An error accured.", "Failed copying files.", "OK");
                        else {
                            if(Clipboard.cutted == false)
                                WindowPrompt("Directory successfully copied.", 0, "OK");
                            else {
                                WindowPrompt("Directory successfully moved.", 0, "OK");
                            }
                        }
                    } else {
                        snprintf(srcpath, sizeof(srcpath), "%s/%s", Clipboard.filepath, Clipboard.filename);
                        int ret = CheckFile(srcpath);
                        if(ret == false)
                            WindowPrompt("File does not exist anymore!", 0, "OK");
                        else {
                            snprintf(destdir, sizeof(destdir), "%s%s/%s", browser.rootdir, browser.dir, Clipboard.filename);
                            StartProgress("Copying file:");
                            int res = CopyFile(srcpath, destdir);
                            StopProgress();
                            if(res < 0)
                                WindowPrompt("ERROR", "Failed copying file.", "OK");
                            else {
                                if(Clipboard.cutted == false)
                                    WindowPrompt("File successfully copied.", 0, "OK");
                                else {
                                    if(RemoveFile(srcpath) == false)
                                        WindowPrompt("Error", "File couldn't be deleted.", "OK");
                                    else
                                        WindowPrompt("File successfully moved.", 0, "OK");
                                }
                            }
                        }
                    }
                    ParseDirectory();
                }
                    fileBrowser.TriggerUpdate();
                //} else
                   // WindowPrompt("Error:", "Writting to SMB doesnt work currently", "OK");
            }

            fileBrowser.DisableTriggerUpdate(false);

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
	options.SetName(i++, "User:");
	options.SetName(i++, "Host:");
	options.SetName(i++, "Username:");
	options.SetName(i++, "Password:");
	options.SetName(i++, "SMB Name:");
	if(Settings.AutoConnect == on)
        options.SetName(i++, "Reconnect SMB");
    else
        options.SetName(i++, "Connect SMB");

	GuiText titleTxt("SMB Settings", 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(70,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
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

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&titleTxt);
	w.Append(&optionBrowser);
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

		options.SetValue(0, "User %i", Settings.CurrentUser+1);
		options.SetValue(1, "%s", Settings.SMBUser[Settings.CurrentUser].Host);
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
                result = WindowPrompt("Do you want to reconnect the SMB?",0,"OK","Cancel");
                if(result) {
                    SMB_Reconnect();
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

	OptionList options(3);
	options.SetName(i++, "Mount Method");
	options.SetName(i++, "Auto Connect");
	options.SetName(i++, "SMB Settings");

	GuiText titleTxt("Settings", 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(70,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
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

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&titleTxt);
	mainWindow->Append(&w);
	mainWindow->Append(&optionBrowser);
	ResumeGui();

	while(menu == MENU_NONE)
	{
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

		if (Settings.MountMethod == METHOD_SD) options.SetValue(0,"SD");
		else if (Settings.MountMethod == METHOD_USB) options.SetValue(0,"USB");
		else if (Settings.MountMethod == METHOD_SMB) options.SetValue(0,"Network");

		if (Settings.AutoConnect == on) options.SetValue(1,"ON");
		else if (Settings.AutoConnect == off) options.SetValue(1,"OFF");

        options.SetValue(2, " ");

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.MountMethod++;
				if(Settings.MountMethod > 2)
                    Settings.MountMethod = 0;
				break;
            case 1:
				Settings.AutoConnect++;
				if(Settings.AutoConnect >= on_off_max)
                    Settings.AutoConnect = off;
				break;
            case 2:
                menu = MENU_SMB_SETTINGS;
                break;
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
		    Settings.Save();
			menu = MENU_BROWSE_DEVICE;
		}
	}

	HaltGui();
	mainWindow->Remove(&optionBrowser);
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

    bgImg = new GuiImage(screenwidth, screenheight, (GXColor){150, 150, 150, 255});
	bgImg->ColorStripe(10);
	mainWindow->Append(bgImg);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	ResumeGui();

	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(80);
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
	ExitApp();

	while(ExitRequested != 2) usleep(THREAD_SLEEP);

	bgMusic->Stop();
	delete bgMusic;
	delete bgImg;
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
