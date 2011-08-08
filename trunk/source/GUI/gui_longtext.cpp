/****************************************************************************
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
#include "gui_longtext.hpp"
#include "Tools/tools.h"
#include "FreeTypeGX.h"

extern FreeTypeGX * fontSystem;

GuiLongText::GuiLongText(const char * t, int s, GXColor c)
	:	GuiText(t, s, c)
{
	maxWidth = 400;
	linestodraw = 9;
	curLineStart = 0;
	FirstLineOffset = 0;
	wText = NULL;

	if(!text)
		return;

	wText = new (std::nothrow) wString(text);
	if(!wText)
	{
		ShowError(tr("Not enough memory."));
		return;
	}

	if(wText->size() == 0)
	{
		wText->push_back(L' ');
		wText->push_back(0);
	}

	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);
	delete [] text;
	text = NULL;

	SetMaxWidth(maxWidth);
}

GuiLongText::GuiLongText(const wchar_t * t, int s, GXColor c)
	:	GuiText((wchar_t *) NULL, s, c)
{
	maxWidth = 400;
	linestodraw = 9;
	curLineStart = 0;
	FirstLineOffset = 0;
	wText = NULL;

	if(!t)
		return;

	wText = new (std::nothrow) wString(t);
	if(!wText)
	{
		ShowError(tr("Not enough memory."));
		return;
	}

	if(wText->size() == 0)
	{
		wText->push_back(L' ');
		wText->push_back(0);
	}

	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);

	SetMaxWidth(maxWidth);
}

GuiLongText::~GuiLongText()
{
	if(wText)
		delete wText;
	wText = NULL;

	TextLines.clear();
	ClearDynamicText();
}

void GuiLongText::SetText(const char * t)
{
	wchar_t * tmp = charToWideChar(t);
	if(!tmp)
		return;

	if(wText)
		delete wText;

	wText = new (std::nothrow) wString(tmp);
	if(!wText)
	{
		ShowError(tr("Not enough memory."));
		return;
	}

	if(wText->size() == 0)
	{
		wText->push_back(L' ');
		wText->push_back(0);
	}

	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);

	delete [] tmp;

	ClearDynamicText();
	CalcLineOffsets();
}

void GuiLongText::SetText(const wchar_t * t)
{
	if(!t)
		return;

	if(wText)
		delete wText;

	wText = new wString(t);
	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);
	CalcLineOffsets();
}

void GuiLongText::SetMaxWidth(int w)
{
	maxWidth = w;
	curLineStart = 0;
	Refresh();
}

void GuiLongText::SetTextLine(int line)
{
	curLineStart = LIMIT(line, 0, (int) TextLines.size()-1);

	FillRows();

	while((int) textDyn.size() < linestodraw && curLineStart > 0)
	{
		PreviousLine();
	}
}

void GuiLongText::SetTextPos(int pos)
{
	if(!wText)
		return;

	int diff = 10000;

	for(u32 i = 0; i < TextLines.size(); i++)
	{
		int curDiff = abs(TextLines[i].LineOffset - pos);
		if(curDiff < diff)
		{
			diff = curDiff;
			curLineStart = i;
		}
	}

	FillRows();

	while((int) textDyn.size() < linestodraw && curLineStart > 0)
	{
		PreviousLine();
	}
}

const wchar_t * GuiLongText::GetText()
{
	return wText->c_str();
}

std::string GuiLongText::GetUTF8String(void) const
{
	return wText->toUTF8();
}

int GuiLongText::GetLineOffset(int ind)
{
	if(TextLines.size() == 0)
		return 0;

	if(ind < 0)
		return TextLines[0].LineOffset;

	if(ind >= (int) TextLines.size()-1)
		return TextLines[TextLines.size()-1].LineOffset;

	return TextLines[ind].LineOffset;
}

const wchar_t * GuiLongText::GetTextLine(int ind)
{
	if(filling || textDyn.size() == 0)
		return NULL;

	if(ind < 0)
		return textDyn[0];

	if(ind >= (int) textDyn.size())
		return textDyn[textDyn.size()-1];

	return textDyn[ind];
}

void GuiLongText::Refresh()
{
	CalcLineOffsets();
	FillRows();
}

void GuiLongText::NextLine()
{
	if(!wText || (curLineStart+1 > ((int) TextLines.size()-linestodraw)))
		return;

	++curLineStart;

	FillRows();
}

void GuiLongText::PreviousLine()
{
	if(!wText || curLineStart-1 < 0)
		return;

	--curLineStart;

	FillRows();
}

void GuiLongText::FillRows()
{
	if(!wText)
		return;

	filling = true;

	ClearDynamicText();

	for(int i = 0; i < linestodraw && i < (int) TextLines.size(); i++)
	{
		if(i >= (int) textDyn.size())
		{
			textDyn.resize(i+1);
			textDyn[i] = new wchar_t[maxWidth];
		}
		int offset = TextLines[curLineStart+i].LineOffset;
		int count = TextLines[curLineStart+i].CharCount+1;
		int n;
		for(n = 0; n < count && offset+n < (int) wText->size(); n++)
			textDyn[i][n] = wText->at(offset+n);

		textDyn[i][n] = 0;
	}

	filling = false;

	return;
}

void GuiLongText::CalcLineOffsets()
{
	if(!wText)
		return;

	TextLines.clear();

	TextLine TmpLine;
	TmpLine.CharCount = 0;
	TmpLine.LineOffset = 0;
	TmpLine.width = 0;

	const wchar_t * origTxt = wText->c_str();
	int ch = 0;
	int lastSpace = -1;
	int currWidth = 0;
	int i = 0;

	while(origTxt[ch])
	{
		currWidth += fontSystem->getCharWidth(origTxt[ch], currentSize, ch > 0 ? origTxt[ch-1] : 0x0000);

		if(currWidth >= maxWidth)
		{
			if(lastSpace > 0)
			{
				ch = lastSpace;
			}
			TmpLine.CharCount = ch-TmpLine.LineOffset;
			TmpLine.width = currWidth;
			TextLines.push_back(TmpLine);
			currWidth = 0;
			lastSpace = -1;
			i = -1;
			TmpLine.LineOffset = ch+1;
		}
		else if(origTxt[ch] == '\n')
		{
			TmpLine.CharCount = ch-TmpLine.LineOffset;
			TmpLine.width = currWidth;
			TextLines.push_back(TmpLine);
			currWidth = 0;
			lastSpace = -1;
			i = -1;
			TmpLine.LineOffset = ch+1;
		}
		else if(origTxt[ch] == ' ')
		{
			lastSpace = ch;
		}

		ch++;
		i++;
	}

	TmpLine.CharCount = ch-TmpLine.LineOffset;
	TmpLine.width = currWidth;

	if(TmpLine.CharCount-1 > 0)
	{
		TmpLine.CharCount -= 1;
		TextLines.push_back(TmpLine);
	}
}

void GuiLongText::Draw()
{
	if(textDyn.size() == 0)
		return;

	if(!this->IsVisible())
		return;

	GXColor c = color;
	c.a = this->GetAlpha();

	int newSize = size*GetScale();

	if(newSize != currentSize)
	{
		currentSize = newSize;

		if(wText)
			textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);
	}

	u16 lineheight = newSize + 6;

	for(u32 i = 0; i < textDyn.size(); i++)
	{
		if(!filling)
			(font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop()+i*lineheight, GetZPosition(), textDyn[i], currentSize, c, style, 0, maxWidth);
	}
}
