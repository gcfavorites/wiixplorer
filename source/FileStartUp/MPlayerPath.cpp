#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <menu.h>

static void CreateMPlayerSMBPath(const char * src, char * dst)
{
    if(!src || !dst)
        return;

    int client = atoi(&src[3]);
    if(client <= 0 || client >= MAXSMBUSERS)
    {
        dst[0] = '\0';
        return;
    }

    const char * filepath = src;

    while(filepath[0] != '/' || filepath[1] == '/')
        filepath++;

    filepath++;

    sprintf(dst, "smb://%s:%s@%s/%s/%s",
            Settings.SMBUser[client-1].User,
            Settings.SMBUser[client-1].Password,
            Settings.SMBUser[client-1].Host,
            Settings.SMBUser[client-1].SMBName,
            filepath);
}

void ConvertToMPlayerPath(const char * src, char * dst)
{
    if(!src || !dst)
        return;

    if(src[0] == 's' && src[1] == 'm' && src[2] == 'b')
    {
        CreateMPlayerSMBPath(src, dst);
        return;
    }

    int i = 0;

    char device[10];

    while(src[i] != ':')
    {
        device[i] = src[i];
        device[i+1] = 0;
        i++;
    }

    char * ptr = (char *) &src[i];

    while(ptr[0] != '/' || ptr[1] == '/')
        ptr++;

    if(strstr(device, "ntfs") != 0)
    {
        sprintf(dst, "ntfs_usb:%s", ptr);
    }
    else
    {
        sprintf(dst, "%s:%s", device, ptr);
    }
}
