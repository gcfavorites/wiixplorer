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
#include "gui_keyboard.h"
#include "Memory/Resources.h"

/**
 * Constructor for the GuiKeyboard class.
 */
GuiKeyboard::GuiKeyboard(const char * t, u32 max)
{
	SetupKeyboard(wfmt(t), max);
}

GuiKeyboard::GuiKeyboard(const wchar_t * t, u32 max)
{
	SetupKeyboard(t, max);
}

void GuiKeyboard::SetupKeyboard(const wchar_t * t, u32 max)
{
	selectable = true;
	kbtextstr.assign(t);
	kbtextmaxlen = max;
	CurrentFirstLetter = 0;
	if(t)
	{
		CurrentFirstLetter = wcslen(t)-MAX_KEYBOARD_DISPLAY+1;
		if(CurrentFirstLetter < 0)
			CurrentFirstLetter = 0;
	}

	height += 100;

	keyTextbox = Resources::GetImageData("keyboard_textbox.png");
	keyTextboxImg = new GuiImage(keyTextbox);
	keyTextboxImg->SetAlignment(ALIGN_CENTER | ALIGN_TOP);
	keyTextboxImg->SetPosition(0, 0);
	this->Append(keyTextboxImg);

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	kbText = new GuiText(GetDisplayText(), 20, (GXColor){0, 0, 0, 0xff});
	kbText->SetAlignment(ALIGN_LEFT | ALIGN_TOP);

	TextPointerBtn = new TextPointer(kbText, 0);
	TextPointerBtn->SetAlignment(ALIGN_CENTER | ALIGN_TOP);
	TextPointerBtn->SetPosition(0, 11);
	TextPointerBtn->SetHoldable(true);
	TextPointerBtn->SetTrigger(trigHeldA);
	TextPointerBtn->Held.connect(this, &GuiKeyboard::OnPointerHeld);
	this->Append(TextPointerBtn);

	TextPointerBtn->PositionChanged(0, 0, 0);
	TextPointerBtn->SetPointerPosition(MAX_KEYBOARD_DISPLAY-1);

	trigLeft = new GuiTrigger;
	trigLeft->SetButtonOnlyTrigger(-1, WiiControls.LeftButton | ClassicControls.LeftButton << 16, GCControls.LeftButton);
	trigRight = new GuiTrigger;
	trigRight->SetButtonOnlyTrigger(-1, WiiControls.RightButton | ClassicControls.RightButton << 16, GCControls.RightButton);

	GoRight = new GuiButton(1, 1);
	GoRight->SetSoundClick(keySoundClick);
	GoRight->SetTrigger(trigRight);
	GoRight->Clicked.connect(this, &GuiKeyboard::OnPositionMoved);
	this->Append(GoRight);

	GoLeft = new GuiButton(1, 1);
	GoLeft->SetSoundClick(keySoundClick);
	GoLeft->SetTrigger(trigLeft);
	GoLeft->Clicked.connect(this, &GuiKeyboard::OnPositionMoved);
	this->Append(GoLeft);

	keyClearText = new GuiText(tr("Clear"), 20, (GXColor){0, 0, 0, 0xff});
	keyClearImg = new GuiImage(keyMedium);
	keyClearOverImg = new GuiImage(keyMediumOver);
	keyClear = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyClear->SetImage(keyClearImg);
	keyClear->SetImageOver(keyClearOverImg);
	keyClear->SetSoundOver(keySoundOver);
	keyClear->SetSoundClick(keySoundClick);
	keyClear->SetLabel(keyClearText);
	keyClear->SetTrigger(trigA);
	keyClear->SetPosition(78, 4*42+80);
	keyClear->SetEffectGrow();
	keyClear->Clicked.connect(this, &GuiKeyboard::OnClearKeyPress);
	this->Append(keyClear);

	this->keyPressed.connect(this, &GuiKeyboard::OnKeyPress);
}

/**
 * Destructor for the GuiKeyboard class.
 */
GuiKeyboard::~GuiKeyboard()
{
	delete TextPointerBtn;
	delete GoRight;
	delete GoLeft;
	delete kbText;
	delete keyTextboxImg;
	delete keyClearText;
	delete keyClearImg;
	delete keyClearOverImg;
	delete keyClear;
	delete trigHeldA;
	delete trigLeft;
	delete trigRight;
	Resources::Remove(keyTextbox);
}

