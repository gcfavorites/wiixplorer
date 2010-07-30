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
#include "SettingsMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindows.h"
#include "ImageOperations/ImageWrite.h"

int MenuImageSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
    char entered[150];

	OptionList options;
	options.SetName(i++, tr("Slideshow Delay"));
	options.SetName(i++, tr("Screenshot Format"));
	options.SetName(i++, tr("Fade Speed"));

	SettingsMenu * Menu = new SettingsMenu(tr("Image Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

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

		ret = Menu->GetClickedOption();

		switch (ret)
		{
            case 0:
                snprintf(entered, sizeof(entered), "%i", Settings.SlideshowDelay);
                if(OnScreenKeyboard(entered, sizeof(entered)))
                {
                    Settings.SlideshowDelay = cut_bounds(atoi(entered), 0, 999999);
                }
				break;
            case 1:
                Settings.ScreenshotFormat = (Settings.ScreenshotFormat+1) % 6;
				break;
            case 2:
                snprintf(entered, sizeof(entered), "%i", Settings.ImageFadeSpeed);
                if(OnScreenKeyboard(entered, sizeof(entered)))
                {
                    Settings.ImageFadeSpeed = cut_bounds(atoi(entered), 1, 255);
                }
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

			options.SetValue(i++, "%i", Settings.SlideshowDelay);

			if(Settings.ScreenshotFormat == IMAGE_PNG)
                options.SetValue(i++, "PNG");
            else if(Settings.ScreenshotFormat == IMAGE_JPEG)
                options.SetValue(i++, "JPEG");
            else if(Settings.ScreenshotFormat == IMAGE_GIF)
                options.SetValue(i++, "GIF");
            else if(Settings.ScreenshotFormat == IMAGE_TIFF)
                options.SetValue(i++, "TIFF");
            else if(Settings.ScreenshotFormat == IMAGE_BMP)
                options.SetValue(i++, "BMP");
            else if(Settings.ScreenshotFormat == IMAGE_GD)
                options.SetValue(i++, "GD");
            else if(Settings.ScreenshotFormat == IMAGE_GD2)
                options.SetValue(i++, "GD2");

			options.SetValue(i++, "%i", Settings.ImageFadeSpeed);
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}
