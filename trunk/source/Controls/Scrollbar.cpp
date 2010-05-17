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
#include "Memory/Resources.h"
#include "Scrollbar.hpp"
#include "main.h"

Scrollbar::Scrollbar(int h, u8 m)
{
    SelItem = 0;
    SelInd = 0;
    RowSize = 0;
    PageSize = 0;
    EntrieCount = 0;
    ScrollSpeed = 5;
    ScrollState = 0;
    listchanged = false;
    Mode = m;

	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
	btnSoundClick = Resources::GetSound(button_click_wav, button_click_wav_size);

    scrollbarTop = Resources::GetImageData(ScrollBG_Top_png, ScrollBG_Top_png_size);
    scrollbarButtom = Resources::GetImageData(ScrollBG_Buttom_png, ScrollBG_Buttom_png_size);
    scrollbarTile = Resources::GetImageData(ScrollBG_Tile_png, ScrollBG_Tile_png_size);
	arrowDown = Resources::GetImageData(scrollbar_arrowdown_png, scrollbar_arrowdown_png_size);
	arrowDownOver = Resources::GetImageData(scrollbar_arrowdown_over_png, scrollbar_arrowdown_over_png_size);
	arrowUp = Resources::GetImageData(scrollbar_arrowup_png, scrollbar_arrowup_png_size);
	arrowUpOver = Resources::GetImageData(scrollbar_arrowup_over_png, scrollbar_arrowup_over_png_size);
	scrollbarBox = Resources::GetImageData(scrollbar_box_png, scrollbar_box_png_size);
	scrollbarBoxOver = Resources::GetImageData(scrollbar_box_over_png, scrollbar_box_over_png_size);

    height = h;
    width = scrollbarTop->GetWidth();

    MinHeight = arrowUp->GetHeight()-7;
    MaxHeight = height-scrollbarBox->GetHeight()-arrowDown->GetHeight()/2-7;

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

    int Tiles = (height-40-scrollbarTop->GetHeight()-scrollbarButtom->GetHeight())/4;
    int PositionY = 25;

	scrollbarTopImg = new GuiImage(scrollbarTop);
	scrollbarTopImg->SetParent(this);
	scrollbarTopImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	scrollbarTopImg->SetPosition(0, PositionY);
	PositionY += scrollbarTop->GetHeight();

	scrollbarTileImg = new GuiImage(scrollbarTile);
	scrollbarTileImg->SetParent(this);
	scrollbarTileImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	scrollbarTileImg->SetPosition(0, PositionY);
	scrollbarTileImg->SetTileVertical(Tiles);
	PositionY += Tiles*scrollbarTile->GetHeight();

	scrollbarButtomImg = new GuiImage(scrollbarButtom);
	scrollbarButtomImg->SetParent(this);
	scrollbarButtomImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	scrollbarButtomImg->SetPosition(0, PositionY);

	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOverImg = new GuiImage(arrowUpOver);
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	arrowUpBtn->SetPosition(0, 0);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetClickable(false);
	arrowUpBtn->SetHoldable(true);
	arrowUpBtn->SetTrigger(trigHeldA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);
    arrowUpBtn->Held.connect(this, &Scrollbar::OnUpButtonHold);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	arrowDownBtn->SetPosition(0, 0);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetClickable(false);
	arrowDownBtn->SetHoldable(true);
	arrowDownBtn->SetTrigger(trigHeldA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);
    arrowDownBtn->Held.connect(this, &Scrollbar::OnDownButtonHold);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	scrollbarBoxBtn->SetPosition(0, MinHeight);
	scrollbarBoxBtn->SetMinY(MinHeight);
	scrollbarBoxBtn->SetMaxY(MaxHeight);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetClickable(false);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);
    scrollbarBoxBtn->Held.connect(this, &Scrollbar::OnBoxButtonHold);
}

Scrollbar::~Scrollbar()
{
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnSoundClick);
	Resources::Remove(scrollbarTop);
	Resources::Remove(scrollbarButtom);
	Resources::Remove(scrollbarTile);
	Resources::Remove(arrowDown);
	Resources::Remove(arrowDownOver);
	Resources::Remove(arrowUp);
	Resources::Remove(arrowUpOver);
	Resources::Remove(scrollbarBox);
	Resources::Remove(scrollbarBoxOver);

	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;

	delete scrollbarTopImg;
	delete scrollbarButtomImg;
	delete scrollbarTileImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;

	delete trigHeldA;
}

void Scrollbar::OnUpButtonHold(GuiElement *sender, int pointer, POINT p)
{
    if(ScrollState < ScrollSpeed)
        return;

    if(Mode == ICONMODE)
    {
        if(SelInd+SelItem-RowSize >= 0)
        {
            SelItem = SelItem-RowSize;
            if(SelItem < 0)
            {
                // move list up by 1
                SelInd = SelInd-RowSize;
                SelItem = SelItem+RowSize;
            }
        }
    }
    else if(Mode == LISTMODE)
    {
        if(SelItem == 0 && SelInd > 0)
		{
			// move list up by 1
			--SelInd;
		}
		else if(SelInd+SelItem > 0)
		{
			--SelItem;
		}
    }

    ScrollState = 0;
    listchanged = true;
}

