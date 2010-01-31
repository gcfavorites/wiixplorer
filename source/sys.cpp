#include <stdio.h>
#include <ogcsys.h>
#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>

#include "network/networkops.h"
#include "Language/gettext.h"
#include "Prompts/PromptWindows.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "video.h"
#include "audio.h"
#include "main.h"
#include "menu.h"
#include "devicemounter.h"
#include "sys.h"

u8 shutdown = 0;
u8 reset = 0;
extern bool boothomebrew;

bool RebootApp()
{
    char filepath[MAXPATHLEN];
    snprintf(filepath, sizeof(filepath), "%s/boot.dol", Settings.UpdatePath);
    int ret = LoadHomebrew(filepath);
    if(ret < 0)
    {
     WindowPrompt(tr("Reboot failed"), tr("Can't load file"), tr("OK"));
     return false;
    }

    boothomebrew = true;

    Taskbar::Instance()->SetMenu(MENU_EXIT);

    return true;
}

void ExitApp()
{
    Settings.Save();
	MainWindow::Instance()->DestroyInstance();
	StopGX();
	ShutdownAudio();
	ClearFontData();
	Resources::DestroyInstance();
	CloseSMBShare();
    NTFS_UnMount();
    SDCard_deInit();
    USBDevice_deInit();
    DiskDrive_deInit();
	DeInit_Network();

	WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();
}

void __Sys_ResetCallback(void)
{
	reset = 1;
}

void __Sys_PowerCallback(void)
{
	shutdown = 1;
}


void Sys_Init(void)
{
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void Sys_Reboot(void)
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
	WPAD_Flush(0);
	WPAD_Disconnect(0);
	WPAD_Shutdown();

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

void Sys_Shutdown(void)
{
	_Sys_Shutdown(ShutdownToDefault);
}

void Sys_ShutdownToIdel(void)
{
	_Sys_Shutdown(ShutdownToIdle);
}

void Sys_ShutdownToStandby(void)
{
	_Sys_Shutdown(ShutdownToStandby);
}

void Sys_LoadMenu(void)
{
	ExitApp();
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void Sys_BackToLoader(void)
{
	if (IsFromHBC())
	{
		ExitApp();
		exit(0);
	}
	// Channel Version
	Sys_LoadMenu();
}

bool IsFromHBC()
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
