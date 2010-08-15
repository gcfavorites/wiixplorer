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
#include "BootHomebrew/BootHomebrew.h"
#include "TextOperations/FontSystem.h"
#include "audio.h"
#include "libwiigui/gui.h"
#include "input.h"
#include "filelist.h"
#include "Settings.h"
#include "devicemounter.h"
#include "sys.h"
#include "Memory/mem2.h"
#include "mload/mload_init.h"

extern "C"
{
    void SetupPDFFontPath(const char * path);
}

bool boothomebrew = false;
Settings Settings;

int main(int argc, char *argv[])
{
	MEM2_init(52); // Initialize 52 MB (max is 53469152 bytes though)
    InitGecko();
    __exception_setreload(30);

	SDCard_Init(); // Initialize file system
	USBDevice_Init(); // Initialize file system
	Settings.Load(argc, argv);

    u8 EntraceIOS = (u8) IOS_GetVersion();

    if(EntraceIOS != Settings.BootIOS || (Settings.BootIOS == 58 && __di_check_ahbprot() != 1))
    {
        SDCard_deInit();
        USBDevice_deInit();

        if(IOS_ReloadIOS(Settings.BootIOS) >= 0 && IOS_GetVersion() >= 202)
            mload_Init();

        SDCard_Init();
        USBDevice_Init();
    }

	Sys_Init();
	InitVideo(); // Initialise video
	SetupPads(); // Initialize input
	InitAudio(); // Initialize audio
	SDGeckoA_Init(); // Initialize file system
	SDGeckoB_Init(); // Initialize file system
    DiskDrive_Init(); //Init DVD Driver
	Settings.LoadLanguage(Settings.LanguagePath);
	SetupPDFFontPath(Settings.UpdatePath);
	SetupDefaultFont(Settings.CustomFontPath);

	if(Settings.MountNTFS)
        NTFS_Mount();

	setlocale(LC_CTYPE, "C-UTF-8");
	setlocale(LC_MESSAGES, "C-UTF-8");

	MainWindow::Instance()->Show();

    if(boothomebrew)
    {
        if(EntraceIOS != IOS_GetVersion())
            IOS_ReloadIOS(EntraceIOS);

        BootHomebrew();
    }

	/* Return to the Wii system menu  if not from HBC*/
    if(!IsFromHBC())
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

	return 0;
}