void Scrollbar::OnDownButtonHold(GuiElement *sender, int pointer, POINT p)
{
    if(ScrollState < ScrollSpeed)
        return;

    if(Mode == ICONMODE)
    {
        int i = RowSize;
        while(SelInd+SelItem+RowSize >= EntrieCount && i > 0 && SelItem > 0 && RowSize < EntrieCount)
        {
            --i;
            --SelItem;
        }
		if(SelInd+SelItem+RowSize < EntrieCount)
		{
            SelItem = SelItem+RowSize;
			if(SelItem >= PageSize)
			{
				// move list down by 1
				SelInd += RowSize;
                SelItem = SelItem-RowSize;
			}
		}
    }
    else if(Mode == LISTMODE)
    {
		if(SelInd+SelItem + 1 < EntrieCount)
		{
			if(SelItem == PageSize-1)
			{
				// move list down by 1
				SelInd++;
			}
			else
			{
				SelItem++;
			}
		}
    }

    ScrollState = 0;
    listchanged = true;
}

void Scrollbar::OnBoxButtonHold(GuiElement *sender, int pointer, POINT p)
{
    if(ScrollState < ScrollSpeed)
        return;

    if(!userInput[pointer].wpad->ir.valid)
        return;

    p.y = p.y+sender->GetTop()-this->GetTop()-scrollbarBox->GetHeight()/2;

    int positionWiimote = cut_bounds(p.y-MinHeight, 0, MaxHeight-MinHeight);

    int newSelected = (int) ((float) positionWiimote / (float) (MaxHeight-MinHeight) * (float) (EntrieCount-1));

    if(Mode == ICONMODE)
    {
        int rows = (int) floor(((float) (newSelected-SelInd-SelItem)) / ((float) RowSize));

        while(SelInd+rows*RowSize >= EntrieCount-PageSize+RowSize)
            rows--;

        int pageIndex = cut_bounds(SelInd+rows*RowSize, 0, EntrieCount-1-RowSize);

        if(newSelected <= 0)
            SelItem = 0;
        else if(newSelected >= EntrieCount-1)
            SelItem = EntrieCount-1-pageIndex;

        SelInd = pageIndex;
    }
    else if(Mode == LISTMODE)
    {
        int diff = newSelected-SelInd-SelItem;

        if(newSelected <= 0)
            SelItem = 0;
        else if(newSelected >= EntrieCount-1)
            SelItem = (PageSize-1 < EntrieCount-1) ? PageSize-1 : EntrieCount-1;

        SelInd = cut_bounds(SelInd+diff, 0, ((EntrieCount-PageSize < 0) ? 0 : EntrieCount-PageSize));
    }

    ScrollState = 0;
    listchanged = true;
}

void Scrollbar::SetPageSize(int size)
{
    if(PageSize == size)
        return;

    PageSize = size;
    listchanged = true;
}

void Scrollbar::SetRowSize(int size)
{
    if(RowSize == size)
        return;

    RowSize = size;
    listchanged = true;
}

void Scrollbar::SetSelectedItem(int pos)
{
    if(SelItem == pos)
        return;

    SelItem = pos;
    listchanged = true;
}

void Scrollbar::SetSelectedIndex(int pos)
{
    if(SelInd == pos)
        return;

    SelInd = pos;
    listchanged = true;
}

void Scrollbar::SetEntrieCount(int cnt)
{
    if(EntrieCount == cnt)
        return;

    EntrieCount = cnt;
    listchanged = true;
}

bool Scrollbar::ListChanged()
{
    bool ret = listchanged;
    listchanged = false;

    return ret;
}

void Scrollbar::SetScrollboxPosition()
{
    if(!listchanged)
        return;

    if(Mode == ICONMODE)
    {
        u8 row = (u8) floor((float) SelItem / (float) RowSize);

        int position = MinHeight+(MaxHeight-MinHeight)*(SelInd+row*RowSize)/(EntrieCount-1);

        if(position < MinHeight)
            position = MinHeight;
        else if(position > MaxHeight || ((SelInd+PageSize >= (EntrieCount-1)) && row > 1))
            position = MaxHeight;

        scrollbarBoxBtn->SetPosition(0, position);
    }
    else if(Mode == LISTMODE)
    {
        int position = MinHeight+(MaxHeight-MinHeight)*(SelInd+SelItem)/(EntrieCount-1);

        if(position < MinHeight)
            position = MinHeight;
        else if(position > MaxHeight || (SelInd+SelItem >= EntrieCount-1))
            position = MaxHeight;

        scrollbarBoxBtn->SetPosition(0, position);
    }
}

void Scrollbar::Draw()
{
	scrollbarTileImg->Draw();
	scrollbarTopImg->Draw();
	scrollbarButtomImg->Draw();
	arrowUpBtn->Draw();
	arrowDownBtn->Draw();
	scrollbarBoxBtn->Draw();

    ++ScrollState;

	UpdateEffects();
}

void Scrollbar::Update(GuiTrigger * t)
{
	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);
	scrollbarBoxBtn->Update(t);
	SetScrollboxPosition();
}
