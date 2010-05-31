 /****************************************************************************
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
#include "gui.h"
#include "Memory/Resources.h"

GuiTooltip::GuiTooltip(const char *t)
{
	text = NULL;
	FontSize = 22;
	ElapseTime = 2.0f;
	color = (GXColor){0, 0, 0, 255};

    tooltipLeft = Resources::GetImageData(tooltip_left_png, tooltip_left_png_size);
    tooltipTile = Resources::GetImageData(tooltip_tile_png, tooltip_tile_png_size);
    tooltipRight = Resources::GetImageData(tooltip_right_png, tooltip_right_png_size);
    leftImage = new GuiImage(tooltipLeft);
    tileImage = new GuiImage(tooltipTile);
    rightImage = new GuiImage(tooltipRight);

	height = leftImage->GetHeight();

	leftImage->SetParent(this);
	tileImage->SetParent(this);
	rightImage->SetParent(this);

	SetText(t);
	SetVisible(false);
}

GuiTooltip::~GuiTooltip()
{
	if(text)
        delete text;

    delete leftImage;
    delete tileImage;
    delete rightImage;

    Resources::Remove(tooltipLeft);
    Resources::Remove(tooltipTile);
    Resources::Remove(tooltipRight);
}

void GuiTooltip::SetText(const char * t)
{
    LOCK(this);

	if(text)
		delete text;
    text = NULL;

	int tile_cnt = 0;
	if(t)
	{
	    text = new GuiText(t, FontSize, color);
		text->SetParent(this);
		text->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
		tile_cnt = (int) ((float) (text->GetTextWidth()-12) / (float) tileImage->GetWidth());
		if(tile_cnt < 0)
            tile_cnt = 0;
	}
	tileImage->SetPosition(leftImage->GetWidth(), 0);
	tileImage->SetTileHorizontal(tile_cnt);
	rightImage->SetPosition(leftImage->GetWidth() + tile_cnt * tileImage->GetWidth(), 0);
	width = leftImage->GetWidth() + tile_cnt * tileImage->GetWidth() + rightImage->GetWidth();
}

void GuiTooltip::SetFontSize(int size)
{
    FontSize = size;

    if(text)
        SetText(text->GetOrigText());
}

void GuiTooltip::SetColor(GXColor c)
{
    color = c;

    if(text)
        SetText(text->GetOrigText());
}

void GuiTooltip::SetElapseTime(float t)
{
    ElapseTime = t;
}

void GuiTooltip::Draw()
{
	if((!IsVisible() && !GetEffect()) || state == STATE_DISABLED)
        return;

	leftImage->Draw();
	tileImage->Draw();
	rightImage->Draw();
	if(text)
        text->Draw();

	this->UpdateEffects();
}
