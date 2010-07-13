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
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "Controls/Taskbar.h"
#include "FileOperations/fileops.h"
#include "Language/gettext.h"
#include "Tools/tools.h"
#include "menu.h"
#include "IOHandler.hpp"

IOHandler * IOHandler::instance = NULL;

extern bool sizegainrunning;

IOHandler::IOHandler()
{
    TaskbarSlot = NULL;
    IOThread = LWP_THREAD_NULL;
    RequestThread = LWP_THREAD_NULL;
    DestroyRequested = false;
    ProcessLocked = false;
    Running = false;
    TotalFileCount = 0;
    TotalSize = 0;
    MinimizeCallback.SetCallback(this, &IOHandler::SetMaximized);

    ThreadStack = (u8 *) memalign(32, 32768);
    if(!ThreadStack)
        return;

	LWP_CreateThread(&IOThread, ThreadCallback, this, ThreadStack, 32768, Settings.CopyThreadPrio);
}

IOHandler::~IOHandler()
{
    DestroyRequested = true;
    Running = false;
    actioncanceled = true;

    if(IOThread != LWP_THREAD_NULL)
    {
        LWP_ResumeThread(IOThread);
        LWP_JoinThread(IOThread, NULL);
    }
	if(ThreadStack)
        free(ThreadStack);

    while(!ProcessQueue.empty())
    {
        delete ProcessQueue.front();
        ProcessQueue.pop();
    }
}

IOHandler * IOHandler::Instance()
{
	if (instance == NULL)
	{
		instance = new IOHandler();
	}
	return instance;
}

void IOHandler::DestroyInstance()
{
    if(instance)
    {
        delete instance;
    }
    instance = NULL;
}

void IOHandler::StartProcess(bool lock)
{
    Running = true;
    ProcessLocked = lock;

    LWP_ResumeThread(IOThread);
    RequestThread = LWP_GetSelf();

    if(!ProcessQueue.back()->Cutted)
        CalcTotalSize();

    if(ProcessLocked)
    {
        LWP_SetThreadPriority(IOThread, Settings.CopyThreadPrio);
        LWP_SuspendThread(RequestThread);
    }
}

void IOHandler::AddProcess(ItemMarker * List, const char * dest, bool Cutted)
{
    ClipboardItem * TmpItem = new ClipboardItem;

    for(int i = 0; i < List->GetItemcount(); i++)
    {
        TmpItem->ItemList.AddItem(List->GetItem(i));
    }

    TmpItem->DestPath = dest;
    TmpItem->Cutted = Cutted;

    ProcessQueue.push(TmpItem);

    if(Cutted)
    {
        if(!Running)
            StartProgress(tr("Calculating total size..."));
        CalcTotalSize();
    }

    IOHandler::Instance()->StartProcess(!Running);
}

