/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "libmad/mp3player.h"

static mutex_t mp3mutex = LWP_MUTEX_NULL;

/**
 * Constructor for the GuiSound class.
 */
GuiSound::GuiSound(const u8 * snd, s32 len, int t)
{
    if(t == SOUND_MP3) {
        sound = NULL;
        sound = new unsigned char[len];
        memcpy((u8*) sound, snd, len);
        LWP_MutexInit(&mp3mutex, true);
    } else {
        sound = snd;
    }
	length = len;
	type = t;
	voice = -1;
	volume = 100;
	loop = false;
}

GuiSound::GuiSound(const u8 * snd, s32 len, int t, int v)
{
	sound = snd;
	length = len;
	type = t;
	voice = -1;
	volume = v;
	loop = false;
}

/**
 * Destructor for the GuiSound class.
 */
GuiSound::~GuiSound()
{
	if(type == SOUND_OGG)
		StopOgg();
    else if(type == SOUND_MP3) {
        LWP_MutexUnlock(mp3mutex);
        MP3Player_Stop();
        LWP_MutexDestroy(mp3mutex);
        mp3mutex = LWP_MUTEX_NULL;
        if(sound) {
            delete [] sound;
            sound = NULL;
        }
    }
}

void GuiSound::Play()
{
	int vol;

	switch(type)
	{
		case SOUND_PCM:
		vol = 255*(volume/100.0);
		voice = ASND_GetFirstUnusedVoice();
		if(voice >= 0)
			ASND_SetVoice(voice, VOICE_STEREO_16BIT, 48000, 0,
				(u8 *)sound, length, vol, vol, NULL);
		break;

		case SOUND_OGG:
		voice = 0;
		if(loop)
			PlayOgg(mem_open((char *)sound, length), 0, OGG_INFINITE_TIME);
		else
			PlayOgg(mem_open((char *)sound, length), 0, OGG_ONE_TIME);
		SetVolumeOgg(255*(volume/100.0));
		break;

		case SOUND_MP3:
		voice = 0;
		MP3Player_PlayBuffer(sound, length, NULL);
		MP3Player_Volume(255*(volume/100.0));
		LWP_MutexLock(mp3mutex);
		break;
	}
}

void GuiSound::Stop()
{
	if(voice < 0)
		return;

	switch(type)
	{
		case SOUND_PCM:
		ASND_StopVoice(voice);
		break;

		case SOUND_OGG:
		StopOgg();
		break;

		case SOUND_MP3:
		LWP_MutexUnlock(mp3mutex);
		MP3Player_Stop();
		break;
	}
}

void GuiSound::Pause()
{
	if(voice < 0)
		return;

    if(type == SOUND_MP3)
        return;             //coming soon

	switch(type)
	{
		case SOUND_PCM:
		ASND_PauseVoice(voice, 1);
		break;

		case SOUND_OGG:
		PauseOgg(1);
		break;
	}
}

void GuiSound::Resume()
{
	if(voice < 0)
		return;

    if(type == SOUND_MP3)
        return;             //coming soon

	switch(type)
	{
		case SOUND_PCM:
		ASND_PauseVoice(voice, 0);
		break;

		case SOUND_OGG:
		PauseOgg(0);
		break;
	}
}

bool GuiSound::IsPlaying()
{
    if(type == SOUND_MP3) {
		LWP_MutexUnlock(mp3mutex);
        bool playing = MP3Player_IsPlaying();
		LWP_MutexLock(mp3mutex);
        return playing;
    }

	if(ASND_StatusVoice(voice) == SND_WORKING || ASND_StatusVoice(voice) == SND_WAITING)
		return true;
	else
		return false;
}

void GuiSound::SetVolume(int vol)
{
	volume = vol;

	if(voice < 0)
		return;

	int newvol = 255*(volume/100.0);

	switch(type)
	{
		case SOUND_PCM:
		ASND_ChangeVolumeVoice(voice, newvol, newvol);
		break;

		case SOUND_OGG:
		SetVolumeOgg(newvol);
		break;

		case SOUND_MP3:
		LWP_MutexUnlock(mp3mutex);
		MP3Player_Volume(newvol);
		LWP_MutexLock(mp3mutex);
		break;
	}
}

void GuiSound::SetLoop(bool l)
{
    if(type == SOUND_MP3)
        LWP_MutexLock(mp3mutex);

	loop = l;
}
s32 GuiSound::GetPlayTime()
{
	return GetTimeOgg();
}

void GuiSound::SetPlayTime(s32 time_pos)
{
	SetTimeOgg(time_pos);
}
