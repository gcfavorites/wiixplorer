 /***************************************************************************
 * Copyright (C) 2009
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
 * networkops.cpp
 *
 * Network operations
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/machine/processor.h>
#include <smb.h>

#include "http.h"
#include "networkops.h"
#include "main.h"

static bool SMB_Mounted = false;
static bool networkinit = false;
static bool network_initiating = false;
static char IP[16];

static lwp_t networkthread = LWP_THREAD_NULL;
static bool networkHalt = true;

/****************************************************************************
 * Close SMB Share
 ****************************************************************************/

void CloseSMBShare()
{
	if(SMB_Mounted)
		smbClose("smb");

	SMB_Mounted = false;
	networkinit = false;
}

/****************************************************************************
 * Reconnect SMB Connection
 ****************************************************************************/

void SMB_Reconnect()
{
	if(SMB_Mounted)
		smbCheckConnection("smb");
    else {
        if(smbInitDevice("smb",
            Settings.SMBUser[Settings.CurrentUser].User,
            Settings.SMBUser[Settings.CurrentUser].Password,
            Settings.SMBUser[Settings.CurrentUser].SMBName,
            Settings.SMBUser[Settings.CurrentUser].Host))
        {
            SMB_Mounted = true;
        } else {
            SMB_Mounted = false;
        }
    }
}

/****************************************************************************
 * Mount SMB Share
 ****************************************************************************/

bool ConnectSMBShare()
{
	if(SMB_Mounted)
		return false;

	if(!networkinit)
		Initialize_Network();

	if(networkinit)
	{
		if(!SMB_Mounted)
		{
			if(smbInitDevice("smb",
                Settings.SMBUser[Settings.CurrentUser].User,
                Settings.SMBUser[Settings.CurrentUser].Password,
                Settings.SMBUser[Settings.CurrentUser].SMBName,
                Settings.SMBUser[Settings.CurrentUser].Host))
			{
				SMB_Mounted = true;
			} else {
			    SMB_Mounted = false;
			}
		}
	}
	networkHalt = true;
	return SMB_Mounted;
}

/****************************************************************************
 * Initialize_Network
 ***************************************************************************/
void Initialize_Network(void) {

    if(networkinit)
        return;

	s32 result;
	network_initiating = true;

    result = if_config(IP, NULL, NULL, true);

   if(result < 0) {
        networkinit = false;
        network_initiating = false;
		return;
	}

    networkinit = true;
    network_initiating = false;
    return;
}

/****************************************************************************
 * Check if network was initialised
 ***************************************************************************/
bool IsNetworkInit(void)
{
    return networkinit;
}

/****************************************************************************
 * Check if network is initialising
 ***************************************************************************/
bool IsNetworkInitiating(void)
{
    return network_initiating;
}

/****************************************************************************
 * Get network IP
 ***************************************************************************/
char * GetNetworkIP(void)
{
    return IP;
}

/****************************************************************************
 * HaltNetwork
 ***************************************************************************/
void HaltNetworkThread()
{
    networkHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(networkthread))
		usleep(100);
}

/****************************************************************************
 * ResumeNetworkThread
 ***************************************************************************/
void ResumeNetworkThread()
{
	networkHalt = false;
	LWP_ResumeThread(networkthread);
}

/*********************************************************************************
 * Networkthread for background network initialize and update check with idle prio
 *********************************************************************************/
static void * networkinitcallback(void *arg)
{
    while(1) {
        if(networkHalt)
            LWP_SuspendThread(networkthread);

        ConnectSMBShare();

        usleep(100);
    }
	return NULL;
}

/****************************************************************************
 * InitNetworkThread with priority 0 (idle)
 ***************************************************************************/
void InitNetworkThread()
{
	LWP_CreateThread (&networkthread, networkinitcallback, NULL, NULL, 0, 0);
}

/****************************************************************************
 * ShutdownThread
 ***************************************************************************/
void ShutdownNetworkThread()
{
	LWP_JoinThread (networkthread, NULL);
	networkthread = LWP_THREAD_NULL;
}
