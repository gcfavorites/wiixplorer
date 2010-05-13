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
 * for Wii-Xplorer 2009
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/machine/processor.h>
#include "libtinysmb/smb.h"
#include "libftp/ftp_devoptab.h"
#include "libftp/FTPServer.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"

#include "http.h"
#include "networkops.h"
#include "netreceiver.h"
#include "update.h"
#include "main.h"

static NetReceiver Receiver;
static bool SMB_Mounted[MAXSMBUSERS] = {false, false, false, false};
static bool networkinit = false;
static char IP[16];
static bool firstRun = false;
static bool ftpReady = false;

static lwp_t networkthread = LWP_THREAD_NULL;
static bool networkHalt = true;
static bool exitRequested = false;

extern bool actioncanceled;


/****************************************************************************
 * Download a file from a given url with a Progressbar
 ****************************************************************************/
int DownloadFileToMem(const char *url, u8 **inbuffer, u32 *size)
{
    if(strncmp(url, "http://", strlen("http://")) != 0)
    {
        ShowError(tr("Not a valid URL"));
		return -1;
    }
	char *path = strchr(url + strlen("http://"), '/');

	if(!path)
	{
        ShowError(tr("Not a valid URL path"));
        return -2;
	}

	int domainlength = path - url - strlen("http://");

	if(domainlength == 0)
	{
        ShowError(tr("Not a valid domain"));
		return -3;
	}

	char domain[domainlength + 1];
	strncpy(domain, url + strlen("http://"), domainlength);
	domain[domainlength] = '\0';

	int connection = GetConnection(domain);

    if(connection < 0)
    {
        ShowError(tr("Could not connect to the server."));
        return -4;
    }

    char header[strlen(path)+strlen(domain)*2+100];
    sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nReferer: %s\r\nUser-Agent: WiiXplorer\r\nConnection: close\r\n\r\n", path, domain, domain);

    char filename[255];
    memset(filename, 0, sizeof(filename));

    u32 filesize = network_request(connection, header, (char *) &filename);

    if(!filesize)
    {
        net_close(connection);
        ShowError(tr("Filesize is 0 Byte."));
        return -5;
    }

    u32 blocksize = 5*1024;

    u8 * buffer = (u8 *) malloc(filesize);
    if(!buffer)
    {
        net_close(connection);
        ShowError(tr("Not enough memory."));
        return -6;
    }

    u32 done = 0;

    StartProgress(tr("Downloading file..."));

    while(done < filesize)
    {
        if(actioncanceled)
        {
            usleep(20000);
            free(buffer);
            StopProgress();
            net_close(connection);
            ShowError(tr("Transfer cancelled."));
            return -10;
        }

        ShowProgress(done, filesize, filename);

        if(blocksize > filesize - done)
            blocksize = filesize - done;


        s32 read = network_read(connection, buffer+done, blocksize);

        if(read < 0)
        {
            free(buffer);
            StopProgress();
            net_close(connection);
            ShowError(tr("Transfer failed"));
            return -8;
        }
        else if(!read)
            break;

        done += read;
    }

    StopProgress();
    net_close(connection);

    *inbuffer = buffer;
    *size = filesize;

    return 1;
}

/****************************************************************************
 * Download a file from a given url to a given path with a Progressbar
 ****************************************************************************/
int DownloadFileToPath(const char *url, const char *dest)
{
    if(strncmp(url, "http://", strlen("http://")) != 0)
    {
        ShowError(tr("Not a valid URL"));
		return -1;
    }
	char *path = strchr(url + strlen("http://"), '/');

	if(!path)
	{
        ShowError(tr("Not a valid URL path"));
        return -2;
	}

	int domainlength = path - url - strlen("http://");

	if(domainlength == 0)
	{
        ShowError(tr("Not a valid domain"));
		return -3;
	}

	char domain[domainlength + 1];
	strncpy(domain, url + strlen("http://"), domainlength);
	domain[domainlength] = '\0';

	int connection = GetConnection(domain);

    if(connection < 0)
    {
        ShowError(tr("Could not connect to the server."));
        return -4;
    }

    char header[strlen(path)+strlen(domain)*2+100];
    sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nReferer: %s\r\nUser-Agent: WiiXplorer\r\nConnection: close\r\n\r\n", path, domain, domain);

    char filename[255];
    memset(filename, 0, sizeof(filename));

    u32 filesize = network_request(connection, header, (char *) &filename);

    if(!filesize)
    {
        net_close(connection);
        ShowError(tr("Filesize is 0 Byte."));
        return -5;
    }

    u32 blocksize = 5*1024;

    u8 *buffer = (u8 *) malloc(blocksize);
    if(!buffer)
    {
        net_close(connection);
        ShowError(tr("Not enough memory."));
        return -6;
    }

    FILE *file = fopen(dest, "wb");
    if(!file)
    {
        net_close(connection);
        free(buffer);
        ShowError(tr("Cannot write to destination."));
        return -7;
    }

    u32 done = 0;

    StartProgress(tr("Downloading file..."));

    while(done < filesize)
    {
        if(actioncanceled)
        {
            usleep(20000);
            free(buffer);
            StopProgress();
            net_close(connection);
            fclose(file);
            ShowError(tr("Transfer cancelled."));
            return -10;
        }

        ShowProgress(done, filesize, filename);

        if(blocksize > filesize - done)
            blocksize = filesize - done;

        s32 read = network_read(connection, buffer, blocksize);

        if(read < 0)
        {
            free(buffer);
            StopProgress();
            net_close(connection);
            fclose(file);
            ShowError(tr("Transfer failed"));
            return -8;
        }
        else if(!read)
            break;

        fwrite(buffer, 1, read, file);

        done += read;
    }

    free(buffer);
    StopProgress();
    net_close(connection);
    fclose(file);

    return done;
}

