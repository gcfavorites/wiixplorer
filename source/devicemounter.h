#ifndef _DEVICEMOUNTER_H_
#define _DEVICEMOUNTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
}
#endif

#endif
