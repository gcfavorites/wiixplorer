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
#include "GXConsole.hpp"

GXConsole::GXConsole(int w, int h)
{
    fontSize = 16;
    HeightBetweenLines = 6;
    Background = NULL;
    width = w;
    height = h;
    RowCount = (u32) floor(height/(fontSize+HeightBetweenLines));
    color = (GXColor) {0, 0, 0, 255};
	style = FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP;
	LWP_MutexInit(&mutex, true);

    if(!fontSystem[fontSize])
    {
        fontSystem[fontSize] = new FreeTypeGX(fontSize);
    }
}

GXConsole::~GXConsole()
{
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    clear();
	LWP_MutexDestroy(mutex);
}

void GXConsole::printf(const char *format, ...)
{
    if(!format)
        return;

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
	    if(strcmp(tmp, "\x1b[2;0H") == 0)
	    {
            clear();
	    }
	    else
	    {
            wchar_t * wtext = new wchar_t[strlen(tmp)+1];
            char2wchar_t(tmp, wtext);
            AddText(wtext);
            delete [] wtext;
	    }

		free(tmp);
	}
	va_end(va);
}

void GXConsole::AddText(const wchar_t * inText)
{
    if(!inText)
        return;

    wString * Text = NULL;

    if(ConsoleRow.size() > 0)
    {
        Text = ConsoleRow.at(ConsoleRow.size()-1);
    }
    else
    {
        Text = new wString();
        AddRow(Text);
    }

    const wchar_t * ptr = inText;

    while(ptr[0] != 0)
    {
        if(IsMaxWidth(Text))
        {
            wString * newString = new wString();
            Text = newString;
            AddRow(Text);
        }
        if(ptr[0] == '\t')
        {
            Text->push_back(' ');
            if(IsMaxWidth(Text))
            {
                wString * newString = new wString();
                Text = newString;
                AddRow(Text);
            }
            Text->push_back(' ');
            if(IsMaxWidth(Text))
            {
                wString * newString = new wString();
                Text = newString;
                AddRow(Text);
            }
            Text->push_back(' ');
            if(IsMaxWidth(Text))
            {
                wString * newString = new wString();
                Text = newString;
                AddRow(Text);
            }
            ++ptr;
            continue;
        }
        else if(ptr[0] == '\n')
        {
            wString * newString = new wString();
            Text = newString;
            AddRow(Text);
            ++ptr;
            continue;
        }

        Text->push_back(ptr[0]);

        ++ptr;
    }
}

bool GXConsole::IsMaxWidth(const wString * text)
{
    int currWidth = fontSystem[fontSize]->getWidth(text->c_str());

    if(currWidth >= width)
    {
        return true;
    }

    return false;
}

void GXConsole::AddRow(const wString * text)
{
    if(ConsoleRow.size() > RowCount)
    {
        RemoveRow(0);
    }

	LWP_MutexLock(mutex);
    ConsoleRow.push_back((wString *) text);
	LWP_MutexUnlock(mutex);
}

void GXConsole::RemoveRow(int row)
{
    if(row < 0 || row >= (int) ConsoleRow.size())
        return;

	LWP_MutexLock(mutex);
    if(ConsoleRow.at(row))
        delete ConsoleRow.at(row);

    ConsoleRow.at(row) = NULL;

    ConsoleRow.erase(ConsoleRow.begin()+row);
	LWP_MutexUnlock(mutex);
}

void GXConsole::clear()
{
	LWP_MutexLock(mutex);
    for(u32 i = 0; i < ConsoleRow.size(); i++)
    {
        if(ConsoleRow.at(i))
            delete ConsoleRow.at(i);

        ConsoleRow.at(i) = NULL;
    }

    ConsoleRow.clear();
	LWP_MutexUnlock(mutex);
}

void GXConsole::SetImage(GuiImage * img)
{
    Background = img;

    if(Background)
    {
        Background->SetParent(this);
    }
}

void GXConsole::SetTextColor(GXColor c)
{
    color = c;
}

void GXConsole::SetFontSize(int size)
{
    fontSize = size;
    RowCount = (u32) floor(height/(fontSize+HeightBetweenLines));

    if(!fontSystem[fontSize])
    {
        fontSystem[fontSize] = new FreeTypeGX(fontSize);
    }
}

void GXConsole::SetHeightBetweenLines(int h)
{
    HeightBetweenLines = h;
    RowCount = (u32) floor(height/(fontSize+HeightBetweenLines));
}

void GXConsole::Draw()
{
    if(Background)
        Background->Draw();

    if(!fontSystem[fontSize])
        fontSystem[fontSize] = new FreeTypeGX(fontSize);

    int PositionX = GetLeft();
    int PositionY = GetTop();
    int PositionZ = GetZPosition();

    for(u32 i = 0; i < ConsoleRow.size(); i++)
    {
        fontSystem[fontSize]->drawText(PositionX, PositionY+i*(HeightBetweenLines+fontSize), PositionZ, ConsoleRow.at(i)->c_str(), color, style);
    }
}

