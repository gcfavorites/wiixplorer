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
#include "Controls/MainWindow.h"
#include "Controls/Resources.h"
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

extern "C"
{
    extern void __exception_setreload(int t);
}

Settings Settings;

int
main(int argc, char *argv[])
{
    IOS_ReloadIOS(202);
    if (mload_init() >= 0)
    {
        data_elf my_data_elf;
        mload_elf((void *) ehcmodule_elf, &my_data_elf);
        mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, 0x47);
    }

	Sys_Init();
	InitVideo(); // Initialise video
	SetupPads(); // Initialize input
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

	for(int i = 0; i < MAX_FONT_SIZE+1; i++)
	{
        // Initialize font system
        fontSystem[i] = NULL;
	}

	MainWindow::Instance()->Show();

	return 0;
}
