#include "mload/mload.h"
#include "ehcmodule_5.h"
#include "usb2storage.h"

static int mload_thread_id = -1;
static data_elf my_data_elf;

int mload_Init()
{
	if (mload_init() >= 0 && mload_thread_id < 0)
	{
		mload_elf(ehcmodule_5, &my_data_elf);
		mload_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
		mload_close();
	}

	return mload_thread_id;
}
