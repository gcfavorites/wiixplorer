#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include "network/SMB.h"
#include "network/FTPClient.h"
#include "DeviceHandler.hpp"

#include "libdisk/fst.h"
#include "libdisk/gcfst.h"
#include "libdisk/iso.h"
#include "libdisk/di2.h"

DeviceHandler * DeviceHandler::instance = NULL;

DeviceHandler::~DeviceHandler()
{
    UnMountAll();

    if(sd)
        delete sd;
    if(gca)
        delete gca;
    if(gcb)
        delete gca;
    if(usb)
        delete usb;
}

DeviceHandler * DeviceHandler::Instance()
{
	if (instance == NULL)
	{
		instance = new DeviceHandler();
	}
	return instance;
}

void DeviceHandler::DestroyInstance()
{
    if(instance)
    {
        delete instance;
    }
    instance = NULL;
}

bool DeviceHandler::MountAll()
{
    bool result = false;

    for(u32 i = SD; i <= FTP10; i++)
    {
        if(Mount(i))
            result = true;
    }

    return result;
}

void DeviceHandler::UnMountAll()
{
    for(u32 i = SD; i <= FTP10; i++)
    {
        UnMount(i);
    }
}

bool DeviceHandler::Mount(int dev)
{
    switch(dev)
    {
        case SD:
            return MountSD();
        case GCSDA:
            return MountGCA();
        case GCSDB:
            return MountGCB();
        case USB1:
        case USB2:
        case USB3:
        case USB4:
        case USB5:
        case USB6:
        case USB7:
        case USB8:
            return MountUSB(dev-USB1);
        case SMB1:
        case SMB2:
        case SMB3:
        case SMB4:
        case SMB5:
        case SMB6:
        case SMB7:
        case SMB8:
        case SMB9:
        case SMB10:
            return ConnectSMBShare(dev-SMB1);
        case FTP1:
        case FTP2:
        case FTP3:
        case FTP4:
        case FTP5:
        case FTP6:
        case FTP7:
        case FTP8:
        case FTP9:
        case FTP10:
            return ConnectFTP(dev-FTP1);
        case DVD:
            return MountDVD();
        default:
            return false;
    }

    return false;
}

bool DeviceHandler::IsInserted(int dev)
{
    switch(dev)
    {
        case SD:
            return SD_Inserted() && sd->IsMounted(0);
        case GCSDA:
            return GCA_Inserted() && gca->IsMounted(0);
        case GCSDB:
            return GCB_Inserted() && gcb->IsMounted(0);
        case USB1:
        case USB2:
        case USB3:
        case USB4:
        case USB5:
        case USB6:
        case USB7:
        case USB8:
            return USB_Inserted() && usb->IsMounted(dev-USB1);
        case SMB1:
        case SMB2:
        case SMB3:
        case SMB4:
        case SMB5:
        case SMB6:
        case SMB7:
        case SMB8:
        case SMB9:
        case SMB10:
            return IsSMB_Mounted(dev-SMB1);
        case FTP1:
        case FTP2:
        case FTP3:
        case FTP4:
        case FTP5:
        case FTP6:
        case FTP7:
        case FTP8:
        case FTP9:
        case FTP10:
            return IsFTPConnected(dev-FTP1); //later
        case DVD:
            return DVD_Inserted();
        default:
            return false;
    }

    return false;
}

void DeviceHandler::UnMount(int dev)
{
    switch(dev)
    {
        case SD:
            UnMountSD();
            return;
        case GCSDA:
            UnMountGCA();
            return;
        case GCSDB:
            UnMountGCB();
            return;
        case USB1:
        case USB2:
        case USB3:
        case USB4:
        case USB5:
        case USB6:
        case USB7:
        case USB8:
            UnMountUSB(dev-USB1);
            return;
        case SMB1:
        case SMB2:
        case SMB3:
        case SMB4:
        case SMB5:
        case SMB6:
        case SMB7:
        case SMB8:
        case SMB9:
        case SMB10:
            CloseSMBShare(dev-SMB1);
            return;
        case FTP1:
        case FTP2:
        case FTP3:
        case FTP4:
        case FTP5:
        case FTP6:
        case FTP7:
        case FTP8:
        case FTP9:
        case FTP10:
            CloseFTP(dev-FTP1);
            return;
        default:
            return;
    }
}

