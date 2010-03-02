#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogc/isfs.h>
#include "Channels.h"

#include "certs.h"

#define ALIGN(x) ((x % 32 != 0) ? (x / 32) * 32 + 32 : x)

Channels::Channels()
{
	Search();
}

Channels::~Channels()
{
}

u64* Channels::GetChannels(u32* count)
{
	u32 countall;
	u32 ret = ES_GetNumTitles(&countall);

	if (ret || !countall)
		return NULL;

	u64* listall = (u64*)memalign(32, countall * sizeof(u64));
	if (!listall)
		return NULL;

	u64* list = (u64*)malloc(countall * sizeof(u64));
	if (!list)
	{
		free(listall);
		return NULL;
	}

	ret = ES_GetTitles(listall, countall);

	*count = 0;
	for (u32 i = 0; i < countall; i++)
	{
		u32 type = listall[i] >> 32;

		if (type == 0x00010001 || type == 0x00010002)
		{
			if ((listall[i] & 0xFFFFFFFF) == 0x48414741 || (listall[i] & 0xFFFFFFFF) == 0x48414641)
				continue; // region free news and forecast channel

			list[(*count)++] = listall[i];
		}
	}

	free(listall);

	return (u64*)realloc(list, *count * sizeof(u64));
}

bool GetApp(u64 title, char *app)
{
	char tmd[ISFS_MAXPATH];
	static fstats stats ATTRIBUTE_ALIGN(32);

	u32 high = (u32)(title >> 32);
	u32 low  = (u32)(title & 0xFFFFFFFF);

	bool ret = false;

	sprintf(tmd, "/title/%08x/%08x/content/title.tmd", high, low);

	s32 fd = ISFS_Open(tmd, ISFS_OPEN_READ);
	if (fd >= 0)
	{
		if (ISFS_GetFileStats(fd, &stats) >= 0)
		{
			char* data = (char*)memalign(32, ALIGN(stats.file_length));
			if (data)
			{
				if (ISFS_Read(fd, data, stats.file_length) > 0x208)
				{
					sprintf(app, "/title/%08x/%08x/content/000000%02x.app\n", high, low, data[0x1e7]);
					ret = true;
				}
				free(data);
			}
		}
		ISFS_Close(fd);
	}

	return ret;
}

bool Channels::GetNameFromApp(u64 title, char *name)
{
	char app[ISFS_MAXPATH];
	static char buffer[0x200] ATTRIBUTE_ALIGN(32);

	s32 ret = -1;

	if (!GetApp(title, app))
		return false;

	s32 fd = ISFS_Open(app, ISFS_OPEN_READ);
	if (fd >= 0)
	{
		if (ISFS_Read(fd, buffer, 0x200) > 0x140)
		{
			if (memcmp((void*)(buffer+0x80), "IMET", 4) == 0)
			{
				for (int i = 0xf1; buffer[i] != 0; i += 2)
				{
					name[++ret] = buffer[i];
				}
				name[++ret] = 0;
			}
		}
		ISFS_Close(fd);
	}

	return (ret > 0);
}

void Channels::Launch(Channel channel)
{
	static u32 count ATTRIBUTE_ALIGN(32);

	ES_GetNumTicketViews(channel.title, &count);
	tikview *views = (tikview*)memalign(32, sizeof(tikview) * count);
	ES_GetTicketViews(channel.title, views, count);

	ES_LaunchTitle(channel.title, views);
}

void Channels::Search()
{
	u32 count;
	u64 *list = GetChannels(&count);
	if (!list) return;

	char name[256];

	ES_Identify((u32*)Certificates, sizeof(Certificates), (u32*)Tmd, sizeof(Tmd), (u32*)Ticket, sizeof(Ticket), 0);

	ISFS_Initialize();

	for (u32 i = 0; i < count; i++)
	{
		if (GetNameFromApp(list[i], name))
		{
			Channel channel;
			strcpy(channel.name, name);
			channel.title = list[i];

			channels.push_back(channel);
		}
	}

	ISFS_Deinitialize();

	free(list);
}
