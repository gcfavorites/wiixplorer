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
#ifndef PROCESSTASK_H_
#define PROCESSTASK_H_

#include "Controls/ThreadedTaskHandler.hpp"
#include "Controls/Task.hpp"
#include "FileOperations/ItemMarker.h"

class ProcessTask : public ThreadedTask, public Task
{
public:
	ProcessTask(const char *title, const ItemMarker *process, const std::string &dest);
	virtual ~ProcessTask();
	virtual void Execute(void) = 0;
protected:
	void ShowProgressWindow(Task *task, int param);
	void DisplayPrompt(const char *title, const char *msg);
	void CalcTotalSize(void);

	ItemMarker Process;
	const std::string destPath;
	u64 CopySize;
	u32 CopyFiles;
};

#endif /* PROCESSTASK_H_ */
