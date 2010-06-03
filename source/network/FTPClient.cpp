 /***************************************************************************
 * Copyright (C) 2010
 * by dude
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
 * netreceiver.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <ogcsys.h>
#include <string.h>
#include "libftp/ftp_devoptab.h"
#include "main.h"

static bool ftpReady = false;

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

void CloseFTP(int client)
{
    char name[10];
    sprintf(name, "ftp%i", client+1);
    ftpClose(name);
}

bool IsFTPConnected(int ftp)
{
	if (ftp < 0 || ftp >= MAXFTPUSERS || !ftpReady)
		return false;

	char name[10];
	sprintf(name, "ftp%i", ftp+1);

	return CheckFTPConnection(name);
}
