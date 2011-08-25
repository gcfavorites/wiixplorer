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
#include "ControlsSettingsMenu.h"
#include "Prompts/PromptWindows.h"
#include "Settings.h"

static inline u16 NextWPAD_Button(u16 button)
{
	if(button == 0)
	{
		return 0x0001;
	}
	else if(button < 0x0010)
	{
		return button << 1;
	}
	else if(button < 0x0080)
	{
		return 0x0080;
	}
	else if(button < 0x1000)
	{
		return button << 1;
	}
	else
	{
		return 0;
	}
}

static inline u16 NextClassic_Button(u16 button)
{
	if(button == 0)
	{
		return 0x0001;
	}
	else if(button < 0x0080)
	{
		return button << 1;
	}
	else if(button < 0x0200)
	{
		return 0x0200;
	}
	else if(button < 0x8000)
	{
		return button << 1;
	}
	else
	{
		return 0;
	}
}

static inline u16 NextPAD_Button(u16 button)
{
	if(button == 0)
	{
		return 0x0001;
	}
	else if(button < 0x0040)
	{
		return button << 1;
	}
	else if(button < 0x0100)
	{
		return 0x0100;
	}
	else if(button < 0x1000)
	{
		return button << 1;
	}
	else
	{
		return 0;
	}
}

static inline const char * GetWPAD_ButtonName(u16 button)
{
	switch(button)
	{
		case 0x0000:
			return tr("WPAD NONE BUTTON");
		case 0x0001:
			return tr("WPAD BUTTON 2");
		case 0x0002:
			return tr("WPAD BUTTON 1");
		case 0x0004:
			return tr("WPAD BUTTON B");
		case 0x0008:
			return tr("WPAD BUTTON A");
		case 0x0010:
			return tr("WPAD BUTTON MINUS");
		case 0x0080:
			return tr("WPAD BUTTON HOME");
		case 0x0100:
			return tr("WPAD BUTTON LEFT");
		case 0x0200:
			return tr("WPAD BUTTON RIGHT");
		case 0x0400:
			return tr("WPAD BUTTON DOWN");
		case 0x0800:
			return tr("WPAD BUTTON UP");
		case 0x1000:
			return tr("WPAD BUTTON PLUS");
		default:
			return " ";
	}
}

static inline const char * GetClassic_ButtonName(u16 button)
{
	switch(button)
	{
		case 0x0000:
			return tr("WPAD CLASSIC NONE BUTTON");
		case 0x0001:
			return tr("WPAD CLASSIC BUTTON UP");
		case 0x0002:
			return tr("WPAD CLASSIC BUTTON LEFT");
		case 0x0004:
			return tr("WPAD CLASSIC BUTTON ZR");
		case 0x0008:
			return tr("WPAD CLASSIC BUTTON X");
		case 0x0010:
			return tr("WPAD CLASSIC BUTTON A");
		case 0x0020:
			return tr("WPAD CLASSIC BUTTON Y");
		case 0x0040:
			return tr("WPAD CLASSIC BUTTON B");
		case 0x0080:
			return tr("WPAD CLASSIC BUTTON ZL");
		case 0x0200:
			return tr("WPAD CLASSIC BUTTON FULL R");
		case 0x0400:
			return tr("WPAD CLASSIC BUTTON PLUS");
		case 0x0800:
			return tr("WPAD CLASSIC BUTTON HOME");
		case 0x1000:
			return tr("WPAD CLASSIC BUTTON MINUS");
		case 0x2000:
			return tr("WPAD CLASSIC BUTTON FULL L");
		case 0x4000:
			return tr("WPAD CLASSIC BUTTON DOWN");
		case 0x8000:
			return tr("WPAD CLASSIC BUTTON RIGHT");
		default:
			return " ";
	}
}

static inline const char * GetPAD_ButtonName(u16 button)
{
	switch(button)
	{
		case 0x0000:
			return tr("GC PAD BUTTON NONE");
		case 0x0001:
			return tr("GC PAD BUTTON LEFT");
		case 0x0002:
			return tr("GC PAD BUTTON RIGHT");
		case 0x0004:
			return tr("GC PAD BUTTON DOWN");
		case 0x0008:
			return tr("GC PAD BUTTON UP");
		case 0x0010:
			return tr("GC PAD TRIGGER Z");
		case 0x0020:
			return tr("GC PAD TRIGGER R");
		case 0x0040:
			return tr("GC PAD TRIGGER L");
		case 0x0100:
			return tr("GC PAD BUTTON A");
		case 0x0200:
			return tr("GC PAD BUTTON B");
		case 0x0400:
			return tr("GC PAD BUTTON X");
		case 0x0800:
			return tr("GC PAD BUTTON Y");
		case 0x1000:
			return tr("GC PAD BUTTON START");
		default:
			return " ";
	}
}

