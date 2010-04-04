/*

Copyright (C) 2008 Joseph Jordan <joe.ftpii@psychlaw.com.au>
This work is derived from Daniel Ehlers' <danielehlers@mindeye.net> srg_vrt branch.

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1.The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software in a
product, an acknowledgment in the product documentation would be
appreciated but is not required.

2.Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3.This notice may not be removed or altered from any source distribution.

*/
#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <sys/dir.h>
#include <unistd.h>
#include <di/di.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>
#include <stdio.h>
#include <string.h>
#include <sys/dir.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "Prompts/DeviceMenu.h"
#include "devicemounter.h"
#include "network/networkops.h"
#include "vrt.h"

static const s32 VRT_DEVICE_ID = 38744;

typedef struct {
    const char *name;
    const char *alias;
    const char *mount_point;
    const char *prefix;
    bool working;
    const DISC_INTERFACE *disc;
} VIRTUAL_PARTITION;


VIRTUAL_PARTITION VIRTUAL_PARTITIONS[] = {
    { "Front SD", "/sd", "sd", "sd:/", false,  &__io_wiisd }, //SD
    { "USB storage device", "/usb", "usb", "usb:/", false, &__io_usbstorage }, // USB
    { "USB storage device", "/ntfs0", "ntfs0", "ntfs0:/", false, NULL }, // NTFS0
    { "USB storage device", "/ntfs1", "ntfs1", "ntfs1:/", false, NULL }, // NTFS1
    { "USB storage device", "/ntfs2", "ntfs2", "ntfs2:/", false, NULL }, // NTFS2
    { "USB storage device", "/ntfs3", "ntfs3", "ntfs3:/", false, NULL }, // NTFS3
    { "USB storage device", "/ntfs4", "ntfs4", "ntfs4:/", false, NULL }, // NTFS4
    { "SMB storage device", "/smb1", "smb1", "smb1:/", false, NULL }, // SMB1
    { "SMB storage device", "/smb2", "smb2", "smb2:/", false, NULL }, // SMB2
    { "SMB storage device", "/smb3", "smb3", "smb3:/", false, NULL }, // SMB3
    { "SMB storage device", "/smb4", "smb4", "smb4:/", false, NULL }, // SMB4

    { "FTP storage device", "/ftp1", "ftp1", "ftp1:/", false, NULL }, // FTP1
    { "FTP storage device", "/ftp2", "ftp2", "ftp2:/", false, NULL }, // FTP2
    { "FTP storage device", "/ftp3", "ftp3", "ftp3:/", false, NULL }, // FTP3
    { "FTP storage device", "/ftp4", "ftp4", "ftp4:/", false, NULL }, // FTP4
    { "DVD storage device", "/dvd", "dvd", "dvd:/", false, NULL }, // DVD

 // unused partition
    { "SD Gecko A", "/carda", "carda", "carda:/", false, &__io_gcsda },
    { "SD Gecko B", "/cardb", "cardb", "cardb:/", false, &__io_gcsdb },
    { "ISO9660 filesystem", "/dvd", "dvd", "dvd:/", false, NULL },
    { "Wii disc image", "/wod", "wod", "wod:/", false, NULL },
    { "Wii disc filesystem", "/fst", "fst", "fst:/", false, NULL },
    { "NAND images", "/nand", "nand", "nand:/", false, NULL },
    { "NAND filesystem", "/isfs", "isfs", "isfs:/", false, NULL },
    { "OTP filesystem", "/otp", "otp", "otp:/", false, NULL }
};
static const u32 MAX_VIRTUAL_PARTITIONS = (sizeof(VIRTUAL_PARTITIONS) / sizeof(VIRTUAL_PARTITION));


void initialise_virtual_path(void)
{
    u32 i;
    for (i = 0; i < MAX_VIRTUAL_PARTITIONS; i++)
		VIRTUAL_PARTITIONS[i].working = false;

    if(SDCard_Inserted())
    {
	VIRTUAL_PARTITIONS[SD].working = true;
    }

    if(USBDevice_Inserted())
    {
	VIRTUAL_PARTITIONS[USB].working = true;
     }

   for(int i = 0; i < NTFS_GetMountCount(); i++)
    {
	VIRTUAL_PARTITIONS[NTFS0].working = true;
    }

    if(Disk_Inserted())
    {
	VIRTUAL_PARTITIONS[DVD].working = true;
    }

    for(int i = 0; i < 4; i++)
    {
        if(IsSMB_Mounted(i))
        {
		VIRTUAL_PARTITIONS[SMB1+i].working = true;
        }
    }

	for(int i = 0; i < 4; i++)
    {
        if(IsFTPConnected(i))
        {
		VIRTUAL_PARTITIONS[FTP1+i].working = true;
        }
    }

 }

