/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * 3Band resampling thanks to libmad
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
#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "SoundDecoder.hpp"
#include "main.h"

static const f32 VSA = (1.0F/4294967295.0F);

static inline s16 Do3Band(EQState *es, s16 sample)
{
	f32 l,m,h;

	es->f1p0 += (es->lf*((f32)sample - es->f1p0))+VSA;
	es->f1p1 += (es->lf*(es->f1p0 - es->f1p1));
	es->f1p2 += (es->lf*(es->f1p1 - es->f1p2));
	es->f1p3 += (es->lf*(es->f1p2 - es->f1p3));
	l = es->f1p3;

	es->f2p0 += (es->hf*((f32)sample - es->f2p0))+VSA;
	es->f2p1 += (es->hf*(es->f2p0 - es->f2p1));
	es->f2p2 += (es->hf*(es->f2p1 - es->f2p2));
	es->f2p3 += (es->hf*(es->f2p2 - es->f2p3));
	h = es->sdm3 - es->f2p3;

	m = es->sdm3 - (h+l);

	l *= es->lg;
	m *= es->mg;
	h *= es->hg;

	es->sdm3 = es->sdm2;
	es->sdm2 = es->sdm1;
	es->sdm1 = (f32)sample;

	return (s16)(l+m+h);
}

SoundDecoder::SoundDecoder()
{
    file_fd = NULL;
    Init();
}

SoundDecoder::SoundDecoder(const char * filepath)
{
    file_fd = new File(filepath, "rb");
    Init();
}

SoundDecoder::SoundDecoder(const u8 * buffer, int size)
{
    file_fd = new File(buffer, size);
    Init();
}

SoundDecoder::~SoundDecoder()
{
    ExitRequested = true;
    while(Decoding)
        usleep(100);

    if(file_fd)
        delete file_fd;
    file_fd = NULL;

    if(RawBuffer)
        free(RawBuffer);
}

void SoundDecoder::Init()
{
    SoundType = SOUND_RAW;
    SoundBlocks = Settings.SoundblockCount;
    SoundBlockSize = Settings.SoundblockSize;
    CurPos = 0;
    Loop = false;
    EndOfFile = false;
    Decoding = false;
    ExitRequested = false;
    SoundBuffer.SetBufferBlockSize(SoundBlockSize);
    SoundBuffer.Resize(SoundBlocks);
    RawBuffer = (u8 *) memalign(32, SoundBlocks*SoundBlockSize);
	Init3BandState(&eqs[0],880,5000,48000);
	Init3BandState(&eqs[1],880,5000,48000);
    RawSamples = 0;
    RawSamplePos.adword = 0;
}

int SoundDecoder::Rewind()
{
    CurPos = 0;
    EndOfFile = false;
    file_fd->rewind();

    return 0;
}

int SoundDecoder::Read(u8 * buffer, int buffer_size, int pos)
{
    int ret = file_fd->read(buffer, buffer_size);
    CurPos += ret;

    return ret;
}

void SoundDecoder::Decode()
{
    if(!file_fd || ExitRequested || EndOfFile)
        return;

    u16 newWhich = SoundBuffer.Which();
    u16 i = 0;
    for (i = 0; i < SoundBuffer.Size()-1; i++)
    {
        if(!SoundBuffer.IsBufferReady(newWhich))
            break;

        newWhich = (newWhich+1) % SoundBuffer.Size();
    }

    if(i == SoundBuffer.Size()-1)
        return;

    Decoding = true;

    int read_size = 0;
    u8 * write_buf = SoundBuffer.GetBuffer(newWhich);
    if(!write_buf)
    {
        ExitRequested = true;
        Decoding = false;
        return;
    }

    if(GetSampleRate() != 48000)
    {
        read_size = Resample16Bit(write_buf, SoundBlockSize, GetSampleRate(), true);
    }
    else
    {
        read_size = CopyRaw16BitData(write_buf, SoundBlockSize, true);
    }

    if(read_size > 0)
    {
        SoundBuffer.SetBufferSize(newWhich, read_size);
        SoundBuffer.SetBufferReady(newWhich, true);
    }

	if(!SoundBuffer.IsBufferReady((newWhich+1) % SoundBuffer.Size()))
        Decode();

    Decoding = false;
}

void SoundDecoder::ReadDecodedData()
{
    u32 readsize = SoundBlocks*SoundBlockSize;
    u32 done = 0;
    RawSamples = 0;
    RawSamplePos.adword = 0;

    if(!Is16Bit())
        readsize /= 2;
    if(!IsStereo())
        readsize /= 2;

	while(done < readsize)
	{
        int ret = Read(&RawBuffer[done], readsize-done, Tell());

        if(ret <= 0)
            break;

        done += ret;
	}

    if(!Is16Bit())
    {
        u8 * BuffCpy = (u8 *) malloc(done);
        if(!BuffCpy)
            return;

        memcpy(BuffCpy, RawBuffer, done);
        done = Convert8BitTo16Bit(BuffCpy, RawBuffer, done, SoundBlocks*SoundBlockSize);
        free(BuffCpy);
    }
    if(!IsStereo())
    {
        u8 * BuffCpy = (u8 *) malloc(done);
        if(!BuffCpy)
            return;

        memcpy(BuffCpy, RawBuffer, done);
        done = ConvertMonoToStereo(BuffCpy, RawBuffer, done, SoundBlocks*SoundBlockSize);
        free(BuffCpy);
    }

	RawSamples = done/4;
}

int SoundDecoder::CopyRaw16BitData(u8 * write_buf, u32 bufsize, bool stereo)
{
    u32 done = 0;
    u32 RawSize = RawSamples*4;
    s16 * src = (s16 *) (RawBuffer+RawSamplePos.adword);
    s16 * dst = (s16 *) write_buf;

    if(stereo)
        bufsize &= ~0x0003;
    else
        bufsize &= ~0x0001;

	while(done < bufsize)
	{
        if(RawSamplePos.adword >= RawSize)
        {
            ReadDecodedData();

            if(RawSamples <= 0)
            {
                if(Loop)
                {
                    Rewind();
                    continue;
                }
                else
                {
                    EndOfFile = true;
                    break;
                }
            }

            src = (s16 *) RawBuffer;
            RawSize = RawSamples*4;
            RawSamplePos.adword = 0;
        }

        *dst++ = *src++;
        RawSamplePos.adword += 2;
        done += 2;
	}

	return done;
}

int SoundDecoder::Resample16Bit(u8 * write_buf, u32 bufsize, u32 SmplRate, bool stereo)
{
    u32 incr = (u32)(((f32)SmplRate/48000.0F)*65536.0F);
    s16 * dst = (s16 *) write_buf;
    s16 * src = (s16 *) RawBuffer;
    u32 done = 0;

    if(stereo)
        bufsize &= ~0x0003;
    else
        bufsize &= ~0x0001;

	while(done < bufsize)
	{
	    if(RawSamplePos.aword.hi >= RawSamples)
	    {
            ReadDecodedData();
            if(RawSamples <= 0)
            {
                if(Loop)
                {
                    Rewind();
                    continue;
                }
                else
                {
                    EndOfFile = true;
                    break;
                }
            }
	    }

        if(stereo)
        {
            *dst++ = Do3Band(&eqs[0], src[RawSamplePos.aword.hi*2]);
            *dst++ = Do3Band(&eqs[1], src[RawSamplePos.aword.hi*2+1]);
            done += 4;
        }
        else
        {
            *dst++ = Do3Band(&eqs[0], src[RawSamplePos.aword.hi]);
            done += 2;
        }
        RawSamplePos.adword += incr;
	}

	return done;
}
