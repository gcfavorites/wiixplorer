/***************************************************************************
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
 * MusicLoader.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "libwiigui/gui.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "menu.h"
#include "main.h"
#include "fileops.h"
#include "gettext.h"
#include "sys.h"

/*** Extern variables ***/
extern GuiSound *bgMusic;


/****************************************************************************
* LoadMusic
***************************************************************************/
void LoadMusic(const char *filepath)
{
    u8 *file = NULL;
    u64 filesize = 0;

	int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &file, &filesize);
	if (ret < 0)
		return;

    char *fileext = strrchr(filepath, '.');

    if(strcasecmp(fileext, ".ogg") == 0) {

        ///Stop and remove old music
        if(bgMusic) {
            delete bgMusic;
            bgMusic = NULL;
        }

        bgMusic = new GuiSound(file, filesize, SOUND_OGG);
        bgMusic->SetVolume(Settings.MusicVolume);
        bgMusic->SetLoop(1);
        bgMusic->Play(); /// startup music

        ///filebuffer is freed/deleted by the memclose of oggplayer.c

    } else if(strcasecmp(fileext, ".mp3") == 0) {

        ///Stop and remove old music
        if(bgMusic) {
            delete bgMusic;
            bgMusic = NULL;
        }

        bgMusic = new GuiSound(file, filesize, SOUND_MP3);
        bgMusic->SetVolume(Settings.MusicVolume);
        bgMusic->SetLoop(1);
        bgMusic->Play(); /// startup music

        ///free the filebuffer
        if(file) {
            free(file);
            file = NULL;
        }

    } else {
        //!This should never occur but oh well
        free(file);
        file = NULL;
        WindowPrompt(tr("This is no OGG/MP3 file"), 0, tr("OK"));
    }
}