ControlsSettingsMenu::ControlsSettingsMenu(GuiFrame *r)
	: SettingsMenu(tr("Controls Settings"), r)
{
	resetBtnTxt = new GuiText(tr("Reset"), 22, (GXColor){0, 0, 0, 255});
	resetBtnImg = new GuiImage(btnOutline);
	resetBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	resetBtn->SetAlignment(ALIGN_CENTER | ALIGN_BOTTOM);
	resetBtn->SetPosition(100+btnOutline->GetWidth()/2, -65);
	resetBtn->SetLabel(resetBtnTxt);
	resetBtn->SetImage(resetBtnImg);
	resetBtn->SetSoundOver(btnSoundOver);
	resetBtn->SetTrigger(trigA);
	resetBtn->SetEffectGrow();
	resetBtn->Clicked.connect(this, &ControlsSettingsMenu::OnResetButtonClick);
	Append(resetBtn);

	SetupOptions();
}

ControlsSettingsMenu::~ControlsSettingsMenu()
{
	Remove(resetBtn);
	delete resetBtnTxt;
	delete resetBtnImg;
	delete resetBtn;
}

void ControlsSettingsMenu::OnResetButtonClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED)
{
	int choice = WindowPrompt(tr("Do you want to reset the control settings?"), 0, tr("Yes"), tr("Cancel"));
	if(choice)
	{
		Settings.Controls.SetDefault();
		SetOptionValues();
	}
}

void ControlsSettingsMenu::SetupOptions()
{
	int i = 0;

	options.SetName(i++, tr("Screenshot Hold Button"));
	options.SetName(i++, tr("Screenshot Press Button"));
	options.SetName(i++, tr("Click Button"));
	options.SetName(i++, tr("Click Button"));
	options.SetName(i++, tr("Click Button"));
	options.SetName(i++, tr("Back Button"));
	options.SetName(i++, tr("Back Button"));
	options.SetName(i++, tr("Back Button"));
	options.SetName(i++, tr("Up Button"));
	options.SetName(i++, tr("Up Button"));
	options.SetName(i++, tr("Up Button"));
	options.SetName(i++, tr("Down Button"));
	options.SetName(i++, tr("Down Button"));
	options.SetName(i++, tr("Down Button"));
	options.SetName(i++, tr("Left Button"));
	options.SetName(i++, tr("Left Button"));
	options.SetName(i++, tr("Left Button"));
	options.SetName(i++, tr("Right Button"));
	options.SetName(i++, tr("Right Button"));
	options.SetName(i++, tr("Right Button"));
	options.SetName(i++, tr("Context Menu Button"));
	options.SetName(i++, tr("Context Menu Button"));
	options.SetName(i++, tr("Context Menu Button"));
	options.SetName(i++, tr("Mark Item Button"));
	options.SetName(i++, tr("Mark Item Button"));
	options.SetName(i++, tr("Mark Item Button"));
	options.SetName(i++, tr("Demark Item Button"));
	options.SetName(i++, tr("Demark Item Button"));
	options.SetName(i++, tr("Demark Item Button"));
	options.SetName(i++, tr("Demark All Items Button"));
	options.SetName(i++, tr("Demark All Items Button"));
	options.SetName(i++, tr("Demark All Items Button"));
	options.SetName(i++, tr("Home Button"));
	options.SetName(i++, tr("Home Button"));
	options.SetName(i++, tr("Home Button"));
	options.SetName(i++, tr("Edit Textline Button"));
	options.SetName(i++, tr("Edit Textline Button"));
	options.SetName(i++, tr("Edit Textline Button"));
	options.SetName(i++, tr("Slide Show Button"));
	options.SetName(i++, tr("Slide Show Button"));
	options.SetName(i++, tr("Slide Show Button"));
	options.SetName(i++, tr("Keyboard Backspace Button"));
	options.SetName(i++, tr("Keyboard Backspace Button"));
	options.SetName(i++, tr("Keyboard Backspace Button"));
	options.SetName(i++, tr("Keyboard Shift Button"));
	options.SetName(i++, tr("Keyboard Shift Button"));
	options.SetName(i++, tr("Keyboard Shift Button"));
	options.SetName(i++, tr("ZoomIn Button"));
	options.SetName(i++, tr("ZoomIn Button"));
	options.SetName(i++, tr("ZoomIn Button"));
	options.SetName(i++, tr("ZoomOut Button"));
	options.SetName(i++, tr("ZoomOut Button"));
	options.SetName(i++, tr("ZoomOut Button"));
	options.SetName(i++, tr("Back in Directory Button"));
	options.SetName(i++, tr("Back in Directory Button"));
	options.SetName(i++, tr("Back in Directory Button"));
	options.SetName(i++, tr("Button Hold Scroll"));
	options.SetName(i++, tr("Button Hold Scroll"));
	options.SetName(i++, tr("Button Hold Scroll"));

	SetOptionValues();
}

