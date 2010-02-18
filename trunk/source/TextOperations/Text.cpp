#include "Text.hpp"

Text::Text(const char * t, int s, GXColor c)
    : GuiText(t, s, c)
{
    maxWidth = 400;
    linestodraw = 9;
    curCharsCount = 0;
    curLinesCount = 0;
    curLineStart = 0;
    wText = NULL;

    if(origText)
        free(origText);
    origText = NULL;

    if(!text)
        return;

    wText = new wString(text);
    textWidth = (font ? font : fontSystem[currentSize])->getWidth(wText->data());
    delete [] text;
    text = NULL;

    SetMaxWidth(400);
}

Text::Text(const wchar_t * t, int s, GXColor c)
    : GuiText(t, s, c)
{
    maxWidth = 400;
    linestodraw = 9;
    curCharsCount = 0;
    curLinesCount = 0;
    curLineStart = 0;
    wText = NULL;

    if(!t)
        return;

    wText = new wString(text);
    textWidth = (font ? font : fontSystem[currentSize])->getWidth(wText->data());
    delete [] text;
    text = NULL;

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
        LineOffset[i] = -1;
	}
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
}

void Text::SetText(const wchar_t * t)
{
    if(!t)
        return;

    if(wText)
        delete wText;

    wText = new wString(t);
    textWidth = (font ? font : fontSystem[currentSize])->getWidth(wText->data());
}

void Text::SetMaxWidth(int w)
{
    maxWidth = w;
    curCharsCount = 0;
    curLinesCount = 0;
    curLineStart = 0;
    Refresh();
}

void Text::SetTextPos(int pos)
{
    if(!wText)
        return;

    int position = wText->find_last_of('\n', pos);
    if(position < 0)
        position = 0;

    curLineStart = position;

    if(curLineStart != 0)
        curLineStart++;

    FillRows();

    while(curLinesCount+1 < linestodraw && curLinesCount > 0)
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
        return LineOffset[0];

    if(ind >= curLinesCount)
        return LineOffset[curLinesCount-1];

    return LineOffset[ind];
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
    FillRows();
}

void Text::NextLine()
{
    if(!wText || curLinesCount < linestodraw)
        return;

    int position = wText->find_first_of('\n', curLineStart);

    if(position < 0)
        PreviousLine();

    curLineStart = position;

    if(curLineStart != 0)
        curLineStart++;

    FillRows();

    if(curLinesCount+1 < linestodraw)
        PreviousLine();
}

void Text::PreviousLine()
{
    if(!wText)
        return;

    if(curLineStart-2 < 0)
        curLineStart = 0;
    else
        curLineStart = wText->find_last_of('\n', curLineStart-2);

    if(curLineStart < 0)
        curLineStart = 0;

    if(curLineStart != 0)
        curLineStart++;

    FillRows();
}

void Text::FillRows()
{
    if(!wText)
        return;

    filling = true;

    ClearRows();

    int linenum = 0;
    curCharsCount = 0;

    const wchar_t * origTxt = wText->data();
    int ch = curLineStart;
    int lastSpace = -1;
    int lastSpaceIndex = -1;
    int currWidth = 0;
    LineOffset[0] = ch;
    int i = 0;

    while(origTxt[ch])
    {
        if(!textDynRow[linenum])
        {
            textDynRow[linenum] = new wchar_t[maxWidth];
        }
        textDynRow[linenum][i] = origTxt[ch];
        textDynRow[linenum][i+1] = 0;

        currWidth += fontSystem[currentSize]->getCharWidth(origTxt[ch]);

        if(currWidth >= maxWidth)
        {
            if(lastSpace > 0)
            {
                textDynRow[linenum][lastSpaceIndex] = 0;
                ch = lastSpace;
            }
            currWidth = 0;
            lastSpace = -1;
            i = -1;
            linenum++;
            LineOffset[linenum] = ch+1;
        }
        else if(origTxt[ch] == '\n')
        {
            currWidth = 0;
            lastSpace = -1;
            i = -1;
            linenum++;
            LineOffset[linenum] = ch+1;
        }
        else if(origTxt[ch] == ' ')
        {
            lastSpace = ch;
            lastSpaceIndex = i;
        }

        if(linenum >= linestodraw)
            break;

        curCharsCount++;
        ch++;
        i++;
    }

    if(origTxt[ch] == 0)
    {
        linenum++;
        LineOffset[linenum] = ch+1;
    }

    filling = false;

	curLinesCount = linenum;
}

void Text::ClearRows()
{
	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
	        memset(textDynRow[i], 0, maxWidth);
	    }
        LineOffset[i] = -1;
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

    int lineheight = newSize + 6;

    for(int i = 0; i < linestodraw; i++)
    {
        if(textDynRow[i] && !filling)
            (font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop()+i*lineheight, textDynRow[i], c, style, 0, maxWidth);
    }
}
