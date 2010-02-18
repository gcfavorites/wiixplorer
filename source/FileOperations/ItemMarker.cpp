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
 * ItemMarker Class
 *
 * for Wii-FileXplorer 2010
 ***************************************************************************/
#include "ItemMarker.h"

ItemMarker::ItemMarker()
{
    Reset();
}

ItemMarker::~ItemMarker()
{
    Reset();
}

void ItemMarker::AddItem(const ItemStruct * file)
{
    if(!file)
        return;

    else if(strlen(file->itempath) == 0)
        return;

    else if(FindItem(file) >= 0)
        return;

    char * filename = strrchr(file->itempath, '/');

    if(filename)
    {
        filename++;
        if(strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
            return;
    }

    ItemStruct tmpClip;
    memcpy(&tmpClip, file, sizeof(ItemStruct));

    Items.push_back(tmpClip);
}

ItemStruct * ItemMarker::GetItem(int ind)
{
    if(ind < 0 || ind >= (int) Items.size())
        return NULL;

    return (ItemStruct *) &Items.at(ind);
}

const char * ItemMarker::GetItemName(int ind)
{
    if(ind < 0 || ind >= (int) Items.size())
        return NULL;

    char * filename = strrchr(Items.at(ind).itempath, '/');

    if(!filename)
        return NULL;

    return (const char *) filename+1;
}

const char * ItemMarker::GetItemPath(int ind)
{
    if(ind < 0 || ind >= (int) Items.size())
        return NULL;

    return (const char *) &Items.at(ind).itempath;
}

u64 ItemMarker::GetItemSize(int ind)
{
    if(ind < 0 || ind >= (int) Items.size())
        return 0;

    return Items.at(ind).itemsize;
}

bool ItemMarker::IsItemDir(int ind)
{
    if(ind < 0 || ind >= (int) Items.size())
        return false;

    return Items.at(ind).isdir;
}

int ItemMarker::GetItemIndex(int ind)
{
    if(ind < 0 || ind >= (int) Items.size())
        return -1;

    return Items.at(ind).itemindex;
}

int ItemMarker::FindItem(const ItemStruct * Item)
{
    if(!Item)
        return -1;

    for(int i = 0; i < (int) Items.size(); i++)
    {
        if(strcasecmp(Item->itempath, Items.at(i).itempath) == 0)
        {
            return i;
        }
    }

    return -1;
}

void ItemMarker::RemoveItem(const ItemStruct * Item)
{
    if(!Item)
        return;

    int num = FindItem(Item);

    if(num < 0)
        return;

    Items.erase(Items.begin()+num);
}

void ItemMarker::Reset()
{
    Items.clear();
}
