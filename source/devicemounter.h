#ifndef _DEVICEMOUNTER_H_
#define _DEVICEMOUNTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <gctypes.h>

int NTFS_Mount();
void NTFS_UnMount();
int NTFS_GetMountCount();
const char *NTFS_GetMountName(int mountIndex);
int USBDevice_Init();
void USBDevice_deInit();
bool USBDevice_Inserted();
bool SDCard_Inserted();
int SDCard_Init();
void SDCard_deInit();
int DiskDrive_Init(bool have_dvdx);
void DiskDrive_deInit();
bool Disk_Inserted();
bool DiskDrive_Mount();
void DiskDrive_UnMount();

#ifdef __cplusplus
}
#endif

#endif
