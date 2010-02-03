 /****************************************************************************
 * Copyright (C) 2009
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
 * TextPointer.cpp
 * for WiiXplorer 2009
 ***************************************************************************/

#include <unistd.h>
#include "Memory/Resources.h"
#include "Prompts/PromptWindows.h"
#include "TextPointer.h"
#include "tools.h"

/**
 * Constructor for the TextPointer class.
 */
TextPointer::TextPointer(GuiText *parent, int FontSize, int lines)
    : GuiButton(343, 240)
{
    fontsize = FontSize;
    Text = parent;
    linestodraw = lines;
    MarkImage = NULL;
    currentline = -1;
    Position_X = 0;
    Position_Y = 0;
    width = Text->GetTextMaxWidth();
    if(width == 0)
        width = Text->GetTextWidth(0);
    height = (linestodraw+1)*(fontsize+6);
    visibility = true;

    TextPointerImgData = Resources::GetImageData(textpointer_img_png, textpointer_img_png_size);
    TextPointerImg = new GuiImage(TextPointerImgData);
    TextPointerImg->SetVisible(visibility);

    SetLabel(Text);
    SetImage(TextPointerImg);
}

TextPointer::TextPointer(GuiText *parent, int FontSize, int lines, int w, int h)
    : GuiButton(343, 240)
{
    fontsize = FontSize;
    Text = parent;
    linestodraw = lines;
    MarkImage = NULL;
    currentline = -1;
    Position_X = 0;
    Position_Y = 0;
    width = w;
    height = h;
    visibility = true;

    TextPointerImgData = Resources::GetImageData(textpointer_img_png, textpointer_img_png_size);
    TextPointerImg = new GuiImage(TextPointerImgData);
    TextPointerImg->SetVisible(visibility);

    SetLabel(Text);
    SetImage(TextPointerImg);
}

/**
 * Destructor for the TextPointer class.
 */
TextPointer::~TextPointer()
{
    Resources::Remove(TextPointerImgData);
	delete TextPointerImg;

	if(MarkImage)
        delete MarkImage;
}

void TextPointer::PositionChanged(int chan, int x, int y)
{
	if(MarkImage)
	{
        delete MarkImage;
        MarkImage = NULL;
	}

    currentChan = chan;
    currentline = -1;
    Position_X = 0;
    Position_Y = 0;
    LetterNumInLine = 0;
    LineOffset = 0;
    Marking = false;

    int linenumber = 0;

    int differenz = 1000;

    int maxlines = (linestodraw > Text->GetTotalLines()) ?  Text->GetTotalLines() : linestodraw;
    if(maxlines < 1)
        maxlines = 1;

    for(int i = 0; i < maxlines; i++)
    {
        int linestart = i*(fontsize+6);
        if(differenz > abs(linestart-y))
        {
            differenz = abs(linestart-y);
            linenumber = i;
        }
    }
    differenz = 1000;
    wchar_t * line = Text->GetDynTextLine(linenumber);

    if(!line)
        return;

    lineLength = wcslen(line)+1;
    wchar_t temp[lineLength];

    for(int i = 0; i < lineLength; i++)
    {
        temp[i] = line[i];
        temp[i+1] = 0;

        int w = fontSystem[fontsize]->getWidth(temp);

        if(differenz > abs(x-w))
        {
            differenz = abs(x-w);
            Position_X = w;
            LetterNumInLine = i+1;
        }
    }

    if(differenz > abs(x))
    {
        differenz = abs(x);
        Position_X = 0;
        LetterNumInLine = 0;
    }

    currentChan = chan;
    currentline = linenumber;
    Position_Y = linenumber*(fontsize+6);
    LineOffset = Text->GetLineBreakOffset(currentline);
    Marking = true;
}

void TextPointer::SetPointerPosition(int LetterPos)
{
    if(LetterPos <= 0)
    {
        Position_X = 0;
        LetterNumInLine = 0;
        Marking = false;
        return;
    }

    wchar_t * line = Text->GetDynTextLine(currentline);

    if(!line)
        return;

    lineLength = wcslen(line)+1;

    if(LetterPos < 0)
        LetterPos = 0;
    else if(LetterPos > lineLength-1)
        LetterPos = lineLength-1;

    wchar_t temp[lineLength];
    memset(temp, 0, lineLength);

    for(int i = 0; i < LetterPos; i++)
    {
        temp[i] = line[i];
        temp[i+1] = 0;
    }

    Position_X = fontSystem[fontsize]->getWidth(temp);
    LetterNumInLine = LetterPos;
    Marking = false;
}

void TextPointer::TextWidthChanged()
{
    width = Text->GetTextMaxWidth();
    if(width == 0)
        width = Text->GetTextWidth(0);
}

int TextPointer::GetCurrentLetter()
{
    return currentline+LetterNumInLine;
}

int TextPointer::EditLine()
{
    if(currentline < 0)
        PositionChanged(0, 0, 0);

    char * origText = new char[strlen(Text->GetOrigText())+MAXPATHLEN];
    memset(origText, 0, strlen(Text->GetOrigText())+MAXPATHLEN);
    sprintf(origText, "%s", Text->GetOrigText());

    char * StartPosition =  &origText[LineOffset];
    char * EndPosition =  &origText[LineOffset+LetterNumInLine];

    char temp[150];
    snprintf(temp, LetterNumInLine+1, "%s", StartPosition);
    int result = OnScreenKeyboard(temp, 150);
    if(result == 1)
    {
        char * temp2 = new char[strlen(origText)+1];
        sprintf(temp2, "%s", EndPosition);
        sprintf(StartPosition, "%s%s", temp, temp2);
        delete [] temp2;
        Text->SetText(origText);
        delete [] origText;
        Text->SetMaxWidth(343, LONGTEXT);
        PositionChanged(0, Position_X, Position_Y);
        return 1;
    }

    delete [] origText;

    return -1;
}

void TextPointer::Draw()
{
    if(frameCount % 30 == 0)
    {
        visibility = !visibility;
        TextPointerImg->SetVisible(visibility);
    }
    else if(TextPointerImg->GetLeft() != Position_X || TextPointerImg->GetTop() != Position_Y)
        TextPointerImg->SetPosition(Position_X, Position_Y);

    GuiButton::Draw();
}

void TextPointer::Update(GuiTrigger * t)
{
    //!saves too many loops
    /*
    if(t == &userInput[currentChan])
    {
        if(t->wpad->btns_h |= WPAD_BUTTON_A && Marking)
        {
            ROUND2FOUR()
        }
        else
        {
            Marking = false;
        }
    }
    */

    GuiButton::Update(t);
}