static bool mounted(VIRTUAL_PARTITION *partition) {
    DIR_ITER *dir = diropen(partition->prefix);
    if (dir) {
        dirclose(dir);
        return true;
    }
    return false;
}


static char *virtual_abspath(char *virtual_cwd, char *virtual_path) {
    char *path;
	u32 state;
	u32  end;
    char *curr_dir;
	char *normalised_path;
	char *token;

	if (virtual_path[0] == '/') {
        path = virtual_path;
    } else {
        size_t path_size = strlen(virtual_cwd) + strlen(virtual_path) + 1;
        if (path_size > MAXPATHLEN || !(path = (char *)malloc(path_size))) return NULL;
        strcpy(path, virtual_cwd);
        strcat(path, virtual_path);
    }

    normalised_path = (char *)malloc(strlen(path) + 1);
    if (!normalised_path) goto endprocess;
    *normalised_path = '\0';
    curr_dir = normalised_path;

    state = 0; // 0:start, 1:slash, 2:dot, 3:dotdot
    token = path;
    while (1) {
        switch (state) {
        case 0:
            if (*token == '/') {
                state = 1;
                curr_dir = normalised_path + strlen(normalised_path);
                strncat(normalised_path, token, 1);
            }
            break;
        case 1:
            if (*token == '.') state = 2;
            else if (*token != '/') state = 0;
            break;
        case 2:
            if (*token == '/' || !*token) {
                state = 1;
                *(curr_dir + 1) = '\0';
            } else if (*token == '.') state = 3;
            else state = 0;
            break;
        case 3:
            if (*token == '/' || !*token) {
                state = 1;
                *curr_dir = '\0';
                char *prev_dir = strrchr(normalised_path, '/');
                if (prev_dir) curr_dir = prev_dir;
                else *curr_dir = '/';
                *(curr_dir + 1) = '\0';
            } else state = 0;
            break;
        }
        if (!*token) break;
        if (state == 0 || *token != '/') strncat(normalised_path, token, 1);
        token++;
    }

    end = strlen(normalised_path);
    while (end > 1 && normalised_path[end - 1] == '/') {
        normalised_path[--end] = '\x00';
    }

endprocess:
    if (path != virtual_path) free(path);
    return normalised_path;
}

/*
    Converts a client-visible path to a real absolute path
    E.g. "/sd/foo"    -> "sd:/foo"
         "/sd"        -> "sd:/"
         "/sd/../usb" -> "usb:/"
    The resulting path will fit in an array of size MAXPATHLEN
    Returns NULL to indicate that the client-visible path is invalid
*/
static char *to_real_path(char *virtual_cwd, char *virtual_path) {
    size_t real_path_size;
	const char *prefix;

	errno = ENOENT;
    if (strchr(virtual_path, ':')) {
        return NULL; // colon is not allowed in virtual path, i've decided =P
    }

    virtual_path = virtual_abspath(virtual_cwd, virtual_path);
    if (!virtual_path) return NULL;

    char *path = NULL;
    char *rest = virtual_path;

    if (!strcmp("/", virtual_path)) {
        // indicate vfs-root with ""
        path = (char *) "";
        goto end;
    }

    prefix = NULL;
    u32 i;
    for (i = 0; i < MAX_VIRTUAL_PARTITIONS; i++) {
        VIRTUAL_PARTITION *partition = VIRTUAL_PARTITIONS + i;
        const char *alias = partition->alias;
        size_t alias_len = strlen(alias);
        if (!strcasecmp(alias, virtual_path) || (!strncasecmp(alias, virtual_path, alias_len) && virtual_path[alias_len] == '/')) {
            prefix = partition->prefix;
            rest += alias_len;
            if (*rest == '/') rest++;
            break;
        }
    }
    if (!prefix) {
        errno = ENODEV;
        goto end;
    }

    real_path_size = strlen(prefix) + strlen(rest) + 1;
    if (real_path_size > MAXPATHLEN) goto end;

    path = (char *)malloc(real_path_size);
    if (!path) goto end;
    strcpy(path, prefix);
    strcat(path, rest);

    end:
    free(virtual_path);
    return path;
}

typedef void * (*path_func)(char *path, ...);

static void *with_virtual_path(void *virtual_cwd, void *void_f, char *virtual_path, s32 failed, ...) {
    char *path = (char *)to_real_path((char *)virtual_cwd, virtual_path);
    if (!path || !*path) return (void *)failed;

    path_func f = (path_func)void_f;
    va_list ap;
    void *args[3];
    unsigned int num_args = 0;
    va_start(ap, failed);
    do {
        void *arg = va_arg(ap, void *);
        if (!arg) break;
        args[num_args++] = arg;
    } while (1);
    va_end(ap);

    void *result;
    switch (num_args) {
        case 0: result = f(path); break;
        case 1: result = f(path, args[0]); break;
        case 2: result = f(path, args[0], args[1]); break;
        case 3: result = f(path, args[0], args[1], args[2]); break;
        default: result = (void *)failed;
    }

    free(path);
    return result;
}

