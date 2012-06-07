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
#include "KeyboardWindow.h"
#include "Memory/Resources.h"
#include "Settings.h"

KeyboardWindow::KeyboardWindow(wchar_t *text, int max)
	: GuiFrame(0, 0)
{
	choice = -1;
	inText = text;
	maxlen = max;

	keyboard = new GuiKeyboard(text, maxlen > 0 ? maxlen : wcslen(text)+1);

	width = keyboard->GetWidth();
	height = keyboard->GetHeight();

	btnSoundOver = Resources::GetSound("button_over.wav");
	btnOutline = Resources::GetImageData("button.png");
	btnOutlineOver = Resources::GetImageData("button_over.png");

	trigA.SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	okBtnTxt = new GuiText(tr("OK"), 22, (GXColor){0, 0, 0, 255});
	okBtnImg = new GuiImage(btnOutline);
	okBtnImgOver = new GuiImage(btnOutlineOver);
	okBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	okBtn->SetAlignment(ALIGN_LEFT | ALIGN_BOTTOM);
	okBtn->SetPosition(25, -25);
	okBtn->SetLabel(okBtnTxt);
	okBtn->SetImage(okBtnImg);
	okBtn->SetImageOver(okBtnImgOver);
	okBtn->SetSoundOver(btnSoundOver);
	okBtn->SetTrigger(&trigA);
	okBtn->SetEffectGrow();
	okBtn->Clicked.connect(this, &KeyboardWindow::OnOkButtonClick);

	cancelBtnTxt = new GuiText(tr("Cancel"), 22, (GXColor){0, 0, 0, 255});
	cancelBtnImg = new GuiImage(btnOutline);
	cancelBtnImgOver = new GuiImage(btnOutlineOver);
	cancelBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	cancelBtn->SetAlignment(ALIGN_RIGHT | ALIGN_BOTTOM);
	cancelBtn->SetPosition(-25, -25);
	cancelBtn->SetLabel(cancelBtnTxt);
	cancelBtn->SetImage(cancelBtnImg);
	cancelBtn->SetImageOver(cancelBtnImgOver);
	cancelBtn->SetSoundOver(btnSoundOver);
	cancelBtn->SetTrigger(&trigA);
	cancelBtn->SetEffectGrow();
	cancelBtn->Clicked.connect(this, &KeyboardWindow::OnCancelButtonClick);

	Append(keyboard);
	Append(okBtn);
	Append(cancelBtn);

	SetAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
}

KeyboardWindow::~KeyboardWindow()
{
	RemoveAll();

	Resources::Remove(btnOutline);
	Resources::Remove(btnOutlineOver);

	Resources::Remove(btnSoundOver);

	delete okBtn;
	delete cancelBtn;

	delete okBtnImg;
	delete okBtnImgOver;
	delete cancelBtnImg;
	delete cancelBtnImgOver;

	delete okBtnTxt;
	delete cancelBtnTxt;

	delete keyboard;
}

void KeyboardWindow::OnOkButtonClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED)
{
	choice = 1;
	wcsncpy(inText, keyboard->GetString(), maxlen);
	inText[maxlen-1] = 0;

	OkButtonClicked(this, inText);
}

void KeyboardWindow::OnCancelButtonClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED)
{
	choice = 0;
}
