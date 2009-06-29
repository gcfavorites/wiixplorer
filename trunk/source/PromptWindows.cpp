#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#include "libwiigui/gui.h"
#include "network/networkops.h"
#include "PromptWindows.h"
#include "fatmounter.h"
#include "fileops.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"
#include "wpad.h"

/*** Variables used only in this file ***/
static GuiText prTxt(NULL, 26, (GXColor){0, 0, 0, 255});
static GuiText speedTxt(NULL, 26, (GXColor){0, 0, 0, 255});
static GuiText sizeTxt(NULL, 26, (GXColor){0, 0, 0, 255});
static GuiText msgTxt(NULL, 26, (GXColor){0, 0, 0, 255});
static GuiImageData progressbar(progressbar_png);
static GuiImage progressbarImg(&progressbar);
static GuiImageData throbber(throbber_png);
static GuiImage throbberImg(&throbber);

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
int OnScreenKeyboard(char * var, u16 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText okBtnTxt("OK", 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return save;
}

/****************************************************************************
* WindowPrompt
*
* Displays a prompt window to user, with information, an error message, or
* presenting a user with a choice of up to 4 Buttons.
*
* Give him 1 Titel, 1 Subtitel and 4 Buttons
* If titel/subtitle or one of the buttons is not needed give him a 0 on that
* place.
***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label,
const char *btn2Label, const char *btn3Label,
const char *btn4Label)
{
    int choice = -1;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);


    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiImageData dialogBox(dialogue_box_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,55);
    titleTxt.SetMaxWidth(430, GuiText::DOTTED);

    GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);
    msgTxt.SetMaxWidth(430, GuiText::DOTTED);

    GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn1Img(&btnOutline);
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn1.SetLabel(&btn1Txt);
    btn1.SetImage(&btn1Img);
    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetSoundClick(&btnClick);
    btn1.SetTrigger(&trigA);
    btn1.SetState(STATE_SELECTED);
    btn1.SetEffectGrow();


    GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn2Img(&btnOutline);
    GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn2.SetLabel(&btn2Txt);
    btn2.SetImage(&btn2Img);
    btn2.SetSoundOver(&btnSoundOver);
    btn2.SetSoundClick(&btnClick);
    if(!btn3Label && !btn4Label)
    btn2.SetTrigger(&trigB);
    btn2.SetTrigger(&trigA);
    btn2.SetEffectGrow();

    GuiText btn3Txt(btn3Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn3Img(&btnOutline);
    GuiButton btn3(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn3.SetLabel(&btn3Txt);
    btn3.SetImage(&btn3Img);
    btn3.SetSoundOver(&btnSoundOver);
    btn3.SetSoundClick(&btnClick);
    if(!btn4Label)
    btn3.SetTrigger(&trigB);
    btn3.SetTrigger(&trigA);
    btn3.SetEffectGrow();

    GuiText btn4Txt(btn4Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn4Img(&btnOutline);
    GuiButton btn4(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn4.SetLabel(&btn4Txt);
    btn4.SetImage(&btn4Img);
    btn4.SetSoundOver(&btnSoundOver);
    btn4.SetSoundClick(&btnClick);
    if(btn4Label)
    btn4.SetTrigger(&trigB);
    btn4.SetTrigger(&trigA);
    btn4.SetEffectGrow();

    if(btn2Label && !btn3Label && !btn4Label) {
    btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn1.SetPosition(40, -45);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-40, -45);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else if(btn2Label && btn3Label && !btn4Label) {
    btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn1.SetPosition(50, -120);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-50, -120);
    btn3.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn3.SetPosition(0, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else if(btn2Label && btn3Label && btn4Label) {
    btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn1.SetPosition(50, -120);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-50, -120);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else if(!btn2Label && btn3Label && btn4Label) {
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn1.SetPosition(0, -120);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-50, -120);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else {
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn1.SetPosition(0, -45);
    btn2.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn2.SetPosition(50, -120);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    }

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);

    if(btn1Label)
    promptWindow.Append(&btn1);
    if(btn2Label)
    promptWindow.Append(&btn2);
    if(btn3Label)
    promptWindow.Append(&btn3);
    if(btn4Label)
    promptWindow.Append(&btn4);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

        if(btn1.GetState() == STATE_CLICKED) {
            choice = 1;
        }
        else if(btn2.GetState() == STATE_CLICKED) {
        if(!btn3Label)
            choice = 0;
        else
            choice = 2;
        }
        else if(btn3.GetState() == STATE_CLICKED) {
        if(!btn4Label)
            choice = 0;
        else
            choice = 3;
        }
        else if(btn4.GetState() == STATE_CLICKED) {
            choice = 0;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return choice;
}

/****************************************************************************
* RightClickMenu
***************************************************************************/
int RightClickMenu(int x, int y)
{
    int choice = -1;
    int numItems = 7;
    int buttonY = 0;

    GuiImageData dialogBox(clickmenu_png);
    GuiImage dialogBoxImg(&dialogBox);
    dialogBoxImg.SetPosition(-8, -dialogBox.GetHeight()/numItems/2);

    GuiWindow promptWindow(dialogBox.GetWidth(), dialogBox.GetHeight());
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(x, y);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    GuiText cutTxt("Cut", 26, (GXColor){0, 0, 0, 255});
    GuiText cutTxtOver("Cut", 26, (GXColor){28, 32, 190, 255});
    GuiButton btnCut(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    btnCut.SetLabel(&cutTxt);
    btnCut.SetLabelOver(&cutTxtOver);
    cutTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    cutTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    btnCut.SetSoundClick(&btnClick);
    btnCut.SetTrigger(&trigA);
    btnCut.SetPosition(0,buttonY);
    btnCut.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText copyTxt("Copy", 26, (GXColor){0, 0, 0, 255});
    GuiText copyTxtOver("Copy", 26, (GXColor){28, 32, 190, 255});
    GuiButton Copybtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Copybtn.SetLabel(&copyTxt);
    Copybtn.SetLabelOver(&copyTxtOver);
    copyTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    copyTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Copybtn.SetSoundClick(&btnClick);
    Copybtn.SetTrigger(&trigA);
    Copybtn.SetPosition(0,buttonY);
    Copybtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText pasteTxt("Paste", 26, (GXColor){0, 0, 0, 255});
    GuiText PasteTxtOver("Paste", 26, (GXColor){28, 32, 190, 255});
    GuiButton Pastebtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Pastebtn.SetLabel(&pasteTxt);
    Pastebtn.SetLabelOver(&PasteTxtOver);
    pasteTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PasteTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Pastebtn.SetSoundClick(&btnClick);
    Pastebtn.SetTrigger(&trigA);
    Pastebtn.SetPosition(0,buttonY);
    Pastebtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText RenameTxt("Rename", 26, (GXColor){0, 0, 0, 255});
    GuiText RenameTxtOver("Rename", 26, (GXColor){28, 32, 190, 255});
    GuiButton Renamebtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Renamebtn.SetLabel(&RenameTxt);
    Renamebtn.SetLabelOver(&RenameTxtOver);
    RenameTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    RenameTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Renamebtn.SetSoundClick(&btnClick);
    Renamebtn.SetTrigger(&trigA);
    Renamebtn.SetPosition(0,buttonY);
    Renamebtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText DeleteTxt("Delete", 26, (GXColor){0, 0, 0, 255});
    GuiText DeleteTxtOver("Delete", 26, (GXColor){28, 32, 190, 255});
    GuiButton Deletebtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Deletebtn.SetLabel(&DeleteTxt);
    Deletebtn.SetLabelOver(&DeleteTxtOver);
    DeleteTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    DeleteTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Deletebtn.SetSoundClick(&btnClick);
    Deletebtn.SetTrigger(&trigA);
    Deletebtn.SetPosition(0,buttonY);
    Deletebtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiText PropertiesTxt("Properties", 26, (GXColor){0, 0, 0, 255});
    GuiText PropertiesTxtOver("Properties", 26, (GXColor){28, 32, 190, 255});
    GuiButton Propertiesbtn(promptWindow.GetWidth(), promptWindow.GetHeight()/numItems);
    Propertiesbtn.SetLabel(&PropertiesTxt);
    Propertiesbtn.SetLabelOver(&PropertiesTxtOver);
    PropertiesTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PropertiesTxtOver.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Propertiesbtn.SetSoundClick(&btnClick);
    Propertiesbtn.SetTrigger(&trigA);
    Propertiesbtn.SetPosition(0,buttonY);
    Propertiesbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    buttonY += promptWindow.GetHeight()/numItems;

    GuiButton NoBtn(screenwidth, screenheight);
    NoBtn.SetPosition(-x, -y);
    NoBtn.SetTrigger(&trigA);
    NoBtn.SetTrigger(&trigB);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&NoBtn);
    promptWindow.Append(&btnCut);
    promptWindow.Append(&Copybtn);
    promptWindow.Append(&Pastebtn);
    promptWindow.Append(&Renamebtn);
    promptWindow.Append(&Deletebtn);
    promptWindow.Append(&Propertiesbtn);

    HaltGui();
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        usleep(THREAD_SLEEP);

        if(shutdown == 1) {
            Sys_Shutdown();
        }
        else if(reset == 1)
            Sys_Reboot();

        else if(btnCut.GetState() == STATE_CLICKED) {
            choice = CUT;
            break;
        }
        else if(Copybtn.GetState() == STATE_CLICKED) {
            choice = COPY;
            break;
        }
        else if(Pastebtn.GetState() == STATE_CLICKED) {
            choice = PASTE;
            break;
        }
        else if(Renamebtn.GetState() == STATE_CLICKED) {
            choice = RENAME;
            break;
        }
        else if(Deletebtn.GetState() == STATE_CLICKED) {
            choice = DELETE;
            break;
        }
        else if(Propertiesbtn.GetState() == STATE_CLICKED) {
            choice = PROPERTIES;
            break;
        }
        else if(NoBtn.GetState() == STATE_CLICKED){
            choice = -2;
            break;
        }
    }

    HaltGui();
    mainWindow->Remove(&promptWindow);
    ResumeGui();

    return choice;
}