void IOHandler::ProcessNext()
{
    if(DestroyRequested)
        return;

    ItemMarker * CurrentProcess = (ItemMarker *) &ProcessQueue.front()->ItemList;

    const char * destpath = ProcessQueue.front()->DestPath.c_str();

    char srcpath[MAXPATHLEN];
    char destdir[MAXPATHLEN];
    int res = 0;
    ProgressWindow::Instance()->SetMinimizable(true);
    ProgressWindow::Instance()->SetMinimized(!ProcessLocked);

    bool Cutted = ProcessQueue.front()->Cutted;
    if(Cutted)
    {
        if(CurrentProcess->GetItemcount() > 0)
        {
            snprintf(srcpath, sizeof(srcpath), "%s/", CurrentProcess->GetItemPath(0));
            snprintf(destdir, sizeof(destdir), "%s/%s/", destpath, CurrentProcess->GetItemName(0));
            if(CompareDevices(srcpath, destdir))
                StartProgress(tr("Moving item(s):"), THROBBER);
            else
                StartProgress(tr("Moving item(s):"));
        }
    }
    else
    {
        StartProgress(tr("Copying item(s):"));
    }
    for(int i = 0; i < CurrentProcess->GetItemcount(); i++)
    {
        if(CurrentProcess->IsItemDir(i) == true)
        {
            snprintf(srcpath, sizeof(srcpath), "%s/", CurrentProcess->GetItemPath(i));
            snprintf(destdir, sizeof(destdir), "%s/%s/", destpath, CurrentProcess->GetItemName(i));
            if(Cutted == false)
            {
                res = CopyDirectory(srcpath, destdir);
            }
            else
            {
                if(strcmp(srcpath, destdir) != 0)
                {
                    res = MoveDirectory(srcpath, destdir);
                }
                else
                {
                    StopProgress();
                    ShowError(tr("You can not cut into the directory itself."));
                    res =  -1;
                }
            }

            if(res == -10)
            {
                StopProgress();
                if(!DestroyRequested)
                    WindowPrompt(tr("Transfering files:"), tr("Action cancelled."), tr("OK"));
                break;
            }
        }
        else
        {
            snprintf(srcpath, sizeof(srcpath), "%s", CurrentProcess->GetItemPath(i));
            snprintf(destdir, sizeof(destdir), "%s/%s", destpath, CurrentProcess->GetItemName(i));
            if(Cutted == false)
            {
                if(strcmp(srcpath, destdir) == 0)
                {
                    snprintf(destdir, sizeof(destdir), "%s/%s %s", destpath, tr("Copy of"), CurrentProcess->GetItemName(i));
                }
                res = CopyFile(srcpath, destdir);
            }
            else
            {
                if(strcmp(srcpath, destdir) == 0)
                {
                    res = 1;
                    continue;
                }
                res = MoveFile(srcpath, destdir);
            }
        }
    }
    if(res < 0 && res != 10 && !DestroyRequested)
    {
        StopProgress();
        if(Cutted)
            ShowError(tr("Failed moving item(s)."));
        else
            ShowError(tr("Failed copying item(s)."));
    }

    CurrentProcess->Reset();
    delete ProcessQueue.front();
    ProcessQueue.pop();
}

void IOHandler::SetMinimized(int mode)
{
    ProgressText.assign(ProgressWindow::Instance()->GetTitle());
    TaskbarSlot = new Task(ProgressText.c_str());
    TaskbarSlot->SetCallback(&MinimizeCallback);
    TaskbarSlot->SetParameter(mode);
    Taskbar::Instance()->AddTask(TaskbarSlot);

    LWP_SetThreadPriority(IOThread, Settings.CopyThreadBackPrio);

    LWP_ResumeThread(RequestThread);
    ProcessLocked = false;
}

void IOHandler::SetMaximized(int Param)
{
    Taskbar::Instance()->RemoveTask(TaskbarSlot);
    TaskbarSlot = NULL;

    ProgressWindow::Instance()->SetMinimized(false);
    StartProgress(ProgressText.c_str(), Param);

    ProcessLocked = true;
    LWP_SetThreadPriority(IOThread, Settings.CopyThreadPrio);
    LWP_SuspendThread(RequestThread);
}

void IOHandler::CalcTotalSize()
{
    ItemMarker * Process = (ItemMarker *) &ProcessQueue.back()->ItemList;

    char filepath[1024];
    sizegainrunning = true;

    for(int i = 0; i < Process->GetItemcount(); i++)
    {
        if(Process->IsItemDir(i) == true)
        {
            snprintf(filepath, sizeof(filepath), "%s/", Process->GetItemPath(i));
            GetFolderSize(filepath, TotalSize, TotalFileCount);
        }
        else
        {
            TotalSize += FileSize(Process->GetItemPath(i));
            ++TotalFileCount;
        }
    }

    sizegainrunning = false;

    ProgressWindow::Instance()->SetTotalValues(TotalSize, TotalFileCount);
}

void * IOHandler::ThreadCallback(void *arg)
{
	((IOHandler *) arg)->InternalThreadHandle();
	return NULL;
}

void IOHandler::InternalThreadHandle()
{
    while(!DestroyRequested)
    {
        if(!Running)
            LWP_SuspendThread(IOThread);

        while(!ProcessQueue.empty() && Running)
            ProcessNext();

        if(TaskbarSlot)
        {
            Taskbar::Instance()->RemoveTask(TaskbarSlot);
            TaskbarSlot = NULL;
        }

        StopProgress();
        ProgressWindow::Instance()->SetMinimized(false);
        ProgressWindow::Instance()->SetMinimizable(false);
        ResetReplaceChoice();

        LWP_ResumeThread(RequestThread);
        TotalFileCount = 0;
        TotalSize = 0;
        ProcessLocked = false;
        Running = false;
    }
}
