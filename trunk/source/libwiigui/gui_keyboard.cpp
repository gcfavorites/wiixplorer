 /****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * Orignal OnScreenKeyboard by Tantric
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
 * gui_keyboard.cpp
 * for WiiXplorer 2009
 ***************************************************************************/
#include "gui_keyboard.h"
#include "Memory/Resources.h"
#include "Language/gettext.h"

static char tmptxt[MAX_KEYBOARD_DISPLAY];

/**
 * Constructor for the GuiKeyboard class.
 */

GuiKeyboard::GuiKeyboard(char * t, u32 max)
{
	width = 540;
	height = 400;
	shift = 0;
	caps = 0;
	selectable = true;
	focus = 0; // allow focus
	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;
	strncpy(kbtextstr, t, max);
	kbtextstr[max] = 0;
	kbtextmaxlen = max;
	CurrentFirstLetter = 0;
	if(t)
	{
        CurrentFirstLetter = strlen(t)-MAX_KEYBOARD_DISPLAY+1;
        if(CurrentFirstLetter < 0)
            CurrentFirstLetter = 0;
	}

	Key thekeys[4][13] = {
	{
        {'`','~'},
		{'1','!'},
		{'2','@'},
		{'3','#'},
		{'4','$'},
		{'5','%'},
		{'6','^'},
		{'7','&'},
		{'8','*'},
		{'9','('},
		{'0',')'},
		{'-','_'}
	},
	{
		{'q','Q'},
		{'w','W'},
		{'e','E'},
		{'r','R'},
		{'t','T'},
		{'y','Y'},
		{'u','U'},
		{'i','I'},
		{'o','O'},
		{'p','P'},
		{'[','{'},
		{']','}'},
		{0x5C,'|'}
	},
	{
		{'a','A'},
		{'s','S'},
		{'d','D'},
		{'f','F'},
		{'g','G'},
		{'h','H'},
		{'j','J'},
		{'k','K'},
		{'l','L'},
		{';',':'},
		{'\'','"'},
		{'=','+'}
	},

	{
		{'z','Z'},
		{'x','X'},
		{'c','C'},
		{'v','V'},
		{'b','B'},
		{'n','N'},
		{'m','M'},
		{',','<'},
		{'.','>'},
		{'/','?'},
		{'\0','\0'}
	}
	};
	memcpy(keys, thekeys, sizeof(thekeys));

	int KeyboardPosition = -25;

	keyTextbox = Resources::GetImageData(keyboard_textbox_png, keyboard_textbox_png_size);
	keyTextboxImg = new GuiImage(keyTextbox);
	keyTextboxImg->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	keyTextboxImg->SetPosition(0, 0);
	this->Append(keyTextboxImg);

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	kbText = new GuiText(GetDisplayText(kbtextstr), 20, (GXColor){0, 0, 0, 0xff});

    TextPointerBtn = new TextPointer(kbText, 20, 0);
	TextPointerBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	kbText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	TextPointerBtn->SetPosition(0, 11);
    TextPointerBtn->SetClickable(false);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->Held.connect(this, &GuiKeyboard::OnPointerHeld);
	this->Append(TextPointerBtn);

    TextPointerBtn->PositionChanged(0, 0, 0);
    TextPointerBtn->SetPointerPosition(MAX_KEYBOARD_DISPLAY-1);

	key = Resources::GetImageData(keyboard_key_png, keyboard_key_png_size);
	keyOver = Resources::GetImageData(keyboard_key_over_png, keyboard_key_over_png_size);
	keyMedium = Resources::GetImageData(keyboard_mediumkey_png, keyboard_mediumkey_png_size);
	keyMediumOver = Resources::GetImageData(keyboard_mediumkey_over_png, keyboard_mediumkey_over_png_size);
	keyLarge = Resources::GetImageData(keyboard_largekey_png, keyboard_largekey_png_size);
	keyLargeOver = Resources::GetImageData(keyboard_largekey_over_png, keyboard_largekey_over_png_size);

	keySoundOver = Resources::GetSound(button_over_pcm, button_over_pcm_size);
	keySoundClick = Resources::GetSound(button_click_pcm, button_click_pcm_size);

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    trigLeft = new GuiTrigger;
	trigLeft->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
    trigRight = new GuiTrigger;
	trigRight->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

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

	keyBackImg = new GuiImage(keyMedium);
	keyBackOverImg = new GuiImage(keyMediumOver);
	keyBackText = new GuiText(tr("Back"), 20, (GXColor){0, 0, 0, 0xff});
	keyBack = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyBack->SetImage(keyBackImg);
	keyBack->SetImageOver(keyBackOverImg);
	keyBack->SetLabel(keyBackText);
	keyBack->SetSoundOver(keySoundOver);
	keyBack->SetSoundClick(keySoundClick);
	keyBack->SetTrigger(trigA);
	keyBack->SetTrigger(trigB);
	keyBack->SetPosition(11*42+40+KeyboardPosition, 0*42+80);
	keyBack->SetEffectGrow();
	this->Append(keyBack);

	keyCapsImg = new GuiImage(keyMedium);
	keyCapsOverImg = new GuiImage(keyMediumOver);
	keyCapsText = new GuiText(tr("Caps"), 20, (GXColor){0, 0, 0, 0xff});
	keyCaps = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyCaps->SetImage(keyCapsImg);
	keyCaps->SetImageOver(keyCapsOverImg);
	keyCaps->SetLabel(keyCapsText);
	keyCaps->SetSoundOver(keySoundOver);
	keyCaps->SetSoundClick(keySoundClick);
	keyCaps->SetTrigger(trigA);
	keyCaps->SetPosition(0+KeyboardPosition, 2*42+80);
	keyCaps->SetEffectGrow();
	this->Append(keyCaps);

	keyShiftImg = new GuiImage(keyMedium);
	keyShiftOverImg = new GuiImage(keyMediumOver);
	keyShiftText = new GuiText(tr("Shift"), 20, (GXColor){0, 0, 0, 0xff});
	keyShift = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyShift->SetImage(keyShiftImg);
	keyShift->SetImageOver(keyShiftOverImg);
	keyShift->SetLabel(keyShiftText);
	keyShift->SetSoundOver(keySoundOver);
	keyShift->SetSoundClick(keySoundClick);
	keyShift->SetTrigger(trigA);
	keyShift->SetPosition(0+KeyboardPosition, 3*42+80);
	keyShift->SetEffectGrow();
	this->Append(keyShift);

	keyLineBreakText = new GuiText(tr("Linebreak"), 18, (GXColor){0, 0, 0, 0xff});
	keyLineBreakImg = new GuiImage(keyMedium);
	keyLineBreakOverImg = new GuiImage(keyMediumOver);
	keyLineBreak = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyLineBreak->SetImage(keyLineBreakImg);
	keyLineBreak->SetImageOver(keyLineBreakOverImg);
	keyLineBreak->SetSoundOver(keySoundOver);
	keyLineBreak->SetSoundClick(keySoundClick);
	keyLineBreak->SetLabel(keyLineBreakText);
	keyLineBreak->SetTrigger(trigA);
	keyLineBreak->SetPosition(9*42+40+KeyboardPosition, 4*42+80);
	keyLineBreak->SetEffectGrow();
	this->Append(keyLineBreak);

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
	keyClear->SetPosition(11*42+40+KeyboardPosition, 4*42+80);
	keyClear->SetEffectGrow();
	this->Append(keyClear);

	keySpaceImg = new GuiImage(keyLarge);
	keySpaceOverImg = new GuiImage(keyLargeOver);
	keySpace = new GuiButton(keyLarge->GetWidth(), keyLarge->GetHeight());
	keySpace->SetImage(keySpaceImg);
	keySpace->SetImageOver(keySpaceOverImg);
	keySpace->SetSoundOver(keySoundOver);
	keySpace->SetSoundClick(keySoundClick);
	keySpace->SetTrigger(trigA);
	keySpace->SetPosition(0, 4*42+80);
	keySpace->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	keySpace->SetEffectGrow();
	this->Append(keySpace);

	char txt[2] = { 0, 0 };

	int Pos = 0;

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<13; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				txt[0] = keys[i][j].ch;
				keyImg[i][j] = new GuiImage(key);
				keyImgOver[i][j] = new GuiImage(keyOver);
				keyTxt[i][j] = new GuiText(txt, 20, (GXColor){0, 0, 0, 0xff});
				keyTxt[i][j]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
				keyTxt[i][j]->SetPosition(0, -10);
				keyBtn[i][j] = new GuiButton(key->GetWidth(), key->GetHeight());
				keyBtn[i][j]->SetImage(keyImg[i][j]);
				keyBtn[i][j]->SetImageOver(keyImgOver[i][j]);
				keyBtn[i][j]->SetSoundOver(keySoundOver);
				keyBtn[i][j]->SetSoundClick(keySoundClick);
				keyBtn[i][j]->SetTrigger(trigA);
				keyBtn[i][j]->SetLabel(keyTxt[i][j]);
				if(i == 1)
                    Pos = i*20;
				else if(i > 0)
                    Pos = (i+2)*20;
                keyBtn[i][j]->SetPosition(j*42+Pos+KeyboardPosition, i*42+80);
				keyBtn[i][j]->SetEffectGrow();
				this->Append(keyBtn[i][j]);
			}
		}
	}
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
	delete keyCapsText;
	delete keyCapsImg;
	delete keyCapsOverImg;
	delete keyCaps;
	delete keyShiftText;
	delete keyShiftImg;
	delete keyShiftOverImg;
	delete keyShift;
	delete keyBackText;
	delete keyBackImg;
	delete keyBackOverImg;
	delete keyBack;
	delete keySpaceImg;
	delete keySpaceOverImg;
	delete keySpace;
	delete keyLineBreakText;
	delete keyLineBreakImg;
	delete keyLineBreakOverImg;
	delete keyLineBreak;
	delete keyClearText;
	delete keyClearImg;
	delete keyClearOverImg;
	delete keyClear;
	delete trigA;
	delete trigB;
	delete trigHeldA;
	delete trigLeft;
	delete trigRight;
	Resources::Remove(keyTextbox);
	Resources::Remove(key);
	Resources::Remove(keyOver);
	Resources::Remove(keyMedium);
	Resources::Remove(keyMediumOver);
	Resources::Remove(keyLarge);
	Resources::Remove(keyLargeOver);
	Resources::Remove(keySoundOver);
	Resources::Remove(keySoundClick);

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<13; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				delete keyImg[i][j];
				delete keyImgOver[i][j];
				delete keyTxt[i][j];
				delete keyBtn[i][j];
			}
		}
	}
}

