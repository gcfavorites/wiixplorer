/***************************************************************************
 * Copyright (C) 2011
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
 ***************************************************************************/
#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "libwiigui/gui.h"
#include "wstring.hpp"

typedef struct
{
	int LineOffset;
	int CharCount;
	int width;
} TextLine;

class Text: public GuiText
{
    public:
        //!Constructor
        //!\param t Text
        //!\param s Font size
        //!\param c Font color
        Text(const char * t, int s, GXColor c);
        Text(const wchar_t * t, int s, GXColor c);
        virtual ~Text();
        //!Sets the text of the GuiText element
        //!\param t Text
        void SetText(const char * t);
        void SetText(const wchar_t * t);
        //!Set the max texwidth
        void SetMaxWidth(int width);
        //!Go to next line
        void NextLine();
        //!Go to previous line
        void PreviousLine();
        //!Refresh the rows to draw
        void Refresh();
        //!Set the text line
        void SetTextLine(int line);
        //!Set to the char pos in text
        void SetTextPos(int pos);
        //!Refresh the rows to draw
        int GetCurrPos() { return curLineStart; };
        //!Get  the count of loaded lines
        int GetLinesCount() { return textDyn.size(); };
        //!Get the total count of lines
        int GetTotalLinesCount() { return TextLines.size(); };
        //!Get the original full Text
        const wchar_t * GetText();
        //!Get the original full Text as wString
        wString * GetwString() { return wText; };
        //!Get the original Text as a UTF-8 text
        std::string GetUTF8String() const;
        //!Get a Textline
        const wchar_t * GetTextLine(int ind);
        //!Get the offset in the text of a drawn Line
        int GetLineOffset(int ind);
        //!Constantly called to draw the text
        void Draw();
    protected:
        void CalcLineOffsets();
        void FillRows();

        wString * wText;
        std::vector<TextLine> TextLines;
        int curLineStart;
        int FirstLineOffset;
        bool filling;
};

#endif
