/***************************************************************************
 * Copyright (C) 2010
 * by dude
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Application Launcher Class
 *
 * for WiiXplorer 2010
 ***************************************************************************/

#include <mxml.h>

#include "Applications.h"
#include "DirList.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Controls/Clipboard.h"
#include "Controls/Taskbar.h"
#include "menu.h"

extern bool boothomebrew;

Applications *Applications::instance = NULL;

Applications::Applications()
{
	Search();
}

Applications::~Applications()
{
}

Applications * Applications::Instance()
{
	if (instance == NULL)
	{
		instance = new Applications();
	}
	return instance;
}

void Applications::DestroyInstance()
{
	if (instance)
	{
		delete instance;
	}
	instance = NULL;
}

void Applications::Launch(int index)
{
	if (LoadHomebrew(applications.at(index).path) < 0)
		return;

	ItemStruct Item;
	memset(&Item, 0, sizeof(ItemStruct));
	snprintf(Item.itempath, sizeof(Item.itempath), "%s", applications.at(index).path);
	Clipboard::Instance()->AddItem(&Item);
	boothomebrew = true;
	Taskbar::Instance()->SetMenu(MENU_EXIT);
}

mxml_error_cb_t xmlerror(const char* error)
{
	return NULL;
}

bool Applications::GetNameFromXML(char *xml, char *name)
{
	mxml_node_t *tree = NULL;
	mxml_node_t *data = NULL;

	bool ret = false;

	FILE *fp = fopen(xml, "rb");
	if (fp)
	{
		mxmlSetErrorCallback((mxml_error_cb_t)xmlerror);

		tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

		fclose(fp);
		if (tree)
		{
			data = mxmlFindElement(tree, tree, "name", NULL, NULL, MXML_DESCEND);
			if (data)
			{
				char *ptr = name;

				while (data->child != NULL && ptr < name+255)
				{
					if (data->child->value.text.whitespace)
						*ptr++ = ' ';

					strcpy(ptr, data->child->value.text.string);
					ptr += strlen(data->child->value.text.string);
					data->child = data->child->next;
				}
				*ptr = 0;

				mxmlDelete(data);
				ret = true;
			}

			mxmlDelete(tree);
		}
	}

	return ret;
}

void Applications::Reload()
{
	applications.clear();
	Search();
}

void Applications::Search()
{
	char apppath[256], hbpath[256], hbname[256], metaname[256];

	if (strcmp(Settings.AppPath, "") == 0)
		return;

	snprintf(apppath, sizeof(apppath), "%s", Settings.AppPath);

	DirList dir(apppath);

	int entries = dir.GetFilecount();

    if (entries > 0)
    {
        for (int j = 0; j < entries; j++)
        {
			if (!dir.IsDir(j))
				continue;

			snprintf(hbpath, sizeof(hbpath), "%s%s", apppath, dir.GetFilename(j));

            DirList binary(hbpath, ".dol,.elf");
            if (binary.GetFilecount() > 0)
            {
                DirList meta(hbpath, ".xml");
                if (meta.GetFileIndex("meta.xml") >= 0)
                {
                    snprintf(metaname, sizeof(metaname), "%s/meta.xml", hbpath);

                    if (!GetNameFromXML(metaname, hbname))
                    {
                        strncpy(hbname, dir.GetFilename(j), sizeof(hbname));
                    }
                }
                else
                {
                    strncpy(hbname, dir.GetFilename(j), sizeof(hbname));
                }

                Application app;
                snprintf(app.path, sizeof(app.path), "%s/%s", binary.GetFilepath(0), binary.GetFilename(0));
                strncpy(app.name, hbname, sizeof(app.name));

                applications.push_back(app);
            }
        }
    }
}
