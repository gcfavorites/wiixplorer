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
#include "Controls/Clipboard.h"
#include "Controls/Application.h"
#include "Controls/Taskbar.h"
#include "Controls/ThreadedTaskHandler.hpp"
#include "Prompts/Properties.h"
#include "Prompts/ArchiveProperties.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/PopUpMenu.h"
#include "FileStartUp/FileStartUp.h"
#include "FileOperations/fileops.h"
#include "FileOperations/CopyTask.h"
#include "FileOperations/MoveTask.h"
#include "FileOperations/DeleteTask.h"
#include "FileOperations/MD5Task.h"
#include "FileOperations/PackTask.h"
#include "menu.h"

void ProcessArcChoice(ArchiveBrowser * browser, int choice, const char * destCandidat)
{
	if(!browser)
		return;

	if(choice == ArcPasteItems)
	{
		int ret = WindowPrompt(tr("Paste the item(s) into this directory?"), Clipboard::Instance()->GetItemName(Clipboard::Instance()->GetItemcount()-1), tr("Yes"), tr("Cancel"));
		if(ret <= 0)
			return;

		PackTask *task = new PackTask(Clipboard::Instance(), browser->GetCurrentPath(), browser->GetArchive(), Settings.CompressionLevel);
		Taskbar::Instance()->AddTask(task);
		ThreadedTaskHandler::Instance()->AddTask(task);

		Clipboard::Instance()->Reset();
	}

	else if(choice == ArcExtractFile)
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
		browser->MarkCurrentItem();
		ItemMarker * Marker = browser->GetItemMarker();

		ArchiveProperties * Prompt = new ArchiveProperties(browser->GetArchive(), Marker);
		Prompt->SetAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
		Prompt->DimBackground(true);
		Application::Instance()->SetUpdateOnly(Prompt);
		Application::Instance()->Append(Prompt);

		Marker->Reset();
	}
}

void ProcessChoice(FileBrowser * browser, int choice)
{
	if(!browser)
		return;

	else if(browser->GetCurrentFilename() && strcmp(browser->GetCurrentFilename(),"..") != 0)
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

				for(int i = 0; i < IMarker->GetItemcount(); i++)
					Clipboard::Instance()->AddItem(IMarker->GetItem(i));

				IMarker->Reset();
				Clipboard::Instance()->Operation = OP_MOVE;
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

				for(int i = 0; i < IMarker->GetItemcount(); i++)
					Clipboard::Instance()->AddItem(IMarker->GetItem(i));

				IMarker->Reset();
				Clipboard::Instance()->Operation = OP_COPY;
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
				if(ProgressWindow::Instance()->IsRunning())
					choice = WindowPrompt(tr("Currently a process is running."), tr("Do you want to append this delete to the queue?"), tr("Yes"), tr("Cancel"));

				if(choice == 1)
				{
					//append selected Item
					browser->MarkCurrentItem();
					//Get ItemMarker
					ItemMarker * IMarker = browser->GetItemMarker();
					DeleteTask *task = new DeleteTask(IMarker);
					Taskbar::Instance()->AddTask(task);
					ThreadedTaskHandler::Instance()->AddTask(task);
					IMarker->Reset();
				}
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
			if(ProgressWindow::Instance()->IsRunning())
				choice = WindowPrompt(tr("Currently a process is running."), tr("Do you want to append this paste to the queue?"), tr("Yes"), tr("Cancel"));

			if(choice == 1)
			{
				if(Clipboard::Instance()->Operation == OP_COPY)
				{
					CopyTask *task = new CopyTask(Clipboard::Instance(), browser->GetCurrentPath());
					Taskbar::Instance()->AddTask(task);
					ThreadedTaskHandler::Instance()->AddTask(task);
				}
				else if(Clipboard::Instance()->Operation == OP_MOVE)
				{
					MoveTask *task = new MoveTask(Clipboard::Instance(), browser->GetCurrentPath());
					Taskbar::Instance()->AddTask(task);
					ThreadedTaskHandler::Instance()->AddTask(task);
				}
			}
		}
	}

	else if(choice == ADDTOZIP)
	{
		int ret = WindowPrompt(browser->GetCurrentFilename(), tr("Would you like to add/append the selected item(s) to a zip?"), tr("Yes"), tr("No"));
		if(ret <= 0)
			return;

		char DestZipPath[MAXPATHLEN];
		snprintf(DestZipPath, sizeof(DestZipPath), "%s/%s", browser->GetCurrentPath(), tr("NewZip.zip"));

		if(!OnScreenKeyboard(DestZipPath, sizeof(DestZipPath)))
			return;

		char * DestCopy = strdup(DestZipPath);
		char * ptr = strrchr(DestCopy ? DestCopy : "", '/');
		if(ptr)
			ptr[1] = '\0';

		CreateSubfolder(DestCopy);
		free(DestCopy);
		DestCopy = ptr = NULL;

		ZipFile Zip(DestZipPath, CheckFile(DestZipPath) ? ZipFile::APPEND : ZipFile::CREATE);

		//append selected Item
		browser->MarkCurrentItem();
		//Get ItemMarker
		ItemMarker * IMarker = browser->GetItemMarker();

		int result = 0;
		StartProgress(tr("Compressing item(s):"));

		for(int i = 0; i < IMarker->GetItemcount(); ++i)
		{
			if(IMarker->IsItemDir(i))
			{
				result = Zip.AddDirectory(IMarker->GetItemPath(i), IMarker->GetItemName(i), Settings.CompressionLevel);
			}
			else
			{
				result = Zip.AddFile(IMarker->GetItemPath(i), IMarker->GetItemName(i), Settings.CompressionLevel, false);
			}
		}
		StopProgress();
		if(result < 0)
			WindowPrompt(fmt(tr("ERROR: %i"), result), tr("Failed adding the item(s)."), tr("OK"));

		IMarker->Reset();
	}

	else if(choice == CHECK_MD5)
	{
		int md5Choice = 1;

		if(ProgressWindow::Instance()->IsRunning())
			md5Choice = WindowPrompt(tr("Currently a process is running."), tr("Do you want to append this process to the queue?"), tr("Yes"), tr("Cancel"));

		if(md5Choice == 1)
		{
			char LogPath[1024];
			snprintf(LogPath, sizeof(LogPath), "%s/MD5.log", browser->GetCurrentPath());

			browser->MarkCurrentItem();
			MD5Task *task = new MD5Task(browser->GetItemMarker(), LogPath);
			Taskbar::Instance()->AddTask(task);
			ThreadedTaskHandler::Instance()->AddTask(task);
			browser->GetItemMarker()->Reset();
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
	else if(choice == PROPERTIES)
	{
		browser->MarkCurrentItem();
		ItemMarker * Marker = browser->GetItemMarker();
		Properties * Prompt = new Properties(Marker);
		Prompt->SetAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
		Prompt->DimBackground(true);
		Application::Instance()->SetUpdateOnly(Prompt);
		Application::Instance()->Append(Prompt);
		Marker->Reset();
	}
}
