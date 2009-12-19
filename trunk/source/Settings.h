/****************************************************************************
 * Settings Class
 * by dimok
 ***************************************************************************/
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#define MAXSMBUSERS         4

typedef struct _SMBData {
	char	Host[50];
	char	User[50];
	char	Password[50];
	char	SMBName[50];
} SMBData;

enum {
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
        //!Default is sd:/apps/WiiXplorer/
        char	UpdatePath[150];
        //!SMB users informations
        SMBData SMBUser[MAXSMBUSERS];

    protected:
        void ParseLine(char *line);
        void TrimLine(char *dest, char *src, int size);
        FILE * file;
};

#endif
