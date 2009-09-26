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
 * Events.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#include "Events.h"
#include <utility>


void Events::Register(void *sender, int type, void *function)
{
	RegisteredEvent *evt;
	
	std::map<void *, RegisteredEvent *>::iterator itr = registeredEvents.find(sender);
	if (itr != registeredEvents.end())
	{
			evt = itr->second;
	}
	else
	{
		evt = (RegisteredEvent *) malloc(sizeof(RegisteredEvent));
		registeredEvents.insert(std::make_pair(sender, evt));
	}
	
	evt->functions.push_back(function);
}

void Events::Execute(void *sender, int type)
{
}