static FILE *lvrt_fopen(char *cwd, char *path, char *mode) {
    return (FILE *)with_virtual_path((void *)cwd, (void *)fopen, path, 0, mode, NULL);
}

static int lvrt_stat(char *cwd, char *path, struct stat *st) {
    char *real_path = to_real_path(cwd, path);
    if (!real_path) return -1;
    else if (!*real_path) {
        st->st_mode = S_IFDIR;
        st->st_size = 31337;
        return 0;
    }
    free(real_path);
    return (int)with_virtual_path((void *)cwd, (void *)stat, path, -1, st, NULL);
}

static int lvrt_chdir(char *cwd, char *path) {
    struct stat st;
    if (lvrt_stat(cwd, path, &st)) {
        return -1;
    } else if (!(st.st_mode & S_IFDIR)) {
        errno = ENOTDIR;
        return -1;
    }
    char *abspath = virtual_abspath(cwd, path);
    if (!abspath) {
        errno = ENOMEM;
        return -1;
    }
    strcpy(cwd, abspath);
    if (cwd[1]) strcat(cwd, "/");
    free(abspath);
    return 0;
}

static int lvrt_unlink(char *cwd, char *path) {
    return (int)with_virtual_path((void *)cwd, (void *)unlink, path, -1, NULL);
}

static int lvrt_mkdir(char *cwd, char *path, mode_t mode) {
    return (int)with_virtual_path((void *)cwd, (void *)mkdir, path, -1, mode, NULL);
}

static int lvrt_rename(char *cwd, char *from_path, char *to_path) {
    char *real_to_path = to_real_path(cwd, to_path);
    if (!real_to_path || !*real_to_path) return -1;
    int result = (int)with_virtual_path((void *)cwd, (void *)rename, from_path, -1, real_to_path, NULL);
    free(real_to_path);
    return result;
}

/*
    When in vfs-root this creates a fake DIR_ITER.
 */
static DIR_ITER *lvrt_diropen(const char *cwd, char *path) {
    char *real_path = to_real_path((char *)cwd, path);
    if (!real_path) return NULL;
    else if (!*real_path) {
        DIR_ITER *iter = (DIR_ITER *)malloc(sizeof(DIR_ITER));
        if (!iter) return NULL;
        iter->device = VRT_DEVICE_ID;
        iter->dirStruct = 0;
        return iter;
    }
    free(real_path);
    return (DIR_ITER *)with_virtual_path((void *)cwd, (void *)diropen, path, 0, NULL);
}

/*
    Yields virtual aliases when iter->device == VRT_DEVICE_ID.
 */
static int lvrt_dirnext(DIR_ITER *iter, char *filename, struct stat *st) {
    if (iter->device == VRT_DEVICE_ID) {
        for (; (int)iter->dirStruct < (int) MAX_VIRTUAL_PARTITIONS; iter->dirStruct= ((char*)(iter->dirStruct))+1) {
            VIRTUAL_PARTITION *partition = VIRTUAL_PARTITIONS + (int)iter->dirStruct;
            if (mounted(partition)) {
                st->st_mode = S_IFDIR;
                st->st_size = 0;
                strcpy(filename, partition->alias + 1);
                iter->dirStruct= ((char*)(iter->dirStruct)+1);
                return 0;
            }
        }
        return -1;
    }
    return dirnext(iter, filename, st);
}

static int lvrt_dirclose(DIR_ITER *iter) {
    if (iter->device == VRT_DEVICE_ID) {
        free(iter);
        return 0;
    }
    return dirclose(iter);
}

FILE *vrt_fopen(char *cwd, char *mode)
{
return lvrt_fopen(cwd, (char *)"", mode);
}

int vrt_stat(char *cwd, struct stat *st)
{
return lvrt_stat(cwd, (char *)"", st);
}

int vrt_chdir(char *cwd)
{
return lvrt_chdir(cwd, (char *)"");
}

int vrt_unlink(char *cwd)
{
return lvrt_unlink(cwd, (char *)"");
}

int vrt_mkdir(char *cwd, mode_t mode)
{
return lvrt_mkdir(cwd, (char *)"", mode);
}

int vrt_rename(char *cwd, char *to_path)
{
return lvrt_rename(cwd, (char *)"", to_path);
}

DIR_ITER *vrt_diropen(const char *cwd)
{
return lvrt_diropen(cwd, (char *)"");
}

int vrt_dirnext(DIR_ITER *iter, char *filename, struct stat *st)
{
return lvrt_dirnext(iter, filename, st);
}

int vrt_dirclose(DIR_ITER *iter)
{
return lvrt_dirclose(iter);
}