void ControlsSettingsMenu::SetOptionValues()
{
	int i = 0;

	options.SetValue(i++, GetWPAD_ButtonName(Settings.Controls.ScreenshotHoldButton));
	options.SetValue(i++, GetWPAD_ButtonName(Settings.Controls.ScreenshotClickButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.ClickButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.ClickButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.ClickButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.BackButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.BackButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.BackButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.UpButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.UpButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.UpButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.DownButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.DownButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.DownButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.LeftButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.LeftButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.LeftButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.RightButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.RightButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.RightButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.ContextMenuButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.ContextMenuButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.ContextMenuButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.MarkItemButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.MarkItemButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.MarkItemButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.DeMarkItemButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.DeMarkItemButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.DeMarkItemButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.DeMarkAllButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.DeMarkAllButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.DeMarkAllButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.HomeButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.HomeButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.HomeButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.EditTextLine));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.EditTextLine));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.EditTextLine));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.SlideShowButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.SlideShowButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.SlideShowButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.KeyBackspaceButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.KeyBackspaceButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.KeyBackspaceButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.KeyShiftButton));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.KeyShiftButton));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.KeyShiftButton));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.ZoomIn));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.ZoomIn));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.ZoomIn));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.ZoomOut));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.ZoomOut));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.ZoomOut));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.UpInDirectory));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.UpInDirectory));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.UpInDirectory));

	options.SetValue(i++, GetWPAD_ButtonName(WiiControls.OneButtonScroll));
	options.SetValue(i++, GetClassic_ButtonName(ClassicControls.OneButtonScroll));
	options.SetValue(i++, GetPAD_ButtonName(GCControls.OneButtonScroll));
}