std::string GuiKeyboard::GetUTF8String() const
{
	return kbtextstr.toUTF8();
}

const wchar_t * GuiKeyboard::GetString()
{
	return kbtextstr.c_str();
}

void GuiKeyboard::AddChar(int pos, wchar_t Char)
{
	if(pos < 0)
		return;

	kbtextstr.insert(pos, 1, Char);

	MoveText(1);
}

void GuiKeyboard::RemoveChar(int pos)
{
	if (pos < 0 || pos >= (int) kbtextstr.size())
		return;

	kbtextstr.erase(pos, 1);
	MoveText(-1);
}

void GuiKeyboard::MoveText(int n)
{
	int strlength = kbtextstr.size();

	if(strlength > MAX_KEYBOARD_DISPLAY)
	{
		CurrentFirstLetter += n;
		if(CurrentFirstLetter < 0)
			CurrentFirstLetter = 0;
	}
	else
		CurrentFirstLetter = 0;

	kbText->SetText(GetDisplayText());
	TextPointerBtn->TextWidthChanged();

	if(strlength > MAX_KEYBOARD_DISPLAY-2)
		TextPointerBtn->SetPointerPosition(TextPointerBtn->GetCurrentLetter());
	else
		TextPointerBtn->SetPointerPosition(TextPointerBtn->GetCurrentLetter()+n);
}

void GuiKeyboard::OnPointerHeld(GuiButton *sender UNUSED, int pointer, const POINT &p)
{
	TextPointerBtn->PositionChanged(pointer, p.x - TextPointerBtn->GetLeft(), p.y - TextPointerBtn->GetTop());
}

void GuiKeyboard::OnPositionMoved(GuiButton *sender, int pointer UNUSED, const POINT &p UNUSED)
{
	sender->ResetState();

	if(sender == GoLeft)
	{
		int currentPointLetter = TextPointerBtn->GetCurrentLetter();
		currentPointLetter--;
		if(currentPointLetter < 0)
		{
			currentPointLetter = 0;
			CurrentFirstLetter--;
			if(CurrentFirstLetter < 0)
				CurrentFirstLetter = 0;
		}
		kbText->SetText(GetDisplayText());
		TextPointerBtn->TextWidthChanged();
		TextPointerBtn->SetPointerPosition(currentPointLetter);
	}
	else if(sender == GoRight)
	{
		int currentPointLetter = TextPointerBtn->GetCurrentLetter();
		currentPointLetter++;
		int strlength = kbtextstr.length();
		if(currentPointLetter > (MAX_KEYBOARD_DISPLAY-1) || currentPointLetter > strlength)
		{
			currentPointLetter--;
			CurrentFirstLetter++;
			if(CurrentFirstLetter > (strlength-MAX_KEYBOARD_DISPLAY+1))
				CurrentFirstLetter = strlength-MAX_KEYBOARD_DISPLAY+1;
		}
		kbText->SetText(GetDisplayText());
		TextPointerBtn->TextWidthChanged();
		TextPointerBtn->SetPointerPosition(currentPointLetter);
	}
}

void GuiKeyboard::OnClearKeyPress(GuiButton *sender, int pointer UNUSED, const POINT &p UNUSED)
{
	if(sender == keyClear)
	{
		CurrentFirstLetter = 0;
		kbtextstr.clear();
		kbText->SetText(GetDisplayText());
		TextPointerBtn->TextWidthChanged();
		TextPointerBtn->SetPointerPosition(0);
	}
}

void GuiKeyboard::OnKeyPress(wchar_t charCode)
{
	if(charCode == 0x08)
	{
		RemoveChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter()-1);
	}
	else if(kbtextstr.length() < kbtextmaxlen)
	{
		AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), charCode);
	}
}

const wchar_t * GuiKeyboard::GetDisplayText()
{
	int len = kbtextstr.size();

	if(len < MAX_KEYBOARD_DISPLAY)
		return kbtextstr.c_str();

	int n = 0;
	int startPos = CurrentFirstLetter;
	int endPos = startPos+MAX_KEYBOARD_DISPLAY;

	for(int i = startPos; i < endPos && i < len; ++i)
	{
		displayTxt[n] = kbtextstr.at(i);
		++n;
	}

	displayTxt[n] = 0;

	return displayTxt;
}
