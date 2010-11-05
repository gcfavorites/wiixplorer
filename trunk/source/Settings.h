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
 * Settings.h
 *
 * Settings Class
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string>
#include <stdio.h>
#include <gctypes.h>
#include "Controls/AppControls.hpp"
#include "FileStartUp/FileExtensions.h"
#include "FTPOperations/ftp_devoptab.h"

#define MAXSMBUSERS         10
#define MAXFTPUSERS         MAX_FTP_MOUNTED

typedef struct _SMBData
{
	char	Host[50];
	char	User[50];
	char	Password[50];
	char	SMBName[50];
} SMBData;

typedef struct _FTPData
{
	char    Host[50];
	char    User[50];
	char    Password[50];
	char    FTPPath[50];
    unsigned short	Port;
	short	Passive;
} FTPData;

typedef struct _FTPServerData
{
    short   AutoStart;
	char    Password[50];
    unsigned short	Port;
} FTPServerData;

enum {
    APP_DEFAULT = 0,
    CONSOLE_DEFAULT,
    JAPANESE,
    ENGLISH,
    GERMAN,
    FRENCH,
    SPANISH,
    ITALIAN,
    DUTCH,
    S_CHINESE,
    T_CHINESE,
    KOREAN,
    MAX_LANGUAGE
};

class Settings
{
    public:
		//!Constructor
        Settings();
		//!Destructor
		~Settings();
		//!Set Default Settings
		void SetDefault();
		//!Load Settings
		bool Load();
        //!Find the config file in the default paths
        bool FindConfig();
		//!Save Settings
        bool Save();
		//!Reset Settings
        bool Reset();
		//!Set a Setting
		//!\param name Settingname
		//!\param value Settingvalue
        bool SetSetting(char * name, char * value);
		//!Load a languagefile
		//!\param language
        bool LoadLanguage(const char *path, int language = -1);
		//!Set the default ColorSettings
        void DefaultColors();

        /**************** Color Variables ************************/

        u32 BackgroundUL;
        u32 BackgroundUR;
        u32 BackgroundBR;
        u32 BackgroundBL;
        u32 ProgressUL;
        u32 ProgressUR;
        u32 ProgressBR;
        u32 ProgressBL;
        u32 ProgressEmptyUL;
        u32 ProgressEmptyUR;
        u32 ProgressEmptyBR;
        u32 ProgressEmptyBL;

        /**************** Settings Variables ************************/


        //!BootDevice determines from which devices to load the config
        char	BootDevice[10];
        //!Boot IOS
        short  	BootIOS;
        //!Current selected SMB user
        short  	CurrentSMBUser;
        //!Current selected FTP user
        short  	CurrentFTPUser;
        //!Music Volume.
        short  	MusicVolume;
        //!BGM Loop Mode
        short  	BGMLoopMode;
        //!AutoConnect Network on StartUp
        short  	AutoConnect;
        //!Update the meta.xml
        short  	UpdateMetaxml;
        //!Update the icon.png
        short  	UpdateIconpng;
        //!Clock mode
        short  	ClockMode;
        //!Scrolling speed
        short  	ScrollSpeed;
        //!Browser mode
        short  	BrowserMode;
        //!SoundblockCount
        short   SoundblockCount;
        //!SoundblockSize
        int     SoundblockSize;
        //!Screenshot image format
        short  	ScreenshotFormat;
        //!Load music to memory or play it from device
        short  	LoadMusicToMem;
		//!Slideshow time between images
		short   SlideshowDelay;
		//!Fade speed between images
		short   ImageFadeSpeed;
		//!Keyboard delete delay
		short   KeyboardDeleteDelay;
		//!Delete temporary path on exit
		short   DeleteTempPath;
		//!CopyThread priority
		short   CopyThreadPrio;
		//!CopyThread background priority
		short   CopyThreadBackPrio;
		//!Rumble of WiiMotes
		short   Rumble;
		//!Should system files be hidden or not
		short   HideSystemFiles;
		//!Should parition formatter be shown
		short   ShowFormatter;
		//!Zip compression level
		short   CompressionLevel;
		//!PDF processing zoom
		float   PDFLoadZoom;
        //!Path to MPlayerPath
        char	MPlayerPath[150];
        //!Path to WiiMC
        char	WiiMCPath[150];
        //!Languagefile path
        char	MusicPath[150];
        //!Languagefile path
        char	LanguagePath[150];
        //!Path to the customfont file.
        char	CustomFontPath[150];
        //!Temporary path for file extraction
        char	TempPath[150];
        //!Path to where the app is located
        //!Default is sd:/apps/WiiExplorer/
        char	UpdatePath[150];
        //!Path to where the homebrew apps are
        char	AppPath[150];
        //!Path to where the screenshots are written
        char	ScreenshotPath[150];
        //!Path to where the url list in xml form is
        char	LinkListPath[150];
        //!Last used path to which the app returns to
        std::string LastUsedPath;
		//!SMB users informations
        SMBData SMBUser[MAXSMBUSERS];
        //!FTP users informations
        FTPData FTPUser[MAXFTPUSERS];
		//!FTP server informations
		FTPServerData FTPServer;
		//!Internal WiiXplorer FileExtensions
		FilesExtensions FileExtensions;
		//!The controls of the App
        AppControls Controls;
        //!Path to the configuration file
        //!only for internal use
        char    ConfigPath[50];
    protected:

        void ParseLine(char *line);
        void TrimLine(char *dest, char *src, int size);
        FILE * file;
};

#endif
