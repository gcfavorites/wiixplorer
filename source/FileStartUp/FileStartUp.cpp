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
#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include "SoundOperations/MusicPlayer.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "BootHomebrew/BootHomebrew.h"
#include "FileStartUp/FileStartUp.h"
#include "TextOperations/TextViewer.h"
#include "TextOperations/PDFLoader.h"
#include "TextOperations/FontSystem.h"
#include "FileOperations/fileops.h"
#include "ImageOperations/ImageLoader.h"
#include "Controls/MainWindow.h"
#include "Controls/IOHandler.hpp"
#include "VideoOperations/WiiMovie.hpp"
#include "FileExtensions.h"
#include "MPlayerPath.h"
#include "menu.h"

int FileStartUp(const char *filepath)
{
    if(!filepath)
        return -1;

    char *fileext = strrchr(filepath, '.');
	char *filename = strrchr(filepath, '/')+1;

    if(Settings.FileExtensions.CompareHomebrew(fileext) == 0)
    {
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
    else if(Settings.FileExtensions.ComparePDF(fileext) == 0)
    {
		int choice = WindowPrompt(filename, tr("Open the file in the PDF Viewer?"), tr("Yes"), tr("Cancel"));
		if (choice == 1)
		{
			PDFLoader(filepath);
		}
    }
    else if(Settings.FileExtensions.CompareImage(fileext) == 0)
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
    else if(Settings.FileExtensions.CompareAudio(fileext) == 0)
    {
        loadMusic:

        int choice = WindowPrompt(filename, tr("Do you want to playback this file?"), tr("Yes"), tr("Add to Playlist"), tr("Parse Directory"), tr("Cancel"));
        if(choice == 1)
        {
            MusicPlayer::Instance()->ClearList();
            MusicPlayer::Instance()->AddEntrie(filepath);
            if(!MusicPlayer::Instance()->Play(0))
                WindowPrompt(tr("Could not load file."), tr("Possible unsupported format."), tr("OK"));
        }
        else if(choice == 2)
        {
            MusicPlayer::Instance()->AddEntrie(filepath);
        }
        else if(choice == 3)
        {
            bool result = MusicPlayer::Instance()->ParsePath(filepath);
            if(!result)
                ShowError(tr("Could not parse directory."));
        }
    }
    else if(Settings.FileExtensions.CompareWiiBinary(fileext) == 0)
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

        int choice = WindowPrompt(filename, tr("Uknown format. Open this file in the TextEditor?"), tr("Yes"), tr("No"));
        if(choice)
            TextViewer(filepath);
    }
    else if(Settings.FileExtensions.CompareArchive(fileext) == 0)
    {
        return ARCHIVE;
    }
    else if(Settings.FileExtensions.CompareLanguageFiles(fileext) == 0)
    {
        int choice = WindowPrompt(tr("Do you want to load this language file?"), filename, tr("Yes"), tr("No"));
        if(choice)
        {
            Settings.LoadLanguage(filepath);
            return RELOAD_BROWSER;
        }
    }
    else if(Settings.FileExtensions.CompareFont(fileext) == 0)
    {
        int choice = WindowPrompt(tr("Do you want to change the font?"), filename, tr("Yes"), tr("No"));
        if(choice)
        {
            snprintf(Settings.CustomFontPath, sizeof(Settings.CustomFontPath), "%s", filepath);
            MainWindow::Instance()->HaltGui();
            ClearFontData();
            bool result = SetupDefaultFont(Settings.CustomFontPath);
            MainWindow::Instance()->ResumeGui();
            if(result)
                WindowPrompt(tr("Fontpath changed."), tr("The new font is loaded."), tr("OK"));
            else
                WindowPrompt(tr("Fontpath changed."), tr("The new font could not be loaded."), tr("OK"));
        }
    }
    //! Those have to be made extra and put before MPlayerCE launch
    //! to launch them inside WiiXplorer.
    else if(Settings.FileExtensions.CompareWiiXplorerMovies(fileext) == 0)
    {
		if(IOHandler::Instance()->IsRunning())
		{
			int choice = WindowPrompt(tr("Currently a process is running."), tr("Running a Video could slowdown the process or freeze the app. Do you want to continue?"), tr("Yes"), tr("Cancel"));
			if(choice == 0)
			    return 0;
		}
        WiiMovie * Video = new WiiMovie(filepath);
        Video->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        Video->SetVolume(Settings.MusicVolume);
        MainWindow::Instance()->Append(Video);
        Video->Play();
        delete Video;
        Video = NULL;
    }
    else if(Settings.FileExtensions.CompareVideo(fileext) == 0)
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
    else
    {
        int choice = WindowPrompt(filename, tr("Do you want to open this file in TextEditor?"), tr("Yes"), tr("No"));
        if(choice)
            TextViewer(filepath);
    }

    return 0;
}
