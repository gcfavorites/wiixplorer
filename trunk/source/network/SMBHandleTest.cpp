#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <smb.h>

#include "network/networkops.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"

static SMBCONN smbhandle;
static SMBFILE smbfile;

int SMB_Write()
{
    s32 ret = 0;

	SMB_Connect(&smbhandle, "Dima", "dimitri", "d", "192.168.178.3");

	smbfile = SMB_OpenFile("Log2.txt", SMB_OPEN_WRITING, SMB_OF_CREATE, smbhandle);

	if(!smbfile) WindowPrompt("No file", 0, "OK");

	FILE *file = fopen("sd:/apps/usbloader_gx/boot.dol", "rb");
	fseek(file, 0, SEEK_END);
	u32 filesize = ftell(file);

	rewind(file);

    u32 read = 0;

    u32 blocksize = 60*1024;

    u8 *buffer = new unsigned char[blocksize*2];

    do {

    ShowProgress(read, filesize, (char*) "test");

	ret = fread(buffer, 1, blocksize, file);

	SMB_WriteFile((const char*) buffer, ret, read, smbfile);

    read += ret;

    } while(read < filesize);

    StopProgress();

	fclose(file);

	delete buffer;

	SMB_CloseFile(smbfile);

	SMB_Close(smbhandle);

	return 0;
}
