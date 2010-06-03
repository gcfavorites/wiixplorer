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

#include "FileOperations/ListFileBrowser.hpp"
#include "FileOperations/IconFileBrowser.hpp"
#include "Controls/Window.h"
#include "Prompts/DeviceMenu.h"
#include "Prompts/PopUpMenu.h"
#include "Prompts/CreditWindow.h"
#include "FileOperations/filebrowser.h"
#include "ArchiveOperations/ArchiveBrowser.h"

enum
{
    ICONBROWSER = 0,
    LISTBROWSER,
};

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
        void SetFilter(u8 filtermode) { DeviceBrowser->SetFilter(filtermode); };
        const char * GetCurrectPath() {  if(!CurBrowser) return NULL; return CurBrowser->GetCurrentPath(); };
    protected:
		void CheckBrowserChanges();
		void CheckDeviceMenu();
		void CheckRightClick();
        void SetDeviceImage();
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);

        int menu;

        Browser * CurBrowser;
        FileBrowser * DeviceBrowser;
        ArchiveBrowser * ArcBrowser;

        GuiFileBrowser * fileBrowser;
        IconFileBrowser * iconBrowser;
        ListFileBrowser * listBrowser;

        DeviceMenu * Device_Menu;
        PopUpMenu * RightClick;
        CreditWindow * Credits;

        GuiImage * BackgroundImg;
        GuiImage * creditsImg;
        GuiImage * RefreshImg;
        GuiImage * deviceImg;
        GuiImage * AdressbarImg;

        GuiImageData * creditsImgData;
        GuiImageData * Refresh;
        GuiImageData * Background;
        GuiImageData * Address;
        GuiImageData * sdstorage;
        GuiImageData * usbstorage;
        GuiImageData * networkstorage;
		GuiImageData * ftpstorage;
        GuiImageData * dvd_ImgData;

        GuiSound * btnSoundClick;
        GuiSound * btnSoundOver;

        GuiText * AdressText;

        GuiButton * CreditsBtn;
        GuiButton * RefreshBtn;
        GuiButton * deviceSwitchBtn;
        GuiButton * Adressbar;
        GuiButton * clickmenuBtn;

        SimpleGuiTrigger * trigA;
        GuiTrigger * trigPlus;
};

#endif
