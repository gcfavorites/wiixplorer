#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>

#include "SoundOperations/gui_bgm.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "BootHomebrew/BootHomebrew.h"
#include "FileStartUp/FileStartUp.h"
#include "TextOperations/TextViewer.h"
#include "FileOperations/fileops.h"
#include "ImageOperations/ImageLoader.h"
#include "Controls/MainWindow.h"
#include "VideoOperations/WiiMovie.hpp"
#include "MPlayerPath.h"
#include "menu.h"

int FileStartUp(const char *filepath)
{
    char *fileext = strrchr(filepath, '.');
	char *filename = strrchr(filepath, '/')+1;

	if(!fileext)
        goto loadtext;      //!to avoid crash with strcasecmp & fileext == NULL

    if(strcasecmp(fileext, ".dol") == 0 || strcasecmp(fileext, ".elf") == 0) {
        int choice = WindowPrompt(tr("Do you want to boot:"), filename, tr("Yes"), tr("No"));
        if(choice)
        {
             int ret = LoadHomebrew(filepath);
             if(ret >= 0)
             {
                 AddBootArgument(filepath);
                 return BOOTHOMEBREW;
             }
        }

        return 0;
    }
    else if(strcasecmp(fileext, ".png") == 0 || strcasecmp(fileext, ".jpg") == 0
            || strcasecmp(fileext, ".jpeg") == 0 || strcasecmp(fileext, ".jpe") == 0
            || strcasecmp(fileext, ".jfif") == 0 || strcasecmp(fileext, ".bmp") == 0
			|| strcasecmp(fileext, ".gif") == 0 || strcasecmp(fileext, ".tga") == 0
			|| strcasecmp(fileext, ".tif") == 0 || strcasecmp(fileext, ".tiff") == 0
			|| strcasecmp(fileext, ".gd") == 0 || strcasecmp(fileext, ".gd2") == 0
			|| strcasecmp(fileext, ".tpl") == 0)
    {
		int choice = WindowPrompt(filename, tr("How do you want to open the file?"), tr("ImageViewer"), tr("ImageConverter"), tr("Cancel"));
		if (choice == 1)
		{
			ImageLoader(filepath);
		}
        else if(choice == 2)
        {
            ImageConverterLoader(filepath);
            return REFRESH_BROWSER;
        }
    }
    else if(strcasecmp(fileext, ".ogg") == 0 || strcasecmp(fileext, ".mp3") == 0
            || strcasecmp(fileext, ".pcm") == 0 || strcasecmp(fileext, ".wav") == 0
            || strcasecmp(fileext, ".aif") == 0)
    {
        loadMusic:

        int choice = WindowPrompt(filename, tr("Do you want to playback this file?"), tr("Yes"), tr("No"));
        if(choice)
        {
            bool result = GuiBGM::Instance()->Load(filepath, false);

            if(result)
                GuiBGM::Instance()->ParsePath(filepath);
        }
    }
    else if(strcasecmp(fileext, ".bin") == 0)
    {
        FILE * f = fopen(filepath, "rb");
        if(!f)
            return -1;

        u32 magic = 0;
        fread(&magic, 1, 4, f);
        if(magic == 'IMD5')
        {
            fseek(f, 0, SEEK_END);
            int filesize = ftell(f);
            u8 * buffer = (u8 *) malloc(filesize);
            rewind(f);
            fread(buffer, 1, filesize, f);
            fclose(f);
            magic = CheckIMD5Type(buffer, filesize);
            free(buffer);
        }
        fclose(f);

        if(magic == 'RIFF' || magic == 'FORM' || magic == 'BNS ')
        {
            goto loadMusic;
        }

        if(magic == 0x55AA382D /* U.8- */)
        {
            return ARCHIVE;
        }

        goto loadtext;
    }
    else if(strcasecmp(fileext, ".7z") == 0 || strcasecmp(fileext, ".zip") == 0 ||
            strcasecmp(fileext, ".rar") == 0 || strcasecmp(fileext, ".bnr") == 0 ||
            strcasecmp(fileext, ".arc") == 0)
    {
        return ARCHIVE;
    }
    else if(strcasecmp(fileext, ".lang") == 0)
    {
        int choice = WindowPrompt(tr("Do you want to load this language file?"), filename, tr("Yes"), tr("No"));
        if(choice)
        {
            Settings.LoadLanguage(filepath);
            return RELOAD_BROWSER;
        }
    }
    else if(strcasecmp(fileext, ".thp") == 0 || strcasecmp(fileext, ".mth") == 0)
    {
        WiiMovie * Video = new WiiMovie(filepath);
        Video->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        Video->SetVolume(Settings.MusicVolume);
        MainWindow::Instance()->Append(Video);
        Video->Play();
        delete Video;
        Video = NULL;
    }
    else if(strcasecmp(fileext, ".ttf") == 0)
    {
        int choice = WindowPrompt(tr("Do you want to change the font?"), filename, tr("Yes"), tr("No"));
        if(choice)
        {
            snprintf(Settings.CustomFontPath, sizeof(Settings.CustomFontPath), "%s", filepath);
            MainWindow::Instance()->HaltGui();
            ClearFontData();
            bool result = LoadCustomFont(Settings.CustomFontPath);
            MainWindow::Instance()->ResumeGui();
            if(result)
                WindowPrompt(tr("Fontpath changed."), tr("The new font is loaded."), tr("OK"));
            else
                WindowPrompt(tr("Fontpath changed."), tr("The new font could not be loaded."), tr("OK"));
        }
    }
    else if(strcasecmp(fileext, ".avi") == 0 || strcasecmp(fileext, ".mp4") == 0 ||
            strcasecmp(fileext, ".wmv") == 0 || strcasecmp(fileext, ".mpg") == 0 ||
            strcasecmp(fileext, ".mkv") == 0)
    {
        int choice = WindowPrompt(tr("Do you want to launch file with MPlayerCE?"), filename, tr("Yes"), tr("No"));
        if(choice)
        {
             int ret = LoadHomebrew(Settings.MPlayerPath);
             if(ret >= 0)
             {
                 AddBootArgument(Settings.MPlayerPath);

                 char mplayerpath[MAXPATHLEN];
                 ConvertToMPlayerPath(filepath, mplayerpath);

                 AddBootArgument(mplayerpath);
                 AddBootArgument(fmt("−quiet"));
                 return BOOTHOMEBREW;
             }
        }
    }
    else {
        loadtext:

        int choice = WindowPrompt(filename, tr("Do you want to open this file in TextViewer?"), tr("Yes"), tr("No"));
        if(choice)
            TextViewer(filepath);
    }

    return 0;
}
