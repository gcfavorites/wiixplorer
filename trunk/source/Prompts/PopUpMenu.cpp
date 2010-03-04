/***************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * Copyright (C) 2010
 * by dude
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
 * PopUpMenu.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/

#include "PopUpMenu.h"
#include "Controls/MainWindow.h"

const int ButtonX = 20;
const u32 ButtonHeight = 32;
const u32 MaxVisible = 10;

PopUpMenu::PopUpMenu(int x, int y)
    :GuiWindow(0, 0)
{
    choice = -1;
	maxTxtWidth = 0;
	scrollIndex = 0;
	hasIcons = false;

	width = 0;
	height = 0;

    PopUpMenuUpper = Resources::GetImageData(ClickMenuUpper_png, ClickMenuUpper_png_size);
	PopUpMenuMiddle = Resources::GetImageData(ClickMenuMiddle_png, ClickMenuMiddle_png_size);
    PopUpMenuLower = Resources::GetImageData(ClickMenuLower_png, ClickMenuLower_png_size);
    PopUpMenuSelect = Resources::GetImageData(menu_selection_png, menu_selection_png_size);
	PopUpMenuScrollUp = Resources::GetImageData(arrow_up_png, arrow_up_png_size);
	PopUpMenuScrollDown = Resources::GetImageData(arrow_down_png, arrow_down_png_size);
	PopUpMenuExpand = Resources::GetImageData(arrow_right_png, arrow_right_png_size);

	PopUpMenuUpperImg = new GuiImage(PopUpMenuUpper);
	PopUpMenuMiddleImg = new GuiImage(PopUpMenuMiddle);
    PopUpMenuLowerImg = new GuiImage(PopUpMenuLower);
	PopUpMenuScrollUpImg = new GuiImage(PopUpMenuScrollUp);
	PopUpMenuScrollDownImg = new GuiImage(PopUpMenuScrollDown);

	PopUpMenuClick = Resources::GetSound(button_click_pcm, button_click_pcm_size);

	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    NoBtn = new GuiButton(screenwidth, screenheight);
    NoBtn->SetPosition(-x, -y);
    NoBtn->SetTrigger(trigA);
    NoBtn->SetTrigger(trigB);
    NoBtn->Clicked.connect(this, &PopUpMenu::OnClick);

	ScrollUp = new GuiButton(16, 16);
	ScrollUp->SetImage(PopUpMenuScrollUpImg);
	ScrollUp->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	ScrollUp->SetTrigger(trigA);
	ScrollUp->SetVisible(false);
	ScrollUp->Clicked.connect(this, &PopUpMenu::OnScrollUp);

	ScrollDown = new GuiButton(16, 16);
	ScrollDown->SetImage(PopUpMenuScrollDownImg);
	ScrollDown->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	ScrollDown->SetTrigger(trigA);
	ScrollDown->SetVisible(false);
	ScrollDown->Clicked.connect(this, &PopUpMenu::OnScrollDown);

    Append(PopUpMenuUpperImg);
    Append(PopUpMenuMiddleImg);
    Append(PopUpMenuLowerImg);
    Append(NoBtn);
	Append(ScrollUp);
	Append(ScrollDown);

	xpos = x;
	ypos = y;
}

PopUpMenu::~PopUpMenu()
{
	MainWindow::Instance()->HaltGui();
	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	RemoveAll();

	Resources::Remove(PopUpMenuMiddle);
	Resources::Remove(PopUpMenuUpper);
	Resources::Remove(PopUpMenuLower);
	Resources::Remove(PopUpMenuSelect);
	Resources::Remove(PopUpMenuScrollUp);
	Resources::Remove(PopUpMenuScrollDown);
	Resources::Remove(PopUpMenuExpand);
	Resources::Remove(PopUpMenuClick);

	for (u32 i = 0; i < Item.size(); i++)
	{
		delete Item[i].ButtonMenuSelect;
		delete Item[i].Button;
		delete Item[i].ButtonTxt;
		delete Item[i].ButtonTxtOver;

		if (Item[i].Icon)
		{
			Resources::Remove(Item[i].Icon);
			delete Item[i].IconImg;
		}

		if (Item[i].ExpandImg)
			delete Item[i].ExpandImg;
	}

	Item.clear();

	delete PopUpMenuMiddleImg;
	delete PopUpMenuUpperImg;
	delete PopUpMenuLowerImg;
	delete PopUpMenuScrollUpImg;
	delete PopUpMenuScrollDownImg;

	delete NoBtn;

	delete trigA;
	delete trigB;

	MainWindow::Instance()->ResumeGui();
}

void PopUpMenu::AddItem(const char *text, const u8 *icon, u32 icon_size, bool submenu)
{
	Items item;

	item.ButtonTxt = new GuiText(text, 24, (GXColor){0, 0, 0, 255});
	item.ButtonTxtOver = new GuiText(text, 24, (GXColor){28, 32, 190, 255});
	item.Button = new GuiButton(0, 0);
	item.ButtonMenuSelect = new GuiImage(PopUpMenuSelect);
	item.ButtonTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	item.ButtonTxtOver->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	item.Button->SetLabel(item.ButtonTxt);
	item.Button->SetLabelOver(item.ButtonTxtOver);
	item.Button->SetSoundClick(PopUpMenuClick);
	item.Button->SetImageOver(item.ButtonMenuSelect);
	item.Button->SetTrigger(trigA);
	item.Button->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	item.Button->Clicked.connect(this, &PopUpMenu::OnClick);

	Append(item.Button);

	if (icon)
	{
		item.Icon = Resources::GetImageData(icon, icon_size);

		item.IconImg = new GuiImage(item.Icon);
		item.IconImg->SetPosition(ButtonX, (Item.size()+1)*ButtonHeight-14);
		item.IconImg->SetScale(0.8);

		hasIcons = true;

		Append(item.IconImg);
	}
	else
	{
		item.Icon = NULL;
		item.IconImg = NULL;
	}

	if (submenu)
	{
		item.ExpandImg = new GuiImage(PopUpMenuExpand);

		Append(item.ExpandImg);
	}
	else
		item.ExpandImg = NULL;

	if (maxTxtWidth < item.ButtonTxt->GetTextWidth())
		maxTxtWidth = item.ButtonTxt->GetTextWidth();

	Item.push_back(item);
}

void PopUpMenu::Finish()
{
	u32 middleheight = ButtonHeight*Item.size();
	int rightmargin = (hasIcons ? ButtonX+40 : ButtonX);

	if (Item.size() > MaxVisible)
	{
		ScrollDown->SetVisible(true);

		middleheight = ButtonHeight*MaxVisible;

		for (u32 i = MaxVisible; i < Item.size(); i++)
		{
			Item[i].Button->SetVisible(false);
			Item[i].Button->SetState(STATE_DISABLED);

			if (Item[i].IconImg)
				Item[i].IconImg->SetVisible(false);

			if (Item[i].ExpandImg)
				Item[i].ExpandImg->SetVisible(false);
		}
	}

	float NewScale = 1.0f;

	PopUpMenuMiddleImg->SetTileVertical(middleheight/4);

	if (maxTxtWidth+rightmargin+40 > (PopUpMenuMiddleImg->GetWidth()))
		NewScale = 1.0f * (maxTxtWidth+rightmargin+40)/(PopUpMenuMiddleImg->GetWidth());

	PopUpMenuUpperImg->SetScaleX(NewScale);
	PopUpMenuMiddleImg->SetScaleX(NewScale);
	PopUpMenuLowerImg->SetScaleX(NewScale);

	u32 scaledX = (NewScale-1)*PopUpMenuMiddleImg->GetWidth()/2;

	height = PopUpMenuUpperImg->GetHeight()+middleheight+PopUpMenuLowerImg->GetHeight();
	width  = maxTxtWidth+rightmargin+20;

	u32 y = ypos;
	u32 x = xpos;

	if (y + height + 60 > (u32)screenheight)
		y = screenheight-height-60;

	if (x + width + 40 > (u32)screenwidth)
		x = screenwidth - width - 40;

	PopUpMenuUpperImg->SetPosition(scaledX, 0);
	PopUpMenuMiddleImg->SetPosition(scaledX, PopUpMenuUpperImg->GetHeight());
	PopUpMenuLowerImg->SetPosition(scaledX, PopUpMenuUpperImg->GetHeight()+middleheight);
	ScrollUp->SetPosition(scaledX+PopUpMenuMiddleImg->GetWidth()/2-ButtonX, 0);
	ScrollDown->SetPosition(scaledX+PopUpMenuMiddleImg->GetWidth()/2-ButtonX, PopUpMenuLowerImg->GetTop());

	for (u32 i = 0; i < Item.size(); i++)
	{
		Item[i].Button->SetSize(maxTxtWidth, ButtonHeight);
		Item[i].Button->SetPosition(rightmargin, (i+1)*ButtonHeight-8);

		Item[i].ButtonMenuSelect->SetScaleX(NewScale);
		Item[i].ButtonMenuSelect->SetPosition(20, -5);

		if (Item[i].ExpandImg)
			Item[i].ExpandImg->SetPosition(maxTxtWidth+rightmargin+5, (i+1)*ButtonHeight-5);
	}

	SetPosition(x, y);
}

int PopUpMenu::GetChoice()
{
	return choice;
}

void PopUpMenu::OnClick(GuiElement *sender, int pointer, POINT p)
{
	sender->ResetState();
	//TODO add the functions instead of the link
	if (sender == NoBtn)
	{
		choice = -10;
		return;
	}

	for (u32 i = 0; i < Item.size(); i++)
	{
		if(sender == Item[i].Button)
		{
			choice = i;
			break;
		}
	}
}

void PopUpMenu::OnScrollUp(GuiElement *sender, int pointer, POINT p)
{
	sender->ResetState();

	Scroll(UP);
}

void PopUpMenu::OnScrollDown(GuiElement *sender, int pointer, POINT p)
{
	sender->ResetState();

	Scroll(DOWN);
}

void PopUpMenu::Scroll(int direction)
{
	choice = -1;
	int step = ButtonHeight;

	if (direction == UP)
	{
		if (scrollIndex < 1)
			return;

		scrollIndex--;
		ScrollDown->SetVisible(true);
	}
	else
	{
		if (scrollIndex >= Item.size()-MaxVisible)
			return;

		scrollIndex++;
		ScrollUp->SetVisible(true);
		step *= -1;
	}

	for (u32 i = 0; i < Item.size(); i++)
	{
		bool visible = (i >= scrollIndex && i < scrollIndex+MaxVisible);
		int state = visible ? STATE_DEFAULT : STATE_DISABLED;

		Item[i].Button->SetVisible(visible);
		Item[i].Button->SetPosition(Item[i].Button->GetLeft()-GetLeft(), Item[i].Button->GetTop()-GetTop()+step);
		Item[i].Button->SetState(state);

		if (Item[i].IconImg)
		{
			Item[i].IconImg->SetVisible(visible);
			Item[i].IconImg->SetPosition(Item[i].IconImg->GetLeft()-GetLeft(), Item[i].IconImg->GetTop()-GetTop()+step);
		}

		if (Item[i].ExpandImg)
		{
			Item[i].ExpandImg->SetVisible(visible);
			Item[i].ExpandImg->SetPosition(Item[i].ExpandImg->GetLeft()-GetLeft(), Item[i].ExpandImg->GetTop()-GetTop()+step);
		}
	}

	if (scrollIndex >= Item.size()-MaxVisible)
		ScrollDown->SetVisible(false);

	if (scrollIndex < 1)
		ScrollUp->SetVisible(false);
}
