/***************************************************************************
 * Copyright (C) 2009
 * by r-win
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
 * Events.h
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#ifndef _EVENTS_H
#define _EVENTS_H

#include <map>
#include <list>

typedef void (*CloseEvent)(void *sender);
typedef void (*ClickEvent)(void *sender);

enum
{
	CLOSE,
	CLICK
};

typedef struct RegisteredEvent
{
	int type;
	std::list<void *> functions;
} RegisteredEvent;

class Events
{
	public:

		void Register(void *sender, int type, void *function);
		void Execute(void *sender, int type);
	private:
		std::map<void *, RegisteredEvent *> registeredEvents;
};

#endif //_EVENTS_H
