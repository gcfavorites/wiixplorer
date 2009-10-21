/****************************************************************************
 * Home Menu
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice of up to 4 Buttons.
 *
 * Give him 1 Titel, 1 Subtitel and 4 Buttons
 * If titel/subtitle or one of the buttons is not needed give him a 0 on that
 * place.
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "network/networkops.h"
#include "prompts/PromptWindows.h"
#include "devicemounter.h"
#include "menu.h"
#include "main.h"
#include "title.h"
#include "audio.h"
#include "filelist.h"
#include "sys.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/***************************************************************************
 * WindowExitPrompt
 ***************************************************************************/

int WindowExitPrompt(const char *title, const char *msg, const char *btn1Label,
					 const char *btn2Label, const char *btn3Label,
					 const char *btn4Label) {
	
    GuiSound * homein = NULL;
    homein = new GuiSound(menuin_ogg, menuin_ogg_size, SOUND_OGG, Settings.MusicVolume);
    homein->SetVolume(Settings.MusicVolume);
    homein->SetLoop(0);
    homein->Play();

    GuiSound * homeout = NULL;
    homeout = new GuiSound(menuout_ogg, menuout_ogg_size, SOUND_OGG, Settings.MusicVolume);
    homeout->SetVolume(Settings.MusicVolume);
    homeout->SetLoop(0);

    int choice = 0;
    u8 HBC=0;
    GuiWindow promptWindow(640,480);
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(0, 0);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.MusicVolume);
    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM, Settings.MusicVolume);

    GuiImageData top(exit_top_png);
    GuiImageData topOver(exit_top_over_png);
    GuiImageData bottom(exit_bottom_png);
    GuiImageData bottomOver(exit_bottom_over_png);
    GuiImageData button(exit_button_png);
    GuiImageData wiimote(wiimote_png);
    GuiImageData close(closebutton_png);

    GuiImageData battery(battery_white_png);
    GuiImageData batteryBar(battery_bar_white_png);
    GuiImageData batteryRed(battery_red_png);
    GuiImageData batteryBarRed(battery_bar_red_png);


#ifdef HW_RVL
    int i = 0, ret = 0, level;
    char txt[3];
    GuiText * batteryTxt[4];
    GuiImage * batteryImg[4];
    GuiImage * batteryBarImg[4];
    GuiButton * batteryBtn[4];

    for (i=0; i < 4; i++) {

        if (i == 0)
            sprintf(txt, "P%d", i+1);
        else
            sprintf(txt, "P%d", i+1);

        batteryTxt[i] = new GuiText(txt, 22, (GXColor) {255,255,255, 255});
        batteryTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        batteryImg[i] = new GuiImage(&battery);
        batteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        batteryImg[i]->SetPosition(36, 0);
        batteryImg[i]->SetTile(0);
        batteryBarImg[i] = new GuiImage(&batteryBar);
        batteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        batteryBarImg[i]->SetPosition(33, 0);

        batteryBtn[i] = new GuiButton(40, 20);
        batteryBtn[i]->SetLabel(batteryTxt[i]);
        batteryBtn[i]->SetImage(batteryBarImg[i]);
        batteryBtn[i]->SetIcon(batteryImg[i]);
        batteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        batteryBtn[i]->SetRumble(false);
        batteryBtn[i]->SetAlpha(70);
        batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
    }

    batteryBtn[0]->SetPosition(180,150);
    batteryBtn[1]->SetPosition(284, 150);
    batteryBtn[2]->SetPosition(388, 150);
    batteryBtn[3]->SetPosition(494, 150);


    char * sig = (char *)0x80001804;
    if (
        sig[0] == 'S' &&
        sig[1] == 'T' &&
        sig[2] == 'U' &&
        sig[3] == 'B' &&
        sig[4] == 'H' &&
        sig[5] == 'A' &&
        sig[6] == 'X' &&
        sig[7] == 'X')
        HBC=1; // Exit to HBC
