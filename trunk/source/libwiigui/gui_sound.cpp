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

/**
 * Constructor for the GuiSound class.
 */
GuiSound::GuiSound(const u8 * snd, s32 len, bool isallocated)
{
    sound = snd;
	length = len;
	type = GetType(snd, len);
	voice = -1;
	volume = 100;
	loop = false;
	allocated = isallocated;
}

/**
 * Destructor for the GuiSound class.
 */
GuiSound::~GuiSound()
{
	this->Stop();

    if(allocated && sound != NULL)
    {
        free((u8*) sound);
        sound = NULL;
    }
}

bool GuiSound::Load(const u8 * snd, s32 len, bool isallocated)
{
	this->Stop();

    if(allocated && sound != NULL)
    {
        free((u8*) sound);
        sound = NULL;
    }

    sound = snd;
	length = len;
	type = GetType(snd, len);
	allocated = isallocated;

	return true;
}

void GuiSound::Play()
{
    Stop();

    voice = 0;

	switch(type)
	{
		case SOUND_PCM:
            voice = ASND_GetFirstUnusedVoice();
            if(voice > 0)
                ASND_SetVoice(voice, VOICE_STEREO_16BIT, 48000, 0,
                              (u8 *)sound, length, volume, volume, NULL);
            break;

		case SOUND_OGG:
            PlayOgg(mem_open((char *)sound, length), 0, loop ? OGG_INFINITE_TIME : OGG_ONE_TIME);
            break;

		case SOUND_MP3:
            MP3Player_PlayBuffer(sound, length, NULL);
            break;
	}

	SetVolume(volume);
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
    if(type == SOUND_MP3 || type == SOUND_OGG)
    {
        if(ASND_StatusVoice(0) == SND_WORKING || ASND_StatusVoice(0) == SND_WAITING)
            return true;
        else
            return false;
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
            MP3Player_Volume(newvol);
            break;
	}
}

void GuiSound::SetLoop(u8 l)
{
	loop = l;
}

int GuiSound::GetType(const u8 * sound, int len)
{
    int MusicType = -1;

    const u8 * check = sound;
    int cnt = 0;

    //!Skip 0 ... whysoever some files have
    while(check[0] == 0 && cnt < len)
    {
        check++;
        cnt++;
    }

    if(cnt >= len)
        return MusicType;

    if(check[0] == 'O' && check[1] == 'g' && check[2] == 'g' && check[3] == 'S')
    {
        MusicType = SOUND_OGG;
    }
    else if((check[0] == 0x49 && check[1] == 0x44 && check[2] == 0x33) ||
            (check[0] == 0xFF && (check[1] == 0xFA || check[1] == 0xFB)))
    {
        MusicType = SOUND_MP3;
    }
    else
    {
        //! If no type found than take it as raw.
        MusicType = SOUND_PCM;
    }

    return MusicType;
}