bool DeviceHandler::MountSD()
{
    if(!sd)
        sd = new PartitionHandle(&__io_wiisd);

    if(sd->GetPartitionCount() < 1)
    {
        delete sd;
        sd = NULL;
        return false;
    }

    //! Mount only one SD Partition
    return sd->Mount(0, DeviceName[SD]);
}

bool DeviceHandler::MountGCA()
{
    if(!gca)
        gca = new PartitionHandle(&__io_gcsda);

    if(gca->GetPartitionCount() < 1)
    {
        delete gca;
        gca = NULL;
        return false;
    }

    //! Mount only one Partition
    return gca->Mount(0, DeviceName[GCSDA]);
}

bool DeviceHandler::MountGCB()
{
    if(!gcb)
        gcb = new PartitionHandle(&__io_gcsdb);

    if(gcb->GetPartitionCount() < 1)
    {
        delete gcb;
        gcb = NULL;
        return false;
    }

    //! Mount only one Partition
    return gcb->Mount(0, DeviceName[GCSDB]);;
}

bool DeviceHandler::MountUSB(int pos)
{
    if(!usb)
        usb = new PartitionHandle(&__io_usbstorage);

    if(usb->GetPartitionCount() < 1)
    {
        delete usb;
        usb = NULL;
        return false;
    }

    if(pos >= usb->GetPartitionCount())
        return false;

    return usb->Mount(pos, DeviceName[USB1+pos]);
}

bool DeviceHandler::MountAllUSB()
{
    if(!usb)
        usb = new PartitionHandle(&__io_usbstorage);

    bool result = false;

    for(int i = 0; i < usb->GetPartitionCount(); i++)
    {
        if(MountUSB(i))
            result = true;
    }

    return result;
}

void DeviceHandler::UnMountUSB(int pos)
{
    if(!usb)
        return;

    if(pos >= usb->GetPartitionCount())
        return;

    usb->UnMount(pos);
}

void DeviceHandler::UnMountAllUSB()
{
    if(!usb)
        return;

    for(int i = 0; i < usb->GetPartitionCount(); i++)
        usb->UnMount(i);

    delete usb;
    usb = NULL;
}

bool DeviceHandler::MountDVD()
{
    char read_buffer[2048];
    if(DI2_ReadDVD(read_buffer, 1, 0) == 0)
        return true;

    bool devicemounted = false;

    UnMountDVD();
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
    if(!devicemounted)
        devicemounted = GCFST_Mount();

    return devicemounted;
}

bool DeviceHandler::DVD_Inserted()
{
    uint32_t cover = 0;
    DI2_GetCoverRegister(&cover);

    if(cover & DVD_COVER_DISC_INSERTED)
        return true;

	return false;
}

void DeviceHandler::UnMountDVD()
{
    FST_Unmount();
    GCFST_Unmount();
    ISO9660_Unmount();
}

int DeviceHandler::PathToDriveType(const char * path)
{
    if(!path)
        return -1;

    for(int i = SD; i <= DVD; i++)
    {
        if(strncmp(path, DeviceName[i], strlen(DeviceName[i])) == 0)
            return i;
    }

    return -1;
}

const char * DeviceHandler::GetFSName(int dev)
{
    if(dev == SD && DeviceHandler::instance->sd)
    {
        return DeviceHandler::instance->sd->GetFSName(0);
    }
    else if(dev == GCSDA && DeviceHandler::instance->gca)
    {
        return DeviceHandler::instance->gca->GetFSName(0);
    }
    else if(dev == GCSDB && DeviceHandler::instance->gcb)
    {
        return DeviceHandler::instance->gcb->GetFSName(0);
    }
    else if(dev >= USB1 && dev <= USB8 && DeviceHandler::instance->usb)
    {
        return DeviceHandler::instance->usb->GetFSName(dev-USB1);
    }

    return NULL;
}
