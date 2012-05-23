/****************************************************************************
 * Copyright (C) 2010-2011 Dimok
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
#ifndef THREADED_TASK_HPP_
#define THREADED_TASK_HPP_

#include <gccore.h>
#include <queue>

class ThreadedTask
{
public:
	ThreadedTask() {}
	virtual ~ThreadedTask() {}
	virtual void Execute(void) = 0;
};

class ThreadedTaskHandler
{
public:
	static ThreadedTaskHandler * Instance() { if(!instance) instance = new ThreadedTaskHandler(); return instance; };
	static void DestroyInstance() { delete instance; instance = NULL; };

	void AddTask(ThreadedTask *Task)
	{
		TaskList.push(Task);
		Resume();
	}

	void Halt(void) { LWP_SuspendThread(Thread); }

	void Resume(void) { LWP_ResumeThread(Thread); }

	void setPriority(int prio) { LWP_SetThreadPriority(Thread, prio); }

	bool isRunning(void) const { return !LWP_ThreadIsSuspended(Thread); }
private:
	ThreadedTaskHandler();
	~ThreadedTaskHandler();

	void ThreadLoop(void *arg);
	static void * ThreadCallback(void *arg);

	static ThreadedTaskHandler *instance;
	u8 *ThreadStack;
	lwp_t Thread;
	bool ExitRequested;
	std::queue<ThreadedTask *> TaskList;
};

#endif
