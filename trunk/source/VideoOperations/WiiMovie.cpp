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
#include "Controls/Application.h"
#include "WiiMovie.hpp"
#include "ImageOperations/TextureConverter.h"
#include "SoundOperations/MusicPlayer.h"
#include "VideoOperations/video.h"
#include "menu.h"

#define SND_BUFFERS	 8
#define FRAME_BUFFERS	 8

static BufferCircle * soundbuffer = NULL;

WiiMovie::WiiMovie(const char * filepath)
{
	VideoFrameCount = 0;
	fps = 0.0f;
	ExitRequested = false;
	Playing = false;
	volume = 255*Settings.MusicVolume/100;
	ReadThread = LWP_THREAD_NULL;
	mutex = LWP_MUTEX_NULL;
	ThreadStack = NULL;

	background = new GuiImage(screenwidth, screenheight, (GXColor){0, 0, 0, 255});

	trigB = new GuiTrigger();
	trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);

	exitBtn = new GuiButton(1, 1);
	exitBtn->SetTrigger(trigB);
	exitBtn->Clicked.connect(this, &WiiMovie::OnExitClick);

	MusicPlayer::Instance()->Pause();

	string file(filepath);
	Video = openVideo(file);
	if(!Video)
	{
		ShowError(tr("Unsupported format!"));
		ExitRequested = true;
		return;
	}

	SndChannels = Video->getNumChannels();
	SndFrequence = Video->getFrequency();
	fps = Video->getFps();
	maxSoundSize = Video->getMaxAudioSamples()*Video->getNumChannels()*2;

	if(Video->hasSound())
	{
		soundbuffer = &SoundBuffer;
		SoundBuffer.Resize(SND_BUFFERS);
		SoundBuffer.SetBufferBlockSize(maxSoundSize*FRAME_BUFFERS);
	}

	ThreadStack = (u8 *) memalign(32, 32768);
	if(!ThreadStack)
		return;

	LWP_MutexInit(&mutex, true);
	LWP_CreateThread (&ReadThread, UpdateThread, this, ThreadStack, 32768, LWP_PRIO_HIGHEST);
}

WiiMovie::~WiiMovie()
{
	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	Playing = false;
	ExitRequested = true;

	ASND_StopVoice(0);
	MusicPlayer::Instance()->Resume();

	if(ReadThread != LWP_THREAD_NULL)
	{
		LWP_ResumeThread(ReadThread);
		LWP_JoinThread(ReadThread, NULL);
	}
	if(mutex != LWP_MUTEX_NULL)
	{
		LWP_MutexUnlock(mutex);
		LWP_MutexDestroy(mutex);
	}
	if(ThreadStack)
		free(ThreadStack);

	for(u32 i = 0; i < Frames.size(); i++)
	{
		if(Frames.at(i))
			free(Frames.at(i));

		Frames.at(i) = NULL;
	}

	soundbuffer = NULL;

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
	FrameLoadLoop();

	return true;
}

void WiiMovie::Stop()
{
	ExitRequested = true;
}

void WiiMovie::SetVolume(int vol)
{
	volume = 255*vol/100;
	ASND_ChangeVolumeVoice(0, volume, volume);
}

void WiiMovie::OnExitClick(GuiButton *sender, int pointer UNUSED, const POINT &p UNUSED)
{
	sender->ResetState();
	Stop();
}

void WiiMovie::SetFullscreen()
{
	if(!Video)
		return;

	float newscale = 1000.0f;

	float vidwidth = (float) GetWidth() * 1.0f;
	float vidheight = (float) GetHeight() * 1.0f;
	int retries = 100;

	while(vidheight * newscale > screenheight || vidwidth * newscale > screenwidth)
	{
		if(vidheight * newscale > screenheight)
			newscale = screenheight/vidheight;
		if(vidwidth * newscale > screenwidth)
			newscale = screenwidth/vidwidth;

		retries--;
		if(retries == 0)
		{
			newscale = 1.0f;
			break;
		}
	}

	SetScale(newscale);
}

void WiiMovie::SetFrameSize(int w, int h)
{
	if(!Video)
		return;

	SetScaleX((float) w /(float) GetWidth());
	SetScaleY((float) h /(float) GetHeight());
}