/****************************************************************************
 * Close SMB Share
 ****************************************************************************/
void CloseSMBShare()
{
    for(int i = 0; i < MAXSMBUSERS; i++) {

        char mountname[10];
        sprintf(mountname, "smb%i", i+1);

        if(SMB_Mounted[i])
            smbClose(mountname);

        SMB_Mounted[i] = false;
    }
}

/****************************************************************************
 * Reconnect SMB Connection
 ****************************************************************************/

void SMB_Reconnect()
{
    CloseSMBShare();
    sleep(1);
    ConnectSMBShare();
}

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

        if(!SMB_Mounted[i])
        {
            if(strcmp(Settings.SMBUser[i].Host, "") != 0)
            {
                if(smbInitDevice(mountname,
                    Settings.SMBUser[i].User,
                    Settings.SMBUser[i].Password,
                    Settings.SMBUser[i].SMBName,
                    Settings.SMBUser[i].Host))
                {
                    SMB_Mounted[i] = true;
                    result = true;
                }
                else
                {
                    SMB_Mounted[i] = false;
                }
            }
            else
            {
                SMB_Mounted[i] = false;
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

    return SMB_Mounted[smb];
}

/****************************************************************************
 * FTP Stuff
 ***************************************************************************/

bool ConnectFTP()
{
	ftpReady = false;

	for (int i = 0; i < MAXFTPUSERS; i++)
	{
		char name[10];
		sprintf(name, "ftp%i", i+1);

		if (strcmp(Settings.FTPUser[i].Host, "") != 0)
		{
		    char User[50];
		    char Password[50];

		    if(strcmp(Settings.FTPUser[i].User, "") == 0)
                snprintf(User, sizeof(User), "anonymous");
            else
                snprintf(User, sizeof(User), "%s", Settings.FTPUser[i].User);

		    if(strcmp(Settings.FTPUser[i].Password, "") == 0)
                snprintf(Password, sizeof(Password), "anonymous@WiiXplorer.com");
            else
                snprintf(Password, sizeof(Password), "%s", Settings.FTPUser[i].Password);

			if (ftpInitDevice(name,
				User,
				Password,
				Settings.FTPUser[i].FTPPath,
				Settings.FTPUser[i].Host,
				Settings.FTPUser[i].Port,
				Settings.FTPUser[i].Passive))
			{
				ftpReady = true;
			}
		}
	}

	return ftpReady;
}

void CloseFTP()
{
	for (int i = 0; i < MAXFTPUSERS; i++)
	{
		char name[10];
		sprintf(name, "ftp%i", i+1);
		ftpClose(name);
    }
}

bool IsFTPConnected(int ftp)
{
	if (ftp < 0 || ftp >= MAXFTPUSERS || !ftpReady)
		return false;

	char name[10];
	sprintf(name, "ftp%i", ftp+1);

	return CheckFTPConnection(name);
}

/****************************************************************************
 * Initialize_Network
 ***************************************************************************/
void Initialize_Network(void)
{
    if(networkinit)
        return;

	s32 result;

    result = if_config(IP, NULL, NULL, true);

    if(result < 0) {
        networkinit = false;
		return;
	}

    networkinit = true;
    return;
}

/****************************************************************************
 * DeInit_Network
 ***************************************************************************/
void DeInit_Network(void)
{
    net_deinit();
}
/****************************************************************************
 * Check if network was initialised
 ***************************************************************************/
bool IsNetworkInit(void)
{
    return networkinit;
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
	{
		usleep(100);
	}
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
    while(!exitRequested)
    {
        if(networkHalt)
        {
            LWP_SuspendThread(networkthread);
			usleep(100);
			continue;
        }

        if(!networkinit)
            Initialize_Network();

        if(!firstRun)
        {
            ConnectSMBShare();
            if(Settings.FTPServer.AutoStart)
                FTPServer::Instance()->StartupFTP();

			ConnectFTP();
            CheckForUpdate();

            LWP_SetThreadPriority(LWP_GetSelf(), 0);
            firstRun = true;
        }

        if(Receiver.CheckIncomming())
        {
            IncommingConnection(Receiver);
        }

        usleep(200000);
    }
	return NULL;
}

/****************************************************************************
 * InitNetworkThread with priority 0 (idle)
 ***************************************************************************/
void InitNetworkThread()
{
	LWP_CreateThread (&networkthread, networkinitcallback, NULL, NULL, 16384, 30);
	ResumeNetworkThread();
}

/****************************************************************************
 * ShutdownThread
 ***************************************************************************/
void ShutdownNetworkThread()
{
    Receiver.FreeData();
    Receiver.CloseConnection();
    exitRequested = true;
    ResumeNetworkThread();
	LWP_JoinThread (networkthread, NULL);
	networkthread = LWP_THREAD_NULL;
}
