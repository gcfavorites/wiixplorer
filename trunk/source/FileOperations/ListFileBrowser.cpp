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

	while(fileBtn.size() > 0)
        RemoveButton(0);
}

void ListFileBrowser::AddButton()
{
    int size = fileBtn.size();

    fileBtnText.resize(size+1);
    fileBtnText[size] = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
    fileBtnText[size]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    fileBtnText[size]->SetPosition(32,0);
    fileBtnText[size]->SetMaxWidth(this->GetWidth() - 200, DOTTED);

    fileBtnTextOver.resize(size+1);
    fileBtnTextOver[size] = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
    fileBtnTextOver[size]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    fileBtnTextOver[size]->SetPosition(32,0);
    fileBtnTextOver[size]->SetMaxWidth(this->GetWidth() - 220, SCROLL_HORIZONTAL);

    fileSizeText.resize(size+1);
    fileSizeText[size] = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
    fileSizeText[size]->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
    fileSizeText[size]->SetPosition(0,0);

    fileSelectionImg.resize(size+1);
    fileSelectionImg[size] = new GuiImage(bgFileSelectionEntry);

    fileBtnIcon.resize(size+1);
    fileBtnIcon[size] = NULL;

    fileBtn.resize(size+1);
    fileBtn[size] = new GuiButton(507,30);
    fileBtn[size]->SetParent(this);
    fileBtn[size]->SetLabel(fileBtnText[size], 0);
    fileBtn[size]->SetLabel(fileSizeText[size], 1);
    fileBtn[size]->SetLabelOver(fileBtnTextOver[size]);
    fileBtn[size]->SetImageOver(fileSelectionImg[size]);
    fileBtn[size]->SetPosition(15,30*size+3);
    fileBtn[size]->SetTrigger(trigA);
    fileBtn[size]->SetSoundClick(btnSoundClick);
    fileBtn[size]->Clicked.connect(this, &ListFileBrowser::OnClicked);
}

void ListFileBrowser::RemoveButton(int i)
{
    if(i < 0 || i >= (int) fileBtn.size())
        return;

    if(fileBtnText[i])
        delete fileBtnText[i];
    if(fileBtnTextOver[i])
        delete fileBtnTextOver[i];
    if(fileSizeText[i])
        delete fileSizeText[i];
    if(fileSelectionImg[i])
        delete fileSelectionImg[i];
    if(fileBtnIcon[i])
        delete fileBtnIcon[i];
    if(fileBtn[i])
        delete fileBtn[i];

    fileBtnText.erase(fileBtnText.begin()+i);
    fileBtnTextOver.erase(fileBtnTextOver.begin()+i);
    fileSizeText.erase(fileSizeText.begin()+i);
    fileSelectionImg.erase(fileSelectionImg.begin()+i);
    fileBtnIcon.erase(fileBtnIcon.begin()+i);
    fileBtn.erase(fileBtn.begin()+i);
}

void ListFileBrowser::SetButton(int i, const char * name, u64 filesize, bool dir, bool enable)
{
    if(i < 0)
        return;

    else if(i >= (int) fileBtn.size())
    {
        AddButton();
    }

    if(!enable)
    {
        fileBtn[i]->SetVisible(false);
        fileBtn[i]->SetState(STATE_DISABLED);
        return;
    }

    if(fileBtnIcon[i])
        delete fileBtnIcon[i];

    fileBtnIcon[i] = GetIconFromExt((name ? strrchr(name, '.') : NULL), dir);

    fileBtnText[i]->SetText(name);
    fileBtnTextOver[i]->SetText(name);

    if(!dir)
    {
        char temp[100];
        if(filesize > KBSIZE && filesize < MBSIZE)
            sprintf(temp, "%0.1fKB", filesize/KBSIZE);
        else if(filesize > MBSIZE && filesize < GBSIZE)
            sprintf(temp, "%0.1fMB", filesize/MBSIZE);
        else if(filesize > GBSIZE)
            sprintf(temp, "%0.1fGB", filesize/GBSIZE);
        else
            sprintf(temp, "%LiB", filesize);

        fileSizeText[i]->SetText(temp);
    }
    else
    {
        fileSizeText[i]->SetText((char *) NULL);
    }

    fileBtn[i]->SetIcon(fileBtnIcon[i]);
    fileBtn[i]->SetVisible(true);

    if(fileBtn[i]->GetState() == STATE_DISABLED)
        fileBtn[i]->SetState(STATE_DEFAULT);
}

