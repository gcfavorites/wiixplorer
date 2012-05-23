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
#ifndef FILELOADTASK_H_
#define FILELOADTASK_H_

#include "FileOperations/ProcessTask.h"

class FileLoadTask : public ProcessTask
{
public:
	FileLoadTask(const std::string &filepath, bool silent);
	virtual ~FileLoadTask();
	virtual void Execute(void);
	void SetAutoDelete(bool b) { bAutoDelete = b; }
	sigslot::signal3<FileLoadTask *, u8 *, u32> LoadingComplete;
private:
	bool m_silent;
	bool bAutoDelete;
};

#endif /* COPYTASK_H_ */
