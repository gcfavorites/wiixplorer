#include <fat.h>
#include <malloc.h>
#include <unistd.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/wiisd_io.h>

#include "usbstorage/usbstorage.h"
#include <ntfs.h>   //has to be after usbstorage.h so our usbstorage.h is loaded and not the libogc one

//these are the only stable and speed is good
#define CACHE 8
#define SECTORS 64

static ntfs_md *ntfs_mounts = NULL;
static int ntfs_mountCount = 0;

int NTFS_Mount()
{
    ntfs_mountCount = ntfsMountAll(&ntfs_mounts, NTFS_DEFAULT | NTFS_RECOVER);

    return ntfs_mountCount;
}

void NTFS_UnMount()
{
    int i = 0;

    if (ntfs_mounts) {
        for (i = 0; i < ntfs_mountCount; i++)
            ntfsUnmount(ntfs_mounts[i].name, true);
        free(ntfs_mounts);
        ntfs_mounts = NULL;
    }
    ntfs_mountCount = 0;
}

int NTFS_GetMountCount()
{
    return ntfs_mountCount;
}

const char *NTFS_GetMountName(int mountIndex)
{
    if(mountIndex < 0 || mountIndex > ntfs_mountCount)
        return NULL;

    return ntfs_mounts[mountIndex].name;
}

int USBDevice_Init()
{
    int ret = -1;
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("usb:/");

    if (fatMount("usb", &__io_usb2storage, 0, CACHE, SECTORS)) {
		ret = 1;
	} else if (fatMount("usb", &__io_usb1storage, 0, CACHE, SECTORS)) {
		ret = 1;
	}

	return ret;
}

void USBDevice_deInit()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("usb:/");
}

int USBDevice_Inserted()
{
    int ret = __io_usb2storage.isInserted();
	if(ret >= 0)
        return ret;
    else
        return __io_usb1storage.isInserted();
}

int SDCard_Inserted()
{
    return __io_wiisd.isInserted();
}

int SDCard_Init()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("sd:/");
	//right now mounts first FAT-partition
	if (fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS))
		return 1;
	return -1;
}

void SDCard_deInit()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("sd:/");
}
