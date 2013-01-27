/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef GUI_KEYBOARD_H_
#define GUI_KEYBOARD_H_

#include "gui_frame.h"
#include "gui_button.h"
#include "gui_text.h"
#include "gui_image.h"
#include "gui_keyboard_alone.h"
#include "TextOperations/TextPointer.h"
#include "TextOperations/wstring.hpp"

#define MAX_KEYBOARD_DISPLAY	40

//!On-screen keyboard
class GuiKeyboard : public GuiKeyboardAlone
{
	public:
		GuiKeyboard(const char * t, u32 max);
		GuiKeyboard(const wchar_t * t, u32 max);
		virtual ~GuiKeyboard();
		const wchar_t * GetString();
		std::string GetUTF8String() const;
		void AddChar(int pos, wchar_t Char);
		void RemoveChar(int pos);
	protected:
		void SetupKeyboard(const wchar_t * t, u32 max);
		void MoveText(int n);
		const wchar_t * GetDisplayText();
		void OnPointerHeld(GuiButton *sender, int pointer, const POINT &p);
		void OnPositionMoved(GuiButton *sender, int pointer, const POINT &p);
		void OnClearKeyPress(GuiButton *sender, int pointer, const POINT &p);
		void OnKeyPress(wchar_t charCode);

		int CurrentFirstLetter;
		wString kbtextstr;
		wchar_t displayTxt[MAX_KEYBOARD_DISPLAY];
		u32 kbtextmaxlen;

		TextPointer * TextPointerBtn;
		GuiButton * GoLeft;
		GuiButton * GoRight;
		GuiText * kbText;
		GuiImage * keyTextboxImg;
		GuiText * keyClearText;
		GuiImage * keyClearImg;
		GuiImage * keyClearOverImg;
		GuiButton * keyClear;
		GuiImageData * keyTextbox;
		GuiTrigger * trigHeldA;
		GuiTrigger * trigLeft;
		GuiTrigger * trigRight;
};

#endif
