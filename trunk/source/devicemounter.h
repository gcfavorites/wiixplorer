#ifndef _DEVICEMOUNTER_H_
#define _DEVICEMOUNTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <gctypes.h>

//!USB NTFS Controls
int NTFS_Mount();
void NTFS_UnMount();
int NTFS_GetMountCount();
const char *NTFS_GetMountName(int mountIndex);

//!USB FAT Controls
int USBDevice_Init();
void USBDevice_deInit();
bool USBDevice_Inserted();

//!SD FAT Controls
bool SDCard_Inserted();
int SDCard_Init();
void SDCard_deInit();

//!SD Gecko FAT Controls
int SDGeckoA_Init();
bool SDGeckoA_Inserted();
void SDGeckoA_deInit();

//!SD Gecko FAT Controls
int SDGeckoB_Init();
bool SDGeckoB_Inserted();
void SDGeckoB_deInit();

//!Wii Disk Drive Controls
int DiskDrive_Init();
void DiskDrive_deInit();
bool Disk_Inserted();
bool DiskDrive_Mount();
void DiskDrive_UnMount();

#ifdef __cplusplus
}
#endif

#endif
