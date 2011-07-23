#include "mload/mload.h"
#include "ehcmodule_5.h"
#include "usb2storage.h"

static int mload_thread_id = -1;

int mload_Init()
{
    if (mload_init() >= 0 && mload_thread_id < 0)
    {
        data_elf my_data_elf;
        mload_elf(ehcmodule_5, &my_data_elf);
		mload_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
    	USB2Enable(true);
    }

    return mload_thread_id;
}

void mload_DeInit()
{
	mload_close();
}
