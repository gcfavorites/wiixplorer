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
 * Settings.cpp
 *
 * Settings Class
 * for WiiXplorer 2009
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Settings.h"
#include "FileOperations/fileops.h"
#include "Language/gettext.h"

#define DEFAULT_APP_PATH    "apps/WiiExplorer/"
#define CONFIGPATH          "config/WiiXplorer/"
#define CONFIGNAME          "WiiXplorer.cfg"
#define LANGPATH      	    "config/WiiXplorer/Languages/"

Settings::Settings()
{
    strcpy(BootDevice, "sd:/");
    snprintf(ConfigPath, sizeof(ConfigPath), "%s%s%s", BootDevice, CONFIGPATH, CONFIGNAME);
    this->SetDefault();
}

Settings::~Settings()
{
}

void Settings::SetDefault()
{
    MountMethod = 0;
    MountNTFS = 0;
    MusicVolume = 80;
    CurrentSMBUser = 0;
	CurrentFTPUser = 0;
    BGMLoopMode = 1;
    AutoConnect = 0;
    UpdateMetaxml = 1;
    UpdateIconpng = 1;
    sprintf(CustomFontPath, "%s%sfont.ttf", BootDevice, CONFIGPATH);
    sprintf(LanguagePath, "%s%s", BootDevice, LANGPATH);
    sprintf(UpdatePath, "%s%s", BootDevice, DEFAULT_APP_PATH);
    strcpy(MusicPath, "");

    for(int i = 0; i < MAXSMBUSERS; i++) {
        strcpy(SMBUser[i].Host, "");
        strcpy(SMBUser[i].User, "");
        strcpy(SMBUser[i].Password, "");
        strcpy(SMBUser[i].SMBName, "");
	}

	for(int i = 0; i < MAXFTPUSERS; i++) {
        strcpy(FTPUser[i].Host, "");
        strcpy(FTPUser[i].User, "");
        strcpy(FTPUser[i].Password, "");
        strcpy(FTPUser[i].FTPPath, "/");
		FTPUser[i].Passive = 0;
	}
}

bool Settings::Save()
{
    char filepath[100];
    char filedest[100];

    snprintf(filepath, sizeof(filepath), "%s", ConfigPath);
    snprintf(filedest, sizeof(filedest), "%s", ConfigPath);
    char * tmppath = strrchr(filedest, '/');
    if(tmppath)
    {
        tmppath++;
        tmppath[0] = '\0';
    }

    if(!CreateSubfolder(filedest))
    {
        //!Try the other device and standard path
        if(strcmp(BootDevice, "usb:/") == 0)
            strcpy(BootDevice, "sd:/");
        else
            strcpy(BootDevice, "usb:/");

        snprintf(filepath, sizeof(filepath), "%s%s%s", BootDevice, CONFIGPATH, CONFIGNAME);
        snprintf(filedest, sizeof(filedest), "%s%s", BootDevice, CONFIGPATH);

        if(!CreateSubfolder(filedest))
            return false;
    }

    file = fopen(filepath, "w");
    if(!file)
    {
        fclose(file);
        return false;
    }

    fprintf(file, "# WiiXplorer Settingsfile\n");
	fprintf(file, "# Note: This file is automatically generated\n\n");
	fprintf(file, "# Main Settings\n\n");
	fprintf(file, "MountMethod = %d\n", MountMethod);
	fprintf(file, "CurrentSMBUser = %d\n", CurrentSMBUser);
	fprintf(file, "CurrentFTPUser = %d\n", CurrentFTPUser);
	fprintf(file, "LanguagePath = %s\n", LanguagePath);
	fprintf(file, "MusicVolume = %d\n", MusicVolume);
	fprintf(file, "MountNTFS = %d\n", MountNTFS);
	fprintf(file, "BGMLoopMode = %d\n", BGMLoopMode);
	fprintf(file, "AutoConnect = %d\n", AutoConnect);
	fprintf(file, "UpdateMetaxml = %d\n", UpdateMetaxml);
	fprintf(file, "UpdateIconpng = %d\n", UpdateIconpng);
	fprintf(file, "MusicPath = %s\n", MusicPath);
	fprintf(file, "CustomFontPath = %s\n", CustomFontPath);
	fprintf(file, "UpdatePath = %s\n", UpdatePath);

	fprintf(file, "\n# SMB Setup Information\n\n");
    for(int i = 0; i < MAXSMBUSERS; i++) {
        fprintf(file, "SMBUser[%d].Host = %s\n", i+1, SMBUser[i].Host);
        fprintf(file, "SMBUser[%d].User = %s\n", i+1, SMBUser[i].User);
        fprintf(file, "SMBUser[%d].Password = %s\n", i+1, SMBUser[i].Password);
        fprintf(file, "SMBUser[%d].SMBName = %s\n\n", i+1, SMBUser[i].SMBName);
	}

	fprintf(file, "\n# FTP Setup Information\n\n");
    for(int i = 0; i < MAXFTPUSERS; i++) {
        fprintf(file, "FTPUser[%d].Host = %s\n", i+1, FTPUser[i].Host);
        fprintf(file, "FTPUser[%d].User = %s\n", i+1, FTPUser[i].User);
        fprintf(file, "FTPUser[%d].Password = %s\n", i+1, FTPUser[i].Password);
        fprintf(file, "FTPUser[%d].FTPPath = %s\n", i+1, FTPUser[i].FTPPath);
		fprintf(file, "FTPUser[%d].Passive = %d\n\n", i+1, FTPUser[i].Passive);
	}

	fclose(file);

	return true;
}

