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
#include <gctypes.h>
#include "Prompts/PromptWindows.h"
#include "network/networkops.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Explorer.h"
#include "menu.h"
#include "sys.h"

static bool firsttimestart = true;
extern int curDevice;

int MenuBrowseDevice()
{
    if(firsttimestart  && Settings.MountMethod >= SMB1 && Settings.MountMethod <= SMB4 && !IsNetworkInit())
    {
        if(WaitSMBConnect() < 2)
            ShowError(tr("Could not connect to the network"));
        firsttimestart = false;
    }

	int menu = MENU_NONE;

    Explorer * Explorer_1 = new Explorer(curDevice);

    MainWindow::Instance()->Append(Explorer_1);
    ResumeGui();

    while(menu == MENU_NONE)
    {
	    usleep(THREAD_SLEEP);

        if(shutdown)
            Sys_Shutdown();

        else if(reset)
            Sys_Reboot();

        menu = Explorer_1->GetMenuChoice();

        if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();
    }

    delete Explorer_1;
    Explorer_1 = NULL;
	ResumeGui();

	return menu;
}
