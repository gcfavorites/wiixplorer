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
 * ProgressWindow
 * WiiXplorer 2010
 *
 * ProgressWindow.cpp
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libwiigui/gui.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Controls/IOHandler.hpp"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/filebrowser.h"
#include "menu.h"
#include "FileOperations/fileops.h"
#include "sys.h"

bool actioncanceled = false;

ProgressWindow * ProgressWindow::instance = NULL;

ProgressWindow::ProgressWindow()
{
    ProgressThread = LWP_THREAD_NULL;
    showProgress = 0;
    TotalDone = 0;
    TotalSize = 0;
    progressDone = 0;
    progressTotal = 100;
    Changed = false;
    ExitRequested = false;
    Minimized = false;
    Minimizable = false;
    ValuesResetable = true;
    ProgressMsg[0] = '\0';

    dialogBox = NULL;
    progressbarOutline = NULL;
    throbber = NULL;
    btnOutline = NULL;
    dialogBoxImg = NULL;
    progressbarOutlineImg = NULL;
    progressbarEmptyImg = NULL;
    progressbarImg = NULL;
    progressbarTotalOutlineImg = NULL;
    progressbarTotalEmptyImg = NULL;
    progressbarTotalImg = NULL;
    throbberImg = NULL;
    buttonImg = NULL;
    MinimizeImg = NULL;
    titleTxt = NULL;
    msgTxt = NULL;
    prTxt = NULL;
    prTotalTxt = NULL;
    prsTxt = NULL;
    prsTotalTxt = NULL;
    speedTxt = NULL;
    sizeTxt = NULL;
    sizeTotalTxt = NULL;
    TimeTxt = NULL;
    AbortTxt = NULL;
    MinimizeTxt = NULL;
    AbortBtn = NULL;
    MinimizeBtn = NULL;
    trigA = NULL;
    soundClick = NULL;
    soundOver = NULL;

    ThreadStack = (u8 *) memalign(32, 32768);
	LWP_CreateThread(&ProgressThread, ThreadCallback, this, ThreadStack, 32768, 70);
}

ProgressWindow::~ProgressWindow()
{
    ExitRequested = true;
    actioncanceled = true;
    showProgress = 0;
    LWP_ResumeThread(ProgressThread);
	LWP_JoinThread(ProgressThread, NULL);
	ProgressThread = LWP_THREAD_NULL;
	if(ThreadStack)
        free(ThreadStack);

	ClearMemory();
}

ProgressWindow * ProgressWindow::Instance()
{
	if (instance == NULL)
	{
		instance = new ProgressWindow();
	}
	return instance;
}

void ProgressWindow::DestroyInstance()
{
    if(instance)
    {
        delete instance;
        instance = NULL;
    }
}

