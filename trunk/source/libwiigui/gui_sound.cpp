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
	type = GetType(snd);
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

    if(allocated)
        free((u8*) sound);
}

bool GuiSound::Load(const u8 * snd, s32 len, bool isallocated)
{
	this->Stop();

    if(allocated)
        free((u8*) sound);

    sound = snd;
	length = len;
	type = GetType(snd);
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
            ASND_StopVoice(0);
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
    if(type == SOUND_MP3)
        return MP3Player_IsPlaying();

    if(type == SOUND_OGG)
    {
        if(StatusOgg() == OGG_STATUS_RUNNING)
            return true;

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

int GuiSound::GetType(const u8 * sound)
{
    //! If no type found than take it as raw.
    int MusicType = SOUND_PCM;

    if(sound[0] == 'O' && sound[1] == 'g' && sound[2] == 'g' && sound[3] == 'S')
    {
        MusicType = SOUND_OGG;
    }
    else if((sound[0] == 0x49 && sound[1] == 0x44 && sound[2] == 0x33) || sound[0] == 0xFF)
    {
        MusicType = SOUND_MP3;
    }

    return MusicType;
}
