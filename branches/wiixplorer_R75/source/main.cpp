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
#include "libwiigui/gui.h"
#include "input.h"
#include "filelist.h"
#include "Settings.h"
#include "devicemounter.h"
#include "sys.h"
#include "mload/mload.h"
#include "mload/ehcmodule_elf.h"

Settings Settings;

int
main(int argc, char *argv[])
{
    IOS_ReloadIOS(202);
    if (mload_init() >= 0) {
        data_elf my_data_elf;
        mload_elf((void *) ehcmodule_elf, &my_data_elf);
        mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, 0x47);
    }

	PAD_Init();
	Wpad_Init();
	Sys_Init();
	InitVideo(); // Initialise video
	InitAudio(); // Initialize audio
	SDCard_Init(); // Initialize file system
	USBDevice_Init(); // Initialize file system

	Settings.Load();
	Settings.LoadLanguage(Settings.LanguagePath);
	LoadCustomFont(Settings.CustomFontPath);

	if(Settings.MountNTFS)
	{
        NTFS_Mount();
	}

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

	for(int i = 0; i < MAX_FONT_SIZE+1; i++)
	{
        // Initialize font system
        fontSystem[i] = NULL;
	}

	InitThreads();
	MainMenu(MENU_BROWSE_DEVICE);
}
