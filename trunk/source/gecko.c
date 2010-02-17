#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#ifdef GEKKO
#include <stdarg.h>

static bool geckoinit = false;

bool InitGecko()
{
    if(geckoinit)
        return true;

	u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
	if (geckoattached)
	{
		usb_flush(EXI_CHANNEL_1);
		geckoinit = true;
		return true;
	}
	else
        return false;
}

void gprintf(const char * format, ...)
{
	if (!geckoinit)
        return;

	char * tmp = NULL;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        usb_sendbuffer_safe(1, tmp, strlen(tmp));
		free(tmp);
	}
	va_end(va);
}

#endif
