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
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include "http.h"
#include "FileOperations/fileops.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"

/****************************************************************************
 * Download a file from a given url with a Progressbar
 ****************************************************************************/
int DownloadFileToMem(const char *url, u8 **inbuffer, u32 *size)
{
	if(strncasecmp(url, "http://", strlen("http://")) != 0)
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

	char header[1024];
	char * ptr = header;
	ptr += sprintf(ptr, "GET %s HTTP/1.1\r\n", path);
	ptr += sprintf(ptr, "Host: %s\r\n", domain);
	ptr += sprintf(ptr, "Referer: %s\r\n", domain);
	ptr += sprintf(ptr, "User-Agent: WiiXplorer\r\n");
	ptr += sprintf(ptr, "Pragma: no-cache\r\n");
	ptr += sprintf(ptr, "Cache-Control: no-cache\r\n");
	ptr += sprintf(ptr, "Connection: close\r\n\r\n");

	char filename[255];
	memset(filename, 0, sizeof(filename));

	u32 filesize = network_request(connection, header, filename);

	if(!filesize)
	{
		net_close(connection);
		ShowError(tr("Filesize is 0 Byte."));
		return -5;
	}

	u32 blocksize = 10*1024;

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
		if(ProgressWindow::Instance()->IsCanceled())
		{
			free(buffer);
			StopProgress();
			net_close(connection);
			ShowError(tr("Transfer cancelled."));
			return PROGRESS_CANCELED;
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
	// finish up the progress
	FinishProgress(filesize);

	StopProgress();
	net_close(connection);

	*inbuffer = buffer;
	*size = filesize;

	return 1;
}

/****************************************************************************
 * Download a file from a given url to a given path with a Progressbar
 ****************************************************************************/
int DownloadFileToPath(const char *orig_url, const char *dest, bool UseFilename)
{
	if(!orig_url || !dest)
	{
		ShowError(tr("No URL or Path specified."));
		return -2;
	}

	bool addhttp = false;

	if(strncasecmp(orig_url, "http://", strlen("http://")) != 0)
	{
		addhttp = true;
	}

	char url[strlen(orig_url) + (addhttp ? strlen("http://") : 0) + 1];

	if(addhttp)
		snprintf(url, sizeof(url), "http://%s", orig_url);
	else
		strcpy(url, orig_url);

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

	char header[1024];
	char * ptr = header;
	ptr += sprintf(ptr, "GET %s HTTP/1.1\r\n", path);
	ptr += sprintf(ptr, "Host: %s\r\n", domain);
	ptr += sprintf(ptr, "Referer: %s\r\n", domain);
	ptr += sprintf(ptr, "User-Agent: WiiXplorer\r\n");
	ptr += sprintf(ptr, "Pragma: no-cache\r\n");
	ptr += sprintf(ptr, "Cache-Control: no-cache\r\n");
	ptr += sprintf(ptr, "Connection: close\r\n\r\n");

	char filename[255];
	memset(filename, 0, sizeof(filename));

	u32 filesize = network_request(connection, header, filename);

	if(!filesize)
	{
		net_close(connection);
		ShowError(tr("Filesize is 0 Byte."));
		return -5;
	}

	u32 blocksize = 10*1024;

	u8 *buffer = (u8 *) malloc(blocksize);
	if(!buffer)
	{
		net_close(connection);
		ShowError(tr("Not enough memory."));
		return -6;
	}

	if(UseFilename)
	{
		if(dest[strlen(dest)-1] != '/')
			strcat((char *) dest, "/");

		CreateSubfolder(dest);

		strcat((char *) dest, filename);
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
		if(ProgressWindow::Instance()->IsCanceled())
		{
			free(buffer);
			StopProgress();
			net_close(connection);
			fclose(file);
			ShowError(tr("Transfer cancelled."));
			return PROGRESS_CANCELED;
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

	// finish up the progress
	FinishProgress(filesize);

	free(buffer);
	StopProgress();
	net_close(connection);
	fclose(file);

	return done;
}
