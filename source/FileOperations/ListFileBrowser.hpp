/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * Original ListBrowser by Tantric (C) 2009
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
#ifndef __LISTBROWSER_HPP_
#define __LISTBROWSER_HPP_

#include "GuiFileBrowser.hpp"

//!Display a list of files
class ListFileBrowser : public GuiFileBrowser
{
	public:
		ListFileBrowser(Browser * filebrowser, int w, int h);
		~ListFileBrowser();
		void ResetState();
        void SetSelected(int i);
        void SetTriggerUpdate(bool t) { triggerupdate = t; };
		void TriggerUpdate() { listChanged = true; };
		void Draw();
		void Update(GuiTrigger * t);
	protected:
        void OnClicked(GuiElement *sender, int pointer, POINT p);
		int selectedItem;
		int numEntries;
		bool listChanged;
		bool triggerupdate;

		GuiText * fileListText[PAGESIZE];
		GuiText * fileListTextOver[PAGESIZE];
		GuiImage * fileListBg[PAGESIZE];
		GuiImage * fileListArchives[PAGESIZE];
		GuiImage * fileListDefault[PAGESIZE];
		GuiImage * fileListFolder[PAGESIZE];
		GuiImage * fileListGFX[PAGESIZE];
		GuiImage * fileListPLS[PAGESIZE];
		GuiImage * fileListSFX[PAGESIZE];
		GuiImage * fileListTXT[PAGESIZE];
		GuiImage * fileListXML[PAGESIZE];
		GuiImage * fileListVID[PAGESIZE];
		GuiButton * fileList[PAGESIZE];

		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;

		GuiImage * bgFileSelectionImg;
		GuiImage * scrollbarImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowDownOverImg;
		GuiImage * arrowUpImg;
		GuiImage * arrowUpOverImg;
		GuiImage * scrollbarBoxImg;
		GuiImage * scrollbarBoxOverImg;

		GuiImageData * bgFileSelectionEntry;
        GuiImageData * fileArchives;
		GuiImageData * fileDefault;
		GuiImageData * fileFolder;
        GuiImageData * fileGFX;
		GuiImageData * filePLS;
		GuiImageData * fileSFX;
        GuiImageData * fileTXT;
        GuiImageData * fileXML;
        GuiImageData * fileVID;
		GuiImageData * scrollbar;
		GuiImageData * arrowDown;
		GuiImageData * arrowDownOver;
		GuiImageData * arrowUp;
		GuiImageData * arrowUpOver;
		GuiImageData * scrollbarBox;
		GuiImageData * scrollbarBoxOver;

		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;
		GuiTrigger * trigA;
		GuiTrigger * trigHeldA;
};

#endif
