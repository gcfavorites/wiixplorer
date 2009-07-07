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
#include "main.h"
#include "fatmounter.h"
#include "sys.h"
#include "wpad.h"

FreeTypeGX *fontSystem;
struct SSettings Settings;

void DefaultSettings()
{
	Settings.MountMethod = METHOD_SD;
	Settings.CurrentUser = 0;
	sprintf(Settings.SMBUser[0].Host, "192.168.178.3");
    sprintf(Settings.SMBUser[0].User, "Test");
    sprintf(Settings.SMBUser[0].Password, "Test");
    sprintf(Settings.SMBUser[0].SMBName, "d");

	sprintf(Settings.SMBUser[1].Host, "192.168.2.11");
    sprintf(Settings.SMBUser[1].User, "Test");
    sprintf(Settings.SMBUser[1].Password, "Test");
    sprintf(Settings.SMBUser[1].SMBName, "L");

	for(int i = 2; i < 4; i++) {
        sprintf(Settings.SMBUser[i].Host, "None");
        sprintf(Settings.SMBUser[i].User, "None");
        sprintf(Settings.SMBUser[i].Password, "None");
        sprintf(Settings.SMBUser[i].SMBName, "None");
	}
}

int
main(int argc, char *argv[])
{
	PAD_Init();
	Wpad_Init();
	Sys_Init();
	InitVideo(); // Initialise video
	InitAudio(); // Initialize audio
	SDCard_Init(); // Initialize file system
	USBDevice_Init(); // Initialize file system

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

	// Initialize font system
	fontSystem = new FreeTypeGX();
	fontSystem->loadFont(NULL, font_ttf, font_ttf_size, 0);
	fontSystem->setCompatibilityMode(FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE);

	InitGUIThreads();
	DefaultSettings();
	MainMenu(MENU_BROWSE_DEVICE);
}
