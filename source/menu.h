/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#include <ogcsys.h>
#include "Controls/MainWindow.h"
#include "main.h"

#define THREAD_SLEEP    100
#define KBSIZE          1024.0
#define MBSIZE          1048576.0
#define GBSIZE          1073741824.0

enum
{
	MENU_EXIT = -1,
	MENU_NONE,
	MENU_SETTINGS,
	MENU_BROWSE_DEVICE,
	MENU_SMB_SETTINGS,
	MENU_FTPCLIENT_SETTINGS,
	MENU_FTPSERVER_SETTINGS,
	MENU_LANGUAGE_BROWSE,
	MENU_FTP,
	MENU_NETWORK_SETTINGS,
	MENU_GENERAL_SETTINGS,
	MENU_EXPLORER_SETTINGS,
	MENU_IMAGE_SETTINGS,
	MENU_SOUND_SETTINGS,
	MENU_BOOT_SETTINGS,
	MENU_FILE_EXTENSIONS,
	MENU_CONTROLS_SETTINGS,
	MENU_PATH_SETUP,
	MENU_COLOR_SETTINGS,
};

#endif
