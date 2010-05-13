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
#ifndef _BGM_H_
#define _BGM_H_

#include <vector>
#include "gui_sound.h"

enum
{
    ONCE = 0,
    LOOP,
    RANDOM_BGM,
    DIR_LOOP,
    MAX_LOOP_MODES
};

class GuiBGM : public GuiSound
{
    public:
		static GuiBGM * Instance();
		static void DestroyInstance();

        bool Load(const char *path, bool silent = true);
        bool LoadStandard();
        bool ParsePath(const char * filepath);
        void Play();
        void Stop();
        bool PlayNext();
        bool PlayPrevious();
        bool PlayRandom();
        void ResumeThread();
        void HaltThread();
    protected:
        GuiBGM();
        ~GuiBGM();
        void AddEntrie(const char * filename);
        void ClearList();

		static void * UpdateBMG(void *arg);
        void UpdateState();
		lwp_t bgmthread;
		bool Stopped;
		bool ExitRequested;

		static GuiBGM *instance;
        int currentPlaying;
        char * currentPath;
        std::vector<char *> PlayList;
};

#endif
