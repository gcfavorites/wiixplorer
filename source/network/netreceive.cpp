#include <gctypes.h>

#include "BootHomebrew/BootHomebrew.h"
#include "Language/gettext.h"
#include "Prompts/PromptWindows.h"
#include "Controls/Taskbar.h"
#include "networkops.h"
#include "netreceiver.h"
#include "menu.h"

extern bool boothomebrew;

void IncommingConnection(NetReceiver & Receiver)
{
    char text[200];
    snprintf(text, sizeof(text),  tr("Do you want to load file from: %s ?"), Receiver.GetIncommingIP());
    int choice = WindowPrompt(tr("Incomming connection."), text, tr("Yes"), tr("No"));
    if(choice)
    {
        const u8 * buffer = Receiver.ReceiveData();
        if(buffer)
        {
            choice = WindowPrompt(tr("Do you want to boot file now?"), Receiver.GetFilename(), tr("Yes"), tr("No"));
            if(choice)
            {
                CopyHomebrewMemory((u8*) buffer, 0, Receiver.GetFilesize());
                boothomebrew = true;
                Taskbar::Instance()->SetMenu(MENU_EXIT);
            }
        }
    }
    Receiver.FreeData();
    Receiver.CloseConnection();
}