bool Settings::Load(int argc, char *argv[])
{
    char testpath[100];
    bool found = false;

    //! Try first standard SD Path
    snprintf(ConfigPath, sizeof(ConfigPath), "sd:/%s%s", CONFIGPATH, CONFIGNAME);
    found = CheckFile(ConfigPath);
    if(!found)
    {
        //! Try standard USB Path
        snprintf(testpath, sizeof(testpath), "usb:/%s%s", CONFIGPATH, CONFIGNAME);
        found = CheckFile(testpath);
        if(found)
        {
            snprintf(ConfigPath, sizeof(ConfigPath), "%s", testpath);
            strcpy(BootDevice, "usb:/");
            found = true;
        }
    }
    if(!found)
    {
        //! Try alternative SD Path
        snprintf(testpath, sizeof(testpath), "sd:/apps/WiiXplorer/%s", CONFIGNAME);
        found = CheckFile(testpath);
        if(found)
        {
            snprintf(ConfigPath, sizeof(ConfigPath), "%s", testpath);
            strcpy(BootDevice, "sd:/");
            found = true;
        }
    }
    if(!found)
    {
        //! Try alternative SD Path
        snprintf(testpath, sizeof(testpath), "sd:/apps/WiiExplorer/%s", CONFIGNAME);
        found = CheckFile(testpath);
        if(found)
        {
            snprintf(ConfigPath, sizeof(ConfigPath), "%s", testpath);
            strcpy(BootDevice, "sd:/");
            found = true;
        }
    }
    if(!found)
    {
        //! Try alternative USB Path
        snprintf(testpath, sizeof(testpath), "usb:/apps/WiiXplorer/%s", CONFIGNAME);
        found = CheckFile(testpath);
        if(found)
        {
            snprintf(ConfigPath, sizeof(ConfigPath), "%s", testpath);
            strcpy(BootDevice, "usb:/");
            found = true;
        }
    }
    if(!found)
    {
        //! Try alternative USB Path
        snprintf(testpath, sizeof(testpath), "usb:/apps/WiiExplorer/%s", CONFIGNAME);
        found = CheckFile(testpath);
        if(found)
        {
            snprintf(ConfigPath, sizeof(ConfigPath), "%s", testpath);
            strcpy(BootDevice, "usb:/");
            found = true;
        }
    }
    if(!found)
    {
        //! If all failed check argv and set standard path
        if (argc >= 1)
        {
            if (!strncasecmp(argv[0], "usb:/", 5))
            {
                strcpy(BootDevice, "usb:/");
                snprintf(ConfigPath, sizeof(ConfigPath), "usb:/%s%s", CONFIGPATH, CONFIGNAME);
                found = true;
            }
            else
            {
                strcpy(BootDevice, "sd:/");
                snprintf(ConfigPath, sizeof(ConfigPath), "sd:/%s%s", CONFIGPATH, CONFIGNAME);
                found = true;
            }
        }
    }

    SetDefault();

    return Load();
}