void ProgressWindow::LoadWindow()
{
	if(!showProgress || Minimized)
        return;

    //! To skip progressbar for fast processes
	usleep(500000);
	if(!showProgress)
        return;

	SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	SetPosition(0, -10);

    if(showProgress == PROGRESSBAR)
        dialogBox = Resources::GetImageData(progress_window_png, progress_window_png_size);
    else if(showProgress == THROBBER || showProgress == AUTO_THROBBER)
        dialogBox = Resources::GetImageData(dialogue_box_png, dialogue_box_png_size);

	dialogBoxImg = new GuiImage(dialogBox);
	Append(dialogBoxImg);

    width = dialogBox->GetWidth();
    height = dialogBox->GetHeight();

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	soundClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	soundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

	titleTxt = new GuiText(ProgressTitle.c_str(), 22, (GXColor){0, 0, 0, 255});
	titleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt->SetPosition(0,50);
	titleTxt->SetMaxWidth(430, DOTTED);
	Append(titleTxt);

    msgTxt = new GuiText((char*) NULL, 22, (GXColor){0, 0, 0, 255});
	msgTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt->SetPosition(0,110);
	msgTxt->SetMaxWidth(430, DOTTED);
	Append(msgTxt);

    btnOutline = Resources::GetImageData(small_button_png, small_button_png_size);
	buttonImg = new GuiImage(btnOutline);
    AbortTxt = new GuiText(tr("Cancel"), 22, (GXColor){0, 0, 0, 255});
	AbortBtn = new GuiButton(buttonImg->GetWidth(), buttonImg->GetHeight());
	AbortBtn->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	AbortBtn->SetPosition(0, -48);
	AbortBtn->SetLabel(AbortTxt);
	AbortBtn->SetImage(buttonImg);
	AbortBtn->SetTrigger(trigA);
	AbortBtn->SetSoundClick(soundClick);
	AbortBtn->SetSoundOver(soundOver);
	AbortBtn->SetEffectGrow();
	Append(AbortBtn);

    if(Minimizable)
    {
        MinimizeImg = new GuiImage(btnOutline);
        MinimizeTxt = new GuiText(tr("Minimize"), 22, (GXColor){0, 0, 0, 255});
        MinimizeBtn = new GuiButton(MinimizeImg->GetWidth(), MinimizeImg->GetHeight());
        MinimizeBtn->SetLabel(MinimizeTxt);
        MinimizeBtn->SetImage(MinimizeImg);
        MinimizeBtn->SetTrigger(trigA);
        MinimizeBtn->SetSoundClick(soundClick);
        MinimizeBtn->SetSoundOver(soundOver);
        MinimizeBtn->SetEffectGrow();
        Append(MinimizeBtn);

        AbortBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        AbortBtn->SetPosition(-60, -48);
        MinimizeBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        MinimizeBtn->SetPosition(60, -48);
    }

    if(showProgress == PROGRESSBAR)
        SetupProgressbar();

    else if(showProgress == THROBBER || showProgress == AUTO_THROBBER)
        SetupThrobber();

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    bool isDimmed = MainWindow::Instance()->IsDimmed();
    int oldState = MainWindow::Instance()->GetState();

	MainWindow::Instance()->SetState(STATE_DISABLED);
	MainWindow::Instance()->SetDim(true);
	MainWindow::Instance()->Append(this);

	MainLoop();

    if(ExitRequested)
        return;

    MainWindow::Instance()->HaltGui();
    ResetEffects();
    MainWindow::Instance()->Remove(this);
    MainWindow::Instance()->SetState(oldState);
    MainWindow::Instance()->SetDim(isDimmed);
    ClearMemory();
    MainWindow::Instance()->ResumeGui();

	actioncanceled = false;
}

void ProgressWindow::MainLoop()
{
    int angle = 0;

	while(showProgress)
	{
	    usleep(30000);

	    if(Changed)
	    {
            msgTxt->SetText(ProgressMsg);

            UpdateValues();

            if(showProgress == THROBBER)
            {
                angle = (angle + 5) % 360;
                throbberImg->SetAngle(angle);
            }
            Changed = false;
	    }

	    if(showProgress == AUTO_THROBBER)
	    {
            angle = (angle + 5) % 360;
            throbberImg->SetAngle(angle);
	    }

        if(shutdown)
        {
            ExitRequested = true;
            Sys_Shutdown();
            return;
        }
        else if(reset)
        {
            ExitRequested = true;
            Sys_Reboot();
            return;
        }

        else if(AbortBtn->GetState() == STATE_CLICKED)
        {
            actioncanceled = true;
            AbortBtn->ResetState();
        }
        else if(MinimizeBtn && MinimizeBtn->GetState() == STATE_CLICKED)
        {
            Minimized = true;
            IOHandler::Instance()->SetMinimized(showProgress);
            showProgress = 0;
            MinimizeBtn->ResetState();
        }
	}

    if(Minimized)
        SetEffect(EFFECT_FADE, -20);
	else
        SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);

    if(!ExitRequested)
        while(GetEffect() > 0) usleep(100);
}

