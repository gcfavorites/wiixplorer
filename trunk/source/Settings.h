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

#define MAXSMBUSERS         4

typedef struct _SMBData
{
	char	Host[50];
	char	User[50];
	char	Password[50];
	char	SMBName[50];
} SMBData;

enum
{
    off,
    on,
    on_off_max
};

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
        //!Overload to determine the BootDevice
        //!Also for later purpose for more argv parsing options
        bool Load(int argc, char *argv[]);
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

        //!Settings variables:

        //!BootDevice determines from which devices to load the config
        char	BootDevice[10];
        //!Current selected device to browse
        short	MountMethod;
        //!Current selected SMB user
        short	CurrentUser;
        //!Auto initialize network and connect to SMB on startup
        short	AutoConnect;
        //!Music Volume.
        short	MusicVolume;
        //!Option to mount or not mount the NTFS volumes
        short	MountNTFS;
        //!Languagefile path
        char	LanguagePath[150];
        //!Path to the customfont file.
        char	CustomFontPath[150];
        //!Path to where the app is located
        //!Default is sd:/apps/WiiExplorer/
        char	UpdatePath[150];
        //!SMB users informations
        SMBData SMBUser[MAXSMBUSERS];

    protected:
        void ParseLine(char *line);
        void TrimLine(char *dest, char *src, int size);
        FILE * file;
};

#endif
