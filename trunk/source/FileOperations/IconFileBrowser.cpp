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
#include "IconFileBrowser.hpp"
#include "Memory/Resources.h"
#include "FileStartUp/FileExtensions.h"
#include "menu.h"

/**
 * Constructor for the IconFileBrowser class.
 */
IconFileBrowser::IconFileBrowser(Browser * filebrowser, int w, int h)
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

	bgFileSelection = Resources::GetImageData(deviceselection_png, deviceselection_png_size);
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

	scrollbar = new Scrollbar(245, ICONMODE);
	scrollbar->SetParent(this);
	scrollbar->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbar->SetPosition(-10, 5);
	scrollbar->SetScrollSpeed(Settings.ScrollSpeed);

    RowSize = (width-25)/(fileDefault->GetWidth()+50);
	PageSize = RowSize*height/(fileDefault->GetHeight()+50);
}

/**
 * Destructor for the IconFileBrowser class.
 */
IconFileBrowser::~IconFileBrowser()
{
    browser = NULL;
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnSoundClick);
	Resources::Remove(bgFileSelection);
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

    delete scrollbar;

	delete trigA;

	for(u32 i = 0; i < Buttons.size(); i++)
	{
	    if(Buttons[i])
            delete Buttons[i];
	    if(ButtonText[i])
            delete ButtonText[i];
	    if(FileSelectionImg[i])
            delete FileSelectionImg[i];
	    if(ButtonImg[i])
            delete ButtonImg[i];
	    if(Tooltip[i])
            delete Tooltip[i];
	}

	Buttons.clear();
	FileSelectionImg.clear();
	ButtonText.clear();
	ButtonImg.clear();
	Tooltip.clear();
}

GuiImage * IconFileBrowser::GetIconFromExt(const char * fileext, bool dir)
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

void IconFileBrowser::AddButton()
{
    GuiImage * BtnImg = NULL;
    ButtonImg.push_back(BtnImg);

    GuiText * BtnTxt = new GuiText((char *) NULL, 14, (GXColor){0, 0, 0, 255});
    BtnTxt->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    BtnTxt->SetPosition(0, -10);
    BtnTxt->SetLinesToDraw(2);
    BtnTxt->SetMaxWidth(fileDefault->GetWidth()+38, WRAP);
    ButtonText.push_back(BtnTxt);

    GuiImage * Marker = new GuiImage(bgFileSelection);
    Marker->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    Marker->SetPosition(0, -17);
    FileSelectionImg.push_back(Marker);

    GuiTooltip * tmpToolTip = new GuiTooltip((char *) NULL);
    tmpToolTip->SetPosition(0, 0);
    Tooltip.push_back(tmpToolTip);

    GuiButton * Btn = new GuiButton(fileDefault->GetWidth()+40, fileDefault->GetHeight()+32);
    Btn->SetParent(this);
    Btn->SetLabel(BtnTxt);
    Btn->SetIcon(BtnImg);
    Btn->SetImageOver(Marker);
    Btn->SetTrigger(trigA);
    Btn->SetSoundClick(btnSoundClick);
    Btn->SetTooltip(tmpToolTip);
    Btn->Clicked.connect(this, &IconFileBrowser::OnButtonClicked);
    Buttons.push_back(Btn);
}