void ControlsSettingsMenu::OnOptionClick(GuiOptionBrowser *sender UNUSED, int option)
{
	switch (option)
	{
		case 0:
			Settings.Controls.ScreenshotHoldButton = NextWPAD_Button(Settings.Controls.ScreenshotHoldButton);
			break;
		case 1:
			Settings.Controls.ScreenshotClickButton = NextWPAD_Button(Settings.Controls.ScreenshotClickButton);
			break;
		case 2:
			WiiControls.ClickButton = NextWPAD_Button(WiiControls.ClickButton);
			break;
		case 3:
			ClassicControls.ClickButton = NextClassic_Button(ClassicControls.ClickButton);
			break;
		case 4:
			GCControls.ClickButton = NextPAD_Button(GCControls.ClickButton);
			break;
		case 5:
			WiiControls.BackButton = NextWPAD_Button(WiiControls.BackButton);
			break;
		case 6:
			ClassicControls.BackButton = NextClassic_Button(ClassicControls.BackButton);
			break;
		case 7:
			GCControls.BackButton = NextPAD_Button(GCControls.BackButton);
			break;
		case 8:
			WiiControls.UpButton = NextWPAD_Button(WiiControls.UpButton);
			break;
		case 9:
			ClassicControls.UpButton = NextClassic_Button(ClassicControls.UpButton);
			break;
		case 10:
			GCControls.UpButton = NextPAD_Button(GCControls.UpButton);
			break;
		case 11:
			WiiControls.DownButton = NextWPAD_Button(WiiControls.DownButton);
			break;
		case 12:
			ClassicControls.DownButton = NextClassic_Button(ClassicControls.DownButton);
			break;
		case 13:
			GCControls.DownButton = NextPAD_Button(GCControls.DownButton);
			break;
		case 14:
			WiiControls.LeftButton = NextWPAD_Button(WiiControls.LeftButton);
			break;
		case 15:
			ClassicControls.LeftButton = NextClassic_Button(ClassicControls.LeftButton);
			break;
		case 16:
			GCControls.LeftButton = NextPAD_Button(GCControls.LeftButton);
			break;
		case 17:
			WiiControls.RightButton = NextWPAD_Button(WiiControls.RightButton);
			break;
		case 18:
			ClassicControls.RightButton = NextClassic_Button(ClassicControls.RightButton);
			break;
		case 19:
			GCControls.RightButton = NextPAD_Button(GCControls.RightButton);
			break;
		case 20:
			WiiControls.ContextMenuButton = NextWPAD_Button(WiiControls.ContextMenuButton);
			break;
		case 21:
			ClassicControls.ContextMenuButton = NextClassic_Button(ClassicControls.ContextMenuButton);
			break;
		case 22:
			GCControls.ContextMenuButton = NextPAD_Button(GCControls.ContextMenuButton);
			break;
		case 23:
			WiiControls.MarkItemButton = NextWPAD_Button(WiiControls.MarkItemButton);
			break;
		case 24:
			ClassicControls.MarkItemButton = NextClassic_Button(ClassicControls.MarkItemButton);
			break;
		case 25:
			GCControls.MarkItemButton = NextPAD_Button(GCControls.MarkItemButton);
			break;
		case 26:
			WiiControls.DeMarkItemButton = NextWPAD_Button(WiiControls.DeMarkItemButton);
			break;
		case 27:
			ClassicControls.DeMarkItemButton = NextClassic_Button(ClassicControls.DeMarkItemButton);
			break;
		case 28:
			GCControls.DeMarkItemButton = NextPAD_Button(GCControls.DeMarkItemButton);
			break;
		case 29:
			WiiControls.DeMarkAllButton = NextWPAD_Button(WiiControls.DeMarkAllButton);
			break;
		case 30:
			ClassicControls.DeMarkAllButton = NextClassic_Button(ClassicControls.DeMarkAllButton);
			break;
		case 31:
			GCControls.DeMarkAllButton = NextPAD_Button(GCControls.DeMarkAllButton);
			break;
		case 32:
			WiiControls.HomeButton = NextWPAD_Button(WiiControls.HomeButton);
			break;
		case 33:
			ClassicControls.HomeButton = NextClassic_Button(ClassicControls.HomeButton);
			break;
		case 34:
			GCControls.HomeButton = NextPAD_Button(GCControls.HomeButton);
			break;
		case 35:
			WiiControls.EditTextLine = NextWPAD_Button(WiiControls.EditTextLine);
			break;
		case 36:
			ClassicControls.EditTextLine = NextClassic_Button(ClassicControls.EditTextLine);
			break;
		case 37:
			GCControls.EditTextLine = NextPAD_Button(GCControls.EditTextLine);
			break;
		case 38:
			WiiControls.SlideShowButton = NextWPAD_Button(WiiControls.SlideShowButton);
			break;
		case 39:
			ClassicControls.SlideShowButton = NextClassic_Button(ClassicControls.SlideShowButton);
			break;
		case 40:
			GCControls.SlideShowButton = NextPAD_Button(GCControls.SlideShowButton);
			break;
		case 41:
			WiiControls.KeyBackspaceButton = NextWPAD_Button(WiiControls.KeyBackspaceButton);
			break;
		case 42:
			ClassicControls.KeyBackspaceButton = NextClassic_Button(ClassicControls.KeyBackspaceButton);
			break;
		case 43:
			GCControls.KeyBackspaceButton = NextPAD_Button(GCControls.KeyBackspaceButton);
			break;
		case 44:
			WiiControls.KeyShiftButton = NextWPAD_Button(WiiControls.KeyShiftButton);
			break;
		case 45:
			ClassicControls.KeyShiftButton = NextClassic_Button(ClassicControls.KeyShiftButton);
			break;
		case 46:
			GCControls.KeyShiftButton = NextPAD_Button(GCControls.KeyShiftButton);
			break;
		case 47:
			WiiControls.ZoomIn = NextWPAD_Button(WiiControls.ZoomIn);
			break;
		case 48:
			ClassicControls.ZoomIn = NextClassic_Button(ClassicControls.ZoomIn);
			break;
		case 49:
			GCControls.ZoomIn = NextPAD_Button(GCControls.ZoomIn);
			break;
		case 50:
			WiiControls.ZoomOut = NextWPAD_Button(WiiControls.ZoomOut);
			break;
		case 51:
			ClassicControls.ZoomOut = NextClassic_Button(ClassicControls.ZoomOut);
			break;
		case 52:
			GCControls.ZoomOut = NextPAD_Button(GCControls.ZoomOut);
			break;
		case 53:
			WiiControls.UpInDirectory = NextWPAD_Button(WiiControls.UpInDirectory);
			break;
		case 54:
			ClassicControls.UpInDirectory = NextClassic_Button(ClassicControls.UpInDirectory);
			break;
		case 55:
			GCControls.UpInDirectory = NextPAD_Button(GCControls.UpInDirectory);
			break;
		case 56:
			WiiControls.OneButtonScroll = NextWPAD_Button(WiiControls.OneButtonScroll);
			break;
		case 57:
			ClassicControls.OneButtonScroll = NextClassic_Button(ClassicControls.OneButtonScroll);
			break;
		case 58:
			GCControls.OneButtonScroll = NextPAD_Button(GCControls.OneButtonScroll);
			break;
		default:
			break;
	}

	SetOptionValues();
}
