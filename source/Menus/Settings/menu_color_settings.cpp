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
#include "SettingsMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ColorSetPrompt.h"

int MenuColorSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
    ColorSetPrompt * ColorPrompt = NULL;
    GXColor ImgColor[4];

	OptionList options;
	options.SetName(i++, tr("Background Upper/Left"));
	options.SetName(i++, tr("Background Upper/Right"));
	options.SetName(i++, tr("Background Bottom/Right"));
	options.SetName(i++, tr("Background Bottom/Left"));
	options.SetName(i++, tr("ProgressBar Upper/Left"));
	options.SetName(i++, tr("ProgressBar Upper/Right"));
	options.SetName(i++, tr("ProgressBar Bottom/Right"));
	options.SetName(i++, tr("ProgressBar Bottom/Left"));
	options.SetName(i++, tr("ProgressBar Empty Upper/Left"));
	options.SetName(i++, tr("ProgressBar Empty Upper/Right"));
	options.SetName(i++, tr("ProgressBar Empty Bottom/Right"));
	options.SetName(i++, tr("ProgressBar Empty Bottom/Left"));

	SettingsMenu * Menu = new SettingsMenu(tr("Color Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

    GuiImageData * btnOutline = Resources::GetImageData("button.png");
	GuiSound * btnSoundOver = Resources::GetSound("button_over.wav");

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
			int choice = WindowPrompt(tr("Do you want to reset the color settings?"), 0, tr("Yes"), tr("Cancel"));
			if(choice)
			{
                Settings.DefaultColors();
                MainWindow::Instance()->GetBGColorPtr()[0] = RGBATOGXCOLOR(Settings.BackgroundUL);
                MainWindow::Instance()->GetBGColorPtr()[1] = RGBATOGXCOLOR(Settings.BackgroundUR);
                MainWindow::Instance()->GetBGColorPtr()[2] = RGBATOGXCOLOR(Settings.BackgroundBR);
                MainWindow::Instance()->GetBGColorPtr()[3] = RGBATOGXCOLOR(Settings.BackgroundBL);
                firstRun = true;
			}
        }

		ret = Menu->GetClickedOption();

		switch (ret)
		{
			case 0:
                ColorPrompt = new ColorSetPrompt(tr("Background Upper/Left"), MainWindow::Instance()->GetBGColorPtr(), 0);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.BackgroundUL = GXCOLORTORGBA(MainWindow::Instance()->GetBGColorPtr()[0]);
				break;
            case 1:
                ColorPrompt = new ColorSetPrompt(tr("Background Upper/Right"), MainWindow::Instance()->GetBGColorPtr(), 1);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.BackgroundUR = GXCOLORTORGBA(MainWindow::Instance()->GetBGColorPtr()[1]);
				break;
			case 2:
                ColorPrompt = new ColorSetPrompt(tr("Background Bottom/Right"), MainWindow::Instance()->GetBGColorPtr(), 2);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.BackgroundBR = GXCOLORTORGBA(MainWindow::Instance()->GetBGColorPtr()[2]);
				break;
			case 3:
                ColorPrompt = new ColorSetPrompt(tr("Background Bottom/Left"), MainWindow::Instance()->GetBGColorPtr(), 3);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.BackgroundBL = GXCOLORTORGBA(MainWindow::Instance()->GetBGColorPtr()[3]);
				break;
            case 4:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Upper/Left"), (GXColor *) &ImgColor, 0);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressUL = GXCOLORTORGBA(ImgColor[0]);
				break;
            case 5:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Upper/Right"), (GXColor *) &ImgColor, 1);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressUR = GXCOLORTORGBA(ImgColor[1]);
				break;
            case 6:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Bottom/Right"), (GXColor *) &ImgColor, 2);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressBR = GXCOLORTORGBA(ImgColor[2]);
				break;
            case 7:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Bottom/Left"), (GXColor *) &ImgColor, 3);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressBL = GXCOLORTORGBA(ImgColor[3]);
				break;
            case 8:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressEmptyUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressEmptyUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressEmptyBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressEmptyBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Upper/Left"), (GXColor *) &ImgColor, 0);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressEmptyUL = GXCOLORTORGBA(ImgColor[0]);
				break;
            case 9:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressEmptyUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressEmptyUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressEmptyBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressEmptyBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Upper/Right"), (GXColor *) &ImgColor, 1);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressEmptyUR = GXCOLORTORGBA(ImgColor[1]);
				break;
            case 10:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressEmptyUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressEmptyUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressEmptyBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressEmptyBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Bottom/Right"), (GXColor *) &ImgColor, 2);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressEmptyBR = GXCOLORTORGBA(ImgColor[2]);
				break;
            case 11:
                ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressEmptyUL);
                ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressEmptyUR);
                ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressEmptyBR);
                ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressEmptyBL);
                ColorPrompt = new ColorSetPrompt(tr("ProgressBar Bottom/Left"), (GXColor *) &ImgColor, 3);
                MainWindow::Instance()->SetState(STATE_DISABLED);
                MainWindow::Instance()->Append(ColorPrompt);
                ColorPrompt->ShowPrompt();
                delete ColorPrompt;
                ColorPrompt = NULL;
                MainWindow::Instance()->SetState(STATE_DEFAULT);
                Settings.ProgressEmptyBL = GXCOLORTORGBA(ImgColor[3]);
				break;
            default:
                break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            options.SetValue(i++, tr("R: %i G: %i B: %i"), MainWindow::Instance()->GetBGColorPtr()[0].r,
                                           MainWindow::Instance()->GetBGColorPtr()[0].g,
                                           MainWindow::Instance()->GetBGColorPtr()[0].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), MainWindow::Instance()->GetBGColorPtr()[1].r,
                                           MainWindow::Instance()->GetBGColorPtr()[1].g,
                                           MainWindow::Instance()->GetBGColorPtr()[1].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), MainWindow::Instance()->GetBGColorPtr()[2].r,
                                           MainWindow::Instance()->GetBGColorPtr()[2].g,
                                           MainWindow::Instance()->GetBGColorPtr()[2].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), MainWindow::Instance()->GetBGColorPtr()[3].r,
                                           MainWindow::Instance()->GetBGColorPtr()[3].g,
                                           MainWindow::Instance()->GetBGColorPtr()[3].b);

            ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressUL);
            ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressUR);
            ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressBR);
            ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressBL);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[0].r, ImgColor[0].g, ImgColor[0].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[1].r, ImgColor[1].g, ImgColor[1].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[2].r, ImgColor[2].g, ImgColor[2].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[3].r, ImgColor[3].g, ImgColor[3].b);

            ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressEmptyUL);
            ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressEmptyUR);
            ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressEmptyBR);
            ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressEmptyBL);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[0].r, ImgColor[0].g, ImgColor[0].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[1].r, ImgColor[1].g, ImgColor[1].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[2].r, ImgColor[2].g, ImgColor[2].b);

            options.SetValue(i++, tr("R: %i G: %i B: %i"), ImgColor[3].r, ImgColor[3].g, ImgColor[3].b);
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