bool Settings::Load()
{
	char line[300];
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s", ConfigPath);

    if(!CheckFile(filepath))
        return false;

	file = fopen(filepath, "r");
	if (!file)
	{
        fclose(file);
        return false;
	}

	while (fgets(line, sizeof(line), file)) {

		if (line[0] == '#') continue;

        this->ParseLine(line);
	}
	fclose(file);

	return true;

}

bool Settings::LoadLanguage(const char *path, int language)
{
    bool ret = false;

    if(language >= 0 || !path) {

        if(language < 0)
            return false;

        char filepath[150];
        char langpath[150];
        snprintf(langpath, sizeof(langpath), "%s", LanguagePath);
        if(langpath[strlen(langpath)-1] != '/')
        {
            char * ptr = strrchr(langpath, '/');
            if(ptr)
            {
                ptr++;
                ptr[0] = '\0';
            }
        }

        if(language == APP_DEFAULT)
        {
            strcpy(LanguagePath, langpath);
            gettextCleanUp();
            return true;
        }
        else if(language == CONSOLE_DEFAULT)
        {
            return LoadLanguage(NULL, CONF_GetLanguage()+2);
        }
        else if(language == JAPANESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/japanese.lang", langpath);
        }
        else if(language == ENGLISH)
        {
            snprintf(filepath, sizeof(filepath), "%s/english.lang", langpath);
        }
        else if(language == GERMAN)
        {
            snprintf(filepath, sizeof(filepath), "%s/german.lang", langpath);
        }
        else if(language == FRENCH)
        {
            snprintf(filepath, sizeof(filepath), "%s/french.lang", langpath);
        }
        else if(language == SPANISH)
        {
            snprintf(filepath, sizeof(filepath), "%s/spanish.lang", langpath);
        }
        else if(language == ITALIAN)
        {
            snprintf(filepath, sizeof(filepath), "%s/italian.lang", langpath);
        }
        else if(language == DUTCH)
        {
            snprintf(filepath, sizeof(filepath), "%s/dutch.lang", langpath);
        }
        else if(language == S_CHINESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/s_chinese.lang", langpath);
        }
        else if(language == T_CHINESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/t_chinese.lang", langpath);
        }
        else if(language == KOREAN)
        {
            snprintf(filepath, sizeof(filepath), "%s%s/korean.lang", BootDevice, langpath);
        }

        ret = gettextLoadLanguage(filepath);
        if(ret)
            strncpy(LanguagePath, filepath, sizeof(LanguagePath));

    } else {

        ret = gettextLoadLanguage(path);
        if(ret)
            strncpy(LanguagePath, path, sizeof(LanguagePath));
    }

    return ret;
}

bool Settings::Reset()
{
    this->SetDefault();

    if(this->Save())
        return true;

	return false;
}

