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
#include <malloc.h>
#include "ThreadedTaskHandler.hpp"

ThreadedTaskHandler * ThreadedTaskHandler::instance = NULL;

ThreadedTaskHandler::ThreadedTaskHandler()
	: ExitRequested(false)
{
	ThreadStack = (u8 *) memalign(32, 65*1024);
	LWP_CreateThread (&Thread, ThreadCallback, this, ThreadStack, 65*1024, 80);
}

ThreadedTaskHandler::~ThreadedTaskHandler()
{
	ExitRequested = true;
	Resume();
	LWP_JoinThread(Thread, NULL);
	free(ThreadStack);
}

void * ThreadedTaskHandler::ThreadCallback(void *arg)
{
	ThreadedTaskHandler * myInstance = (ThreadedTaskHandler *) arg;

	while(!myInstance->ExitRequested)
	{
		LWP_SuspendThread(myInstance->Thread);

		while(!myInstance->ExitRequested && !myInstance->TaskList.empty())
		{
			myInstance->TaskList.front()->Execute();
			myInstance->TaskList.pop();
		}
	}

	return NULL;
}