/****************************************************************************
 * ShowProgress
 *
 * Updates the variables used by the progress window for drawing a progress
 * bar.
 ***************************************************************************/
void ShowProgress(u32 done, u32 total, char * filename, int progressmode)
{
    if(progressmode == PROGRESSBAR) {

 	static time_t start;
	//first time
	if (!done) {
		start    = time(0);
	}

	//Elapsed time
	u32 elapsed = time(0) - start;
	if(elapsed < 1) {
        usleep(THREAD_SLEEP);
        elapsed = 1;
	}
	//Calculate speed in KB/s
	u32 speed = 0;
    if(done)
        speed = done/(elapsed * KBSIZE);

    //Calculate percentage/size
	f32 percent = (done * 100.0) / total;

    prTxt.SetTextf("%0.2f", percent);

    speedTxt.SetTextf("%s %dKB/s","Speed:", speed);

	sizeTxt.SetTextf("%0.2fMB/%0.2fMB", done/MBSIZE, total/MBSIZE);

    msgTxt.SetText(filename);

	progressbarImg.SetTile(percent);

    } else {
        usleep(THREAD_SLEEP);
        static u16 angle;
        angle += 10;
        if(angle > 360)
            angle = 0 + (angle - 360);
        throbberImg.SetAngle(angle);
        msgTxt.SetText(filename);
    }
}

