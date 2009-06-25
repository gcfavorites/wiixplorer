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
				DoRumble(i);
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
	int i, choice;

	ShutoffRumble();

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

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 100);

	GuiImageData btnOutline(button_png);
	GuiText backBtnTxt("Go Back", 24, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(30, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiText copyBtnTxt("Copy", 24, (GXColor){0, 0, 0, 255});
	GuiImage copyBtnImg(&btnOutline);
	GuiButton copyBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	copyBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	copyBtn.SetPosition(230, -35);
	copyBtn.SetLabel(&copyBtnTxt);
	copyBtn.SetImage(&copyBtnImg);
	copyBtn.SetTrigger(&trigA);
	copyBtn.SetEffectGrow();

	GuiText pasteBtnTxt("Paste", 24, (GXColor){0, 0, 0, 255});
	GuiImage pasteBtnImg(&btnOutline);
	GuiButton pasteBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	pasteBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	pasteBtn.SetPosition(430, -35);
	pasteBtn.SetLabel(&pasteBtnTxt);
	pasteBtn.SetImage(&pasteBtnImg);
	pasteBtn.SetTrigger(&trigA);
	pasteBtn.SetEffectGrow();

	GuiText removeBtnTxt("Remove", 24, (GXColor){0, 0, 0, 255});
	GuiImage removeBtnImg(&btnOutline);
	GuiButton removeBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	removeBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	removeBtn.SetPosition(230, 35);
	removeBtn.SetLabel(&removeBtnTxt);
	removeBtn.SetImage(&removeBtnImg);
	removeBtn.SetTrigger(&trigA);
	removeBtn.SetEffectGrow();

	GuiWindow buttonWindow(screenwidth, screenheight);
	buttonWindow.Append(&backBtn);
	buttonWindow.Append(&copyBtn);
	buttonWindow.Append(&pasteBtn);
	buttonWindow.Append(&removeBtn);

	HaltGui();
	mainWindow->Append(&titleTxt);
	mainWindow->Append(&fileBrowser);
	mainWindow->Append(&buttonWindow);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		// update file browser based on arrow buttons
		// set MENU_EXIT if A button pressed on a file
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
					}
					else
					{
						menu = MENU_BROWSE_DEVICE;
						break;
					}
				}
				else
				{
					ShutoffRumble();
					mainWindow->SetState(STATE_DISABLED);
					// load file
					mainWindow->SetState(STATE_DEFAULT);
				}
			}
		}
		if(backBtn.GetState() == STATE_CLICKED)
			menu = MENU_SETTINGS;

		else if(copyBtn.GetState() == STATE_CLICKED) {
		    if(browserList[browser.selIndex].isdir)
                choice = WindowPrompt("Copy directory?", 0, "Yes", "Cancel",0,0);
            else
		        choice = WindowPrompt("Copy file?", 0, "Yes", "Cancel",0,0);
            if(choice == 1) {
                sprintf(Clipboard.filepath, "%s%s", browser.rootdir, browser.dir);
                sprintf(Clipboard.filename, "%s", browserList[browser.selIndex].filename);
                if(browserList[browser.selIndex].isdir)
                    Clipboard.isdir = true;
                else
                    Clipboard.isdir = false;
            }
			copyBtn.ResetState();
		}

		else if(pasteBtn.GetState() == STATE_CLICKED) {
		    if(Settings.MountMethod != SMB) {
		    choice = WindowPrompt("Paste into current directory?", 0, "Yes", "Cancel",0,0);
		    if(choice == 1) {
		        char srcpath[MAXPATHLEN];
		        char destdir[MAXPATHLEN];
                if(Clipboard.isdir == true) {
                    snprintf(srcpath, sizeof(srcpath), "%s/%s/", Clipboard.filepath, Clipboard.filename);
                    snprintf(destdir, sizeof(destdir), "%s%s/%s/", browser.rootdir, browser.dir, Clipboard.filename);
                    int res = ProgressWindow("Copying files:", srcpath, destdir, COPYDIR);
                    if(res < 0)
                        WindowPrompt("An error accured.", "Failed copying files.", "OK",0,0,0);
                    else
                        WindowPrompt("Directory successfully copied.", 0, "OK",0,0,0);
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
                    else
                        WindowPrompt("File successfully copied.", 0, "OK",0,0,0);
                }
                }
                ParseDirectory();
                fileBrowser.TriggerUpdate();
		    }
		    } else
                WindowPrompt("Error:", "Writting to SMB doesnt work currently", "OK", 0, 0, 0);
		    pasteBtn.ResetState();
		}

		else if(removeBtn.GetState() == STATE_CLICKED) {
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
		    removeBtn.ResetState();
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

	GuiText titleTxt("Settings", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText fileBtnTxt("File Browser", 22, (GXColor){0, 0, 0, 255});
	fileBtnTxt.SetMaxWidth(btnLargeOutline.GetWidth()-30);
	GuiImage fileBtnImg(&btnLargeOutline);
	GuiImage fileBtnImgOver(&btnLargeOutlineOver);
	GuiButton fileBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	fileBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	fileBtn.SetPosition(50, 120);
	fileBtn.SetLabel(&fileBtnTxt);
	fileBtn.SetImage(&fileBtnImg);
	fileBtn.SetImageOver(&fileBtnImgOver);
	fileBtn.SetSoundOver(&btnSoundOver);
	fileBtn.SetTrigger(&trigA);
	fileBtn.SetEffectGrow();

	GuiText videoBtnTxt("Video", 22, (GXColor){0, 0, 0, 255});
	videoBtnTxt.SetMaxWidth(btnLargeOutline.GetWidth()-30);
	GuiImage videoBtnImg(&btnLargeOutline);
	GuiImage videoBtnImgOver(&btnLargeOutlineOver);
	GuiButton videoBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	videoBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	videoBtn.SetPosition(0, 120);
	videoBtn.SetLabel(&videoBtnTxt);
	videoBtn.SetImage(&videoBtnImg);
	videoBtn.SetImageOver(&videoBtnImgOver);
	videoBtn.SetSoundOver(&btnSoundOver);
	videoBtn.SetTrigger(&trigA);
	videoBtn.SetEffectGrow();

	GuiText savingBtnTxt1("Saving", 22, (GXColor){0, 0, 0, 255});
	GuiText savingBtnTxt2("&", 18, (GXColor){0, 0, 0, 255});
	GuiText savingBtnTxt3("Loading", 22, (GXColor){0, 0, 0, 255});
	savingBtnTxt1.SetPosition(0, -20);
	savingBtnTxt3.SetPosition(0, +20);
	GuiImage savingBtnImg(&btnLargeOutline);
	GuiImage savingBtnImgOver(&btnLargeOutlineOver);
	GuiButton savingBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	savingBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	savingBtn.SetPosition(-50, 120);
	savingBtn.SetLabel(&savingBtnTxt1, 0);
	savingBtn.SetLabel(&savingBtnTxt2, 1);
	savingBtn.SetLabel(&savingBtnTxt3, 2);
	savingBtn.SetImage(&savingBtnImg);
	savingBtn.SetImageOver(&savingBtnImgOver);
	savingBtn.SetSoundOver(&btnSoundOver);
	savingBtn.SetTrigger(&trigA);
	savingBtn.SetEffectGrow();

	GuiText menuBtnTxt("Menu", 22, (GXColor){0, 0, 0, 255});
	menuBtnTxt.SetMaxWidth(btnLargeOutline.GetWidth()-30);
	GuiImage menuBtnImg(&btnLargeOutline);
	GuiImage menuBtnImgOver(&btnLargeOutlineOver);
	GuiButton menuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	menuBtn.SetPosition(-125, 250);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetTrigger(&trigA);
	menuBtn.SetEffectGrow();

	GuiText networkBtnTxt("Network", 22, (GXColor){0, 0, 0, 255});
	networkBtnTxt.SetMaxWidth(btnLargeOutline.GetWidth()-30);
	GuiImage networkBtnImg(&btnLargeOutline);
	GuiImage networkBtnImgOver(&btnLargeOutlineOver);
	GuiButton networkBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	networkBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	networkBtn.SetPosition(125, 250);
	networkBtn.SetLabel(&networkBtnTxt);
	networkBtn.SetImage(&networkBtnImg);
	networkBtn.SetImageOver(&networkBtnImgOver);
	networkBtn.SetSoundOver(&btnSoundOver);
	networkBtn.SetTrigger(&trigA);
	networkBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	GuiText resetBtnTxt("Reset Settings", 22, (GXColor){0, 0, 0, 255});
	GuiImage resetBtnImg(&btnOutline);
	GuiImage resetBtnImgOver(&btnOutlineOver);
	GuiButton resetBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	resetBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	resetBtn.SetPosition(-100, -35);
	resetBtn.SetLabel(&resetBtnTxt);
	resetBtn.SetImage(&resetBtnImg);
	resetBtn.SetImageOver(&resetBtnImgOver);
	resetBtn.SetSoundOver(&btnSoundOver);
	resetBtn.SetTrigger(&trigA);
	resetBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&fileBtn);
	w.Append(&videoBtn);
	w.Append(&savingBtn);
	w.Append(&menuBtn);

#ifdef HW_RVL
	w.Append(&networkBtn);
#endif

	w.Append(&exitBtn);
	w.Append(&resetBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(fileBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_BROWSE_DEVICE;
		}
		else if(videoBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(savingBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(networkBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
		else if(resetBtn.GetState() == STATE_CLICKED)
		{
			resetBtn.ResetState();

			int choice = WindowPrompt(
				"Reset Settings",
				"Are you sure that you want to reset your settings?",
				"Yes",
				"No",
				0, 0);
			if(choice == 1)
			{
				// reset settings
			}
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MenuSettingsFile
 ***************************************************************************/

static int MenuSettingsFile()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	OptionList options;
	sprintf(options.name[i++], "Mount Method");
	options.length = i;

	GuiText titleTxt("Settings", 28, (GXColor){255, 255, 255, 255});
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
			menu = MENU_SETTINGS;
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
			case MENU_SETTINGS_FILE:
				currentMenu = MenuSettingsFile();
				break;
			case MENU_BROWSE_DEVICE:
				currentMenu = MenuBrowseDevice();
				break;
			default: // unrecognized menu
				currentMenu = MenuSettings();
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
