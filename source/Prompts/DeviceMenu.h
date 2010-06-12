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
 * Device Menu
 *
 * for WiiXplorer 2009
 ***************************************************************************/
 #ifndef __DEVICE_MENU_H_
 #define __DEVICE_MENU_H_

#include <gctypes.h>
#include <unistd.h>
#include "libwiigui/gui.h"

enum
{
    SD = 0,
    GCSDA,
    GCSDB,
    USB,
    NTFS0,
    NTFS1,
    NTFS2,
    NTFS3,
    NTFS4,
    SMB1,
    SMB2,
    SMB3,
    SMB4,
    SMB5,
    SMB6,
    SMB7,
    SMB8,
    SMB9,
    SMB10,
    FTP1,
    FTP2,
    FTP3,
    FTP4,
    FTP5,
    FTP6,
    FTP7,
    FTP8,
    FTP9,
    FTP10,
	DVD,
    MAXDEVICES
};

const char DeviceName[MAXDEVICES][6] =
{
    "sd",
    "gca",
    "gcb",
    "usb",
    "ntfs0",
    "ntfs1",
    "ntfs2",
    "ntfs3",
    "ntfs4",
    "smb1",
    "smb2",
    "smb3",
    "smb4",
    "smb5",
    "smb6",
    "smb7",
    "smb8",
    "smb9",
    "smb10",
    "ftp1",
    "ftp2",
    "ftp3",
    "ftp4",
    "ftp5",
    "ftp6",
    "ftp7",
    "ftp8",
    "ftp9",
    "ftp10",
	"dvd",
};

class DeviceMenu : public GuiWindow
{
    public:
        DeviceMenu(int x, int y);
        ~DeviceMenu();
        int GetChoice();
    private:
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);

        int deviceCount;
        int deviceSelection[MAXDEVICES];
        int tile;
        int choice;

        GuiImage * centerImg;
        GuiImage * leftImg;
        GuiImage * rightImg;
        GuiImage * deviceImgs[MAXDEVICES];
        GuiImage * deviceImgOver[MAXDEVICES];

        GuiImageData * device_choose_right_Data;
        GuiImageData * device_choose_left_Data;
        GuiImageData * device_choose_center_Data;
        GuiImageData * menu_select;
        GuiImageData * sd_ImgData;
        GuiImageData * usb_ImgData;
        GuiImageData * smb_ImgData;
        GuiImageData * ftp_ImgData;
		GuiImageData * dvd_ImgData;

        GuiSound * btnClick;

        GuiText * deviceText[MAXDEVICES];

        GuiButton * NoBtn;
        GuiButton * deviceBtn[MAXDEVICES];

        SimpleGuiTrigger * trigA;
        GuiTrigger * trigB;
};

#endif //__DEVICE_MENU_H_