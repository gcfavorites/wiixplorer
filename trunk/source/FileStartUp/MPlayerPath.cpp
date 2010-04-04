#include <stdio.h>
#include <string.h>

void ConvertToMPlayerPath(const char * src, char * dst)
{
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
