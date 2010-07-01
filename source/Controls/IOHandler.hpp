/****************************************************************************
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
 * WiiXplorer 2010
 ***************************************************************************/
#ifndef IOHANDLER_HPP_
#define IOHANDLER_HPP_

#include <queue>
#include <string>
#include "FileOperations/ItemMarker.h"
#include "Controls/Callback.hpp"
#include "Controls/Task.hpp"

typedef struct
{
    ItemMarker ItemList;
    std::string DestPath;
    bool Cutted;
} ClipboardItem;

class IOHandler
{
    public:
		static IOHandler * Instance();
		static void DestroyInstance();

        void StartProcess(bool lock = true);
        void AddProcess(ItemMarker * List, const char * dest, bool Cutted = false);
        void SetMinimized(int mode);
        void SetMaximized(int mode);
        bool IsRunning() { return Running; };
    protected:
		IOHandler();
		~IOHandler();
		static void * ThreadCallback(void *arg);
		void InternalThreadHandle();
        void ProcessNext();
        void CalcTotalSize();
		static IOHandler * instance;

        std::string ProgressText;
        std::queue<ClipboardItem *> ProcessQueue;
        u32 TotalFileCount;
        u64 TotalSize;
        bool Running;
		bool DestroyRequested;
		bool ProcessLocked;
		u8 * ThreadStack;
		lwp_t IOThread;
		lwp_t RequestThread;
		Task * TaskbarSlot;
		TCallback<IOHandler> MinimizeCallback;
};

#endif
