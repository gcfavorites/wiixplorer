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
#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "GUI/gui.h"
#include "GUI/gui_longtext.hpp"
#include "Controls/Scrollbar.hpp"
#include "TextPointer.h"

//!Display a list of files
class TextEditor : public GuiFrame, public sigslot::has_slots<>
{
	public:
		TextEditor(const wchar_t *intext, int LinesToDraw, const char *path);
		virtual ~TextEditor();
		static void LoadFile(const char *filepath);
		void SetText(const wchar_t *intext);
		void WriteTextFile(const char * path);
		void Update(GuiTrigger * t);
	protected:
		int EditLine();
		void OnListChange(int selItem, int selIndex);
		void OnButtonClick(GuiButton *sender, int pointer, const POINT &p);
		void OnPointerHeld(GuiButton *sender, int pointer, const POINT &p);

		bool ExitEditor;
		bool FileEdited;
		int linestodraw;
		u32 filesize;
		char * filepath;

		Scrollbar * scrollbar;

		/** Buttons **/
		GuiButton * maximizeBtn;
		GuiButton * minimizeBtn;
		GuiButton * closeBtn;
		GuiButton * PlusBtn;
		TextPointer * TextPointerBtn;

		/** Images **/
		GuiImage * bgTexteditorImg;
		GuiImage * closeImg;
		GuiImage * closeImgOver;
		GuiImage * maximizeImg;
		GuiImage * minimizeImg;

		/** ImageDatas **/
		GuiImageData * bgTexteditorData;
		GuiImageData * closeImgData;
		GuiImageData * closeImgOverData;
		GuiImageData * maximizeImgData;
		GuiImageData * minimizeImgData;

		/** Sounds **/
		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;

		/** Triggers **/
		GuiTrigger * trigA;
		GuiTrigger * trigHeldA;
		GuiTrigger * trigPlus;
		GuiTrigger * trigB;

		/** Texts **/
		GuiText * filenameTxt;
		GuiLongText * MainFileTxt;
};

#endif
