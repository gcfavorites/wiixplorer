#include <gctypes.h>
#include <malloc.h>
#include <ogc/machine/processor.h>

#include "IosLoader.h"
#include "DeviceControls/DeviceHandler.hpp"
#include "FileOperations/fileops.h"
#include "mload/mload.h"
#include "mload/mload_init.h"
#include "mload/ehcmodule_5.h"
#include "Tools/tools.h"
#include "sys.h"

#define MEM2_PROT		0x0D8B420A
#define ES_MODULE_START	((u16 *)0x939F0000)
#define ES_MODULE_END	(ES_MODULE_START + 0x4000)
#define ES_HACK_OFFSET	4

/*
 * Buffer variables for the IOS info to avoid loading it several times
 */
static int currentIOS = -1;
static iosinfo_t *currentIOSInfo = NULL;

/******************************************************************************
 * Public Methods:
 ******************************************************************************/
/*
 * Check if the ios passed is a Hermes ios.
 */
bool IosLoader::IsHermesIOS(s32 ios)
{
	return (ios == 222 || ios == 223 || ios == 224 || ios == 225 || ios == 202);
}

/*
 * Check if the ios passed is a Waninkoko ios.
 */
bool IosLoader::IsWaninkokoIOS(s32 ios)
{
	if(ios < 200 || ios > 255)
		return false;

	return !IsHermesIOS(ios);
}

/*
 * Check if the ios passed is a d2x ios.
 */
bool IosLoader::IsD2X(s32 ios)
{
	iosinfo_t *info = GetIOSInfo(ios);
	if(!info)
		return false;

	bool res = (strncasecmp(info->name, "d2x", 3) == 0);

	return res;
}

/*
 * Reloads a certain IOS, do the necessary deinit/reinit routines
 * and keep access rights of IOS
 */
s32 IosLoader::ReloadAppIos(s32 ios)
{
	if(ios == IOS_GetVersion())
		return 0;

	//! unmount before IOS reload
	DeviceHandler::Instance()->UnMountSD();
	DeviceHandler::Instance()->UnMountAllUSB();
	USB_Deinitialize();
	MagicPatches(0);

	s32 result = ReloadIosKeepingRights(ios);

	MagicPatches(1);
	//! remount after IOS reload
	DeviceHandler::Instance()->MountSD();
	DeviceHandler::Instance()->MountAllUSB();

	return result;
}

/*
 * Reloads a certain IOS and keeps the AHBPROT flag enabled if available.
 */
s32 IosLoader::ReloadIosKeepingRights(s32 ios)
{
	if (CheckAHBPROT())
	{
		static const u16 ticket_check[] = {
			0x685B,			// ldr  r3, [r3, #4] ; Get TMD pointer
			0x22EC, 0x0052,	// movs r2, 0x1D8	; Set offset of access rights field in TMD
			0x189B,			// adds r3, r3, r2   ; Add offset to TMD pointer
			0x681B,			// ldr  r3, [r3]	 ; Load access rights. We'll hack it with full access rights!!!
			0x4698,			// mov  r8, r3	   ; Store it for the DVD video bitcheck later
			0x07DB			// lsls r3, r3, 0x1F ; check AHBPROT bit
		};

		/* Disable MEM 2 protection */
		write16(MEM2_PROT, 2);

		for (u16 *patchme = ES_MODULE_START; patchme < ES_MODULE_END; patchme++)
		{
			if (!memcmp(patchme, ticket_check, sizeof(ticket_check)))
			{
				/* Apply patch */
				patchme[ES_HACK_OFFSET] = 0x23FF; // li r3, 0xFF ; Set full access rights

				/* Flush cache */
				DCFlushRange(patchme+ES_HACK_OFFSET, 2);
				break;
			}
		}
	}

	s32 result = ReloadIos(ios);
	if(result >= 0 && IsHermesIOS())
		mload_Init();

	/* Reload IOS. MEM2 protection is implicitly re-enabled */
	return result;
}

/*
 * Reads the ios info struct from the .app file.
 * @return pointer to iosinfo_t on success else NULL. The user is responsible for freeing the buffer.
 */
iosinfo_t *IosLoader::GetIOSInfo(s32 ios)
{
	if(currentIOS == ios && currentIOSInfo)
		return currentIOSInfo;

	if(currentIOSInfo)
	{
		free(currentIOSInfo);
		currentIOSInfo = NULL;
	}

	currentIOS = ios;
	char filepath[ISFS_MAXPATH];
	u64 TicketID = ((((u64) 1) << 32) | ios);
	u32 TMD_Length;

	s32 ret = ES_GetStoredTMDSize(TicketID, &TMD_Length);
	if (ret < 0)
		return NULL;

	signed_blob *TMD = (signed_blob*) memalign(32, ALIGN32(TMD_Length));
	if (!TMD)
		return NULL;

	ret = ES_GetStoredTMD(TicketID, TMD, TMD_Length);
	if (ret < 0)
	{
		free(TMD);
		return NULL;
	}

	snprintf(filepath, sizeof(filepath), "nand:/title/%08x/%08x/content/%08x.app", 0x00000001, ios, *(u8 *)((u32)TMD+0x1E7));

	free(TMD);

	u8 *buffer = NULL;
	u32 filesize = 0;

	LoadFileToMem(filepath, &buffer, &filesize);

	if(!buffer)
		return NULL;

	iosinfo_t *iosinfo = (iosinfo_t *) buffer;

	if(iosinfo->magicword != 0x1ee7c105 || iosinfo->magicversion != 1)
	{
		free(buffer);
		return NULL;
	}

	iosinfo = (iosinfo_t *) realloc(buffer, sizeof(iosinfo_t));
	if(!iosinfo)
		iosinfo = (iosinfo_t *) buffer;

	currentIOSInfo = iosinfo;

	return iosinfo;
}
