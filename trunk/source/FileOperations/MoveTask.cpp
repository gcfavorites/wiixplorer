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
#include "MoveTask.h"
#include "Controls/Application.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindow.h"
#include "Prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"

MoveTask::MoveTask(const ItemMarker *p, const std::string &dest)
	: ProcessTask(tr("Moving item(s):"), p, dest)
{
	this->SetTitle(tr("Moving item(s):"));
}

MoveTask::~MoveTask()
{
}

void MoveTask::Execute(void)
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

	ProgressWindow::Instance()->SetTitle(this->getTitle().c_str());

    //! On same device we move files instead of copy them
	if(CompareDevices(Process.GetItemPath(0), destPath.c_str()))
	{
        ProgressWindow::Instance()->SetCompleteValues(0, CopyFiles);
        ProgressWindow::Instance()->SetUnit(tr("files"));
	}
	else
	{
        ProgressWindow::Instance()->SetCompleteValues(0, CopySize);
	}

	int result = 0;
	char srcpath[MAXPATHLEN];
	char destpath[MAXPATHLEN];

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
			continue;

		if(Process.IsItemDir(i) == true)
		{
			int ret = MoveDirectory(srcpath, destpath);
			if(ret < 0)
				result = ret;
		}
		else
		{
			int ret = MoveFile(srcpath, destpath);
			if(ret < 0)
				result = ret;
		}
	}

	ProgressWindow::Instance()->SetUnit(NULL);

	if(result < 0 && result != -10 && !Application::isClosing())
	{
		ThrowMsg(tr("Error:"), tr("Failed moving some item(s)."));
	}

	TaskEnd(this);
}
