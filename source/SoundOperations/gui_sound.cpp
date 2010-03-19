/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "libwiigui/gui.h"
#include <mp3player.h>
#include "SoundOperations/SoundDecoder.h"

#define MAX_SND_VOICES      16

static bool VoiceUsed[MAX_SND_VOICES] =
{
    true, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false, false
};

static int GetFirstUnusedVoice()
{
    for(int i = 1; i < MAX_SND_VOICES; i++)
    {
        if(VoiceUsed[i] == false)
            return i;
    }

    return -1;
}

/**
 * Constructor for the GuiSound class.
 */
GuiSound::GuiSound(const u8 * snd, s32 len, bool isallocated)
{
    sound = NULL;
	length = 0;
	type = 0;
	voice = -1;
	volume = 100;
	loop = false;
	allocated = false;
	format = VOICE_STEREO_16BIT;
	frequency = 48000;
	Load(snd, len, isallocated);
}

/**
 * Destructor for the GuiSound class.
 */
GuiSound::~GuiSound()
{
	this->Stop();

    if(allocated && sound != NULL)
    {
        free(sound);
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

    if(*((u32 *) snd) == 'IMD5')
    {
        const u8 * file = snd+32;
        if(*((u32 *) file) == 'LZ77')
        {
            u32 size = 0;
            sound = uncompressLZ77(file, len-32, &size);
            length = size;
        }
        else
        {
            length = len-32;
            sound = (u8 *) malloc(length);
            memcpy(sound, file, length);
        }

        if(isallocated)
            free((u8 *) snd);

        allocated = true;
    }
    else
    {
        sound = (u8 *) snd;
        length = len;
        allocated = isallocated;
    }

	type = GetType();

	return true;
}

void GuiSound::Play()
{
    if(!sound)
        return;

    Stop();

    voice = 0;

	switch(type)
	{
		case SOUND_PCM:
            voice = GetFirstUnusedVoice();
            if(voice > 0)
            {
                ASND_SetVoice(voice, format, frequency, 0,
                              (u8 *)sound, length, volume, volume, NULL);
                VoiceUsed[voice] = true;
            }
            break;

		case SOUND_OGG:
            PlayOgg(sound, length, 0, loop ? OGG_INFINITE_TIME : OGG_ONE_TIME);
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

    if(voice > 1)
        VoiceUsed[voice] = false;

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

static bool CheckMP3Signature(const u8 * buffer)
{
    const char MP3_Magic[][3] =
    {
        {'I', 'D', '3'},    //'ID3'
        {0xff, 0xfe},       //'MPEG ADTS, layer III, v1.0 [protected]', 'mp3', 'audio/mpeg'),
        {0xff, 0xff},       //'MPEG ADTS, layer III, v1.0', 'mp3', 'audio/mpeg'),
        {0xff, 0xfa},       //'MPEG ADTS, layer III, v1.0 [protected]', 'mp3', 'audio/mpeg'),
        {0xff, 0xfb},       //'MPEG ADTS, layer III, v1.0', 'mp3', 'audio/mpeg'),
        {0xff, 0xf2},       //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
        {0xff, 0xf3},       //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
        {0xff, 0xf4},       //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
        {0xff, 0xf5},       //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
        {0xff, 0xf6},       //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
        {0xff, 0xf7},       //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
        {0xff, 0xe2},       //'MPEG ADTS, layer III, v2.5 [protected]', 'mp3', 'audio/mpeg'),
        {0xff, 0xe3},       //'MPEG ADTS, layer III, v2.5', 'mp3', 'audio/mpeg'),
    };

    if(buffer[0] == MP3_Magic[0][0] && buffer[1] == MP3_Magic[0][1] &&
       buffer[2] == MP3_Magic[0][2])
    {
        return true;
    }

    for(int i = 1; i < 13; i++)
    {
        if(buffer[0] == MP3_Magic[i][0] && buffer[1] == MP3_Magic[i][1])
            return true;
    }

    return false;
}

int GuiSound::GetType()
{
    int MusicType = -1;

    const u8 * check = sound;
    int cnt = 0;

    //!Skip 0 ... whysoever some files have
    while(check[0] == 0 && cnt < length)
    {
        check++;
        cnt++;
    }

    if(cnt >= length)
        return MusicType;

    if(*((u32*) check) == 'OggS')
    {
        MusicType = SOUND_OGG;
    }
    else if(*((u32*) check) == 'RIFF')
    {
        SoundBlock Block = DecodefromWAV(sound, length);

        if(allocated)
            free(sound);

        sound = NULL;
        length = 0;

        if(!Block.buffer)
            return SOUND_PCM;

        length = Block.size;
        sound = Block.buffer;
        format = Block.format;
        frequency = Block.frequency;

        allocated = true;
        MusicType = SOUND_PCM;
    }
    else if(*((u32*) check) == 'BNS ')
    {
        SoundBlock Block = DecodefromBNS(sound, length);

        if(allocated)
            free(sound);

        sound = NULL;
        length = 0;

        if(!Block.buffer)
            return SOUND_PCM;

        length = Block.size;
        sound = Block.buffer;
        format = Block.format;
        frequency = Block.frequency;
        MusicType = SOUND_PCM;

        allocated = true;
    }
    else if(*((u32*) check) == 'FORM')
    {
        SoundBlock Block = DecodefromAIFF(sound, length);

        if(allocated)
            free(sound);

        sound = NULL;
        length = 0;

        if(!Block.buffer)
            return SOUND_PCM;

        length = Block.size;
        sound = Block.buffer;
        format = Block.format;
        frequency = Block.frequency;
        MusicType = SOUND_PCM;

        allocated = true;
    }
    else if(CheckMP3Signature(check) == true)
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
