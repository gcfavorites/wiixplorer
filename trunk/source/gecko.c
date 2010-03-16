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

void gsenddata(const u8 *data, int length, const char *filename)
{
	if (!(geckoinit))return;

	// First, send a "\x1b[2B]" line (this will tell geckoreader that binary data is comming up next)
	const char *binary_data = "\x1b[2B]\n";

	usb_sendbuffer_safe(1, binary_data, strlen(binary_data));

	u8 filenamelength = filename == NULL ? 0 : strlen(filename);

	// Send the length
	usb_sendbuffer_safe(1, (u8 *) &length, 4);
	usb_sendbuffer_safe(1, (u8 *) &filenamelength, 1);
	usb_sendbuffer_safe(1, data, length);
	if (filename != NULL)
	{
		usb_sendbuffer_safe(1, filename, strlen(filename));
	}
}

char ascii(char s)
{
    if(s < 0x20)
        return '.';
    if(s > 0x7E)
        return '.';
    return s;
}

void ghexdump(void *d, int len)
{
    u8 *data;
    int i, off;
    data = (u8*)d;

    gprintf("\n       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  0123456789ABCDEF");
    gprintf("\n====  ===============================================  ================\n");

    for (off=0; off<len; off += 16)
    {
        gprintf("%04x  ",off);
        for(i=0; i<16; i++)
            if((i+off)>=len)
                gprintf("   ");
        else
            gprintf("%02x ",data[off+i]);

        gprintf(" ");
        for(i=0; i<16; i++)
            if((i+off)>=len)
                gprintf(" ");
            else
                gprintf("%c",ascii(data[off+i]));
        gprintf("\n");
    }
}

#endif
