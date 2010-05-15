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
#ifndef __ICONBROWSER_HPP_
#define __ICONBROWSER_HPP_

#include "Controls/Scrollbar.hpp"
#include "GuiFileBrowser.hpp"

using namespace std;

class IconFileBrowser : public GuiFileBrowser
{
	public:
		IconFileBrowser(Browser * filebrowser, int w, int h);
		~IconFileBrowser();
		void ResetState();
        void SetSelected(int i);
        void SetTriggerUpdate(bool t) { triggerupdate = t; };
		void TriggerUpdate() { listChanged = true; };
		void Draw();
		void Update(GuiTrigger * t);
	protected:
        void OnButtonClicked(GuiElement *sender, int pointer, POINT p);
        GuiImage * GetIconFromExt(const char * fileext, bool dir);
        void AddButton();
        void SetButton(int i, const char * name, bool dir, bool enable, int x, int y);
        void RemoveButton(int i);

        u16 RowSize;
        u16 PageSize;
		int selectedItem;
		int numEntries;
		bool listChanged;
		bool triggerupdate;

		vector<GuiText *> ButtonText;
		vector<GuiImage *> FileSelectionImg;
		vector<GuiImage *> ButtonImg;
		vector<GuiButton *> Buttons;

		Scrollbar * scrollbar;

		GuiImageData * bgFileSelection;
        GuiImageData * fileArchives;
		GuiImageData * fileDefault;
		GuiImageData * fileFolder;
        GuiImageData * fileGFX;
		GuiImageData * filePLS;
		GuiImageData * fileSFX;
        GuiImageData * fileTXT;
        GuiImageData * fileXML;
        GuiImageData * fileVID;
        GuiImageData * filePDF;

		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;

		GuiTrigger * trigA;
};

#endif
