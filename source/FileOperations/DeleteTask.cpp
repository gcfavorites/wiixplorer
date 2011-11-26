/****************************************************************************
 * Copyright (C) 2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "DeleteTask.h"
#include "Controls/Application.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindow.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"

DeleteTask::DeleteTask(const ItemMarker *p)
	: ProcessTask(tr("Deleting item(s):"), p, "")
{
}

DeleteTask::~DeleteTask()
{
}

void DeleteTask::Execute(void)
{
	TaskBegin(this);
	gprintf("EXEC\n");
	// No items to process
	if(Process.GetItemcount() == 0)
	{
		TaskEnd(this);
		return;
	}

	if(ProgressWindow::Instance()->IsRunning())
		ProgressWindow::Instance()->SetTitle(tr("Calculating delete size..."));
	else
		StartProgress(tr("Calculating delete size..."));

	CalcTotalSize();

	ProgressWindow::Instance()->SetTitle(tr("Deleting item(s):"));
    ProgressWindow::Instance()->SetCompleteValues(0, CopyFiles);
    ProgressWindow::Instance()->SetUnit(tr("files"));

	int result = 0;
	char srcpath[MAXPATHLEN];
	memset(srcpath, 0, sizeof(srcpath));

	for(int i = 0; i < Process.GetItemcount(); i++)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			result = -10;
			break;
		}

		strncpy(srcpath, Process.GetItemPath(i), sizeof(srcpath)-1);

		if(Process.IsItemDir(i) == true)
		{
			int ret = RemoveDirectory(srcpath);
			if(ret < 0)
				result = ret;
		}
		else
		{
			int ret = RemoveFile(srcpath);
			if(ret < 0)
				result = ret;
		}
	}

	ProgressWindow::Instance()->SetUnit(NULL);

	if(result < 0 && result != -10 && !Application::isClosing())
	{
		ThrowMsg(tr("Error:"), tr("Failed deleting some item(s)."));
	}

	TaskEnd(this);
}
