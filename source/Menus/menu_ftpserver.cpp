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
#include <unistd.h>
#include "libftp/FTPServerMenu.h"
#include "Controls/MainWindow.h"
#include "network/networkops.h"
#include "Prompts/PromptWindow.h"
#include "menu.h"

int MenuFTPServer()
{
    int menu = MENU_NONE;
    PromptWindow * Prompt = new PromptWindow(tr("Preparing the network."), tr("Please wait..."));
    MainWindow::Instance()->Append(Prompt);

    if(!IsNetworkInit() && Settings.AutoConnect == off)
        Initialize_Network();
    else
        HaltNetworkThread();

	CloseFTP();

    delete Prompt;
    Prompt = NULL;

    if(!IsNetworkInit())
    {
        ShowError(tr("No network connection."));
        return MENU_BROWSE_DEVICE;
    }

    FTPServerMenu * FTPMenu = new FTPServerMenu();
    FTPMenu->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    FTPMenu->SetPosition(0, 30);

    MainWindow::Instance()->Append(FTPMenu);

    while(menu == MENU_NONE)
    {
        usleep(100);

        menu = FTPMenu->GetMenu();
    }

    delete FTPMenu;

    ResumeNetworkThread();

	return menu;
}