bool Settings::SetSetting(char *name, char *value)
{
    int i = 0;

    if (strcmp(name, "MountMethod") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			MountMethod = i;
		}
		return true;
	}
	else if (strcmp(name, "CurrentSMBUser") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			CurrentSMBUser = i;
		}
		return true;
	}
	else if (strcmp(name, "CurrentFTPUser") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			CurrentFTPUser = i;
		}
		return true;
	}
	else if (strcmp(name, "LanguagePath") == 0) {
        strncpy(LanguagePath, value, sizeof(LanguagePath));
		return true;
	}
	else if (strcmp(name, "MusicVolume") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			MusicVolume = i;
		}
		return true;
	}
	else if (strcmp(name, "MountNTFS") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			MountNTFS = i;
		}
		return true;
	}
	else if (strcmp(name, "BGMLoopMode") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			BGMLoopMode = i;
		}
		return true;
	}
	else if (strcmp(name, "AutoConnect") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			AutoConnect = i;
		}
		return true;
	}
	else if (strcmp(name, "UpdateMetaxml") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			UpdateMetaxml = i;
		}
		return true;
	}
	else if (strcmp(name, "UpdateIconpng") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			UpdateIconpng = i;
		}
		return true;
	}
	else if (strcmp(name, "CustomFontPath") == 0) {
        strncpy(CustomFontPath, value, sizeof(CustomFontPath));
		return true;
	}
	else if (strcmp(name, "MusicPath") == 0) {
        strncpy(MusicPath, value, sizeof(MusicPath));
		return true;
	}
	else if (strcmp(name, "UpdatePath") == 0) {
        strncpy(UpdatePath, value, sizeof(UpdatePath));
		return true;
	}
	else {
	    char temp[80];
	    int n = 0;

	    for(n = 0; n < MAXSMBUSERS; n++) {
	        sprintf(temp, "SMBUser[%d].Host", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(SMBUser[n].Host, value, sizeof(SMBUser[n].Host));
                return true;
            }
            sprintf(temp, "SMBUser[%d].User", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(SMBUser[n].User, value, sizeof(SMBUser[n].User));
                return true;
            }
            sprintf(temp, "SMBUser[%d].Password", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(SMBUser[n].Password, value, sizeof(SMBUser[n].Password));
                return true;
            }
            sprintf(temp, "SMBUser[%d].SMBName", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(SMBUser[n].SMBName, value, sizeof(SMBUser[n].SMBName));
                return true;
            }
	    }

	    for(n = 0; n < MAXFTPUSERS; n++) {
	        sprintf(temp, "FTPUser[%d].Host", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(FTPUser[n].Host, value, sizeof(FTPUser[n].Host));
                return true;
            }
            sprintf(temp, "FTPUser[%d].User", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(FTPUser[n].User, value, sizeof(FTPUser[n].User));
                return true;
            }
            sprintf(temp, "FTPUser[%d].Password", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(FTPUser[n].Password, value, sizeof(FTPUser[n].Password));
                return true;
            }
            sprintf(temp, "FTPUser[%d].FTPPath", n+1);
            if (stricmp(name, temp) == 0) {
                strncpy(FTPUser[n].FTPPath, value, sizeof(FTPUser[n].FTPPath));
                return true;
            }
            sprintf(temp, "FTPUser[%d].Passive", n+1);
            if (stricmp(name, temp) == 0) {
				if (sscanf(value, "%d", &i) == 1) {
					FTPUser[n].Passive = i;
				}
                return true;
            }
		}
	}

    return false;
}

void Settings::ParseLine(char *line)
{
    char temp[200], name[200], value[200];

    strncpy(temp, line, sizeof(temp));

    char * eq = strchr(temp, '=');

    if(!eq) return;

    *eq = 0;

    this->TrimLine(name, temp, sizeof(name));
    this->TrimLine(value, eq+1, sizeof(value));

	this->SetSetting(name, value);
}

void Settings::TrimLine(char *dest, char *src, int size)
{
	int len;
	while (*src == ' ') src++;
	len = strlen(src);
	while (len > 0 && strchr(" \r\n", src[len-1])) len--;
	if (len >= size) len = size-1;
	strncpy(dest, src, len);
	dest[len] = 0;
}
