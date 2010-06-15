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
 #include <unistd.h>
#include "Prompts/PromptWindows.h"
#include "ImageOperations/ImageWrite.h"
#include "Prompts/DeviceMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "SettingsMenu.h"
#include "menu_settings.h"
#include "main.h"

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

const char * GetWPAD_ButtonName(u16 button)
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

const char * GetClassic_ButtonName(u16 button)
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

const char * GetPAD_ButtonName(u16 button)
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

/****************************************************************************
 * MenuControlsSettings
 ***************************************************************************/
int MenuControlsSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;

	OptionList options;
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

	SettingsMenu * Menu = new SettingsMenu(tr("Controls Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

    GuiImageData * btnOutline = Resources::GetImageData(button_png, button_png_size);
	GuiSound * btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    SimpleGuiTrigger trigA(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	GuiText resetBtnTxt(tr("Reset"), 22, (GXColor){0, 0, 0, 255});
	GuiImage resetBtnImg(btnOutline);
	GuiButton resetBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
	resetBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	resetBtn.SetPosition(100+btnOutline->GetWidth()/2, -65);
	resetBtn.SetLabel(&resetBtnTxt);
	resetBtn.SetImage(&resetBtnImg);
	resetBtn.SetSoundOver(btnSoundOver);
	resetBtn.SetTrigger(&trigA);
	resetBtn.SetEffectGrow();

	MainWindow::Instance()->Append(&resetBtn);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

		if(Menu->GetMenu() != MENU_NONE)
		{
			menu = Menu->GetMenu();
		}
        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
        {
			menu = Taskbar::Instance()->GetMenu();
        }
        else if(resetBtn.GetState() == STATE_CLICKED)
        {
			resetBtn.ResetState();
			int choice = WindowPrompt(tr("Do you want to reset the control settings?"), 0, tr("Yes"), tr("Cancel"));
			if(choice)
			{
                Settings.Controls.SetDefault();
                firstRun = true;
			}
        }

		ret = Menu->GetClickedOption();

		switch (ret)
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
            default:
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

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
        }
	}
	MainWindow::Instance()->HaltGui();
	MainWindow::Instance()->Remove(&resetBtn);
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnOutline);
    delete Menu;
	MainWindow::Instance()->ResumeGui();

    Settings.Save();

	return menu;
}
