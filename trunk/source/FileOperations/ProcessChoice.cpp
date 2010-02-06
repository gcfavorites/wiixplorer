/***************************************************************************
 * Copyright (C) 2009
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
 * ProcessChoice.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include "ProcessChoice.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PromptWindows.h"
#include "FileStartUp/FileStartUp.h"
#include "FileOperations/fileops.h"

/*** Extern functions ***/
extern CLIPBOARD Clipboard;

void ProcessArcChoice(ArchiveBrowser * browser, int choice, const char * destCandidat)
{
    if(!browser)
        return;

    if(choice == ArcOpen)
    {
        WindowPrompt(tr("Opening is not yet implemented"), tr("Extract first and open than."), tr("OK"));
    }
    else if(choice == ArcExtractFile)
    {
        bool directory = browser->IsCurrentDir();

        int ret = WindowPrompt((directory ? tr("Extract this folder?") : tr("Extract this file?")), browser->GetCurrentDisplayname(), tr("Yes"), tr("Cancel"));
        if(ret <= 0)
            return;

        char dest[MAXPATHLEN];
        snprintf(dest, sizeof(dest), "%s", destCandidat);
        int result = OnScreenKeyboard(dest, sizeof(dest));
        if(result)
        {

            if(!directory)
                StartProgress(tr("Extracting file:"));
            else
                StartProgress(tr("Extracting folder:"));
            result = browser->ExtractCurrentItem(dest);
            StopProgress();
            if(result <= 0)
            {
                ShowError((directory ? tr("Failed extracting the folder.") : tr("Failed extracting file.")));
            }
            else
            {
                WindowPrompt((directory ? tr("Folder successfully extracted.") : tr("File successfully extracted.")), 0, tr("OK"));
            }
        }
    }
    else if(choice == ArcExtractAll)
    {
        int ret = WindowPrompt(tr("Extract full archive?"), 0, tr("Yes"), tr("Cancel"));
        if(ret <= 0)
            return;

        char dest[MAXPATHLEN];
        snprintf(dest, sizeof(dest), "%s", destCandidat);
        int result = OnScreenKeyboard(dest, sizeof(dest));
        if(result)
        {
            result = browser->ExtractAll(dest);
            StopProgress();
            if(result <= 0)
            {
                ShowError(tr("Failed extracting the archive."));
            }
            else
            {
                WindowPrompt(tr("Archive successfully extracted."), 0, tr("OK"));
            }
        }
    }
}

