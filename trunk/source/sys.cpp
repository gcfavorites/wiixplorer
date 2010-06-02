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
#include <stdio.h>
#include <ogcsys.h>
#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>

#include "network/networkops.h"
#include "Prompts/PromptWindows.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Controls/MainWindow.h"
#include "Controls/Clipboard.h"
#include "Controls/Taskbar.h"
#include "libftp/FTPServer.h"
#include "Memory/mem2.hpp"
#include "VideoOperations/video.h"
#include "mload/mload_init.h"
#include "audio.h"
#include "main.h"
#include "menu.h"
#include "devicemounter.h"
#include "sys.h"

bool shutdown = false;
bool reset = false;

extern bool boothomebrew;

extern "C" bool RebootApp()
{
    char filepath[MAXPATHLEN];
    snprintf(filepath, sizeof(filepath), "%s/boot.dol", Settings.UpdatePath);
    int ret = LoadHomebrew(filepath);
    if(ret < 0)
    {
         WindowPrompt(tr("Reboot failed"), tr("Can't load file"), tr("OK"));
         return false;
    }

    AddBootArgument(filepath);

    boothomebrew = true;

    Taskbar::Instance()->SetMenu(MENU_EXIT);

    return true;
}

extern "C" void ExitApp()
{
    Settings.Save();
	Clipboard::DestroyInstance();
	MainWindow::DestroyInstance();
	StopGX();
	ShutdownAudio();
	ClearFontData();
	Resources::DestroyInstance();
	FTPServer::DestroyInstance();
	CloseFTP();
	CloseSMBShare();
    NTFS_UnMount();
    SDCard_deInit();
    USBDevice_deInit();
    DiskDrive_deInit();
	DeInit_Network();
	MEM2_cleanup();
    mload_DeInit();

	WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();
}

extern "C" void __Sys_ResetCallback(void)
{
	reset = true;
}

extern "C" void __Sys_PowerCallback(void)
{
	shutdown = true;
}


extern "C" void Sys_Init(void)
{
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

extern "C" void Sys_Reboot(void)
{
	ExitApp();
	STM_RebootSystem();
}

#define ShutdownToDefault	0
#define ShutdownToIdle		1
#define ShutdownToStandby	2

static void _Sys_Shutdown(int SHUTDOWN_MODE)
{
	ExitApp();

	if((CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE &&  SHUTDOWN_MODE != ShutdownToStandby) || SHUTDOWN_MODE == ShutdownToIdle) {
		s32 ret;

		ret = CONF_GetIdleLedMode();
		if(ret >= 0 && ret <= 2)
			STM_SetLedMode(ret);

		STM_ShutdownToIdle();
	} else {
		STM_ShutdownToStandby();
	}
}

extern "C" void Sys_Shutdown(void)
{
	_Sys_Shutdown(ShutdownToDefault);
}

extern "C" void Sys_ShutdownToIdle(void)
{
	_Sys_Shutdown(ShutdownToIdle);
}

extern "C" void Sys_ShutdownToStandby(void)
{
	_Sys_Shutdown(ShutdownToStandby);
}

extern "C" void Sys_LoadMenu(void)
{
	ExitApp();
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

extern "C" void Sys_BackToLoader(void)
{
	if (IsFromHBC())
	{
		ExitApp();
		exit(0);
	}
	// Channel Version
	Sys_LoadMenu();
}

extern "C" bool IsFromHBC()
{
    if(!(*((u32*) 0x80001800)))
        return false;

    char * signature = (char *) 0x80001804;
    if(strncmp(signature, "STUBHAXX", 8) == 0)
    {
        return true;
    }

    return false;
}

extern "C" void LoadCIOS()
{
    int IOS_Rev = GetIOS_Rev(202);

    if(IOS_Rev > 1 && IOS_Rev < 7)
    {
        int ret = IOS_ReloadIOS(202);
        if(ret >= 0)
            mload_Init();
    }
}

extern "C" int GetIOS_Rev(u32 ios)
{
	u32 num_titles = 0, i = 0;
	u64 tid = 0;
	u64 * titles = NULL;
	s32 ret = 0;

	ret = ES_GetNumTitles(&num_titles);
	if(ret < 0)
		return -1;

	if(num_titles < 1)
        return -1;

	titles = (u64 *) memalign(32, num_titles * sizeof(u64) + 32);
	if(!titles)
	    return -1;

	ret = ES_GetTitles(titles, num_titles);
	if(ret < 0)
	{
		free(titles);
		return -1;
	}

	for(i=0; i < num_titles; i++)
	{
		if ((titles[i] & 0xFFFFFFFF) == ios)
		{
			tid = titles[i];
			break;
		}
	}

	free(titles);
	titles = NULL;

	if(!tid)
		return -1;

	signed_blob * s_tmd = (signed_blob *) memalign(32, MAX_SIGNED_TMD_SIZE);

	u32 tmd_size;

	if (ES_GetStoredTMDSize(tid, &tmd_size) < 0)
	{
        free(s_tmd);
		return -1;
	}

	if (ES_GetStoredTMD(tid, s_tmd, tmd_size) < 0)
	{
        free(s_tmd);
		return -1;
	}

	tmd * t = (tmd *) SIGNATURE_PAYLOAD(s_tmd);

	ret = t->title_version;

    free(s_tmd);
    s_tmd = NULL;

	return ret;
}
