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
 #include <unistd.h>
#include "Prompts/PromptWindows.h"
#include "SoundOperations/gui_bgm.h"
#include "ImageOperations/ImageWrite.h"
#include "Prompts/DeviceMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "network/networkops.h"
#include "network/ChangeLog.h"
#include "network/update.h"
#include "Explorer.h"
#include "SettingsMenu.h"
#include "menu_settings.h"

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
int MenuSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Language"));
	options.SetName(i++, tr("Clock Mode"));
	options.SetName(i++, tr("Explorer Settings"));
	options.SetName(i++, tr("Boot Settings"));
	options.SetName(i++, tr("Image Settings"));
	options.SetName(i++, tr("Sound Settings"));
	options.SetName(i++, tr("Network Settings"));
	options.SetName(i++, tr("Path Setup"));

	SettingsMenu * Menu = new SettingsMenu(tr("Settings"), &options, MENU_BROWSE_DEVICE);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

		switch (ret)
		{
			case 0:
				menu = MENU_LANGUAGE_BROWSE;
				break;
            case 1:
				Settings.ClockMode = (Settings.ClockMode+1) % 2;
				break;
			case 2:
                menu = MENU_EXPLORER_SETTINGS;
				break;
			case 3:
                menu = MENU_BOOT_SETTINGS;
				break;
            case 4:
                menu = MENU_IMAGE_SETTINGS;
				break;
            case 5:
                menu = MENU_SOUND_SETTINGS;
				break;
            case 6:
                menu = MENU_NETWORK_SETTINGS;
				break;
            case 7:
                menu = MENU_PATH_SETUP;
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

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

            if (Settings.ClockMode == 1)
                options.SetValue(i++, tr("12H"));
            else
                options.SetValue(i++, tr("24H"));

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");

            options.SetValue(i++, " ");
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}

int MenuExplorerSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Browser Mode"));
	options.SetName(i++, tr("Scrolling Speed"));
	options.SetName(i++, tr("PDF Processing Zoom"));

	SettingsMenu * Menu = new SettingsMenu(tr("Explorer Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

		switch (ret)
		{
            case 0:
				Settings.BrowserMode = (Settings.BrowserMode+1) % 2;
				break;
            case 1:
                Settings.ScrollSpeed = (Settings.ScrollSpeed+1) % 21;
                break;
            case 2:
                char entered[150];
                snprintf(entered, sizeof(entered), "%0.2f", Settings.PDFLoadZoom);
                if(OnScreenKeyboard(entered, 149))
                {
					Settings.PDFLoadZoom = atof(entered);
					WindowPrompt(tr("Warning:"), tr("This option could mess up the pdf view."), tr("OK"));
                }
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if (Settings.BrowserMode == ICONBROWSER)
                options.SetValue(i++, tr("Icon Mode"));
            else
                options.SetValue(i++, tr("List Mode"));

            options.SetValue(i++, "%i", Settings.ScrollSpeed);

            options.SetValue(i++, "%0.2f", Settings.PDFLoadZoom);
        }
	}

    delete Menu;

	return menu;
}

int MenuSoundSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	char entered[150];
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Music Volume"));
	options.SetName(i++, tr("Music Loop Mode"));
	options.SetName(i++, tr("Load Music to Memory"));
	options.SetName(i++, tr("Soundblocks"));
	options.SetName(i++, tr("Soundblock Size"));

	SettingsMenu * Menu = new SettingsMenu(tr("Sound Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

		switch (ret)
		{
            case 0:
				Settings.MusicVolume += 10;
				if(Settings.MusicVolume > 100)
                    Settings.MusicVolume = 0;
                GuiBGM::Instance()->SetVolume(Settings.MusicVolume);
				break;
            case 1:
                Settings.BGMLoopMode++;
                if(Settings.BGMLoopMode >= MAX_LOOP_MODES)
                    Settings.BGMLoopMode = 0;
                GuiBGM::Instance()->SetLoop(Settings.BGMLoopMode);
				break;
            case 2:
                Settings.LoadMusicToMem = (Settings.LoadMusicToMem+1) % 2;
				break;
            case 3:
                snprintf(entered, sizeof(entered), "%i", Settings.SoundblockCount);
                if(Settings.LoadMusicToMem != on && OnScreenKeyboard(entered, 149))
                {
					Settings.SoundblockCount = atoi(entered);
					WindowPrompt(tr("Warning:"), tr("The effect will take with next music load. It might break music playback."), tr("OK"));
					if(Settings.SoundblockSize*Settings.SoundblockCount > 512*1024)
                        WindowPrompt(tr("WARNING:"), tr("The buffer size is really high. If the app doesn't start after this delete your config files."), tr("OK"));
                }
				break;
            case 4:
                snprintf(entered, sizeof(entered), "%i", Settings.SoundblockSize);
                if(Settings.LoadMusicToMem != on && OnScreenKeyboard(entered, 149))
                {
					Settings.SoundblockSize = atoi(entered);
					WindowPrompt(tr("Warning:"), tr("The effect will take with next music load. It might break music playback."), tr("OK"));
					if(Settings.SoundblockSize*Settings.SoundblockCount > 512*1024)
                        WindowPrompt(tr("WARNING:"), tr("The buffer size is really high. If the app doesn't start after this delete your config files."), tr("OK"));
                }
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if(Settings.MusicVolume > 0)
                options.SetValue(i++, "%i", Settings.MusicVolume);
            else
                options.SetValue(i++, tr("OFF"));

            if(Settings.BGMLoopMode == ONCE) options.SetValue(i++,tr("Play Once"));
            else if (Settings.BGMLoopMode == LOOP) options.SetValue(i++,tr("Loop"));
            else if (Settings.BGMLoopMode == RANDOM_BGM) options.SetValue(i++,tr("Random"));
            else if (Settings.BGMLoopMode == DIR_LOOP) options.SetValue(i++,tr("Play Directory"));

            if(Settings.LoadMusicToMem == on) options.SetValue(i++, tr("ON"));
            else options.SetValue(i++, tr("OFF"));

            if(Settings.LoadMusicToMem == on) options.SetValue(i++, tr("Memory Buffer"));
            else options.SetValue(i++, "%i (%0.1f KB)", Settings.SoundblockCount, Settings.SoundblockCount*Settings.SoundblockSize/1024.0f);

            if(Settings.LoadMusicToMem == on) options.SetValue(i++, tr("Memory Buffer"));
            else options.SetValue(i++, "%i Bytes", Settings.SoundblockSize);
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}

int MenuImageSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Slideshow Delay"));
	options.SetName(i++, tr("Screenshot Format"));

	SettingsMenu * Menu = new SettingsMenu(tr("Image Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

		switch (ret)
		{
            case 0:
                Settings.SlideshowDelay += 5;
                if(Settings.SlideshowDelay > 60)
                    Settings.SlideshowDelay = 5;
				break;
            case 1:
                Settings.ScreenshotFormat = (Settings.ScreenshotFormat+1) % 6;
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

			options.SetValue(i++, "%i", Settings.SlideshowDelay);

			if(Settings.ScreenshotFormat == IMAGE_PNG)
                options.SetValue(i++, "PNG");
            else if(Settings.ScreenshotFormat == IMAGE_JPEG)
                options.SetValue(i++, "JPEG");
            else if(Settings.ScreenshotFormat == IMAGE_GIF)
                options.SetValue(i++, "GIF");
            else if(Settings.ScreenshotFormat == IMAGE_TIFF)
                options.SetValue(i++, "TIFF");
            else if(Settings.ScreenshotFormat == IMAGE_BMP)
                options.SetValue(i++, "BMP");
            else if(Settings.ScreenshotFormat == IMAGE_GD)
                options.SetValue(i++, "GD");
            else if(Settings.ScreenshotFormat == IMAGE_GD2)
                options.SetValue(i++, "GD2");
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}

int MenuBootSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Bootup Mount"));
	options.SetName(i++, tr("Mount NTFS"));

	SettingsMenu * Menu = new SettingsMenu(tr("Boot Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

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
				Settings.MountNTFS++;
				if(Settings.MountNTFS >= on_off_max)
                    Settings.MountNTFS = off;
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++,DeviceName[Settings.MountMethod]);

            if (Settings.MountNTFS == on) options.SetValue(i++,tr("ON"));
            else if (Settings.MountNTFS == off) options.SetValue(i++,tr("OFF"));
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}

int MenuPathSetup()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
    char entered[150];

	OptionList options;
	options.SetName(i++, tr("App Path"));
	options.SetName(i++, tr("MPlayerCE Path"));
	options.SetName(i++, tr("Customfont Path"));

	SettingsMenu * Menu = new SettingsMenu(tr("Path Setup"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

		switch (ret)
		{
            case 0:
                snprintf(entered, sizeof(entered), "%s", Settings.AppPath);
                if(OnScreenKeyboard(entered, 149))
                {
					if (entered[strlen(entered)-1] != '/')
						strcat(entered, "/");
					snprintf(Settings.AppPath, sizeof(Settings.AppPath), "%s", entered);
                    WindowPrompt(tr("AppPath changed"), 0, tr("OK"));
                }
				break;
            case 1:
                snprintf(entered, sizeof(entered), "%s", Settings.MPlayerPath);
                if(OnScreenKeyboard(entered, 149))
                {
					if (entered[strlen(entered)-1] != '/')
						strcat(entered, "/");
                    if(strstr(entered, "boot.dol") == 0)
						strcat(entered, "boot.dol");
					snprintf(Settings.MPlayerPath, sizeof(Settings.MPlayerPath), "%s", entered);
                    WindowPrompt(tr("MPlayerPath changed"), 0, tr("OK"));
                }
				break;
            case 2:
                snprintf(entered, sizeof(entered), "%s", Settings.CustomFontPath);
                if(OnScreenKeyboard(entered, 149))
                {
                    snprintf(Settings.CustomFontPath, sizeof(Settings.CustomFontPath), "%s", entered);
                    MainWindow::Instance()->HaltGui();
                    ClearFontData();
                    bool result = LoadCustomFont(Settings.CustomFontPath);
                    MainWindow::Instance()->ResumeGui();
                    if(result)
                        WindowPrompt(tr("Fontpath changed."), tr("The new font is loaded."), tr("OK"));
                    else
                        WindowPrompt(tr("Fontpath changed."), tr("The new font could not be loaded."), tr("OK"));
                }
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++, "%s", Settings.AppPath);

            options.SetValue(i++, "%s", Settings.MPlayerPath);

            options.SetValue(i++, "%s", Settings.CustomFontPath);
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}

/****************************************************************************
 * MenuNetworkSettings
 ***************************************************************************/
int MenuNetworkSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Auto Connect"));
	options.SetName(i++, tr("Update Meta.xml"));
	options.SetName(i++, tr("Update Icon.png"));
	options.SetName(i++, tr("Update (App) Path"));
	options.SetName(i++, tr("SMB Settings"));
	options.SetName(i++, tr("FTP Client Settings"));
	options.SetName(i++, tr("FTP Server Settings"));

	SettingsMenu * Menu = new SettingsMenu(tr("Network Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

    GuiImageData * btnOutline = Resources::GetImageData(button_png, button_png_size);
	GuiSound * btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    SimpleGuiTrigger trigA(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText updateBtnTxt(tr("Update App"), 22, (GXColor){0, 0, 0, 255});
	GuiImage updateBtnImg(btnOutline);
	GuiButton updateBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	updateBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	updateBtn.SetPosition(-50, -65);
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
                menu = MENU_FTPCLIENT_SETTINGS;
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

    delete Menu;

	HaltGui();
	MainWindow::Instance()->Remove(&updateBtn);
	ResumeGui();

	Resources::Remove(btnOutline);
	Resources::Remove(btnSoundOver);

    Settings.Save();

	return menu;
}

/****************************************************************************
 * MenuSMBSettings
 ***************************************************************************/
int MenuSMBSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("User:"));
	options.SetName(i++, tr("Host:"));
	options.SetName(i++, tr("Username:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("SMB Name:"));
	options.SetName(i++, tr("Reconnect SMB"));

	SettingsMenu * Menu = new SettingsMenu(tr("SMB Settings"), &options, MENU_NETWORK_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

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

    delete Menu;

    Settings.Save();

	return menu;
}

/****************************************************************************
 * MenuFTPClientSettings
 ***************************************************************************/
int MenuFTPClientSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("User:"));
	options.SetName(i++, tr("Host:"));
	options.SetName(i++, tr("Username:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("FTP Port:"));
	options.SetName(i++, tr("FTP Path:"));
	options.SetName(i++, tr("Passive Mode:"));
	options.SetName(i++, tr("Reconnect FTP"));

	SettingsMenu * Menu = new SettingsMenu(tr("FTP Client Settings"), &options, MENU_NETWORK_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

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

    delete Menu;

    Settings.Save();

	return menu;
}

/****************************************************************************
 * MenuFTPServerSettings
 ***************************************************************************/
int MenuFTPServerSettings()
{
	int menu = MENU_NONE;
	int ret, result = 0;
	int i = 0;
    char entered[150];
    bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Auto Start:"));
	options.SetName(i++, tr("Password:"));
	options.SetName(i++, tr("FTP Port:"));

	SettingsMenu * Menu = new SettingsMenu(tr("FTP Server Settings"), &options, MENU_NETWORK_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

		switch (ret)
		{
            case 0:
                Settings.FTPServer.AutoStart = (Settings.FTPServer.AutoStart+1) % 2;
                break;
            case 1:
                entered[0] = 0;
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    snprintf(Settings.FTPServer.Password, sizeof(Settings.FTPServer.Password), "%s", entered);
                }
                break;
            case 2:
                snprintf(entered, sizeof(entered), "%d", Settings.FTPServer.Port);
                result = OnScreenKeyboard(entered, 149);
                if(result) {
                    Settings.FTPServer.Port = (u16) atoi(entered);
                }
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if(Settings.FTPServer.AutoStart == 1)
                options.SetValue(i++, tr("ON"));
            else
                options.SetValue(i++, tr("OFF"));

			if (strcmp(Settings.FTPServer.Password, "") != 0)
				options.SetValue(i++,"********");
			else
				options.SetValue(i++," ");

            options.SetValue(i++,"%i", Settings.FTPServer.Port);
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}
