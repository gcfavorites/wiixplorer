/****************************************************************************
 * Networkhandle for Wiixplorer
 * by dimok
 *
 * HTTP operations
 * Written by dhewg/bushing
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <smb.h>
#include <ogc/machine/processor.h>

#include "libwiigui/gui.h"
#include "http.h"
#include "networkops.h"
#include "main.h"

static s32 connection;
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
 * Close Network Connection
 ****************************************************************************/

void CloseConnection() {

    net_close(connection);

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
