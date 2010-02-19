#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "libwiigui/gui.h"
#include "wstring.hpp"

class Text : public GuiText
{
    public:
		//!Constructor
		//!\param t Text
		//!\param s Font size
		//!\param c Font color
        Text(const char * t, int s, GXColor c);
        Text(const wchar_t * t, int s, GXColor c);
        ~Text();
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
		//!Set to the char pos in text
        void SetTextPos(int pos);
		//!Refresh the rows to draw
        int GetCurrPos() { return curLineStart; };
		//!Get the count of loaded lines
        int GetCharsCount() { return curCharsCount; };
		//!Get the count of loaded lines
        int GetLinesCount() { return curLinesCount; };
        //!Get the original full Text
        const wchar_t * GetText();
        //!Get the original Text as a UTF-8 text
        //!memory is allocated in this
        //!which needs to be deleted later
        std::string GetUTF8String() const;
        //!Get a Textline
        const wchar_t * GetTextLine(int ind);
        //!Get the offset in the text of a drawn Line
        int GetLineOffset(int ind);
		//!Constantly called to draw the text
		void Draw();
    protected:
        void FillRows();
        void ClearRows();

		wString * wText;
		int LineOffset[MAX_LINES_TO_DRAW];
		int curCharsCount;
		int curLinesCount;
		int curLineStart;
		bool filling;
};

#endif