#endif

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

    GuiText titleTxt(tr("HOME Menu"), 36, (GXColor) {255, 255, 255, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(-180,40);
    titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    GuiText closeTxt(tr("Close"), 28, (GXColor) {0, 0, 0, 255});
    closeTxt.SetPosition(10,3);
    GuiImage closeImg(&close);
    GuiButton closeBtn(close.GetWidth(), close.GetHeight());
    closeBtn.SetImage(&closeImg);
    closeBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    closeBtn.SetPosition(190,30);
    closeBtn.SetLabel(&closeTxt);
    closeBtn.SetRumble(false);
    closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    GuiImage btn1Img(&top);
    GuiImage btn1OverImg(&topOver);
    GuiButton btn1(&btn1Img,&btn1OverImg, 0, 3, 0, 0, &trigA, &btnSoundOver, &btnClick,0);
	btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    GuiText btn2Txt((HBC!=1?tr("Homebrew Channel"):btn1Label), 28, (GXColor) {0, 0, 0, 255});
    GuiImage btn2Img(&button);
    GuiButton btn2(&btn2Img,&btn2Img, 2, 5, -150, 0, &trigA, &btnSoundOver, &btnClick,1);
    btn2.SetLabel(&btn2Txt);
    btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
    btn2.SetRumble(false);
    if (HBC==1) {
        btn2.SetPosition(-150, 0);
    }

    GuiText btn3Txt(btn2Label, 28, (GXColor) {0, 0, 0, 255});
    GuiImage btn3Img(&button);
    GuiButton btn3(&btn3Img,&btn3Img, 2, 5, 150, 0, &trigA, &btnSoundOver, &btnClick,1);
    btn3.SetLabel(&btn3Txt);
    btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
    btn3.SetRumble(false);
    btn3.SetPosition(150, 0);

    GuiImage btn4Img(&bottom);
    GuiImage btn4OverImg(&bottomOver);
    GuiButton btn4(&btn4Img,&btn4OverImg, 0, 4, 0, 0, 0, &btnSoundOver, &btnClick,0);
    btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

    GuiImage wiimoteImg(&wiimote);
    wiimoteImg.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
    wiimoteImg.SetPosition(50,210);
	
	GuiButton home(1,1);
    home.SetTrigger(&trigHome);
	GuiButton B(1,1);
    B.SetTrigger(&trigB);

    promptWindow.Append(&btn2);
    promptWindow.Append(&btn3);
    promptWindow.Append(&btn4);
    promptWindow.Append(&btn1);
    promptWindow.Append(&closeBtn);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&wiimoteImg);
	promptWindow.Append(&home);
	promptWindow.Append(&B);

#ifdef HW_RVL
    promptWindow.Append(batteryBtn[0]);
    promptWindow.Append(batteryBtn[1]);
    promptWindow.Append(batteryBtn[2]);
    promptWindow.Append(batteryBtn[3]);
#endif
	
	HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while (choice == 0) {
	
        VIDEO_WaitVSync();

#ifdef HW_RVL
        for (i=0; i < 4; i++) {
            if (WPAD_Probe(i, NULL) == WPAD_ERR_NONE) { // controller connected
                level = (userInput[i].wpad.battery_level / 100.0) * 4;
                if (level > 4) level = 4;

                if (level <= 1) {
                    batteryBarImg[i]->SetImage(&batteryBarRed);
                    batteryImg[i]->SetImage(&batteryRed);
				} else {
                    batteryBarImg[i]->SetImage(&batteryBar);
				}

				batteryImg[i]->SetTile(level);

                batteryBtn[i]->SetAlpha(255);
            } else { // controller not connected
                batteryImg[i]->SetTile(0);
                batteryImg[i]->SetImage(&battery);
                batteryBtn[i]->SetAlpha(70);
            }
        }
#endif

        if (shutdown == 1) {
            Sys_Shutdown();
        }
        if (reset == 1)
            Sys_Reboot();
        
		if (btn1.GetState() == STATE_CLICKED ||
			home.GetState() == STATE_CLICKED ||
			B.GetState() == STATE_CLICKED) {
            choice = 1;
			btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
            btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
            btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
            titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
#ifdef HW_RVL
            for (int i = 0; i < 4; i++)
                batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
#endif
        } else if (btn4.GetState() == STATE_SELECTED) {
            wiimoteImg.SetPosition(50,165);
        } else if (btn2.GetState() == STATE_CLICKED) {
            ret = WindowPrompt(tr("Are you sure?"), 0, tr("Yes"), tr("No"));
            if (ret == 1) {
                if (HBC!=1) {
					ExitGUIThreads();
					ShutdownAudio();
                    StopGX();
                    WII_Initialize();
					WII_BootHBC();
                    
                }
                choice = 2;
            }
            HaltGui();
            mainWindow->SetState(STATE_DISABLED);
            promptWindow.SetState(STATE_DEFAULT);
            mainWindow->ChangeFocus(&promptWindow);
            ResumeGui();
            btn2.ResetState();
        } else if (btn3.GetState() == STATE_CLICKED) {
            ret = WindowPrompt(tr("Are you sure?"), 0, tr("Yes"), tr("No"));
            if (ret == 1) {
                choice = 3;
            }
            HaltGui();
            mainWindow->SetState(STATE_DISABLED);
            promptWindow.SetState(STATE_DEFAULT);
            mainWindow->ChangeFocus(&promptWindow);
            ResumeGui();
            btn3.ResetState();
        /*} else if (btn4.GetState() == STATE_CLICKED) {
            btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
            btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
            btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
            titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
#ifdef HW_RVL
            for (int i = 0; i < 4; i++)
                batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
#endif
            choice = 0;*/
        } else if (btn4.GetState() != STATE_SELECTED) {
            wiimoteImg.SetPosition(50,210);
        }
    }
    homeout->Play();
    while (btn1.GetEffect() > 0) usleep(50);
    while (promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    homein->Stop();
    delete homein;
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    while (homeout->IsPlaying() > 0) usleep(50);
    homeout->Stop();
    delete homeout;
    ResumeGui();
    return choice;
}
