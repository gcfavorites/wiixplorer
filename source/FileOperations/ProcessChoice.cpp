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
#include "Prompts/PopUpMenu.h"
#include "FileStartUp/FileStartUp.h"
#include "Controls/Clipboard.h"
#include "FileOperations/fileops.h"
#include "FileOperations/MD5Logger.hpp"
#include "Controls/IOHandler.hpp"
#include "Controls/MainWindow.h"
#include "Prompts/Properties.h"
#include "Prompts/ArchiveProperties.h"

void ProcessArcChoice(ArchiveBrowser * browser, int choice, const char * destCandidat)
{
    if(!browser)
        return;

    if(choice == ArcExtractFile)
    {
        int ret = WindowPrompt(tr("Extract the selected item(s)?"), browser->GetCurrentName(), tr("Yes"), tr("Cancel"));
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
            ResetReplaceChoice();
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
        if(dest[strlen(dest)-1] != '/')
            strcat(dest, "/");

        strncat(dest, browser->GetArchiveName(), sizeof(dest));

        char * ext = strrchr(dest, '.');
        if(ext)
            ext[0] = 0;
        strcat(dest, "/");

        int result = OnScreenKeyboard(dest, sizeof(dest));
        if(result)
        {
            result = browser->ExtractAll(dest);
            StopProgress();
            ResetReplaceChoice();
            if(result <= 0)
            {
                ShowError(tr("Failed extracting the archive."));
            }
            browser->GetItemMarker()->Reset();
        }
    }
    else if(choice == ArcProperties)
    {
        ArchiveProperties * Prompt = new ArchiveProperties(browser->GetCurrentItemStructure());
        Prompt->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        MainWindow::Instance()->SetDim(true);
        MainWindow::Instance()->Append(Prompt);

        while(Prompt->GetChoice() == -1)
            usleep(100);

        delete Prompt;
        Prompt = NULL;
        MainWindow::Instance()->SetDim(false);
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
                        Clipboard::Instance()->AddItem(IMarker->GetItem(i));
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
                        Clipboard::Instance()->AddItem(IMarker->GetItem(i));
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
                if(!RenameFile(srcpath, destdir))
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
    else if(choice >= 0 && choice != PASTE && choice != NEWFOLDER && choice != PROPERTIES)
        WindowPrompt(tr("You cant use this operation on:"), tr("Directory .."), tr("OK"));

    if(choice == PASTE)
    {
        choice = WindowPrompt(Clipboard::Instance()->GetItemName(Clipboard::Instance()->GetItemcount()-1), tr("Paste item(s) into current directory?"), tr("Yes"), tr("Cancel"));
        if(choice == 1)
        {
            if(IOHandler::Instance()->IsRunning())
            {
                choice = WindowPrompt(tr("Currently a process is running."), tr("Do you want to append this paste to the queue?"), tr("Yes"), tr("Cancel"));
            }
            if(choice == 1)
            {
                IOHandler::Instance()->AddProcess(Clipboard::Instance(), browser->GetCurrentPath(), Clipboard::Instance()->Cutted);
            }
        }
    }

    else if(choice == CHECK_MD5)
    {
        MD5Logger Logger;
        char LogPath[1024];
        snprintf(LogPath, sizeof(LogPath), "%s/MD5.log", browser->GetCurrentPath());

        browser->MarkCurrentItem();
        Logger.LogMD5(LogPath, browser->GetItemMarker());
        browser->GetItemMarker()->Reset();
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
    else if(choice == PROPERTIES)
    {
        browser->UnMarkCurrentItem();
        browser->MarkCurrentItem();
        ItemMarker * Marker = browser->GetItemMarker();

        if(Marker->GetItemcount() == 0)
        {
            ItemStruct * Item = new ItemStruct;
            memset(Item, 0, sizeof(ItemStruct));
            Item->itempath = strdup(browser->GetCurrentPath());
            Item->isdir = true;
            Marker->AddItem(Item);
        }

        Properties * Prompt = new Properties(Marker);
        Prompt->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        MainWindow::Instance()->SetDim(true);
        MainWindow::Instance()->Append(Prompt);

        while(Prompt->GetChoice() == -1)
            usleep(100);

        delete Prompt;
        Prompt = NULL;
        MainWindow::Instance()->SetDim(false);
        Marker->Reset();
    }
}
