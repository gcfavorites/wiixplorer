#include "usbstorage.h"
#include <fat.h>
#include <sdcard/wiisd_io.h>

//these are the only stable and speed is good
#define CACHE 8
#define SECTORS 64

int USBDevice_Init()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("usb:/");
	//right now mounts first FAT-partition
	if (fatMount("usb", &__io_usbstorage, 0, CACHE, SECTORS)) {
		//try first mount with libogc
		return 1;
	}
	return -1;
}

void USBDevice_deInit()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("usb:/");
	__io_usbstorage.shutdown();
}

int isSdInserted()
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
	__io_wiisd.shutdown();
}
