#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>

#include "network/networkops.h"
#include "devicemounter.h"
#include "dolloader.h"
#include "elfloader.h"

static u8 *homebrewbuffer = (u8 *)0x92000000;
static u32 homebrewsize = 0;


int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len)
{
    if(homebrewbuffer == (u8 *)0x92000000)
    {
        homebrewbuffer = (u8 *) malloc(len);
        homebrewsize = len;
    }
    else
    {
        homebrewsize += len;
        u8 *tempbuffer = (u8 *) realloc(homebrewbuffer, homebrewsize);
        if(!tempbuffer)
        {
            if(homebrewbuffer) {
                free(homebrewbuffer);
                homebrewbuffer = (u8 *)0x92000000;
            }
            homebrewsize = 0;
            return -1;
        } else {
            homebrewbuffer = tempbuffer;
        }
    }

    memcpy((homebrewbuffer)+pos, temp, len);

    return 1;
}

void FreeHomebrewBuffer()
{
    if(homebrewbuffer)
    {
        free(homebrewbuffer);
        homebrewbuffer = (u8 *)0x92000000;
    }
}

int BootHomebrew(const char *path, const char * filereference) {

    entrypoint entry;
    u32 cpu_isr;

    if (homebrewbuffer == (u8 *)0x92000000 || homebrewsize == 0) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

    struct __argv args;
    bzero(&args, sizeof(args));
    args.argvMagic = ARGV_MAGIC;
    u32 stringlength = strlen(path) + (filereference ? (strlen(filereference)+1) : 0) + 2;
    args.length = stringlength;
    args.commandLine = (char*)malloc(args.length);
    if (!args.commandLine)
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    sprintf(args.commandLine, "%s", path);
    if(filereference)
    {
        sprintf(&args.commandLine[strlen(path)+1], "%s", filereference);
        args.argc = 2;
    }
    else
        args.argc = 1;
    args.commandLine[args.length - 1] = '\0';
    args.argv = &args.commandLine;
    args.endARGV = args.argv + 1;

    int ret = valid_elf_image(homebrewbuffer);
    if (ret == 1)
        entry = (entrypoint) load_elf_image(homebrewbuffer);
    else
        entry = (entrypoint) load_dol(homebrewbuffer, &args);

    FreeHomebrewBuffer();

    if (!entry)
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable (cpu_isr);
    __exception_closeall();
    entry();
    _CPU_ISR_Restore (cpu_isr);

    return 0;
}
