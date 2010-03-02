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
#include "Controls/Clipboard.h"
#include "FileOperations/fileops.h"

extern bool replaceall;
extern bool replacenone;

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
        int ret = WindowPrompt(tr("Extract the selected item(s)?"), browser->GetCurrentDisplayname(), tr("Yes"), tr("Cancel"));
        if(ret <= 0)
            return;

        char dest[MAXPATHLEN];
        snprintf(dest, sizeof(dest), "%s", destCandidat);
        int result = OnScreenKeyboard(dest, sizeof(dest));
        if(result)
        {
            //append selected Item
            browser->MarkCurrentItem();
            //Get ItemMarker
            ItemMarker * IMarker = browser->GetItemMarker();

            StartProgress(tr("Extracting item(s):"));
            for(int i = 0; i < IMarker->GetItemcount(); i++)
            {
                result = browser->ExtractItem(IMarker->GetItemIndex(i), dest);
            }
            StopProgress();
            replaceall = false;
            replacenone = false;
            if(result <= 0)
            {
                ShowError(tr("Failed extracting the item(s)."));
            }
            IMarker->Reset();
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
            replaceall = false;
            replacenone = false;
            if(result <= 0)
            {
                ShowError(tr("Failed extracting the archive."));
            }
            browser->GetItemMarker()->Reset();
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
            choice = WindowPrompt(browser->GetCurrentFilename(), tr("Cut current marked item(s)?"), tr("Yes"), tr("Cancel"));
            if(choice == 1)
            {
                Clipboard::Instance()->Reset();
                //append selected Item
                browser->MarkCurrentItem();
                //Get ItemMarker
                ItemMarker * IMarker = browser->GetItemMarker();
                if(IMarker)
                {
                    for(int i = 0; i < IMarker->GetItemcount(); i++)
                    {
                        ItemStruct * Item = IMarker->GetItem(i);
                        Clipboard::Instance()->AddItem(Item);
                    }
                }
                IMarker->Reset();
                Clipboard::Instance()->Cutted = true;
            }
        }

        else if(choice == COPY)
        {
            choice = WindowPrompt(browser->GetCurrentFilename(), tr("Copy current marked item(s)?"), tr("Yes"), tr("Cancel"));
            if(choice == 1)
            {
                Clipboard::Instance()->Reset();
                //append selected Item
                browser->MarkCurrentItem();
                //Get ItemMarker
                ItemMarker * IMarker = browser->GetItemMarker();

                if(IMarker)
                {
                    for(int i = 0; i < IMarker->GetItemcount(); i++)
                    {
                        ItemStruct * Item = IMarker->GetItem(i);
                        Clipboard::Instance()->AddItem(Item);
                    }
                }
                IMarker->Reset();
                Clipboard::Instance()->Cutted = false;
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
                    WindowPrompt(tr("Failed renaming item"), tr("Name might already exists."), tr("OK"));
            }
        }

        else if(choice == DELETE)
        {
            char currentpath[MAXPATHLEN];
            snprintf(currentpath, sizeof(currentpath), "%s/", browser->GetCurrentSelectedFilepath());
            choice = WindowPrompt(browser->GetCurrentFilename(), tr("Delete the selected item(s) and its content?"), tr("Yes"), tr("Cancel"));
            if(choice == 1)
            {
                ItemMarker * IMarker = browser->GetItemMarker();
                //append selected Item
                browser->MarkCurrentItem();

                StartProgress(tr("Deleting files:"), THROBBER);
                for(int i = 0; i < IMarker->GetItemcount(); i++)
                {
                    if(IMarker->IsItemDir(i))
                    {
                        snprintf(currentpath, sizeof(currentpath), "%s/", IMarker->GetItemPath(i));
                        int res = RemoveDirectory(currentpath);
                        if(res == -10)
                        {
                            StopProgress();
                            WindowPrompt(tr("Deleting files:"), tr("Action cancelled."), tr("OK"));
                            break;
                        }
                        else if(res < 0)
                        {
                            StopProgress();
                            ShowError(tr("Directory couldn't be deleted."));
                            break;
                        }
                    }
                    else
                    {
                        snprintf(currentpath, sizeof(currentpath), "%s", IMarker->GetItemPath(i));
                        if(RemoveFile(currentpath) == false)
                        {
                            StopProgress();
                            ShowError(tr("File couldn't be deleted."));
                            break;
                        }
                    }
                }
                StopProgress();
                IMarker->Reset();
            }
        }
    }
    else if(choice >= 0 && choice != PASTE && choice != NEWFOLDER)
        WindowPrompt(tr("You cant use this operation on:"), tr("Directory .."), tr("OK"));

    if(choice == PASTE)
    {
        choice = WindowPrompt(Clipboard::Instance()->GetItemName(Clipboard::Instance()->GetItemcount()-1), tr("Paste item(s) into current directory?"), tr("Yes"), tr("Cancel"));
        if(choice == 1)
        {
            char srcpath[MAXPATHLEN];
            char destdir[MAXPATHLEN];
            int res = 0;
            bool Cutted = Clipboard::Instance()->Cutted;
            if(Cutted)
            {
                if(Clipboard::Instance()->GetItemcount() > 0)
                {
                    snprintf(srcpath, sizeof(srcpath), "%s/", Clipboard::Instance()->GetItemPath(0));
                    snprintf(destdir, sizeof(destdir), "%s/%s/", browser->GetCurrentPath(), Clipboard::Instance()->GetItemName(0));
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
            for(int i = 0; i < Clipboard::Instance()->GetItemcount(); i++)
            {
                if(Clipboard::Instance()->IsItemDir(i) == true)
                {
                    snprintf(srcpath, sizeof(srcpath), "%s/", Clipboard::Instance()->GetItemPath(i));
                    snprintf(destdir, sizeof(destdir), "%s/%s/", browser->GetCurrentPath(), Clipboard::Instance()->GetItemName(i));
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
                        WindowPrompt(tr("Transfering files:"), tr("Action cancelled."), tr("OK"));
                        break;
                    }
                }
                else
                {
                    snprintf(srcpath, sizeof(srcpath), "%s", Clipboard::Instance()->GetItemPath(i));
                    snprintf(destdir, sizeof(destdir), "%s/%s", browser->GetCurrentPath(), Clipboard::Instance()->GetItemName(i));
                    if(strcmp(srcpath, destdir) != 0)
                    {
                        if(Cutted == false)
                            res = CopyFile(srcpath, destdir);
                        else
                            res = MoveFile(srcpath, destdir);
                    }
                    else
                    {
                        StopProgress();
                        ShowError(tr("You cannot read/write from/to the same file."));
                        res = -1;
                        break;
                    }
                }
            }
            StopProgress();
            replaceall = false;
            replacenone = false;
            if(res < 0 && res != 10)
            {
                if(Cutted)
                    ShowError(tr("Failed moving item(s)."));
                else
                    ShowError(tr("Failed copying item(s)."));
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
