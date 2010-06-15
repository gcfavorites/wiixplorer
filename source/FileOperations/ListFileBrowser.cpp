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
#include "ListFileBrowser.hpp"
#include "Memory/Resources.h"
#include "FileStartUp/FileExtensions.h"
#include "menu.h"

/**
 * Constructor for the ListFileBrowser class.
 */
ListFileBrowser::ListFileBrowser(Browser * filebrowser, int w, int h)
    : GuiFileBrowser(filebrowser, w, h)
{
	width = w;
	height = h;
	selectedItem = 0;
	numEntries = 0;
	browser = filebrowser;
	listChanged = true; // trigger an initial list update
	triggerupdate = true; // trigger disable

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
	btnSoundClick = Resources::GetSound(button_click_wav, button_click_wav_size);

	bgFileSelectionEntry = Resources::GetImageData(bg_browser_selection_png, bg_browser_selection_png_size);
	fileArchives = Resources::GetImageData(icon_archives_png, icon_archives_png_size);
	fileDefault = Resources::GetImageData(icon_default_png, icon_default_png_size);
	fileFolder = Resources::GetImageData(icon_folder_png, icon_folder_png_size);
	fileGFX = Resources::GetImageData(icon_gfx_png, icon_gfx_png_size);
	filePLS = Resources::GetImageData(icon_pls_png, icon_pls_png_size);
	fileSFX = Resources::GetImageData(icon_sfx_png, icon_sfx_png_size);
	fileTXT = Resources::GetImageData(icon_txt_png, icon_txt_png_size);
	fileXML = Resources::GetImageData(icon_xml_png, icon_xml_png_size);
	fileVID = Resources::GetImageData(icon_video_png, icon_video_png_size);
	filePDF = Resources::GetImageData(icon_pdf_png, icon_pdf_png_size);

	scrollbar = new Scrollbar(245);
	scrollbar->SetParent(this);
	scrollbar->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbar->SetPosition(-10, 5);
	scrollbar->SetScrollSpeed(Settings.ScrollSpeed);

	for(int i=0; i<PAGESIZE; i++)
	{
		fileListText[i] = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
		fileListText[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		fileListText[i]->SetPosition(10,0);
		fileListText[i]->SetMaxWidth(this->GetWidth() - (90), DOTTED);

		fileListTextOver[i] = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
		fileListTextOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		fileListTextOver[i]->SetPosition(10,0);
		fileListTextOver[i]->SetMaxWidth(this->GetWidth() - (100), SCROLL_HORIZONTAL);

		fileListBg[i] = new GuiImage(bgFileSelectionEntry);
		fileListArchives[i] = new GuiImage(fileArchives);
		fileListDefault[i] = new GuiImage(fileDefault);
		fileListFolder[i] = new GuiImage(fileFolder);
		fileListGFX[i] = new GuiImage(fileGFX);
		fileListPLS[i] = new GuiImage(filePLS);
		fileListSFX[i] = new GuiImage(fileSFX);
		fileListTXT[i] = new GuiImage(fileTXT);
		fileListXML[i] = new GuiImage(fileXML);
		fileListVID[i] = new GuiImage(fileVID);
		fileListPDF[i] = new GuiImage(filePDF);

		fileList[i] = new GuiButton(507,30);
		fileList[i]->SetParent(this);
		fileList[i]->SetLabel(fileListText[i]);
		fileList[i]->SetLabelOver(fileListTextOver[i]);
		fileList[i]->SetImageOver(fileListBg[i]);
		fileList[i]->SetPosition(7,30*i+3);
		fileList[i]->SetTrigger(trigA);
		fileList[i]->SetSoundClick(btnSoundClick);
		fileList[i]->Clicked.connect(this, &ListFileBrowser::OnClicked);
	}
}

/**
 * Destructor for the ListFileBrowser class.
 */
ListFileBrowser::~ListFileBrowser()
{
    browser = NULL;
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnSoundClick);
	Resources::Remove(bgFileSelectionEntry);
	Resources::Remove(fileArchives);
	Resources::Remove(fileDefault);
	Resources::Remove(fileFolder);
	Resources::Remove(fileGFX);
	Resources::Remove(filePLS);
	Resources::Remove(fileSFX);
	Resources::Remove(fileTXT);
	Resources::Remove(fileXML);
	Resources::Remove(fileVID);
	Resources::Remove(filePDF);

	delete trigA;

	delete scrollbar;

	for(int i=0; i<PAGESIZE; i++)
	{
		delete fileListText[i];
		delete fileListTextOver[i];
		delete fileList[i];
		delete fileListBg[i];
		delete fileListArchives[i];
		delete fileListDefault[i];
		delete fileListFolder[i];
		delete fileListGFX[i];
		delete fileListPLS[i];
		delete fileListSFX[i];
		delete fileListTXT[i];
		delete fileListXML[i];
		delete fileListVID[i];
		delete fileListPDF[i];
	}
}

