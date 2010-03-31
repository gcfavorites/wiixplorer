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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gctypes.h>
#include "OptionList.hpp"

OptionList::OptionList()
{
}

OptionList::~OptionList()
{
    ClearList();
}

void OptionList::SetName(int i, const char *format, ...)
{
    if(i < (int) name.size())
    {
        if(name.at(i))
            delete [] name.at(i);

        name.at(i) = NULL;
    }

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
	    if(i >= (int) name.size())
	    {
	        Resize(i+1);
	    }

		name.at(i) = new char[strlen(tmp)+1];
		sprintf(name.at(i), tmp);
		free(tmp);

        listChanged = true;
	}
	va_end(va);
}
void OptionList::SetValue(int i, const char *format, ...)
{
    if(i < (int) value.size())
    {
        if(value.at(i))
            delete [] value.at(i);

        value.at(i) = NULL;
    }

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
	    if(i >= (int) value.size())
	    {
	        Resize(i+1);
	    }

		value.at(i) = new char[strlen(tmp)+1];
		sprintf(value.at(i), tmp);
		free(tmp);

        listChanged = true;
	}
	va_end(va);
}

const char * OptionList::GetName(int i)
{
    if(i < 0 || i >= (int) name.size())
        return NULL;

    return name.at(i);
}

const char * OptionList::GetValue(int i)
{
    if(i < 0 || i >= (int) value.size())
        return NULL;

    return value.at(i);
}

void OptionList::Resize(int size)
{
    while(size < (int) name.size())
    {
        RemoveOption(name.size()-1);
    }

    int oldsize = name.size();

    name.resize(size);
    value.resize(size);

    for(u32 i = oldsize; i < name.size(); i++)
    {
        name.at(i) = NULL;
        value.at(i) = NULL;
    }
}

void OptionList::RemoveOption(int i)
{
    if(i < 0 || i >= (int) name.size())
        return;

    if(name.at(i))
    {
        delete [] name.at(i);
        name.at(i) = NULL;
    }
    if(value.at(i))
    {
        delete [] value.at(i);
        value.at(i) = NULL;
    }

    name.erase(name.begin()+i);
    value.erase(value.begin()+i);
}

void OptionList::ClearList()
{
	for (u32 i = 0; i < name.size(); i++)
	{
	    if(name.at(i))
	    {
            delete [] name.at(i);
            name.at(i) = NULL;
	    }
	}
	for (u32 i = 0; i < value.size(); i++)
	{
	    if(value.at(i))
	    {
            delete [] value.at(i);
            value.at(i) = NULL;
	    }
	}

	name.clear();
	value.clear();
}