GuiImage * ListFileBrowser::GetIconFromExt(const char * fileext, bool dir)
{
    if(dir)
        return (new GuiImage(fileFolder));

    if(fileext)
    {
        if(Settings.FileExtensions.CompareImage(fileext) == 0)
        {
            return (new GuiImage(fileGFX));
        }
        else if(Settings.FileExtensions.CompareAudio(fileext) == 0)
        {
            return (new GuiImage(fileSFX));
        }
        else if(strcasecmp(fileext, ".pls") == 0 || strcasecmp(fileext, ".m3u") == 0)
        {
            return (new GuiImage(filePLS));
        }
        else if(strcasecmp(fileext, ".txt") == 0)
        {
            return (new GuiImage(fileTXT));
        }
        else if(strcasecmp(fileext, ".xml") == 0)
        {
            return (new GuiImage(fileXML));
        }
        else if(Settings.FileExtensions.CompareWiiBinary(fileext) == 0 || Settings.FileExtensions.CompareArchive(fileext) == 0)
        {
            return (new GuiImage(fileArchives));
        }
        else if(Settings.FileExtensions.CompareVideo(fileext) == 0)
        {
            return (new GuiImage(fileVID));
        }
        else if(Settings.FileExtensions.ComparePDF(fileext) == 0)
        {
            return (new GuiImage(filePDF));
        }
    }

    return (new GuiImage(fileDefault));
}

void ListFileBrowser::OnClicked(GuiElement *sender UNUSED, int pointer UNUSED, POINT p UNUSED)
{
    state = STATE_CLICKED;
}

void ListFileBrowser::SetSelected(int i)
{
    if(i < 0 || i >= (int) fileBtn.size())
        return;

    selectedItem = i;
    fileBtn[selectedItem]->SetState(STATE_SELECTED);
}

void ListFileBrowser::ResetState()
{
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for(u32 i = 0; i< fileBtn.size(); i++)
	{
		fileBtn[i]->ResetState();
	}
}

/**
 * Draw the button on screen
 */
void ListFileBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	for(u32 i = 0; i < fileBtn.size(); i++)
	{
		fileBtn[i]->Draw();
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

    if(browser)
        browser->UpdateMarker(t);

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
			if(selectedItem == (int) fileBtn.size()-1)
			{
				// move list down by 1
				browser->SetPageIndex(browser->GetPageIndex()+1);
				listChanged = true;
			}
			else if((int) fileBtn.size() > selectedItem+1 && fileBtn[selectedItem+1]->IsVisible())
			{
				fileBtn[selectedItem]->ResetState();
				fileBtn[++selectedItem]->SetState(STATE_SELECTED, t->chan);
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
		else if((int) fileBtn.size() > selectedItem && selectedItem > 0)
		{
			fileBtn[selectedItem]->ResetState();
			fileBtn[--selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
	}

	if(numEntries != browser->GetEntrieCount())
	{
	    numEntries = browser->GetEntrieCount();
	    scrollbar->SetEntrieCount(numEntries);
        listChanged = true;
	}

	for(int i = 0; i < PAGESIZE; i++)
	{
		if(listChanged)
		{
			if(browser->GetPageIndex()+i < browser->GetEntrieCount())
			{
				SetButton(i, browser->GetItemName(browser->GetPageIndex()+i), browser->GetFilesize(browser->GetPageIndex()+i), browser->IsDir(browser->GetPageIndex()+i), true);
			}
			else
			{
				SetButton(i, NULL, 0, false, false);
			}
		}

		if(i != selectedItem && fileBtn[i]->GetState() == STATE_SELECTED)
			fileBtn[i]->ResetState();
		else if(i == selectedItem && fileBtn[i]->GetState() == STATE_DEFAULT)
			fileBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->wpad->ir.valid && !fileBtn[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
			t->chan = -1;

		fileBtn[i]->Update(t);
		t->chan = currChan;

		if(fileBtn[i]->GetState() == STATE_SELECTED)
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
		        fileBtn[i]->SetState(STATE_SELECTED);
		    }
		}
	}

    scrollbar->SetPageSize(PAGESIZE);
    scrollbar->SetRowSize(0);
    scrollbar->SetSelectedItem(selectedItem);
    scrollbar->SetSelectedIndex(browser->GetPageIndex());

	listChanged = false;
}
