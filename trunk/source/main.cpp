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

#include "FreeTypeGX.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "Settings.h"
#include "fatmounter.h"
#include "sys.h"

Settings Settings;

int
main(int argc, char *argv[])
{
    IOS_ReloadIOS(202);

	PAD_Init();
	Wpad_Init();
	Sys_Init();
	InitVideo(); // Initialise video
	InitAudio(); // Initialize audio
	SDCard_Init(); // Initialize file system
	USBDevice_Init(); // Initialize file system

	Settings.Load();
	Settings.LoadLanguage(Settings.Language);

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);


	// Initialize font system
	InitFreeType((u8*)font_ttf, font_ttf_size);

	InitThreads();
	MainMenu(MENU_BROWSE_DEVICE);
}
