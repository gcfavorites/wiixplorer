#include <fat.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/wiisd_io.h>

#include "usbstorage/usbstorage.h"

//these are the only stable and speed is good
#define CACHE 8
#define SECTORS 64

int USBDevice_Init()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("usb:/");

    if (fatMount("usb", &__io_usb2storage, 0, CACHE, SECTORS)) {
		return 1;
	} else if (fatMount("usb", &__io_usb1storage, 0, CACHE, SECTORS)) {
		return 1;
	}
	return -1;
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
