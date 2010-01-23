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

extern const u8		app_booter_dol[];
extern const u32	app_booter_dol_size;

static u8 *homebrewbuffer = (u8 *) 0x92000000;
static u32 homebrewsize = 0;

int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len)
{
    homebrewsize += len;
    memcpy((homebrewbuffer)+pos, temp, len);

    return 1;
}

void FreeHomebrewBuffer()
{
    homebrewbuffer = (u8 *)0x92000000;
    homebrewsize = 0;
}

int BootHomebrew(const char *path, const char * filereference)
{
    if(homebrewsize == 0)
        return -1;

    entrypoint entry;
    u32 cpu_isr;

    u8 * buffer = (u8*) malloc(app_booter_dol_size);
    if(!buffer)
        return -1;

    memcpy(buffer, app_booter_dol, app_booter_dol_size);

    struct __argv args;
    bzero(&args, sizeof(args));
    args.argvMagic = ARGV_MAGIC;
    u32 stringlength = strlen(path) + (filereference ? (strlen(filereference)+1) : 0) + 2;
    args.length = stringlength;
    args.commandLine = (char*) malloc(args.length);
    if (!args.commandLine)
        return -1;
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

    entry = (entrypoint) load_dol(buffer, &args);

    if(buffer)
        free(buffer);

    if (!entry)
        return -1;

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable (cpu_isr);
    __exception_closeall();
    entry();
    _CPU_ISR_Restore (cpu_isr);

    return 0;
}