void ProcessChoice(FileBrowser * browser, int choice)
{
    if(!browser)
        return;

    else if(strcmp(browser->GetCurrentFilename(),"..") != 0)
    {
        if(choice == CUT)
        {
            if(browser->IsCurrentDir())
                choice = WindowPrompt(browser->GetCurrentFilename(), tr("Cut directory?"), tr("Yes"), tr("Cancel"));
            else
                choice = WindowPrompt(browser->GetCurrentFilename(), tr("Cut file?"), tr("Yes"), tr("Cancel"));
            if(choice == 1)
            {
                sprintf(Clipboard.filepath, "%s", browser->GetCurrentPath());
                sprintf(Clipboard.filename, "%s", browser->GetCurrentFilename());
                if(browser->IsCurrentDir())
                    Clipboard.isdir = true;
                else
                    Clipboard.isdir = false;
                Clipboard.cutted = true;
            }
        }

        else if(choice == COPY)
        {
            if(browser->IsCurrentDir())
                choice = WindowPrompt(browser->GetCurrentFilename(), tr("Copy directory?"), tr("Yes"), tr("Cancel"));
            else
                choice = WindowPrompt(browser->GetCurrentFilename(), tr("Copy file?"), tr("Yes"), tr("Cancel"));
            if(choice == 1)
            {
                sprintf(Clipboard.filepath, "%s", browser->GetCurrentPath());
                sprintf(Clipboard.filename, "%s", browser->GetCurrentFilename());
                if(browser->IsCurrentDir())
                    Clipboard.isdir = true;
                else
                    Clipboard.isdir = false;
                Clipboard.cutted = false;
            }
        }

        else if(choice == RENAME)
        {
            char srcpath[MAXPATHLEN];
            char destdir[MAXPATHLEN];
            snprintf(srcpath, sizeof(srcpath), "%s", browser->GetCurrentSelectedFilepath());
            char entered[151];
            snprintf(entered, sizeof(entered), "%s", browser->GetCurrentFilename());
            int result = OnScreenKeyboard(entered, 150);
            if(result == 1)
            {
                snprintf(destdir, sizeof(destdir), "%s/%s", browser->GetCurrentPath(), entered);
                int ret = rename(srcpath, destdir);
                if(ret != 0)
                    WindowPrompt(tr("Failed renaming file"), tr("Name already exists."), tr("OK"));
            }
        }

        else if(choice == DELETE) {
            if(browser->IsCurrentDir())
            {
                char currentpath[MAXPATHLEN];
                snprintf(currentpath, sizeof(currentpath), "%s/", browser->GetCurrentSelectedFilepath());
                choice = WindowPrompt(browser->GetCurrentFilename(), tr("Delete directory and its content?"), tr("Yes"), tr("Cancel"));
                if(choice == 1)
                {
                    StartProgress(tr("Deleting files:"), THROBBER);
                    int res = RemoveDirectory(currentpath);
                    StopProgress();
                    if(res == -10)
                        WindowPrompt(tr("Deleting files:"), tr("Action cancelled."), tr("OK"));
                    else if(res < 0)
                        ShowError(tr("Directory couldn't be deleted."));
                    else
                        WindowPrompt(tr("Directory successfully deleted."), 0, tr("OK"));
                }
            }
            else
            {
                char currentpath[MAXPATHLEN];
                snprintf(currentpath, sizeof(currentpath), "%s", browser->GetCurrentSelectedFilepath());
                choice = WindowPrompt(browser->GetCurrentFilename(), tr("Delete this file?"), tr("Yes"), tr("Cancel"));
                if(choice == 1)
                {
                    if(RemoveFile(currentpath) == false)
                        ShowError(tr("File couldn't be deleted."));
                }
            }
        }
    }
    else if(choice >= 0 && choice != PASTE && choice != NEWFOLDER)
        WindowPrompt(tr("You cant use this operation on:"), tr("Directory .."), tr("OK"));

    if(choice == PASTE)
    {
        choice = WindowPrompt(Clipboard.filename, tr("Paste into current directory?"), tr("Yes"), tr("Cancel"));
        if(choice == 1)
        {
            char srcpath[MAXPATHLEN];
            char destdir[MAXPATHLEN];
            if(Clipboard.isdir == true)
            {
                snprintf(srcpath, sizeof(srcpath), "%s/%s/", Clipboard.filepath, Clipboard.filename);
                snprintf(destdir, sizeof(destdir), "%s/%s/", browser->GetCurrentPath(), Clipboard.filename);
                int res = 0;
                if(Clipboard.cutted == false)
                {
                    StartProgress(tr("Copying files:"));
                    res = CopyDirectory(srcpath, destdir);
                    StopProgress();
                }
                else
                {
                    if(strcmp(srcpath, destdir) != 0)
                    {
                        if(CompareDevices(srcpath, destdir))
                            StartProgress(tr("Moving files:"), THROBBER);
                        else
                            StartProgress(tr("Moving files:"));
                        res = MoveDirectory(srcpath, destdir);
                        StopProgress();
                    }
                    else
                    {
                        ShowError(tr("You can not cut into the directory itself."));
                        res =  -1;
                    }
                }

                if(res == -10)
                    WindowPrompt(tr("Transfering files:"), tr("Action cancelled."), tr("OK"));
                else if(res < 0)
                    ShowError(tr("Failed copying files."));
                else
                {
                    if(Clipboard.cutted == false)
                        WindowPrompt(tr("Directory successfully copied."), 0, tr("OK"));
                    else
                        WindowPrompt(tr("Directory successfully moved."), 0, tr("OK"));
                }
            }
            else
            {
                snprintf(srcpath, sizeof(srcpath), "%s/%s", Clipboard.filepath, Clipboard.filename);
                int ret = CheckFile(srcpath);
                if(ret == false)
                    WindowPrompt(tr("File does not exist anymore!"), 0, tr("OK"));
                else
                {
                    snprintf(destdir, sizeof(destdir), "%s/%s", browser->GetCurrentPath(), Clipboard.filename);
                    if(strcmp(srcpath, destdir) != 0)
                    {
                        StartProgress(tr("Copying file:"));
                        int res = 0;
                        if(Clipboard.cutted == false)
                            res = CopyFile(srcpath, destdir);
                        else
                            res = MoveFile(srcpath, destdir);
                        StopProgress();
                        if(res < 0)
                            ShowError(tr("Failed copying file."));
                        else
                        {
                            if(Clipboard.cutted == false)
                                WindowPrompt(tr("File successfully copied."), 0, tr("OK"));
                            else
                                WindowPrompt(tr("File successfully moved."), 0, tr("OK"));
                        }
                    }
                    else
                        ShowError(tr("You cannot read/write from/to the same file."));
                }
            }
        }
    }

    else if(choice == NEWFOLDER)
    {
        char entered[151];
        snprintf(entered, sizeof(entered), tr("New Folder"));
        int result = OnScreenKeyboard(entered, 150);
        if(result == 1)
        {
            char currentpath[MAXPATHLEN];
            snprintf(currentpath, sizeof(currentpath), "%s/%s/", browser->GetCurrentPath(), entered);
            bool ret = CreateSubfolder(currentpath);
            if(ret == false)
                ShowError(tr("Unable to create folder."));
        }
    }
}
