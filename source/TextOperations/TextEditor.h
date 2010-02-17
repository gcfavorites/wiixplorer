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
 * TextEditor.cpp
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "libwiigui/gui.h"
#include "TextPointer.h"
#include "Text.hpp"

//!Display a list of files
class TextEditor : public GuiWindow
{
	public:
		TextEditor(char *intext, int LinesToDraw, const char *path);
		~TextEditor();
		void DisableTriggerUpdate(bool set);
		void SetText(const char *intext);
        void WriteTextFile(const char * path);
        int GetState();
		void ResetState();
		void Update(GuiTrigger * t);
	protected:
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);
        void OnPointerHeld(GuiElement *sender, int pointer, POINT p);
		bool triggerdisabled;
		bool ExitEditor;
		bool LineEditing;
		bool FileEdited;
		u32 filesize;
		char * filepath;

        /** Buttons **/
		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;
		GuiButton * maximizeBtn;
		GuiButton * minimizeBtn;
		GuiButton * closeBtn;
		GuiButton * PlusBtn;
		TextPointer * TextPointerBtn;

        /** Images **/
		GuiImage * bgTexteditorImg;
		GuiImage * scrollbarImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowDownOverImg;
		GuiImage * arrowUpImg;
		GuiImage * arrowUpOverImg;
		GuiImage * scrollbarBoxImg;
		GuiImage * scrollbarBoxOverImg;
		GuiImage * closeImg;
		GuiImage * closeImgOver;
		GuiImage * maximizeImg;
		GuiImage * minimizeImg;

        /** ImageDatas **/
		GuiImageData * bgTexteditorData;
		GuiImageData * scrollbar;
		GuiImageData * arrowDown;
		GuiImageData * arrowDownOver;
		GuiImageData * arrowUp;
		GuiImageData * arrowUpOver;
		GuiImageData * scrollbarBox;
		GuiImageData * scrollbarBoxOver;
		GuiImageData * closeImgData;
		GuiImageData * closeImgOverData;
		GuiImageData * maximizeImgData;
		GuiImageData * minimizeImgData;

        /** Sounds **/
		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;

        /** Triggers **/
		GuiTrigger * trigA;
		GuiTrigger * trigPlus;
		GuiTrigger * trigHeldA;
		GuiTrigger * trigB;

        /** Texts **/
		GuiText * filenameTxt;
		Text * MainFileTxt;
};

#endif