void ProgressWindow::SetupProgressbar()
{
    GXColor ImgColor[4];
    ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressEmptyUL);
    ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressEmptyUR);
    ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressEmptyBR);
    ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressEmptyBL);

	progressbarEmptyImg = new GuiImage(360, 36, (GXColor *) &ImgColor);
	progressbarEmptyImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	progressbarEmptyImg->SetPosition(58, 155);
	Append(progressbarEmptyImg);

    ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressUL);
    ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressUR);
    ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressBR);
    ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressBL);

	progressbarOutline = Resources::GetImageData(progressbar_outline_png, progressbar_outline_png_size);

	int OutLineMiddle = progressbarOutline->GetHeight()/2;

    progressbarImg = new GuiImage(4, 36, (GXColor *) &ImgColor);
    progressbarImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	progressbarImg->SetPosition(58, 155);
	Append(progressbarImg);

	progressbarOutlineImg = new GuiImage(progressbarOutline);
	progressbarOutlineImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	progressbarOutlineImg->SetPosition(58, 155);
	Append(progressbarOutlineImg);

    prTxt = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
	prTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	prTxt->SetPosition(219, 155+OutLineMiddle-9);
	Append(prTxt);

	prsTxt = new GuiText("%", 20, (GXColor){0, 0, 0, 255});
	prsTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	prsTxt->SetPosition(273, 155+OutLineMiddle-9);
	Append(prsTxt);

    sizeTxt = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
    sizeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	sizeTxt->SetPosition(55, 155+50);
	Append(sizeTxt);

    TimeTxt = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
    TimeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	TimeTxt->SetPosition(255, 155+50);
	Append(TimeTxt);

	//! Total progress
    ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressEmptyUL);
    ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressEmptyUR);
    ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressEmptyBR);
    ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressEmptyBL);

	progressbarTotalEmptyImg = new GuiImage(360, 36, (GXColor *) &ImgColor);
	progressbarTotalEmptyImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	progressbarTotalEmptyImg->SetPosition(58, 235);
	Append(progressbarTotalEmptyImg);

    ImgColor[0] = RGBATOGXCOLOR(Settings.ProgressUL);
    ImgColor[1] = RGBATOGXCOLOR(Settings.ProgressUR);
    ImgColor[2] = RGBATOGXCOLOR(Settings.ProgressBR);
    ImgColor[3] = RGBATOGXCOLOR(Settings.ProgressBL);

    progressbarTotalImg = new GuiImage(4, 36, (GXColor *) &ImgColor);
    progressbarTotalImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	progressbarTotalImg->SetPosition(58, 235);
	Append(progressbarTotalImg);

	progressbarTotalOutlineImg = new GuiImage(progressbarOutline);
	progressbarTotalOutlineImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	progressbarTotalOutlineImg->SetPosition(58, 235);
	Append(progressbarTotalOutlineImg);

    prTotalTxt = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
	prTotalTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	prTotalTxt->SetPosition(219, 235+OutLineMiddle-9);
	Append(prTotalTxt);

	prsTotalTxt = new GuiText("%", 20, (GXColor){0, 0, 0, 255});
	prsTotalTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	prsTotalTxt->SetPosition(273, 235+OutLineMiddle-9);
	Append(prsTotalTxt);

    sizeTotalTxt = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
    sizeTotalTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	sizeTotalTxt->SetPosition(55, 235+50);
	Append(sizeTotalTxt);

    speedTxt = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 255});
    speedTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	speedTxt->SetPosition(345, 235+50);
	Append(speedTxt);
}

void ProgressWindow::SetupThrobber()
{
    throbber = Resources::GetImageData(throbber_png, throbber_png_size);
    throbberImg = new GuiImage(throbber);
	throbberImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	throbberImg->SetPosition(0, 25);
	Append(throbberImg);
}