void ListFileBrowser::OnClicked(GuiElement *sender, int pointer, POINT p)
{
    state = STATE_CLICKED;
}

void ListFileBrowser::SetSelected(int i)
{
    if(i < 0 || i >= PAGESIZE)
        return;

    selectedItem = i;
    fileList[selectedItem]->SetState(STATE_SELECTED);
}

void ListFileBrowser::ResetState()
{
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for(int i=0; i<PAGESIZE; i++)
	{
		fileList[i]->ResetState();
	}
}

/**
 * Draw the button on screen
 */
void ListFileBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	for(int i=0; i<PAGESIZE; i++)
	{
		fileList[i]->Draw();
	}

	scrollbar->Draw();

	UpdateEffects();
}

void ListFileBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t || !triggerupdate)
		return;

	scrollbar->Update(t);

    if(scrollbar->ListChanged())
    {
        selectedItem = scrollbar->GetSelectedItem();
        browser->SetPageIndex(scrollbar->GetSelectedIndex());
        listChanged = true;
    }


	if(t->wpad->btns_d & WiiControls.DeMarkAllButton ||
       t->wpad->btns_d & (ClassicControls.DeMarkAllButton << 16) ||
       t->pad.btns_d & GCControls.DeMarkAllButton)
	{
        browser->ResetMarker();
	}
	else if(t->wpad->btns_d & WiiControls.MarkItemButton ||
            t->wpad->btns_d & (ClassicControls.MarkItemButton << 16) ||
            t->pad.btns_d & GCControls.MarkItemButton)
	{
	    browser->MarkCurrentItem();
	}
	else if(t->wpad->btns_d & WiiControls.DeMarkItemButton ||
            t->wpad->btns_d & (ClassicControls.DeMarkItemButton << 16) ||
            t->pad.btns_d & GCControls.DeMarkItemButton)
	{
	    browser->UnMarkCurrentItem();
	}

	if(t->Right())
	{
		if(browser->GetPageIndex() < browser->GetEntrieCount() && browser->GetEntrieCount() > PAGESIZE)
		{
			int pageIndex = browser->GetPageIndex() + PAGESIZE;
			if(pageIndex+PAGESIZE >= browser->GetEntrieCount())
				pageIndex = browser->GetEntrieCount()-PAGESIZE;
			browser->SetPageIndex(pageIndex);
			listChanged = true;
		}
	}
	else if(t->Left())
	{
		if(browser->GetPageIndex() > 0)
		{
			int pageIndex = browser->GetPageIndex() - PAGESIZE;
			if(pageIndex < 0)
				pageIndex = 0;
			browser->SetPageIndex(pageIndex);
			listChanged = true;
		}
	}
	else if(t->Down())
	{
		if(browser->GetPageIndex() + selectedItem + 1 < browser->GetEntrieCount())
		{
			if(selectedItem == PAGESIZE-1)
			{
				// move list down by 1
				browser->SetPageIndex(browser->GetPageIndex()+1);
				listChanged = true;
			}
			else if(fileList[selectedItem+1]->IsVisible())
			{
				fileList[selectedItem]->ResetState();
				fileList[++selectedItem]->SetState(STATE_SELECTED, t->chan);
			}
		}
	}
	else if(t->Up())
	{
		if(selectedItem == 0 &&	 browser->GetPageIndex() + selectedItem > 0)
		{
			// move list up by 1
			browser->SetPageIndex(browser->GetPageIndex()-1);
			listChanged = true;
		}
		else if(selectedItem > 0)
		{
			fileList[selectedItem]->ResetState();
			fileList[--selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
	}

	if(numEntries != browser->GetEntrieCount())
	{
	    numEntries = browser->GetEntrieCount();
	    scrollbar->SetEntrieCount(numEntries);
        listChanged = true;
	}

	for(int i=0; i<PAGESIZE; i++)
	{
		if(listChanged)
		{
			if(browser->GetPageIndex()+i < browser->GetEntrieCount())
			{
				if(fileList[i]->GetState() == STATE_DISABLED)
					fileList[i]->SetState(STATE_DEFAULT);

				fileList[i]->SetVisible(true);

				fileListText[i]->SetText(browser->GetItemDisplayname(browser->GetPageIndex()+i));
				fileListTextOver[i]->SetText(browser->GetItemDisplayname(browser->GetPageIndex()+i));

				if(browser->IsDir(browser->GetPageIndex()+i)) // directory
				{
					fileList[i]->SetIcon(fileListFolder[i]);
					fileListText[i]->SetPosition(30,0);
					fileListTextOver[i]->SetPosition(30,0);
				}
				else
				{
				    char *fileext = strrchr(browser->GetItemDisplayname(browser->GetPageIndex()+i), '.');
					fileListText[i]->SetPosition(32,0);
					fileListTextOver[i]->SetPosition(32,0);
				    if(fileext)
				    {
                        if(Settings.FileExtensions.CompareImage(fileext) == 0)
                        {
                            fileList[i]->SetIcon(fileListGFX[i]);
                        }
                        else if(Settings.FileExtensions.CompareAudio(fileext) == 0)
                        {
                            fileList[i]->SetIcon(fileListSFX[i]);
                        }
                        else if(strcasecmp(fileext, ".pls") == 0 || strcasecmp(fileext, ".m3u") == 0)
                        {
                            fileList[i]->SetIcon(fileListPLS[i]);
                        }
                        else if(strcasecmp(fileext, ".txt") == 0)
                        {
                            fileList[i]->SetIcon(fileListTXT[i]);
                        }
                        else if(strcasecmp(fileext, ".xml") == 0)
                        {
                            fileList[i]->SetIcon(fileListXML[i]);
                        }
                        else if(Settings.FileExtensions.CompareWiiBinary(fileext) == 0 || Settings.FileExtensions.CompareArchive(fileext) == 0)
                        {
                            fileList[i]->SetIcon(fileListArchives[i]);
                        }
                        else if(Settings.FileExtensions.CompareVideo(fileext) == 0)
                        {
                            fileList[i]->SetIcon(fileListVID[i]);
                        }
                        else if(Settings.FileExtensions.ComparePDF(fileext) == 0)
                        {
                            fileList[i]->SetIcon(fileListPDF[i]);
                        }
                        else
                        {
                            fileList[i]->SetIcon(fileListDefault[i]);
                        }
				    }
				    else
				    {
                        fileList[i]->SetIcon(fileListDefault[i]);
				    }
				}
			}
			else
			{
				fileList[i]->SetVisible(false);
				fileList[i]->SetState(STATE_DISABLED);
			}
		}

		if(i != selectedItem && fileList[i]->GetState() == STATE_SELECTED)
			fileList[i]->ResetState();
		else if(i == selectedItem && fileList[i]->GetState() == STATE_DEFAULT)
			fileList[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->wpad->ir.valid && !fileList[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
			t->chan = -1;

		fileList[i]->Update(t);
		t->chan = currChan;

		if(fileList[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
			browser->SetSelectedIndex(browser->GetPageIndex() + i);
		}

		ItemMarker * IMarker = browser->GetItemMarker();
		int itemCount = IMarker->GetItemcount();

		for(int n = 0; n < itemCount; n++)
		{
		    if(browser->GetPageIndex() + i == IMarker->GetItemIndex(n))
		    {
		        fileList[i]->SetState(STATE_SELECTED);
		    }
		}
	}

    scrollbar->SetPageSize(PAGESIZE);
    scrollbar->SetRowSize(0);
    scrollbar->SetSelectedItem(selectedItem);
    scrollbar->SetSelectedIndex(browser->GetPageIndex());

	listChanged = false;
}
