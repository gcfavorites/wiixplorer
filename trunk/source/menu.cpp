/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>

#include "libwiigui/gui.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "filelist.h"
#include "filebrowser.h"
#include "fileops.h"
#include "PromptWindows.h"
#include "network/networkops.h"
#include "sys.h"

GuiWindow * mainWindow = NULL;

static GuiSound * bgMusic = NULL;
static GuiImageData * pointer[4];
static GuiImageData * background = NULL;
static GuiImage * bgImg = NULL;
static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static CLIPBOARD Clipboard;
static int ExitRequested = 0;

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
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void InitGUIThreads()
{
	LWP_CreateThread (&guithread, UpdateGUI, NULL, NULL, 0, 70);
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

	// populate initial directory listing
	if(BrowseDevice(Settings.MountMethod) <= 0)
	{
		int choice = WindowPrompt("Error",
		"Unable to display files on selected load device.",
		"Retry",
		"Check Settings",0,0);

		if(choice) {
			return MENU_BROWSE_DEVICE;
		}
		else
			return MENU_SETTINGS;
	}

	int menu = MENU_NONE;

	GuiText titleTxt("Browse Files", 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(100,50);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
    GuiTrigger trigMinus;
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 100);

	GuiImageData btnOutline(button_png);
	GuiText SettingsBtnTxt("Settings", 24, (GXColor){0, 0, 0, 255});
	GuiImage SettingsBtnImg(&btnOutline);
	GuiButton SettingsBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	SettingsBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	SettingsBtn.SetPosition(300, -35);
	SettingsBtn.SetLabel(&SettingsBtnTxt);
	SettingsBtn.SetImage(&SettingsBtnImg);
	SettingsBtn.SetTrigger(&trigA);
	SettingsBtn.SetEffectGrow();

	GuiText ExitBtnTxt("Exit", 24, (GXColor){0, 0, 0, 255});
	GuiImage ExitBtnImg(&btnOutline);
	GuiButton ExitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	ExitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	ExitBtn.SetPosition(30, -35);
	ExitBtn.SetLabel(&ExitBtnTxt);
	ExitBtn.SetImage(&ExitBtnImg);
	ExitBtn.SetTrigger(&trigA);
	ExitBtn.SetEffectGrow();

	GuiButton clickmenuBtn(screenwidth, screenheight);
	clickmenuBtn.SetTrigger(&trigPlus);

	GuiWindow buttonWindow(screenwidth, screenheight);
	buttonWindow.Append(&clickmenuBtn);
	buttonWindow.Append(&SettingsBtn);
	buttonWindow.Append(&ExitBtn);

	HaltGui();
	mainWindow->Append(&titleTxt);
	mainWindow->Append(&fileBrowser);
	mainWindow->Append(&buttonWindow);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

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
					} else {
						menu = MENU_BROWSE_DEVICE;
						break;
					}
				} else {
					mainWindow->SetState(STATE_DISABLED);
					mainWindow->SetState(STATE_DEFAULT);
				}
			}
		}

        if(SettingsBtn.GetState() == STATE_CLICKED)
			menu = MENU_SETTINGS;

        if(ExitBtn.GetState() == STATE_CLICKED)
			menu = MENU_EXIT;

		else if(clickmenuBtn.GetState() == STATE_CLICKED) {

		    int x = 0, y = 0;
            if(userInput[clickmenuBtn.GetStateChan()].wpad.ir.valid) {
                x = userInput[clickmenuBtn.GetStateChan()].wpad.ir.x;
                y = userInput[clickmenuBtn.GetStateChan()].wpad.ir.y;
            }

            if(strcmp(browserList[browser.selIndex].filename,".") != 0
                && strcmp(browserList[browser.selIndex].filename,"..") != 0
                && fileBrowser.IsInside(x, y)) {


            fileBrowser.DisableTriggerUpdate(true);
            choice = RightClickMenu(x, y);

            if(choice == CUT) {
                if(browserList[browser.selIndex].isdir)
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Cut directory?", "Yes", "Cancel",0,0);
                else
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Cut file?", "Yes", "Cancel",0,0);
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
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Copy directory?", "Yes", "Cancel",0,0);
                else
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Copy file?", "Yes", "Cancel",0,0);
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

            else if(choice == PASTE) {
                if(Settings.MountMethod != SMB) {
                choice = WindowPrompt(Clipboard.filename, "Paste into current directory?", "Yes", "Cancel",0,0);
                if(choice == 1) {
                    char srcpath[MAXPATHLEN];
                    char destdir[MAXPATHLEN];
                    if(Clipboard.isdir == true) {
                        snprintf(srcpath, sizeof(srcpath), "%s/%s/", Clipboard.filepath, Clipboard.filename);
                        snprintf(destdir, sizeof(destdir), "%s%s/%s/", browser.rootdir, browser.dir, Clipboard.filename);
                        int res = 0;
                        if(Clipboard.cutted == false)
                            res = ProgressWindow("Copying files:", srcpath, destdir, COPYDIR);
                        else
                            res = ProgressWindow("Moving files:", srcpath, destdir, MOVEDIR);
                        if(res < 0)
                            WindowPrompt("An error accured.", "Failed copying files.", "OK",0,0,0);
                        else {
                            if(Clipboard.cutted == false)
                                WindowPrompt("Directory successfully copied.", 0, "OK",0,0,0);
                            else {
                                WindowPrompt("Directory successfully moved.", 0, "OK",0,0,0);
                            }
                        }
                    } else {
                    snprintf(srcpath, sizeof(srcpath), "%s/%s", Clipboard.filepath, Clipboard.filename);
                    int ret = CheckFile(srcpath);
                    if(ret == false)
                        WindowPrompt("File does not exist anymore!", 0, "OK", 0, 0, 0);
                    else {
                        snprintf(destdir, sizeof(destdir), "%s%s/%s", browser.rootdir, browser.dir, Clipboard.filename);
                        int res = ProgressWindow("Copying file:", srcpath, destdir, COPYFILE);
                        if(res < 0)
                            WindowPrompt("ERROR", "Failed copying file.", "OK",0,0,0);
                        else {
                            if(Clipboard.cutted == false)
                                WindowPrompt("File successfully copied.", 0, "OK",0,0,0);
                            else {
                                if(RemoveFile(srcpath) == false) {
                                    WindowPrompt("Error", "File couldn't be deleted.", "OK",0,0,0);
                                } else
                                    WindowPrompt("File successfully moved.", 0, "OK",0,0,0);
                            }
                        }
                    }
                    }
                    ParseDirectory();
                    fileBrowser.TriggerUpdate();
                }
                } else
                    WindowPrompt("Error:", "Writting to SMB doesnt work currently", "OK", 0, 0, 0);
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
                        WindowPrompt("Failed renaming file", "Failname already exists.", "OK", 0, 0, 0);
                    ParseDirectory();
                    fileBrowser.TriggerUpdate();
                }
            }

            else if(choice == DELETE) {
                if(browserList[browser.selIndex].isdir) {
                    char currentpath[MAXPATHLEN];
                    snprintf(currentpath, sizeof(currentpath), "%s%s/%s/", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Delete directory and its content?", "Yes", "Cancel",0,0);
                    if(choice == 1) {
                        int res = ProgressWindow("Deleting files:", currentpath, NULL, DELETEDIR, THROBBER);
                        if(res < 0)
                            WindowPrompt("Error", "Directory couldn't be deleted.", "OK",0,0,0);
                        else
                            WindowPrompt("Directory successfully deleted.", 0, "OK",0,0,0);
                        ParseDirectory();
                        fileBrowser.TriggerUpdate();
                    }
                } else {
                    char currentpath[MAXPATHLEN];
                    snprintf(currentpath, sizeof(currentpath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                    choice = WindowPrompt(browserList[browser.selIndex].filename, "Delete this file?", "Yes", "Cancel",0,0);
                    if(choice == 1) {
                        if(RemoveFile(currentpath) == false) {
                            WindowPrompt("Error", "File couldn't be deleted.", "OK",0,0,0);
                        }
                        ParseDirectory();
                        fileBrowser.TriggerUpdate();
                    }
                }
            }

            else if(choice == PROPERTIES) {
                //TO DO PROPERTIES
            }

            fileBrowser.DisableTriggerUpdate(false);
        }
            clickmenuBtn.ResetState();
        } else {
            clickmenuBtn.ResetState();
        }
	}
	HaltGui();
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(&buttonWindow);
	mainWindow->Remove(&fileBrowser);
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
	OptionList options;
	sprintf(options.name[i++], "Mount Method");
	options.length = i;

	GuiText titleTxt("Settings", 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

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
	optionBrowser.SetCol2Position(185);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(Settings.MountMethod > 2)
			Settings.MountMethod = 0;

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

		if (Settings.MountMethod == METHOD_SD) sprintf (options.value[0],"SD");
		else if (Settings.MountMethod == METHOD_USB) sprintf (options.value[0],"USB");
		else if (Settings.MountMethod == METHOD_SMB) sprintf (options.value[0],"Network");

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.MountMethod++;
				break;
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_BROWSE_DEVICE;
		}
	}

	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
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

    background = new GuiImageData(background_png);
	bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	ResumeGui();

	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(50);
	bgMusic->SetLoop(1);
	bgMusic->Play(); // startup music

	while(currentMenu != MENU_EXIT)
	{
		switch (currentMenu)
		{
			case MENU_SETTINGS:
				currentMenu = MenuSettings();
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
	ExitGUIThreads();

	while(ExitRequested != 2) usleep(THREAD_SLEEP);

	bgMusic->Stop();
	delete bgMusic;
	delete bgImg;
	delete mainWindow;
	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];

    //last point in programm to make sure the allocated memory is freed
	Sys_BackToLoader();
}