void GuiKeyboard::AddChar(int pos, char Char)
{
    int stringlength = strlen(kbtextstr);

    if(stringlength+1 > (int) kbtextmaxlen)
        return;

    char * origText = strdup(kbtextstr);
    memset(kbtextstr, 0, sizeof(kbtextstr));


    int n = 0;
    int i = 0;

    while(i < stringlength+1)
    {
        if(n == pos)
        {
            kbtextstr[n] = Char;
            n++;
        }

        kbtextstr[n] = origText[i];

        n++;
        i++;
    }

    free(origText);

    MoveText(1);
}

void GuiKeyboard::RemoveChar(int pos)
{
    int stringlength = strlen(kbtextstr);

    int n = 0;
    int i = 0;

    while(i < stringlength+1)
    {
        if(i == pos)
        {
            i++;
        }

        kbtextstr[n] = kbtextstr[i];

        n++;
        i++;
    }
    kbtextstr[n] = '\0';

    MoveText(-1);
}

void GuiKeyboard::MoveText(int n)
{
    int strlength = strlen(kbText->GetOrigText())+1;

    if(strlength > MAX_KEYBOARD_DISPLAY-1)
    {
        CurrentFirstLetter += n;
        if(CurrentFirstLetter < 0)
            CurrentFirstLetter = 0;
    }

    kbText->SetText(GetDisplayText(kbtextstr));
    TextPointerBtn->TextWidthChanged();

    if(strlength > MAX_KEYBOARD_DISPLAY-1)
        TextPointerBtn->SetPointerPosition(TextPointerBtn->GetCurrentLetter());
    else
        TextPointerBtn->SetPointerPosition(TextPointerBtn->GetCurrentLetter()+n);
}

