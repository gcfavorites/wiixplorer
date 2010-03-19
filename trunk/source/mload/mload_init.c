#include "mload/mload.h"
#include "filelist.h"


static int mload_thread_id = -1;

int mload_Init()
{
    int ret = -1;

    ret = mload_init();
    if (ret >= 0)
    {
        data_elf my_data_elf;
        mload_elf((void *) ehcmodule_elf, &my_data_elf);
        mload_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, 0x47);
    }

    return mload_thread_id;
}

void mload_DeInit()
{
	mload_stop_thread(mload_thread_id);
	mload_close();
}
