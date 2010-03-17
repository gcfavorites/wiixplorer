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
 * WiiMovie.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <unistd.h>
#include "Controls/MainWindow.h"
#include "WiiMovie.hpp"
#include "ImageOperations/TextureConverter.h"
#include "SoundOperations/gui_bgm.h"
#include "menu.h"

#define SND_BUFFERS     20

static u8 which = 0;
static vector<s16> soundbuffer[2];
static u16 sndsize[2] = {0, 0};
static u16 MaxSoundSize = 0;

WiiMovie::WiiMovie(const char * filepath)
{
    VideoFrameCount = 0;
    ExitRequested = false;
    Playing = false;
    volume = 255*80/100;

	background = new GuiImage(screenwidth, screenheight, (GXColor){0, 0, 0, 255});

    trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    exitBtn = new GuiButton(1, 1);
    exitBtn->SetTrigger(trigB);
	exitBtn->Clicked.connect(this, &WiiMovie::OnExitClick);

    GuiBGM::Instance()->Stop();

    string file(filepath);
    Video = openVideo(file);
    if(!Video)
    {
        ShowError(tr("Unsupported format!"));
        ExitRequested = true;
    }

    SndChannels = (Video->getNumChannels() == 2) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
    SndFrequence = Video->getFrequency();
    MaxSoundSize = Video->getMaxAudioSamples()*2;

    LWP_MutexInit(&mutex, true);
	LWP_CreateThread (&ReadThread, UpdateThread, this, NULL, 0, LWP_PRIO_HIGHEST);
}

WiiMovie::~WiiMovie()
{
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    Playing = true;
    ExitRequested = true;

    LWP_JoinThread(ReadThread, NULL);
    LWP_MutexDestroy(mutex);

    ASND_StopVoice(0);
    GuiBGM::Instance()->Play();

    for(u32 i = 0; i < Frames.size(); i++)
    {
        if(Frames.at(i))
            free(Frames.at(i));

        Frames.at(i) = NULL;
    }

    for(u8 i = 0; i < 2; i++)
    {
        soundbuffer[i].clear();
        sndsize[i] = 0;
    }
    which = 0;

    Frames.clear();

    delete background;
    delete exitBtn;
    delete trigB;

    if(Video)
       closeVideo(Video);
}

bool WiiMovie::Play()
{
    if(!Video)
        return false;

    Playing = true;
    PlayTime.reset();
    LWP_ResumeThread(ReadThread);

    InternalUpdate();

	return true;
}

void WiiMovie::Stop()
{
    ExitRequested = true;
}

void WiiMovie::SetVolume(int vol)
{
    volume = 255*vol/100;
}

void WiiMovie::OnExitClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();
    Stop();
}

void WiiMovie::FillBuffer()
{
    while(Frames.size() < 10 && !ExitRequested)
        LoadNextFrame();
}

extern "C" void callback(int voice)
{
    if(soundbuffer[which].size() == 0 || sndsize[which] < MaxSoundSize*(SND_BUFFERS-1))
        return;

    if(ASND_AddVoice(0, (u8*) &soundbuffer[which][0], sndsize[which]) != SND_OK)
    {
        return;
    }

    which ^= 1;

    sndsize[which] = 0;
}

void * WiiMovie::UpdateThread(void *arg)
{
	while(!((WiiMovie *) arg)->ExitRequested)
	{
        ((WiiMovie *) arg)->InternalThreadUpdate();
	}
	return NULL;
}

void WiiMovie::InternalThreadUpdate()
{
    if(!Playing)
        LWP_SuspendThread(ReadThread);

    u32 FramesNeeded = (u32) (PlayTime.elapsed()*Video->getFps());

    while(VideoFrameCount < FramesNeeded)
    {
        LWP_MutexLock(mutex);
        Video->loadNextFrame();
        LWP_MutexUnlock(mutex);

        ++VideoFrameCount;

        if(Video->hasSound())
        {
            if(sndsize[which] > MaxSoundSize*(SND_BUFFERS-1))
                return;

            if(soundbuffer[which].size() == 0)
                soundbuffer[which].resize(Video->getMaxAudioSamples()*2*SND_BUFFERS);

            sndsize[which] += Video->getCurrentBuffer(&soundbuffer[which][sndsize[which]/2])*2*2;

            if(ASND_StatusVoice(0) == SND_UNUSED && sndsize[which] >= MaxSoundSize*(SND_BUFFERS-1))
            {
                ASND_StopVoice(0);
                ASND_SetVoice(0, SndChannels, SndFrequence, 0, (u8 *) &soundbuffer[which][0], sndsize[which], volume, volume, callback);
                which ^= 1;
            }
        }
    }

    usleep(100);
}

void WiiMovie::LoadNextFrame()
{
    if(!Video || !Playing)
    {
        usleep(100);
        return;
    }

    VideoFrame VideoF;
    LWP_MutexLock(mutex);
    Video->getCurrentFrame(VideoF);
    LWP_MutexUnlock(mutex);

    if(!VideoF.getData())
        return;

    width = VideoF.getWidth();
    height = VideoF.getHeight();

    u8 * videobuffer = ConvertToFlippedRGBA(VideoF.getData(), VideoF.getWidth(), VideoF.getHeight());

    Frames.push_back(videobuffer);
}

void WiiMovie::InternalUpdate()
{
    while(!ExitRequested)
    {
        LoadNextFrame();

        while(Frames.size() > 10 && !ExitRequested)
            usleep(100);
    }
}

void WiiMovie::Draw()
{
    if(!Video || Frames.size() == 0)
        return;

    background->Draw();

    if(Frames.at(3))
    {
        Menu_DrawImg(GetLeft(), GetTop(), 1.0f, width, height, Frames.at(3), 0, GetScaleX(), GetScaleY(), GetAlpha());
    }

    if(Frames.size() > 4)
    {
        if(Frames.at(0))
            free(Frames.at(0));
        Frames.at(0) = NULL;
        Frames.erase(Frames.begin());
    }
}

void WiiMovie::Update(GuiTrigger * t)
{
    exitBtn->Update(t);
}

u8 * WiiMovie::ConvertToFlippedRGBA(const u8 * src, u32 width, u32 height)
{
    u32 block, i, c, ar, gb;

    int len =  ((width+3)>>2)*((height+3)>>2)*32*2;
    if(len%32)
        len += (32-len%32);

    u8 * dst = (u8 *) memalign(32, len);
    if(!dst)
        return NULL;

    for (block = 0; block < height; block += 4)
    {
        for (i = 0; i < width; i += 4)
        {
            /* Alpha and Red */
            for (c = 0; c < 4; ++c)
            {
                for (ar = 0; ar < 4; ++ar)
                {
                    u32 y = height - 1 - (c + block);
                    u32 x = ar + i;
                    u32 offset = ((((y >> 2) * (width >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
                    /* Alpha pixels */
                    dst[offset] = 255;
                    /* Red pixels */
                    dst[offset+1] = src[((i + ar) + ((block + c) * width)) * 3];
                }
            }

            /* Green and Blue */
            for (c = 0; c < 4; ++c)
            {
                for (gb = 0; gb < 4; ++gb)
                {
                    u32 y = height - 1 - (c + block);
                    u32 x = gb + i;
                    u32 offset = ((((y >> 2) * (width >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
                    /* Green pixels */
                    dst[offset+32] = src[(((i + gb) + ((block + c) * width)) * 3) + 1];
                    /* Blue pixels */
                    dst[offset+33] = src[(((i + gb) + ((block + c) * width)) * 3) + 2];
                }
            }
        }
    }

    DCFlushRange(dst, len);

    return dst;
}
