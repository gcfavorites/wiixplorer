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
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <locale.h>

#include "FreeTypeGX.h"
#include "Controls/MainWindow.h"
#include "VideoOperations/video.h"
#include "audio.h"
#include "libwiigui/gui.h"
#include "input.h"
#include "filelist.h"
#include "Settings.h"
#include "devicemounter.h"
#include "sys.h"
#include "Memory/mem2.hpp"
#include "mload/mload_init.h"

extern "C"
{
    extern void __exception_setreload(int t);
}

Settings Settings;

int main(int argc, char *argv[])
{
	MEM2_init(48); // Initialize 48 MB
	MEM2_takeBigOnes(true);
    InitGecko();

    __exception_setreload(20);

    if(IOS_ReloadIOS(202) >= 0)
        mload_Init();

    //for later purpose
    //LWP_SetThreadPriority(LWP_GetSelf(), 60);

	Sys_Init();
	InitVideo(); // Initialise video
	SetupPads(); // Initialize input
	InitAudio(); // Initialize audio
	SDCard_Init(); // Initialize file system
	USBDevice_Init(); // Initialize file system
    DiskDrive_Init(false); //Init DVD Driver

	Settings.Load(argc, argv);
	Settings.LoadLanguage(Settings.LanguagePath);
	LoadCustomFont(Settings.CustomFontPath);

	if(Settings.MountNTFS)
	{
        NTFS_Mount();
	}

	for(int i = 0; i < MAX_FONT_SIZE+1; i++)
	{
        // Initialize font system
        fontSystem[i] = NULL;
	}

	setlocale(LC_CTYPE, "C-UTF-8");
	setlocale(LC_MESSAGES, "C-UTF-8");

	MainWindow::Instance()->Show();

	/* Return to the Wii system menu  if not from HBC*/
    if(!IsFromHBC())
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

	return 0;
}
