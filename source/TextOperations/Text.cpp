#include "Text.hpp"

Text::Text(const char * t, int s, GXColor c)
    : GuiText(t, s, c)
{
    maxWidth = 400;
    linestodraw = 9;
    curLinesCount = 0;
    curLineStart = 0;
    FirstLineOffset = 0;
    wText = NULL;

    if(origText)
        free(origText);
    origText = NULL;

    if(!text)
        return;

    wText = new (std::nothrow) wString(text);
    if(!wText)
    {
        ShowError(tr("Not enough memory."));
        return;
    }

    textWidth = (font ? font : fontSystem[currentSize])->getWidth(wText->data());
    delete [] text;
    text = NULL;

    SetMaxWidth(400);
}

Text::Text(const wchar_t * t, int s, GXColor c)
    : GuiText((wchar_t *) NULL, s, c)
{
    maxWidth = 400;
    linestodraw = 9;
    curLinesCount = 0;
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

    if(!fontSystem[currentSize])
    {
        fontSystem[currentSize] = new FreeTypeGX(currentSize);
    }
    textWidth = (font ? font : fontSystem[currentSize])->getWidth(wText->data());

    SetMaxWidth(400);
}

Text::~Text()
{
    if(wText)
        delete wText;
    wText = NULL;

	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
            delete [] textDynRow[i];
        textDynRow[i] = NULL;
	}
    TextLines.clear();
}

void Text::SetText(const char * t)
{
    wchar_t * tmp = charToWideChar(t);
    if(!tmp)
        return;

    if(wText)
        delete wText;

    wText = new wString(tmp);
    textWidth = (font ? font : fontSystem[currentSize])->getWidth(wText->data());

    delete [] tmp;
    CalcLineOffsets();
}

void Text::SetText(const wchar_t * t)
{
    if(!t)
        return;

    if(wText)
        delete wText;

    wText = new wString(t);
    textWidth = (font ? font : fontSystem[currentSize])->getWidth(wText->data());
    CalcLineOffsets();
}

void Text::SetMaxWidth(int w)
{
    maxWidth = w;
    curLinesCount = 0;
    curLineStart = 0;
    Refresh();
}

void Text::SetTextLine(int line)
{
    curLineStart = cut_bounds(line, 0, TextLines.size()-1);

    FillRows();

    while(curLinesCount+1 <= linestodraw && curLineStart > 0)
    {
        PreviousLine();
    }
}

void Text::SetTextPos(int pos)
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

    while(curLinesCount+1 <= linestodraw && curLineStart > 0)
    {
        PreviousLine();
    }
}

const wchar_t * Text::GetText()
{
    return wText->c_str();
}

std::string Text::GetUTF8String(void) const
{
    return wText->toUTF8();
}

int Text::GetLineOffset(int ind)
{
    if(ind < 0)
        return TextLines[0].LineOffset;
    if(ind >= (int) TextLines.size()-1)
        return TextLines[TextLines.size()-1].LineOffset;

    return TextLines[ind].LineOffset;
}

const wchar_t * Text::GetTextLine(int ind)
{
    if(filling)
        return NULL;

    if(ind < 0)
        return textDynRow[0];

    if(ind >= curLinesCount)
        return textDynRow[curLinesCount-1];

    return textDynRow[ind];
}

void Text::Refresh()
{
    CalcLineOffsets();
    FillRows();
}

void Text::NextLine()
{
    if(!wText || (curLineStart+1 > ((int) TextLines.size())-linestodraw))
        return;

    ++curLineStart;

    FillRows();
}

void Text::PreviousLine()
{
    if(!wText || curLineStart-1 < 0)
        return;

    --curLineStart;

    FillRows();
}

void Text::FillRows()
{
    if(!wText)
        return;

    filling = true;

    ClearRows();

    curLinesCount = 0;

    for(int i = 0; (i <= linestodraw) && (curLineStart+i < (int) TextLines.size()); i++)
    {
        if(!textDynRow[i])
        {
            textDynRow[i] = new wchar_t[maxWidth];
        }
        u32 offset = TextLines[curLineStart+i].LineOffset;
        u32 count = TextLines[curLineStart+i].CharCount;

        const wchar_t * ptrSrc = (wText->substr(offset, count)).c_str();
        for(u32 n = 0; n < count; n++)
            textDynRow[i][n] = *ptrSrc++;

        textDynRow[i][count] = '\0';

        curLinesCount++;
    }

    filling = false;

    return;
}

void Text::CalcLineOffsets()
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
    int lastSpaceIndex = -1;
    int currWidth = 0;
    int i = 0;

    while(origTxt[ch])
    {
        currWidth += fontSystem[currentSize]->getCharWidth(origTxt[ch]);

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
            lastSpaceIndex = i;
        }

        ch++;
        i++;
    }

    TmpLine.CharCount = ch-TmpLine.LineOffset;
    TmpLine.width = currWidth;
    TextLines.push_back(TmpLine);
}

void Text::ClearRows()
{
	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
	        memset(textDynRow[i], 0, maxWidth);
	    }
	}

}

void Text::Draw()
{
    if(!textDynRow[0])
        return;

	if(!this->IsVisible())
		return;

	GXColor c = color;
	c.a = this->GetAlpha();

	int newSize = size*this->GetScale();

	if(newSize > MAX_FONT_SIZE)
		newSize = MAX_FONT_SIZE;

	if(newSize != currentSize)
	{
	    if(font)
	    {
	        font->ChangeFontSize(newSize);
	    }
        else if(!fontSystem[newSize])
            fontSystem[newSize] = new FreeTypeGX(newSize);

        if(wText)
            textWidth = (font ? font : fontSystem[newSize])->getWidth(wText->data());
		currentSize = newSize;
	}

    u16 lineheight = newSize + 6;

    for(int i = 0; i < linestodraw; i++)
    {
        if(textDynRow[i] && !filling)
            (font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop()+i*lineheight, GetZPosition(), textDynRow[i], c, style, 0, maxWidth);
    }
}
