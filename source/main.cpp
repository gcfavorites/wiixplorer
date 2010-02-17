/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * main.cpp
 * Basic template/demonstration of libwiigui capabilities. For a
 * full-featured app using many more extensions, check out Snes9x GX.
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
#include "video.h"
#include "audio.h"
#include "libwiigui/gui.h"
#include "input.h"
#include "filelist.h"
#include "Settings.h"
#include "devicemounter.h"
#include "sys.h"
#include "memory/mem2.hpp"
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

    IOS_ReloadIOS(202);
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
