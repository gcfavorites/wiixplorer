#include "mload_modules.h"
#include "usb2storage.h"
#include "mload/modules/ehcmodule_2.h"
#include "mload/modules/ehcmodule_3.h"
#include "mload/modules/ehcmodule_5.h"
#include "mload/modules/dip_plugin_2.h"
#include "mload/modules/dip_plugin_3.h"
#include "mload/modules/opendip.h"

int mload_Init()
{
    int ret = -1;

    const u8 * ehcmodule = 0;
    u32 ehcmodule_size = 0;
    const u8 * dipmodule = 0;
    u32 dipmodule_size = 0;

    switch(IOS_GetRevision())
    {
        case 2:
            ehcmodule = ehcmodule_2;
            ehcmodule_size = ehcmodule_2_size;
            dipmodule = dip_plugin_2;
            dipmodule_size = dip_plugin_2_size;
            break;
        case 3:
            ehcmodule = ehcmodule_3;
            ehcmodule_size = ehcmodule_3_size;
            dipmodule = dip_plugin_3;
            dipmodule_size = dip_plugin_3_size;
            break;
        case 4:
        default:
            ehcmodule = ehcmodule_5;
            ehcmodule_size = ehcmodule_5_size;
            dipmodule = opendip;
            dipmodule_size = opendip_size;
            break;
    }

    u8 *ehc_cfg = search_for_ehcmodule_cfg((u8 *) ehcmodule, ehcmodule_size);
    if (ehc_cfg)
    {
        ehc_cfg += 12;
        ehc_cfg[0] = 0; // USB Port 0
    }

    ret = load_modules(ehcmodule, ehcmodule_size, dipmodule, dipmodule_size);
    if(ret >= 0)
        USB2Enable(true);

    return ret;
}

void mload_DeInit()
{
	mload_close();
}
