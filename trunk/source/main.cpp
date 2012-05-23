/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include "Controls/Application.h"
#include "VideoOperations/video.h"
#include "sys.h"

int main(int argc UNUSED, char *argv[] UNUSED)
{
	__exception_setreload(30);
	// Initialize video before anything else as otherwise green stripes are produced, need to figure out why...
	InitVideo();

	Application::Instance()->init();
	Application::Instance()->show();
	Application::Instance()->exec();

	/* Return to the Wii system menu  if not from HBC*/
	if(!IsFromHBC())
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

	return 0;
}
