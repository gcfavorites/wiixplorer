/***************************************************************************
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
#include <sys/dir.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "Controls/MainWindow.h"
#include "Prompts/PromptWindows.h"
#include "Memory/Resources.h"
#include "MusicPlayer.h"
#include "filelist.h"
#include "Language/gettext.h"
#include "FileOperations/fileops.h"
#include "FileStartUp/FileExtensions.h"
#include "Tools/StringTools.h"
#include "main.h"
#include "sys.h"

MusicPlayer * MusicPlayer::instance = NULL;

MusicPlayer::MusicPlayer()
    : GuiWindow(0, 0)
{
    btnSoundOver = NULL;
    playerImgData = NULL;
    navi_defaultImgData = NULL;
    navi_upImgData = NULL;
    navi_downImgData = NULL;
    navi_leftImgData = NULL;
    navi_rightImgData = NULL;
    trigA = NULL;
    trigB = NULL;
    BackButton = NULL;
    PlayBtn = NULL;
    StopBtn = NULL;
    NextBtn = NULL;
    PreviousBtn = NULL;
    BackgroundImg = NULL;
    CircleImg = NULL;
    PlayTitle = NULL;

    LoopMode = 0;
    currentPlaying = 0;
    CircleImageDelay = 0;
    DisplayGUI = false;
    Stopped = false;
    ExitRequested = false;
    MainSound = new GuiSound(bg_music_ogg, bg_music_ogg_size, false, 0);

    TitleList.LoadList();

    if(strcmp(Settings.MusicPath, "") == 0 || TitleList.size() == 0)
    {
        LoadStandard();
    }
    else if(TitleList.size() > 0)
    {
        currentPlaying = TitleList.GetCurrentPlaying(Settings.MusicPath);
        Play(currentPlaying);
    }

	LWP_CreateThread (&bgmthread, UpdateBMG, this, NULL, 16384, 0);
}

MusicPlayer::~MusicPlayer()
{
    ExitRequested = true;

    ResumeThread();
    LWP_JoinThread(bgmthread, NULL);
    bgmthread = LWP_THREAD_NULL;

    Hide();
};

MusicPlayer * MusicPlayer::Instance()
{
	if (instance == NULL)
	{
		instance = new MusicPlayer();
	}
	return instance;
}

void MusicPlayer::ResumeThread()
{
    LWP_ResumeThread(bgmthread);
}

void MusicPlayer::HaltThread()
{
    LWP_SuspendThread(bgmthread);
}

void MusicPlayer::DestroyInstance()
{
    if(instance)
    {
        delete instance;
    }
    instance = NULL;
}

bool MusicPlayer::LoadStandard()
{
    strcpy(Settings.MusicPath, "");

    currentPlaying = 0;

    Title.assign(tr("WiiXplorer Default Music"));

    if(PlayTitle)
        PlayTitle->SetText(Title.c_str());

    MainSound->Load(bg_music_ogg, bg_music_ogg_size, false);
    MainSound->Play();

    return true;
}

bool MusicPlayer::Load(const char * path, bool silent)
{
    if(!path)
    {
        LoadStandard();
        return false;
    }

    if(strcmp(path, "") == 0)
    {
        LoadStandard();
        return false;
    }

    if(Settings.LoadMusicToMem)
    {
        u64 filesize;
        u8 * file = NULL;

        int ret = -1;
        if(silent)
            ret = LoadFileToMem(path, &file, &filesize);
        else
            ret = LoadFileToMemWithProgress(tr("Loading file:"), path, &file, &filesize);

        if (ret < 0)
        {
            LoadStandard();
            return false;
        }

        if(!MainSound->Load(file, (u32) filesize, true))
        {
            if(file)
                free(file);
            LoadStandard();
            return false;
        }
    }
    else
    {
        if(!MainSound->Load(path))
        {
            LoadStandard();
            return false;
        }
    }

    char * filename = strrchr(path, '/');
    if(filename)
        Title.assign(filename+1);

    if(PlayTitle)
        PlayTitle->SetText(Title.c_str());

    MainSound->Play();
    Stopped = false;
    Paused = false;

    return true;
}

void MusicPlayer::Resume()
{
    MainSound->Play();
    Stopped = false;
    Paused = false;
}

void MusicPlayer::Stop()
{
    Paused = false;
    Stopped = true;
    MainSound->Stop();
}

bool MusicPlayer::Play()
{
    return Play(currentPlaying);
}

bool MusicPlayer::Play(int pos)
{
    if(pos < 0 || pos >= (int) TitleList.size())
        return LoadStandard();

    currentPlaying = pos;

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s", TitleList.at(currentPlaying));

    return Load(Settings.MusicPath, true);
}

bool MusicPlayer::PlayNext()
{
    if(TitleList.size() == 0)
        return LoadStandard();

    currentPlaying++;
    if(currentPlaying >= (int) TitleList.size())
        currentPlaying = 0;

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s", TitleList.at(currentPlaying));

    return Load(Settings.MusicPath, true);
}

bool MusicPlayer::PlayPrevious()
{
    if(TitleList.size() == 0)
        return LoadStandard();

    currentPlaying--;
    if(currentPlaying < 0)
        currentPlaying = TitleList.size()-1;

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s", TitleList.at(currentPlaying));

    return Load(Settings.MusicPath, true);
}

bool MusicPlayer::PlayRandom()
{
    if(TitleList.size() == 0)
        return LoadStandard();

    srand(time(NULL));

    currentPlaying = rand() % TitleList.size();

    //just in case
    if(currentPlaying < 0)
        currentPlaying = TitleList.size()-1;
    else if(currentPlaying >= (int) TitleList.size())
        currentPlaying = 0;

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s", TitleList.at(currentPlaying));

    return Load(Settings.MusicPath, true);
}

void MusicPlayer::Show()
{
    Setup();
    TitleList.Show();
    SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 40);
    MainWindow::Instance()->Append(this);
    DisplayGUI = true;
    int choice = -1;

    while(DisplayGUI)
    {
        usleep(100);

		if(shutdown)
			Sys_Shutdown();
		else if(reset)
			Sys_Reboot();

        choice = TitleList.GetChoice();

        if(choice >= 0)
        {
            char * filename = strrchr(TitleList.at(choice), '/');
            if(filename)
                filename++;

            SetState(STATE_DISABLED);
            int ret = WindowPrompt(tr("How do you want to proceed?"), filename, tr("Play File"), tr("Remove from list"), tr("Clear List"), tr("Cancel"), false);
            if(ret == 1)
            {
                Play(choice);
            }
            else if(ret == 2)
            {
                TitleList.RemoveEntrie(choice);
                currentPlaying = TitleList.GetCurrentPlaying(Settings.MusicPath);
            }
            else if(ret == 3)
            {
                ClearList();
            }
            SetState(STATE_DEFAULT);
        }
        else if(PlayBtn->GetState() == STATE_CLICKED)
        {
            CircleImg->SetImage(navi_upImgData);
            CircleImageDelay = 50;
            Play(TitleList.GetSelectedItem());
            PlayBtn->ResetState();
        }
        else if(StopBtn->GetState() == STATE_CLICKED)
        {
            CircleImg->SetImage(navi_downImgData);
            CircleImageDelay = 50;
            Stop();
            StopBtn->ResetState();
        }
        else if(PreviousBtn->GetState() == STATE_CLICKED)
        {
            CircleImg->SetImage(navi_leftImgData);
            CircleImageDelay = 50;
            PlayPrevious();
            PreviousBtn->ResetState();
        }
        else if(NextBtn->GetState() == STATE_CLICKED)
        {
            CircleImg->SetImage(navi_rightImgData);
            CircleImageDelay = 50;
            PlayNext();
            NextBtn->ResetState();
        }
    }

    Hide();
    TitleList.Hide();
}

void MusicPlayer::Hide()
{
    if(!ExitRequested)
    {
        MainWindow::Instance()->ResumeGui();
        SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 40);
        while(this->GetEffect() > 0) usleep(100);

        MainWindow::Instance()->HaltGui();
        if(parentElement)
            ((GuiWindow *) parentElement)->Remove(this);
        MainWindow::Instance()->ResumeGui();
    }

    RemoveAll();

    if(trigA)
        delete trigA;
    if(trigB)
        delete trigB;

    Resources::Remove(btnSoundOver);
    Resources::Remove(playerImgData);
    Resources::Remove(navi_defaultImgData);
    Resources::Remove(navi_upImgData);
    Resources::Remove(navi_downImgData);
    Resources::Remove(navi_leftImgData);
    Resources::Remove(navi_rightImgData);

    if(BackButton)
        delete BackButton;
    if(PlayBtn)
        delete PlayBtn;
    if(StopBtn)
        delete StopBtn;
    if(NextBtn)
        delete NextBtn;
    if(PreviousBtn)
        delete PreviousBtn;

    if(BackgroundImg)
        delete BackgroundImg;
    if(CircleImg)
        delete CircleImg;

    if(PlayTitle)
        delete PlayTitle;

    btnSoundOver = NULL;

    playerImgData = NULL;
    navi_defaultImgData = NULL;
    navi_upImgData = NULL;
    navi_downImgData = NULL;
    navi_leftImgData = NULL;
    navi_rightImgData = NULL;

    trigA = NULL;
    trigB = NULL;

    BackButton = NULL;
    PlayBtn = NULL;
    StopBtn = NULL;
    NextBtn = NULL;
    PreviousBtn = NULL;

    BackgroundImg = NULL;
    CircleImg = NULL;

    PlayTitle = NULL;
}

void MusicPlayer::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == BackButton)
    {
        DisplayGUI = false;
    }
}

void MusicPlayer::Setup()
{
    trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
    trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);

	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
    playerImgData = Resources::GetImageData(player_png, player_png_size);
    navi_defaultImgData = Resources::GetImageData(navi_default_png, navi_default_png_size);
    navi_upImgData = Resources::GetImageData(navi_up_png, navi_up_png_size);
    navi_downImgData = Resources::GetImageData(navi_down_png, navi_down_png_size);
    navi_leftImgData = Resources::GetImageData(navi_left_png, navi_left_png_size);
    navi_rightImgData = Resources::GetImageData(navi_right_png, navi_right_png_size);

    BackButton = new GuiButton(35, 40);
    BackButton->SetPosition(404, 115);
    BackButton->SetSoundOver(btnSoundOver);
    BackButton->SetTrigger(trigA);
    BackButton->SetTrigger(trigB);
    BackButton->Clicked.connect(this, &MusicPlayer::OnButtonClick);

    PlayBtn = new GuiButton(68, 40);
    PlayBtn->SetPosition(77, 39);
    PlayBtn->SetTrigger(trigA);

    StopBtn = new GuiButton(68, 40);
    StopBtn->SetPosition(77, 124);
    StopBtn->SetTrigger(trigA);

    PreviousBtn = new GuiButton(38, 60);
    PreviousBtn->SetPosition(44, 74);
    PreviousBtn->SetTrigger(trigA);

    NextBtn = new GuiButton(38, 60);
    NextBtn->SetPosition(137, 74);
    NextBtn->SetTrigger(trigA);

    BackgroundImg = new GuiImage(playerImgData);
    CircleImg = new GuiImage(navi_defaultImgData);
    CircleImg->SetPosition(39, 34);

    PlayTitle = new GuiText(Title.c_str(), 22, (GXColor) {0, 0, 0, 255});
    PlayTitle->SetPosition(220, 126);
    PlayTitle->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PlayTitle->SetMaxWidth(155, SCROLL_HORIZONTAL);

    TitleList.SetPosition(90, 56);
	TitleList.SetMaxHeight(GetTop()-170+TitleList.GetHeight());

    Append(&TitleList);
    Append(BackgroundImg);
    Append(CircleImg);
    Append(BackButton);
    Append(PlayBtn);
    Append(StopBtn);
    Append(NextBtn);
    Append(PreviousBtn);
    Append(PlayTitle);
}

void MusicPlayer::Update(GuiTrigger * t)
{
    if(CircleImageDelay > 0)
    {
        --CircleImageDelay;
        if(CircleImageDelay == 0 && CircleImg != NULL)
            CircleImg->SetImage(navi_defaultImgData);
    }

    GuiWindow::Update(t);
}

void * MusicPlayer::UpdateBMG(void * arg)
{
	((MusicPlayer *) arg)->UpdateState();
	return NULL;
}

void MusicPlayer::UpdateState()
{
    while(!ExitRequested)
    {
        usleep(100);
        HaltThread();

        if(!MainSound->IsPlaying() && !Stopped && !Paused)
        {
            if(LoopMode > 0 && strcmp(Settings.MusicPath, "") == 0)
            {
                //!Standard Music is always looped except on loop = 0
                Play();
            }
            else if(LoopMode == PLAYLIST_LOOP)
            {
                PlayNext();
            }
            else if(LoopMode == RANDOM_MUSIC)
            {
                PlayRandom();
            }
        }
    }
}