#include <fat.h>
#include <malloc.h>
#include <unistd.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/wiisd_io.h>

#include "usbstorage/usbstorage.h"
#include <ntfs.h>   //has to be after usbstorage.h so our usbstorage.h is loaded and not the libogc one

#include "devicemounter.h"
#include "libdisk/fst.h"
#include "libdisk/iso.h"
#include "libdisk/di2.h"

//these are the only stable and speed is good
#define CACHE 8
#define SECTORS 64

static ntfs_md *ntfs_mounts = NULL;
static int ntfs_mountCount = 0;
static DISC_INTERFACE * sd = (DISC_INTERFACE *) &__io_wiisd;
static DISC_INTERFACE * usb = NULL;

int NTFS_Mount()
{
    ntfs_mountCount = ntfsMountAll(&ntfs_mounts, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);

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

    if (fatMount("usb", &__io_usb2storage, 0, CACHE, SECTORS))
    {
        usb = (DISC_INTERFACE *) &__io_usb2storage;
		ret = 1;
	}
	else if (fatMount("usb", &__io_usb1storage, 0, CACHE, SECTORS))
	{
        usb = (DISC_INTERFACE *) &__io_usb1storage;
		ret = 1;
	}

	return ret;
}

void USBDevice_deInit()
{
    if(!usb)
        return;

	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("usb:/");
	usb->shutdown();
}

bool USBDevice_Inserted()
{
    if(!usb)
        return false;

	return usb->isInserted();
}

bool SDCard_Inserted()
{
    return sd->isInserted();
}

int SDCard_Init()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("sd:/");
	//right now mounts first FAT-partition
	if (fatMount("sd", sd, 0, CACHE, SECTORS))
		return 1;
	return -1;
}

void SDCard_deInit()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("sd:/");
	sd->shutdown();
}

int DiskDrive_Init(bool have_dvdx)
{
    int result = -1;

	if(have_dvdx)
        result = DI2_Init();
	else
        result = DI2_InitNoDVDx();

	return result; //Init DVD Driver
}

void DiskDrive_deInit()
{
    FST_Unmount();
    ISO9660_Unmount();
    DI2_Close(); //Deinit DVD Driver
}

bool Disk_Inserted()
{
    uint32_t cover = 0;
    DI2_GetCoverRegister(&cover);

    if(cover & DVD_COVER_DISC_INSERTED)
        return true;

	return false;
}

bool DiskDrive_Mount()
{
    if(DI2_GetStatus() & DVD_READY)
        return true;

    bool devicemounted = false;

    DiskDrive_UnMount();

    DI2_Mount();
	time_t timer1, timer2;
	timer1 = time(0);

	while(DI2_GetStatus() & DVD_INIT)
	{
		timer2 = time(0);
		if(timer2-timer1 > 15)
            return false;

		usleep(5000);
	}

    devicemounted = ISO9660_Mount();

    if(!devicemounted)
        devicemounted = FST_Mount();

    return devicemounted;
}

void DiskDrive_UnMount()
{
    FST_Unmount();
    ISO9660_Unmount();
}