void IconFileBrowser::SetButton(int i, const char * name, bool dir, bool enable, int x, int y)
{
    if(i < 0)
        return;

    else if(i >= (int) Buttons.size())
    {
        AddButton();
    }

    if(!enable)
    {
        Buttons[i]->SetVisible(false);
        Buttons[i]->SetState(STATE_DISABLED);
        return;
    }

    if(ButtonImg[i])
        delete ButtonImg[i];

    ButtonImg[i] = GetIconFromExt((name ? strrchr(name, '.') : NULL), dir);
    ButtonImg[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    ButtonText[i]->SetText(name);
    Tooltip[i]->SetText(name);
    if(40+x+Tooltip[i]->GetWidth() > width)
    {
        Tooltip[i]->SetPosition(width-(40+x+Tooltip[i]->GetWidth()), -30);
    }
    else
    {
        Tooltip[i]->SetPosition(40, -30);
    }

    if(Tooltip[i]->GetLeft() < 0)
    {
        Tooltip[i]->SetPosition(10-x-GetLeft(), -30);
    }

    Buttons[i]->SetIcon(ButtonImg[i]);
    Buttons[i]->SetVisible(true);
    Buttons[i]->SetPosition(x, y);

    if(Buttons[i]->GetState() == STATE_DISABLED)
        Buttons[i]->SetState(STATE_DEFAULT);
}

void IconFileBrowser::RemoveButton(int i)
{
    if(i < 0 || i >= (int) Buttons.size())
        return;

    if(ButtonImg.at(i))
        delete ButtonImg.at(i);
    ButtonImg.erase(ButtonImg.begin()+i);

    if(ButtonText.at(i))
        delete ButtonText.at(i);
    ButtonText.erase(ButtonText.begin()+i);

    if(FileSelectionImg.at(i))
        delete FileSelectionImg.at(i);
    FileSelectionImg.erase(FileSelectionImg.begin()+i);

    if(Buttons.at(i))
        delete Buttons.at(i);
    Buttons.erase(Buttons.begin()+i);
}

void IconFileBrowser::OnButtonClicked(GuiElement *sender, int pointer, POINT p)
{
    state = STATE_CLICKED;
}

void IconFileBrowser::SetSelected(int i)
{
    if(i < 0 || i >= PAGESIZE)
        return;

    selectedItem = i;
    Buttons[selectedItem]->SetState(STATE_SELECTED);
}

void IconFileBrowser::ResetState()
{
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for(u32 i=0; i < Buttons.size(); i++)
	{
		Buttons[i]->ResetState();
	}
}

/**
 * Draw the button on screen
 */
void IconFileBrowser::Draw()
{
	if(!IsVisible())
		return;

	scrollbar->Draw();

	for(u32 i = 0; i < Buttons.size(); i++)
	{
	    Buttons[i]->Draw();
	}

    //needs a redraw for overrendering
	for(u32 i = 0; i < Buttons.size(); i++)
	{
        Tooltip[i]->Draw();
	}

	UpdateEffects();
}

void IconFileBrowser::Update(GuiTrigger * t)
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
		if(browser->GetPageIndex() + selectedItem + 1 < browser->GetEntrieCount())
		{
			if(selectedItem == PageSize-1)
			{
				// move list down by 1
				browser->SetPageIndex(browser->GetPageIndex()+RowSize);
                Buttons[selectedItem]->ResetState();
                selectedItem = PageSize-RowSize;
                Buttons[selectedItem]->SetState(STATE_SELECTED, t->chan);
				listChanged = true;
			}
			else if(Buttons[selectedItem+1]->IsVisible())
			{
				Buttons[selectedItem]->ResetState();
				Buttons[++selectedItem]->SetState(STATE_SELECTED, t->chan);
			}
		}
	}
	else if(t->Left())
	{
		if(selectedItem == 0 &&	 browser->GetPageIndex() + selectedItem > 0)
		{
			// move list up by 1
			browser->SetPageIndex(browser->GetPageIndex()-RowSize);
			Buttons[selectedItem]->ResetState();
			selectedItem = RowSize-1;
			Buttons[selectedItem]->SetState(STATE_SELECTED, t->chan);
			listChanged = true;
		}
		else if(selectedItem > 0)
		{
			Buttons[selectedItem]->ResetState();
			Buttons[--selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
	}
	else if(t->Up())
	{
		if(browser->GetPageIndex() + selectedItem - RowSize >= 0)
		{
            Buttons[selectedItem]->ResetState();
            selectedItem = selectedItem-RowSize;
			if(selectedItem < 0)
			{
				// move list up by 1
				browser->SetPageIndex(browser->GetPageIndex()-RowSize);
                selectedItem = selectedItem+RowSize;
			}
            Buttons[selectedItem]->SetState(STATE_SELECTED, t->chan);
            listChanged = true;
		}
	}
	else if(t->Down())
	{
        int i = RowSize;
        while(browser->GetPageIndex()+selectedItem+RowSize >= browser->GetEntrieCount() &&
              i > 0 && selectedItem > 0 && RowSize < browser->GetEntrieCount())
        {
            --i;
            --selectedItem;
        }
		if(browser->GetPageIndex() + selectedItem + RowSize < browser->GetEntrieCount())
		{
            Buttons[selectedItem]->ResetState();
            selectedItem = selectedItem+RowSize;
			if(selectedItem >= PageSize)
			{
				// move list down by 1
				browser->SetPageIndex(browser->GetPageIndex()+RowSize);
                selectedItem = selectedItem-RowSize;
			}
            Buttons[selectedItem]->SetState(STATE_SELECTED, t->chan);
            listChanged = true;
		}
	}

	//endNavigation:
	if(numEntries != browser->GetEntrieCount())
	{
	    numEntries = browser->GetEntrieCount();
	    scrollbar->SetEntrieCount(numEntries);
        listChanged = true;
	}

    u16 x = 25, y = 7;

	for(int i = 0; i < PageSize; i++)
	{
		if(listChanged)
		{
			if(browser->GetPageIndex()+i < browser->GetEntrieCount())
			{
                const char * name = browser->GetItemDisplayname(browser->GetPageIndex()+i);
                bool dir = browser->IsDir(browser->GetPageIndex()+i);

                SetButton(i, name, dir, true, x, y);
			}
			else
			{
                SetButton(i, NULL, false, false, 0, 0);
			}
            x += fileDefault->GetWidth()+50;
            if(x >= (u32) width-80)
            {
                x = 25;
                y += fileDefault->GetHeight()+50;
            }
		}

		if(i != selectedItem && Buttons[i]->GetState() == STATE_SELECTED)
			Buttons[i]->ResetState();
		else if(i == selectedItem && Buttons[i]->GetState() == STATE_DEFAULT)
			Buttons[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->wpad->ir.valid && !Buttons[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
			t->chan = -1;

		Buttons[i]->Update(t);
		t->chan = currChan;

		if(Buttons[i]->GetState() == STATE_SELECTED)
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
		        Buttons[i]->SetState(STATE_SELECTED);
		    }
		}
	}

    scrollbar->SetPageSize(PageSize);
    scrollbar->SetRowSize(RowSize);
    scrollbar->SetSelectedItem(selectedItem);
    scrollbar->SetSelectedIndex(browser->GetPageIndex());

	listChanged = false;
}
