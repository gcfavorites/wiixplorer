 /****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include "Prompts/DeviceMenu.h"
#include "network/networkops.h"
#include "devicemounter.h"
#include "ftpii/virtualpath.h"
#include "menu.h"

extern "C" void MountVirtualDevices()
{
    if(SDCard_Inserted())
    {
        VirtualMountDevice(fmt("%s:/", DeviceName[SD]));
    }
    if(USBDevice_Inserted())
    {
        VirtualMountDevice(fmt("%s:/", DeviceName[USB]));
    }
    for(int i = 0; i < NTFS_GetMountCount(); i++)
    {
        VirtualMountDevice(fmt("%s:/", DeviceName[NTFS0+i]));
    }
    if(Disk_Inserted())
    {
        VirtualMountDevice(fmt("%s:/", DeviceName[DVD]));
    }
    for(int i = 0; i < MAXSMBUSERS; i++)
    {
        if(IsSMB_Mounted(i))
        {
            VirtualMountDevice(fmt("%s:/", DeviceName[SMB1+i]));
        }
    }
}
