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
#include "ProcessTask.h"
#include "Controls/Taskbar.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"

ProcessTask::ProcessTask(const char *title, const ItemMarker *p, const std::string &dest)
	: Task(title), destPath(dest)
{
	TaskType = Task::PROCESS;
	CopyFiles = 0;
	CopySize = 0;
	Process = *p;

	ShowNormal.connect(this, &ProcessTask::ShowProgressWindow);
}

ProcessTask::~ProcessTask()
{
	if(Taskbar::Instance()->TaskCount() == 0)
		StopProgress();
}

void ProcessTask::ShowProgressWindow(Task *task UNUSED, int param UNUSED)
{
	ProgressWindow::Instance()->OpenWindow();
}

void ProcessTask::CalcTotalSize(void)
{
	char filepath[1024];

	for(int i = 0; i < Process.GetItemcount(); i++)
	{
		if(ProgressWindow::Instance()->IsCanceled())
			break;

		if(Process.IsItemDir(i) == true)
		{
			snprintf(filepath, sizeof(filepath), "%s/", Process.GetItemPath(i));
			GetFolderSize(filepath, CopySize, CopyFiles, ProgressWindow::Instance()->IsCanceled());
		}
		else
		{
			CopySize += FileSize(Process.GetItemPath(i));
			++CopyFiles;
		}
	}
}
