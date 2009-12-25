/***************************************************************************
 * Copyright (C) 2009
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
 * Explorer Class
 *
 * for WiiXplorer 2009
 ***************************************************************************/
 #ifndef __EXPLORER_H_
 #define __EXPLORER_H_

#include <gctypes.h>
#include <unistd.h>

#include "libwiigui/gui_filebrowser.h"
#include "Controls/Window.h"
#include "Prompts/DeviceMenu.h"
#include "Prompts/RightClickMenu.h"
#include "FileOperations/filebrowser.h"
#include "ArchiveOperations/ArchiveBrowser.h"

class Explorer : public GuiWindow
{
    public:
        Explorer();
        Explorer(int device);
        Explorer(const char * path);
        ~Explorer();
        void Setup();
        int LoadPath(const char * path);
        int LoadDevice(int device);
        int GetMenuChoice();
        void SetState(int s);
    private:
		void ArchiveChanges();
		void CheckBrowserChanges();
		void CheckDeviceMenu();
		void CheckRightClick();
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);

        int menu;
        int currentDevice;
        bool CreditsPressed;
        u32 filecount;

        FileBrowser * Browser;
        ArchiveBrowser * ArcBrowser;
        GuiFileBrowser * fileBrowser;

        DeviceMenu * Device_Menu;
        RightClickMenu * RightClick;

        GuiImage * BackgroundImg;
        GuiImage * creditsImg;
        GuiImage * deviceImg;
        GuiImage * AdressbarImg;

        GuiImageData * creditsImgData;
        GuiImageData * Background;
        GuiImageData * Address;
        GuiImageData * sdstorage;
        GuiImageData * usbstorage;
        GuiImageData * networkstorage;

        GuiSound * btnSoundClick;
        GuiSound * btnSoundOver;

        GuiText * AdressText;

        GuiButton * CreditsBtn;
        GuiButton * deviceSwitchBtn;
        GuiButton * Adressbar;
        GuiButton * clickmenuBtn;

        GuiTrigger * trigA;
        GuiTrigger * trigPlus;
        GuiTrigger * trigMinus;
};

#endif