void GuiKeyboard::OnPointerHeld(GuiElement *sender, int pointer, POINT p)
{
    TextPointerBtn->PositionChanged(pointer, p.x, p.y);
}

void GuiKeyboard::OnPositionMoved(GuiElement *sender, int pointer, POINT p)
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
        kbText->SetText(GetDisplayText(kbtextstr));
        TextPointerBtn->TextWidthChanged();
        TextPointerBtn->SetPointerPosition(currentPointLetter);
    }
    else if(sender == GoRight)
    {
        int currentPointLetter = TextPointerBtn->GetCurrentLetter();
        currentPointLetter++;
        int strlength = strlen(kbtextstr);
        if(currentPointLetter > (MAX_KEYBOARD_DISPLAY-1) || currentPointLetter > strlength)
        {
            currentPointLetter--;
            CurrentFirstLetter++;
            if(CurrentFirstLetter > (strlength-MAX_KEYBOARD_DISPLAY+1))
                CurrentFirstLetter = strlength-MAX_KEYBOARD_DISPLAY+1;
        }
        kbText->SetText(GetDisplayText(kbtextstr));
        TextPointerBtn->TextWidthChanged();
        TextPointerBtn->SetPointerPosition(currentPointLetter);
    }
}

void GuiKeyboard::Update(GuiTrigger * t)
{
	if(_elements.size() == 0 || (state == STATE_DISABLED && parentElement))
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->Update(t); }
		catch (const std::exception& e) { }
	}

	bool update = false;

	if(keySpace->GetState() == STATE_CLICKED)
	{
		if(strlen(kbtextstr) < kbtextmaxlen)
		{
			AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), ' ');
		}
		keySpace->SetState(STATE_SELECTED, t->chan);
	}
	else if(keyBack->GetState() == STATE_CLICKED)
	{
        RemoveChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter()-1);
		keyBack->SetState(STATE_SELECTED, t->chan);
	}
	else if(keyLineBreak->GetState() == STATE_CLICKED)
	{
        AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), '\n');
		keyLineBreak->SetState(STATE_SELECTED, t->chan);
	}
	else if(keyClear->GetState() == STATE_CLICKED)
	{
	    strcpy(kbtextstr, "");
        kbText->SetText(GetDisplayText(kbtextstr));
        TextPointerBtn->TextWidthChanged();
        TextPointerBtn->SetPointerPosition(0);
		keyClear->SetState(STATE_SELECTED, t->chan);
	}
	else if(keyShift->GetState() == STATE_CLICKED)
	{
		shift ^= 1;
		keyShift->SetState(STATE_SELECTED, t->chan);
		update = true;
	}
	else if(keyCaps->GetState() == STATE_CLICKED)
	{
		caps ^= 1;
		keyCaps->SetState(STATE_SELECTED, t->chan);
		update = true;
	}

	char txt[2] = { 0, 0 };

	startloop:

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<13; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				if(update)
				{
					if(shift || caps)
						txt[0] = keys[i][j].chShift;
					else
						txt[0] = keys[i][j].ch;

					keyTxt[i][j]->SetText(txt);
				}

				if(keyBtn[i][j]->GetState() == STATE_CLICKED)
				{
					if(strlen(kbtextstr) < kbtextmaxlen)
					{
						if(shift || caps)
						{
							AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), keys[i][j].chShift);
						}
						else
						{
							AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), keys[i][j].ch);
						}
					}
					keyBtn[i][j]->SetState(STATE_SELECTED, t->chan);

					if(shift)
					{
						shift ^= 1;
						update = true;
						goto startloop;
					}
				}
			}
		}
	}

	this->ToggleFocus(t);

	if(focus) // only send actions to this window if it's in focus
	{
		// pad/joystick navigation
		if(t->Right())
			this->MoveSelectionHor(1);
		else if(t->Left())
			this->MoveSelectionHor(-1);
		else if(t->Down())
			this->MoveSelectionVert(1);
		else if(t->Up())
			this->MoveSelectionVert(-1);
	}
}

const char * GuiKeyboard::GetDisplayText(const char * t)
{
	if(!t)
		return NULL;

	int len = strlen(t);

	if(len < MAX_KEYBOARD_DISPLAY)
		return t;

    int n = 0;
    int startPos = CurrentFirstLetter;
    int endPos = startPos+MAX_KEYBOARD_DISPLAY-1;
    for(int i = startPos; i < endPos && i < len; i++)
    {
        tmptxt[n] = t[i];
        tmptxt[n+1] = '\0';
        n++;
    }

	return tmptxt;
}
