#include <stdarg.h>
#include "Controls/GXConsole.hpp"
#include "input.h"

static GXConsole * Console = NULL;

extern "C" void gxprintf(const char * format, ...)
{
	if(!Console || shutdown || reset)
		return;

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		Console->printf(tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

extern "C" void SetGXConsole(void * console)
{
	Console = (GXConsole *) console;
}
