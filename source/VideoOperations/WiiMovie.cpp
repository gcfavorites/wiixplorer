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
#include "libwiigui/gui_bgm.h"
#include "menu.h"

WiiMovie::WiiMovie(const char * filepath)
{
    VideoFrameCount = 0;
    ExitRequested = false;
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

    FrameCountGlobal = frameCount;
}

WiiMovie::~WiiMovie()
{
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    ExitRequested = true;

    ASND_StopVoice(0);
    GuiBGM::Instance()->Play();

    for(u32 i = 0; i < Frames.size(); i++)
    {
        if(Frames.at(i).videobuffer)
            free(Frames.at(i).videobuffer);

        Frames.at(i).videobuffer = NULL;
        Frames.at(i).soundbuffer.clear();

    }

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

    FrameCountGlobal = frameCount;
    Playing = true;

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

void WiiMovie::LoadNextFrame()
{
    if(!Video || !Playing)
    {
        usleep(100);
        return;
    }

    u32 FramesNeeded = VideoFrameCount+frameCount-FrameCountGlobal;
    FrameCountGlobal = frameCount;

    do
    {
        Video->loadNextFrame();
        ++VideoFrameCount;
    }
    while(VideoFrameCount < FramesNeeded);

    VideoFrame VideoF;
    Video->getCurrentFrame(VideoF);

    width = VideoF.getWidth();
    height = VideoF.getHeight();

    VFrame TmpFrame;

    TmpFrame.videobuffer = ConvertToFlippedRGBA(VideoF.getData(), VideoF.getWidth(), VideoF.getHeight());

    if(Video->hasSound())
    {
        TmpFrame.soundbuffer.resize(Video->getMaxAudioSamples()*2);
        TmpFrame.soundlength = Video->getCurrentBuffer(&TmpFrame.soundbuffer[0])*2*2;
    }
    Frames.push_back(TmpFrame);
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

    if(Frames.at(3).videobuffer)
    {
        Menu_DrawImg(GetLeft(), GetTop(), 1.0f, width, height, Frames.at(3).videobuffer, 0, GetScaleX(), GetScaleY(), GetAlpha());
    }

    if(Video->hasSound())
    {
        if(ASND_StatusVoice(0) == SND_UNUSED)
        {
            ASND_StopVoice(0);
            ASND_SetVoice(0, SndChannels, SndFrequence, 0, (u8 *) &Frames.at(3).soundbuffer[0], Frames.at(3).soundlength, volume, volume, NULL);
        }
        else
        {
            ASND_AddVoice(0, (u8 *) &Frames.at(3).soundbuffer[0], Frames.at(3).soundlength);
        }
    }

    if(Frames.size() > 5)
    {
        if(Frames.at(0).videobuffer)
            free(Frames.at(0).videobuffer);
        Frames.at(0).soundbuffer.clear();
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
