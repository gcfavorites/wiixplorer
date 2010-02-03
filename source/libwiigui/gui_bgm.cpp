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
#include "gui_bgm.h"
#include "FileOperations/fileops.h"
#include "Language/gettext.h"
#include "main.h"

GuiBGM *GuiBGM::instance = NULL;

GuiBGM::GuiBGM()
    :GuiSound(bg_music_ogg, bg_music_ogg_size)
{
    loop = 0;
    currentPath = NULL;
    currentPlaying = 0;
    ExitRequested = false;
	LWP_CreateThread (&bgmthread, UpdateBMG, this, NULL, 0, 0);
}

GuiBGM::~GuiBGM()
{
    ExitRequested = true;

    if(currentPath)
        delete [] currentPath;

    ClearList();

    LWP_JoinThread(bgmthread, NULL);
    bgmthread = LWP_THREAD_NULL;
};


GuiBGM * GuiBGM::Instance()
{
	if (instance == NULL)
	{
		instance = new GuiBGM();
	}
	return instance;
}

void GuiBGM::DestroyInstance()
{
    if(instance)
    {
        delete instance;
    }
    instance = NULL;
}

bool GuiBGM::LoadStandard()
{
    ClearList();
    if(currentPath)
    {
        delete [] currentPath;
        currentPath = NULL;
    }

    strcpy(Settings.MusicPath, "");

    GuiSound::Load(bg_music_ogg, bg_music_ogg_size, false);
    Play();

    return true;
}

bool GuiBGM::Load(const char * path, bool silent)
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

    if(!GuiSound::Load(file, (u32) filesize, true))
    {
        free(file);
        LoadStandard();
        return false;
    }

    return true;
}

bool GuiBGM::ParsePath(const char * filepath)
{
    ClearList();

    if(currentPath)
        delete [] currentPath;

    currentPath = new char[strlen(filepath)+1];

    if(!currentPath)
        return false;

    sprintf(currentPath, "%s", filepath);

    char * pathptr = strrchr(currentPath, '/');
    if(pathptr)
    {
        pathptr++;
        pathptr[0] = 0;
    }
    else
        return false;

    char * LoadedFilename = strrchr(filepath, '/')+1;

    char filename[1024];
    struct stat st;

    DIR_ITER * dir = diropen(currentPath);
    if (dir == NULL)
    {
        return false;
    }

    u32 counter = 0;

    while (dirnext(dir,filename,&st) == 0)
    {
        char * fileext = strrchr(filename, '.');
        if(fileext)
        {
            if(strcasecmp(fileext, ".mp3") == 0 || strcasecmp(fileext, ".ogg") == 0
               || strcasecmp(fileext, ".pcm") == 0)
            {
                AddEntrie(filename);

                if(strcmp(LoadedFilename, filename) == 0)
                    currentPlaying = counter;

                counter++;
            }
        }
    }

    dirclose(dir);

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s", filepath);

    return true;
}

void GuiBGM::AddEntrie(const char * filename)
{
    if(!filename)
        return;

    char * NewEntrie = new char[strlen(filename)+1];

    if(!NewEntrie)
        return;

    sprintf(NewEntrie, "%s", filename);

    PlayList.push_back(NewEntrie);
}

void GuiBGM::ClearList()
{
    for(u32 i = 0; i < PlayList.size(); i++)
    {
        if(PlayList.at(i) != NULL)
        {
            delete [] PlayList.at(i);
            PlayList.at(i) = NULL;
        }
    }

    PlayList.clear();
}

bool GuiBGM::PlayNext()
{
    if(!currentPath)
        return false;

    currentPlaying++;
    if(currentPlaying >= (int) PlayList.size())
        currentPlaying = 0;

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s%s", currentPath, PlayList.at(currentPlaying));

    if(!Load(Settings.MusicPath, true))
        return false;

    Play();

    return true;
}

bool GuiBGM::PlayPrevious()
{
    if(!currentPath)
        return false;

    currentPlaying--;
    if(currentPlaying < 0)
        currentPlaying = PlayList.size()-1;

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s%s", currentPath, PlayList.at(currentPlaying));

    if(!Load(Settings.MusicPath, true))
        return false;

    Play();

    return true;
}

bool GuiBGM::PlayRandom()
{
    if(!currentPath)
        return false;

    srand(time(NULL));

    currentPlaying = rand() % PlayList.size();

    //just in case
    if(currentPlaying < 0)
        currentPlaying = PlayList.size()-1;
    else if(currentPlaying >= (int) PlayList.size())
        currentPlaying = 0;

    snprintf(Settings.MusicPath, sizeof(Settings.MusicPath), "%s%s", currentPath, PlayList.at(currentPlaying));

    if(!Load(Settings.MusicPath, true))
        return false;

    Play();

    return true;
}

void * GuiBGM::UpdateBMG(void * arg)
{
	((GuiBGM *) arg)->UpdateState();
	return NULL;
}

void GuiBGM::UpdateState()
{
    while(!ExitRequested)
    {
        if(!IsPlaying())
        {
            if(loop > 0 && strcmp(Settings.MusicPath, "") == 0)
            {
                //!Standard Music is always looped except on loop = 0
                Play();
            }
            else if(loop == DIR_LOOP)
            {
                PlayNext();
            }
            else if(loop == RANDOM_BGM)
            {
                PlayRandom();
            }
        }

        usleep(200000);
    }
}
