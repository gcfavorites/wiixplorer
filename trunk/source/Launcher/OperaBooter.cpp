/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <algorithm>
#include <ogc/wiilaunch.h>
#include "FileOperations/fileops.h"
#include "Language/gettext.h"
#include "Prompts/PromptWindows.h"
#include "network/FileDownloader.h"
#include "Tools/tools.h"
#include "main.h"
#include "sys.h"
#include "OperaBooter.hpp"

#define OPERA_USA   0x0001000148414445LL
#define OPERA_JAP   0x000100014841444ALL
#define OPERA_PAL   0x0001000148414450LL

OperaBooter::OperaBooter(const char * xml)
{
    xmlfile = NULL;
    xmlbuffer = NULL;
    OperaID = 0;
    u64 size = 0;

    if(!xml)
        return;

    FilePath = xml;
    LoadFileToMem(xml, &xmlbuffer, &size);

    if(!xmlbuffer)
    {
        xmlfile = mxmlNewXML("1.0");
        mxmlSetWrapMargin(0);
    }
    else
    {
        xmlfile = mxmlLoadString(NULL, (const char *) xmlbuffer, MXML_OPAQUE_CALLBACK);
    }

    if(!xmlfile)
        return;

    ParseXML();

    if(FindTitle(OPERA_USA))
    {
        OperaID = OPERA_USA;
    }
    else if(FindTitle(OPERA_PAL))
    {
        OperaID = OPERA_PAL;
    }
    else if(FindTitle(OPERA_JAP))
    {
        OperaID = OPERA_JAP;
    }
}

OperaBooter::~OperaBooter()
{
    if(xmlfile)
        mxmlDelete(xmlfile);
    if(xmlbuffer)
        free(xmlbuffer);

    ClearList();
}

bool OperaBooter::AddLink()
{
    int choice = WindowPrompt(tr("Do you want to add an URL?"), 0, tr("Yes"), tr("Cancel"));
    if(!choice)
        return true;

    choice = WindowPrompt(tr("Please enter first the URL-Name."), tr("This name will be shown in the Start Menu"), tr("OK"));
    if(!choice)
        return true;

    char name[150];
    char addr[300];
    bool downloadlink = false;

    choice = OnScreenKeyboard(name, sizeof(name));
    if(!choice)
        return true;

    choice = WindowPrompt(tr("Please enter now the URL."), tr("This link will be used in Opera."), tr("OK"));
    if(!choice)
        return true;

    choice = OnScreenKeyboard(addr, sizeof(addr));
    if(!choice)
        return true;

    choice = WindowPrompt(tr("What kind of URL did you enter?"), 0, tr("Website Link"), tr("Download Link"));
    if(choice == 1)
        downloadlink = false;
    else
        downloadlink = true;

    return AddLink(name, addr, downloadlink);
}

bool OperaBooter::AddLink(const char * name, const char * addr, bool downloadlink)
{
    if(!xmlfile)
        return false;

    mxml_node_t * url = mxmlNewElement(xmlfile, "url");
    mxmlElementSetAttr(url, "name", name);
    mxmlElementSetAttr(url, "addr", addr);
    mxmlElementSetAttr(url, "downloadlink", fmt("%i", downloadlink));

    int position = LinkList.size();

    LinkList.resize(position+1);

    LinkList[position].name = strdup(name);
    LinkList[position].addr = strdup(addr);
    LinkList[position].downloadlink = downloadlink;

	if(LinkList.size() > 1)
        Sort();

    return SaveXML();
}

void OperaBooter::RemoveLink(int pos)
{
    if(pos < 0 || pos >= (int) LinkList.size())
        return;

    if(LinkList[pos].name)
        free(LinkList[pos].name);
    if(LinkList[pos].addr)
        free(LinkList[pos].addr);

    LinkList.erase(LinkList.begin()+pos);

	if(LinkList.size() > 1)
        Sort();

    SaveXML();
}

