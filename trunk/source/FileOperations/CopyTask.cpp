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
#include "CopyTask.h"
#include "Controls/Application.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindow.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"

CopyTask::CopyTask(const ItemMarker *p, const std::string &dest)
	: ProcessTask(tr("Copying item(s):"), p, dest)
{
}

CopyTask::~CopyTask()
{
}

void CopyTask::Execute(void)
{
	TaskBegin(this);

	// No items to process
	if(Process.GetItemcount() == 0)
	{
		TaskEnd(this);
		return;
	}

	if(ProgressWindow::Instance()->IsRunning())
		ProgressWindow::Instance()->SetTitle(tr("Calculating transfer size..."));
	else
		StartProgress(tr("Calculating transfer size..."));

	CalcTotalSize();

	ProgressWindow::Instance()->SetTitle(tr("Copying item(s):"));
	ProgressWindow::Instance()->SetCompleteValues(0, CopySize);

	char srcpath[MAXPATHLEN];
	char destpath[MAXPATHLEN];
	int result = 0;

	for(int i = 0; i < Process.GetItemcount(); i++)
	{
		if(ProgressWindow::Instance()->IsCanceled())
		{
			result = -10;
			break;
		}

		snprintf(srcpath, sizeof(srcpath), "%s", Process.GetItemPath(i));
		snprintf(destpath, sizeof(destpath), "%s/%s", destPath.c_str(), Process.GetItemName(i));

		if(strcmp(srcpath, destpath) == 0)
			snprintf(destpath, sizeof(destpath), "%s/%s %s", destPath.c_str(), tr("Copy of"), Process.GetItemName(i));

		if(Process.IsItemDir(i) == true)
		{
			int ret = CopyDirectory(srcpath, destpath);
			if(ret < 0)
				result = ret;
		}
		else
		{
			int ret = CopyFile(srcpath, destpath);
			if(ret < 0)
				result = ret;
		}
	}

	if(result < 0 && result != -10 && !Application::isClosing())
	{
		ThrowMsg(tr("Error:"), tr("Failed copying some item(s)."));
	}

	TaskEnd(this);
}