void WiiMovie::SetAspectRatio(float Aspect)
{
	if(!Video)
		return;

	float vidwidth = (float) GetHeight()*GetScaleY()*Aspect;

	SetScaleX((float) GetWidth()/vidwidth);
}

extern "C" void THPSoundCallback(int voice UNUSED)
{
	if(!soundbuffer || !soundbuffer->IsBufferReady())
		return;

	if(ASND_AddVoice(0, soundbuffer->GetBuffer(), soundbuffer->GetBufferSize()) != SND_OK)
	{
		return;
	}

	soundbuffer->LoadNext();
}

void WiiMovie::FrameLoadLoop()
{
	while(!ExitRequested)
	{
		LoadNextFrame();

		while(Frames.size() > FRAME_BUFFERS && !ExitRequested)
			usleep(100);
	}
}

void * WiiMovie::UpdateThread(void *arg)
{
	WiiMovie * Movie = static_cast<WiiMovie *>(arg);

	while(!Movie->ExitRequested)
	{
		Movie->ReadNextFrame();

		usleep(100);
	}

	return NULL;
}

void WiiMovie::ReadNextFrame()
{
	if(!Playing)
		LWP_SuspendThread(ReadThread);

	u32 FramesNeeded = (u32) (PlayTime.elapsed()*fps);

	while(VideoFrameCount < FramesNeeded)
	{
		LWP_MutexLock(mutex);
		Video->loadNextFrame();
		LWP_MutexUnlock(mutex);

		++VideoFrameCount;

		if(Video->hasSound())
		{
			u32 newWhich = SoundBuffer.Which();
			int i = 0;
			for (i = 0; i < SoundBuffer.Size()-2; ++i)
			{
				if(!SoundBuffer.IsBufferReady(newWhich))
					break;

				newWhich = (newWhich+1) % SoundBuffer.Size();
			}

			if(i == SoundBuffer.Size()-2)
				return;

			int currentSize = SoundBuffer.GetBufferSize(newWhich);

			currentSize += Video->getCurrentBuffer((s16 *) (&SoundBuffer.GetBuffer(newWhich)[currentSize]))*SndChannels*2;
			SoundBuffer.SetBufferSize(newWhich, currentSize);

			if(currentSize >= (FRAME_BUFFERS-1)*maxSoundSize)
				SoundBuffer.SetBufferReady(newWhich, true);

			if(ASND_StatusVoice(0) == SND_UNUSED && SoundBuffer.IsBufferReady())
			{
				ASND_StopVoice(0);
				ASND_SetVoice(0, (SndChannels == 2) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, SndFrequence, 0,
							  SoundBuffer.GetBuffer(), SoundBuffer.GetBufferSize(), volume, volume, THPSoundCallback);
				SoundBuffer.LoadNext();
			}
		}
	}
}

void WiiMovie::LoadNextFrame()
{
	if(!Video || !Playing)
		return;

	LWP_MutexLock(mutex);
	Video->getCurrentFrame(VideoF);
	LWP_MutexUnlock(mutex);

	if(!VideoF.getData())
		return;

	if(width != VideoF.getWidth())
	{
		width = VideoF.getWidth();
		height = VideoF.getHeight();
		SetFullscreen();
	}

	Frames.push_back(RGB8ToRGBA8(VideoF.getData(), width, height));
}

void WiiMovie::Draw()
{
	if(!Video)
		return;

	background->Draw();

	if(Frames.size() > FRAME_BUFFERS-2)
	{
		Menu_DrawImg(Frames.at(FRAME_BUFFERS-2), width, height, GX_TF_RGBA8, GetLeft(), GetTop(), 1.0f, 0, GetScaleX(), GetScaleY(), GetAlpha(), minwidth, maxwidth, minheight, maxheight);
	}

	if(Frames.size() > FRAME_BUFFERS-1)
	{
		if(Frames.at(0))
			free(Frames.at(0));
		Frames.erase(Frames.begin());
	}
}

void WiiMovie::Update(GuiTrigger * t)
{
	exitBtn->Update(t);
}