bool OperaBooter::ParseXML()
{
    if(!xmlfile)
        return false;

    ClearList();

    mxml_node_t * node = mxmlFindElement(xmlfile, xmlfile, "url", NULL, NULL, MXML_DESCEND_FIRST);
    if (node == NULL)
        return false;

    u32 position = 0;

	while(node != NULL)
	{
		const char * name = mxmlElementGetAttr(node, "name");
		const char * addr = mxmlElementGetAttr(node, "addr");
		const char * linkformat = mxmlElementGetAttr(node, "downloadlink");
        bool downloadlink = linkformat ? (atoi(linkformat) != 0) : false;

		if(name && addr)
		{
            LinkList.resize(position+1);

            LinkList[position].name = strdup(name);
            LinkList[position].addr = strdup(addr);
            LinkList[position].downloadlink = downloadlink;
            ++position;
		}

		node = mxmlFindElement(node, xmlfile, "url", NULL, NULL, MXML_NO_DESCEND);
	}

	if(LinkList.size() > 1)
        Sort();

    return true;
}

void OperaBooter::ClearList()
{
    for(u32 i = 0; i < LinkList.size(); i++)
    {
        if(LinkList[i].name)
            free(LinkList[i].name);
        if(LinkList[i].addr)
            free(LinkList[i].addr);
    }
    LinkList.clear();
    std::vector<Link>().swap(LinkList);
}

bool OperaBooter::SaveXML()
{
    mxml_node_t * newfile = mxmlNewXML("1.0");
    if(!newfile)
        return false;

    mxmlSetWrapMargin(0);

    FILE *f = fopen(FilePath.c_str(), "wb");
    if(!f)
    {
        mxmlDelete(newfile);
        ShowError("%s %s", tr("Cannot write to this path:"), FilePath.c_str());
        return false;
    }

    for(u32 i = 0; i < LinkList.size(); i++)
    {
        mxml_node_t * url = mxmlNewElement(newfile, "url");
        mxmlElementSetAttr(url, "name", LinkList[i].name);
        mxmlElementSetAttr(url, "addr", LinkList[i].addr);
        mxmlElementSetAttr(url, "downloadlink", fmt("%i", LinkList[i].downloadlink));
    }

    mxmlSaveFile(newfile, f, NULL);
    fclose(f);

    mxmlDelete(xmlfile);
    xmlfile = newfile;

    return true;
}

const char * OperaBooter::GetName(int pos)
{
    if(pos < 0 || pos >= (int) LinkList.size())
        return NULL;

    return LinkList[pos].name;
}

const char * OperaBooter::GetLink(int pos)
{
    if(pos < 0 || pos >= (int) LinkList.size())
        return NULL;

    return LinkList[pos].addr;
}

bool OperaBooter::Launch(int pos)
{
    if(pos < 0 || pos >= (int) LinkList.size())
        return false;

    if(LinkList[pos].downloadlink)
        return this->DownloadFile(pos);

    if(OperaID == 0)
    {
        ShowError(tr("Opera Channel not found on the system."));
        return false;
    }

    ExitApp();

    int ret = WII_LaunchTitleWithArgs(OperaID, 0, LinkList[pos].addr, NULL);
    if(ret < 0)
	{
        ShowError(tr("Failed to load the channel."));
		return false;
	}

	return true;
}

bool OperaBooter::DownloadFile(int pos)
{
    if(pos < 0 || pos >= (int) LinkList.size())
        return false;

    char url[1024];
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), Settings.LastUsedPath.c_str());

    int choice = WindowPrompt(tr("Please enter the a path."), tr("The file will be downloaded to this path."), tr("OK"));
    if(!choice)
        return true;

    choice = OnScreenKeyboard(filepath, sizeof(filepath));
    if(!choice)
        return true;

    if(strncasecmp(LinkList[pos].addr, "http://", strlen("http://")) != 0)
        snprintf(url, sizeof(url), "http://%s", LinkList[pos].addr);
    else
        snprintf(url, sizeof(url), "%s", LinkList[pos].addr);

    return (DownloadFileToPath(url, filepath, true) >= 0);
}

void OperaBooter::Sort()
{
    std::sort(LinkList.begin(), LinkList.end(), SortCallback);
}

bool OperaBooter::SortCallback(const Link & f1, const Link & f2)
{
    if(stricmp(f1.name, f2.name) > 0)
        return false;
    else
        return true;
}