void ProgressWindow::UpdateValues()
{
    if(showProgress != PROGRESSBAR)
        return;

    //! File progress
    float Percent = progressTotal > 0 ? 100.0f*progressDone/progressTotal : 0.0f;

    progressbarImg->SetSize((int) (Percent*3.6f), 36);

    if(Percent >= 100.0f)
        prTxt->SetTextf("%0.1f", Percent);
    else
        prTxt->SetTextf("%0.2f", Percent);

    int speed = 0;

    if(TimerSize > 0)
        speed = (int) (TimerSize/ProgressTimer.elapsed());

    speedTxt->SetTextf("%iKB/s", (int) (speed/KBSIZE));

    int TimeLeft = 0;
    if(speed > 0)
        TimeLeft = (TotalSize-(TotalDone+progressDone))/speed;

    u32 h =  TimeLeft / 3600;
    u32 m = (TimeLeft / 60) % 60;
    u32 s =  TimeLeft % 60;

    TimeTxt->SetTextf("%s %02i:%02i:%02i", tr("Time left:"), h, m, s);

    if(progressTotal > KBSIZE && progressTotal < MBSIZE)
        sizeTxt->SetTextf("%0.2fKB/%0.2fKB", progressDone/KBSIZE, progressTotal/KBSIZE);
    else if(progressTotal > MBSIZE && progressTotal < GBSIZE)
        sizeTxt->SetTextf("%0.2fMB/%0.2fMB", progressDone/MBSIZE, progressTotal/MBSIZE);
    else if(progressTotal > GBSIZE)
        sizeTxt->SetTextf("%0.2fGB/%0.2fGB", progressDone/GBSIZE, progressTotal/GBSIZE);
    else
        sizeTxt->SetTextf("%lldB/%lldB", progressDone, progressTotal);

    //! Total progress

    float TotalPercent = 0.0f;
    if(TotalSize > 0)
        TotalPercent = 100.0f*(TotalDone+progressDone)/TotalSize;

    if(TotalPercent > 100.0f)
        TotalPercent = 100.0f;

    progressbarTotalImg->SetSize((int) (TotalPercent*3.6f), 36);

    if(TotalPercent >= 100.0f)
        prTotalTxt->SetTextf("%0.1f", TotalPercent);
    else
        prTotalTxt->SetTextf("%0.2f", TotalPercent);

    if(TotalSize > KBSIZE && TotalSize < MBSIZE)
        sizeTotalTxt->SetTextf("%0.2fKB/%0.2fKB", (TotalDone+progressDone)/KBSIZE, TotalSize/KBSIZE);
    else if(TotalSize > MBSIZE && TotalSize < GBSIZE)
        sizeTotalTxt->SetTextf("%0.2fMB/%0.2fMB", (TotalDone+progressDone)/MBSIZE, TotalSize/MBSIZE);
    else if(TotalSize > GBSIZE)
        sizeTotalTxt->SetTextf("%0.2fGB/%0.2fGB", (TotalDone+progressDone)/GBSIZE, TotalSize/GBSIZE);
    else
        sizeTotalTxt->SetTextf("%lldB/%lldB", (TotalDone+progressDone), TotalSize);
}

void * ProgressWindow::ThreadCallback(void *arg)
{
    ((ProgressWindow *) arg)->InternalUpdate();
	return NULL;
}

void ProgressWindow::InternalUpdate()
{
	while(!ExitRequested)
	{
		if(!showProgress)
			LWP_SuspendThread(ProgressThread);

        if(!ExitRequested)
            LoadWindow();
	}
}

void ProgressWindow::StartProgress(const char *title, int progressmode)
{
    if(title)
        ProgressTitle.assign(title);
    if(titleTxt)
        titleTxt->SetText(title);

	showProgress = progressmode;

    TimerSize = 0;
    ProgressTimer.reset();

    LWP_ResumeThread(ProgressThread);
}

void ProgressWindow::ShowProgress(u64 done, u64 total, const char *msg)
{
    if(total < 0)
		return;

	if(done > total)
		done = total;

    if(msg)
        strcpy(ProgressMsg, msg);
    else
        strcpy(ProgressMsg, "");

    //progress start for this file
    if(!done)
    {
	    TotalDone += progressTotal;
	    if(TotalSize == 0)
            TotalSize = total;
    }

    if(done > progressDone)
        TimerSize += done-progressDone;

    progressDone = done;
    progressTotal = total;

    Changed = true;
}

