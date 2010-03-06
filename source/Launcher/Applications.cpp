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
#include "FileOperations/filebrowser.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Controls/Clipboard.h"
#include "Controls/Taskbar.h"
#include "menu.h"

extern bool boothomebrew;

Applications::Applications()
{
	Search();
}

Applications::~Applications()
{
}

void Applications::Launch(Application app)
{
	if (LoadHomebrew(app.path) < 0)
		return;

	ItemStruct Item;
	memset(&Item, 0, sizeof(ItemStruct));
	snprintf(Item.itempath, sizeof(Item.itempath), "%s", app.path);
	Clipboard::Instance()->AddItem(&Item);
	boothomebrew = true;
	Taskbar::Instance()->SetMenu(MENU_EXIT);
}

bool Applications::GetNameFromXML(char *xml, char *name)
{
	mxml_node_t *tree;
	mxml_node_t *data;

	bool ret = false;

	FILE *fp = fopen(xml, "rb");
	if (fp)
	{
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

void Applications::Search()
{
	char apppath[256], hbpath[256], hbname[256], metaname[256];

	FileBrowser browser;

    snprintf(apppath, sizeof(apppath), "%s", Settings.AppPath);

    int entries = browser.BrowsePath(apppath);

    if (entries > 1)
    {
        for (int j = 1; j < entries; j++)
        {
            sprintf(hbpath, "%s%s", apppath, browser.GetItemFilename(j));

            DirList binary(hbpath, ".dol,.elf");
            if (binary.GetFilecount() > 0)
            {
                DirList meta(hbpath, ".xml");
                if (meta.GetFileIndex("meta.xml") >= 0)
                {
                    sprintf(metaname, "%s/meta.xml", hbpath);

                    if (!GetNameFromXML(metaname, hbname))
                    {
                        strcpy(hbname, browser.GetItemFilename(j));
                    }
                }
                else
                {
                    strcpy(hbname, browser.GetItemFilename(j));
                }

                Application app;
                sprintf(app.path, "%s/%s", binary.GetFilepath(0), binary.GetFilename(0));
                strcpy(app.name, hbname);

                applications.push_back(app);
            }
        }
    }
}