/****************************************************************************
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
int
ProgressWindow(const char *title, char * source, char *dest, int process, int mode)
{
	int ret = -1;

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

	GuiImageData btnOutline(button_png);
	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData progressbarOutline(progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 40);

	GuiImageData progressbarEmpty(progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 40);
	progressbarEmptyImg.SetTile(100);

    progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 40);

	throbberImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	throbberImg.SetPosition(0, 40);

	GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);
	titleTxt.SetMaxWidth(430, GuiText::DOTTED);

	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,120);
	msgTxt.SetMaxWidth(430, GuiText::DOTTED);

	GuiText prsTxt("%", 26, (GXColor){0, 0, 0, 255});
	prsTxt.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	prsTxt.SetPosition(-188,40);

    speedTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	speedTxt.SetPosition(275,-50);

    sizeTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	sizeTxt.SetPosition(50, -50);

	prTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	prTxt.SetPosition(200, 40);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
    if(mode == PROGRESSBAR) {
        promptWindow.Append(&progressbarEmptyImg);
        promptWindow.Append(&progressbarImg);
        promptWindow.Append(&progressbarOutlineImg);
        promptWindow.Append(&prTxt);
        promptWindow.Append(&prsTxt);
        promptWindow.Append(&speedTxt);
        promptWindow.Append(&sizeTxt);
	} else {
	    promptWindow.Append(&throbberImg);
	}

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

    if(process == COPYDIR)
        ret = CopyDirectory(source, dest);
    else if(process == COPYFILE)
        ret = CopyFile(source, dest);
    else if(process == MOVEDIR)
        ret = MoveDirectory(source, dest);
    else if(process == DELETEDIR)
        ret =RemoveDirectory(source);
    else if(process == DELETEFILE)
        ret = RemoveFile(source);

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

    return ret;
}