void ProgressWindow::StopProgress()
{
	showProgress = 0;
	if(ValuesResetable)
	{
        progressTotal = 0;
        TotalDone = 0;
        TotalSize = 0;
	}

	while(!LWP_ThreadIsSuspended(ProgressThread) && !ExitRequested)
        usleep(100);
}

void ProgressWindow::ClearMemory()
{
    RemoveAll();

    if(soundClick)
        Resources::Remove(soundClick);
    if(soundOver)
        Resources::Remove(soundOver);

    if(dialogBox)
        Resources::Remove(dialogBox);
    if(progressbarOutline)
        Resources::Remove(progressbarOutline);
    if(throbber)
        Resources::Remove(throbber);
    if(btnOutline)
        Resources::Remove(btnOutline);

    if(dialogBoxImg)
		delete dialogBoxImg;
    if(progressbarOutlineImg)
		delete progressbarOutlineImg;
    if(progressbarEmptyImg)
		delete progressbarEmptyImg;
    if(progressbarImg)
		delete progressbarImg;
    if(progressbarTotalOutlineImg)
		delete progressbarTotalOutlineImg;
    if(progressbarTotalEmptyImg)
		delete progressbarTotalEmptyImg;
    if(progressbarTotalImg)
		delete progressbarTotalImg;
    if(throbberImg)
		delete throbberImg;
    if(buttonImg)
		delete buttonImg;
    if(MinimizeImg)
		delete MinimizeImg;

    if(titleTxt)
        delete titleTxt;
    if(msgTxt)
        delete msgTxt;
    if(prTxt)
        delete prTxt;
    if(prsTxt)
        delete prsTxt;
    if(prTotalTxt)
        delete prTotalTxt;
    if(prsTotalTxt)
        delete prsTotalTxt;
    if(speedTxt)
        delete speedTxt;
    if(sizeTxt)
        delete sizeTxt;
    if(sizeTotalTxt)
        delete sizeTotalTxt;
    if(TimeTxt)
        delete TimeTxt;
    if(AbortTxt)
        delete AbortTxt;
    if(MinimizeTxt)
        delete MinimizeTxt;

    if(AbortBtn)
        delete AbortBtn;
    if(MinimizeBtn)
        delete MinimizeBtn;

    if(trigA)
        delete trigA;

    dialogBox = NULL;
    progressbarOutline = NULL;
    throbber = NULL;
    btnOutline = NULL;
    dialogBoxImg = NULL;
    progressbarOutlineImg = NULL;
    progressbarEmptyImg = NULL;
    progressbarImg = NULL;
    progressbarTotalOutlineImg = NULL;
    progressbarTotalEmptyImg = NULL;
    progressbarTotalImg = NULL;
    throbberImg = NULL;
    buttonImg = NULL;
    MinimizeImg = NULL;
    titleTxt = NULL;
    msgTxt = NULL;
    prTxt = NULL;
    prTotalTxt = NULL;
    prsTxt = NULL;
    prsTotalTxt = NULL;
    speedTxt = NULL;
    sizeTxt = NULL;
    sizeTotalTxt = NULL;
    TimeTxt = NULL;
    AbortTxt = NULL;
    MinimizeTxt = NULL;
    AbortBtn = NULL;
    MinimizeBtn = NULL;
    trigA = NULL;
    soundClick = NULL;
    soundOver = NULL;
}

void StartProgress(const char *title, int mode)
{
    ProgressWindow::Instance()->StartProgress(title, mode);
}

void ShowProgress(u64 done, u64 total, const char *filename)
{
    ProgressWindow::Instance()->ShowProgress(done, total, filename);
}

void StopProgress()
{
    ProgressWindow::Instance()->StopProgress();
}
