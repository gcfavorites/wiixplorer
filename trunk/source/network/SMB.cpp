 /***************************************************************************
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
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <smb.h>
#include "main.h"

/****************************************************************************
 * Mount SMB Share
 ****************************************************************************/
bool ConnectSMBShare()
{
    bool result = false;

    for(int i = 0; i < MAXSMBUSERS; i++)
    {
        char mountname[10];
        sprintf(mountname, "smb%i", i+1);

        if(strcmp(Settings.SMBUser[i].Host, "") != 0)
        {
            if(smbInitDevice(mountname,
                Settings.SMBUser[i].User,
                Settings.SMBUser[i].Password,
                Settings.SMBUser[i].SMBName,
                Settings.SMBUser[i].Host))
            {
                result = true;
            }
        }
    }
	return result;
}

/****************************************************************************
 * IsSMB_Mounted
 ***************************************************************************/
bool IsSMB_Mounted(int smb)
{
    if(smb < 0 || smb >= MAXSMBUSERS)
        return false;

    char mountname[10];
    sprintf(mountname, "smb%i", smb+1);

    return smbCheckConnection(mountname);
}

/****************************************************************************
 * Close SMB Share
 ****************************************************************************/
void CloseSMBShare()
{
    for(int i = 0; i < MAXSMBUSERS; i++)
    {
        char mountname[10];
        sprintf(mountname, "smb%i", i+1);

        if(IsSMB_Mounted(i))
            smbClose(mountname);
    }
}

void CloseSMBShare(int connection)
{
    if(connection < 0 || connection >= MAXSMBUSERS)
        return;

    char mountname[10];
    sprintf(mountname, "smb%i", connection+1);

    if(IsSMB_Mounted(connection))
        smbClose(mountname);
}

/****************************************************************************
 * Reconnect SMB Connection
 ****************************************************************************/
void SMB_Reconnect()
{
    CloseSMBShare();
    usleep(200000);
    ConnectSMBShare();
}
