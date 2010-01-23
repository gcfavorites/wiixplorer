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
 * DeviceMenu.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include "DeviceMenu.h"
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"
#include "Language/gettext.h"
#include "network/networkops.h"
#include "devicemounter.h"
#include "FileOperations/filebrowser.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

DeviceMenu::DeviceMenu(int x, int y)
    :GuiWindow(0, 0)
{
    deviceCount = 0;
    choice = -1;

    //! Menu imagedata
    device_choose_right_Data = Resources::GetImageData(device_choose_right_png, device_choose_right_png_size);
    device_choose_left_Data = Resources::GetImageData(device_choose_left_png, device_choose_left_png_size);
    device_choose_center_Data = Resources::GetImageData(device_choose_center_png, device_choose_center_png_size);
    menu_select = Resources::GetImageData(menu_selection_png, menu_selection_png_size);

    //! Device imagedata
    sd_ImgData = Resources::GetImageData(sdstorage_png, sdstorage_png_size);
    usb_ImgData = Resources::GetImageData(usbstorage_png, usbstorage_png_size);
    smb_ImgData = Resources::GetImageData(networkstorage_png, networkstorage_png_size);
    dvd_ImgData = Resources::GetImageData(dvdstorage_png, dvdstorage_png_size);

    //! Menu images
    centerImg = new GuiImage(device_choose_center_Data);
    leftImg = new GuiImage(device_choose_left_Data);
    rightImg = new GuiImage(device_choose_right_Data);

	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	btnClick = Resources::GetSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    for(int i = 0; i < MAXDEVICES; i++)
    {
        deviceBtn[i] = NULL;
        deviceImgs[i] = NULL;
        deviceImgOver[i] = NULL;
        deviceText[i] = NULL;
        deviceSelection[i] = -1;
    }

    int PositionX = leftImg->GetWidth();

    if(SDCard_Inserted())
    {
        deviceText[deviceCount] = new GuiText(DeviceName[SD], FontSize, (GXColor){0, 0, 0, 255});
        deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        deviceText[deviceCount]->SetPosition(0, 2);
        deviceImgs[deviceCount] = new GuiImage(sd_ImgData);
        deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        deviceImgOver[deviceCount] = new GuiImage(menu_select);
        deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
        deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
        deviceBtn[deviceCount]->SetSoundClick(btnClick);
        deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
        deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
        deviceBtn[deviceCount]->SetTrigger(trigA);
        deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        deviceBtn[deviceCount]->SetPosition(PositionX, PositionY);
        deviceBtn[deviceCount]->Clicked.connect(this, &DeviceMenu::OnButtonClick);
        PositionX += deviceImgs[deviceCount]->GetWidth()+10;

        deviceSelection[deviceCount] = SD;

        deviceCount++;
    }

    if(USBDevice_Inserted())
    {
        deviceText[deviceCount] = new GuiText(DeviceName[USB], FontSize, (GXColor){0, 0, 0, 255});
        deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        deviceText[deviceCount]->SetPosition(0, 2);
        deviceImgs[deviceCount] = new GuiImage(usb_ImgData);
        deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        deviceImgOver[deviceCount] = new GuiImage(menu_select);
        deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
        deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
        deviceBtn[deviceCount]->SetSoundClick(btnClick);
        deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
        deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
        deviceBtn[deviceCount]->SetTrigger(trigA);
        deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        deviceBtn[deviceCount]->SetPosition(PositionX, PositionY);
        deviceBtn[deviceCount]->Clicked.connect(this, &DeviceMenu::OnButtonClick);
        PositionX += deviceImgs[deviceCount]->GetWidth()+10;

        deviceSelection[deviceCount] = USB;

        deviceCount++;
    }

	char text[50];
    for(int i = 0; i < NTFS_GetMountCount(); i++)
    {
        sprintf(text, "%s", NTFS_GetMountName(i));
        deviceText[deviceCount] = new GuiText(text, FontSize, (GXColor){0, 0, 0, 255});
        deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        deviceText[deviceCount]->SetPosition(0, 2);
        deviceImgs[deviceCount] = new GuiImage(usb_ImgData);
        deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        deviceImgOver[deviceCount] = new GuiImage(menu_select);
        deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
        deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
        deviceBtn[deviceCount]->SetSoundClick(btnClick);
        deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
        deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
        deviceBtn[deviceCount]->SetTrigger(trigA);
        deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        deviceBtn[deviceCount]->SetPosition(PositionX, PositionY);
        deviceBtn[deviceCount]->Clicked.connect(this, &DeviceMenu::OnButtonClick);
        PositionX += deviceImgs[deviceCount]->GetWidth()+10;

        deviceSelection[deviceCount] = NTFS0+i;

        deviceCount++;
    }

    if(Disk_Inserted())
    {
        bool dvdmounted = DiskDrive_Mount();
        if(dvdmounted)
        {
            deviceText[deviceCount] = new GuiText(DeviceName[DVD], FontSize, (GXColor){0, 0, 0, 255});
            deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            deviceText[deviceCount]->SetPosition(0, 2);
            deviceImgs[deviceCount] = new GuiImage(dvd_ImgData);
            deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            deviceImgOver[deviceCount] = new GuiImage(menu_select);
            deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
            deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
            deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
            deviceBtn[deviceCount]->SetSoundClick(btnClick);
            deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
            deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
            deviceBtn[deviceCount]->SetTrigger(trigA);
            deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            deviceBtn[deviceCount]->SetPosition(PositionX, PositionY);
            deviceBtn[deviceCount]->Clicked.connect(this, &DeviceMenu::OnButtonClick);
            PositionX += deviceImgs[deviceCount]->GetWidth()+10;

            deviceSelection[deviceCount] = DVD;

            deviceCount++;
        }
    }

    for(int i = 0; i < 4; i++)
    {
        if(IsSMB_Mounted(i))
        {
            deviceText[deviceCount] = new GuiText(DeviceName[SMB1+i], FontSize, (GXColor){0, 0, 0, 255});
            deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            deviceText[deviceCount]->SetPosition(0, 2);
            deviceImgs[deviceCount] = new GuiImage(smb_ImgData);
            deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            deviceImgOver[deviceCount] = new GuiImage(menu_select);
            deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
            deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
            deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
            deviceBtn[deviceCount]->SetSoundClick(btnClick);
            deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
            deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
            deviceBtn[deviceCount]->SetTrigger(trigA);
            deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            deviceBtn[deviceCount]->SetPosition(PositionX, PositionY);
            deviceBtn[deviceCount]->Clicked.connect(this, &DeviceMenu::OnButtonClick);
            PositionX += deviceImgs[deviceCount]->GetWidth()+10;

            deviceSelection[deviceCount] = SMB1+i;

            deviceCount++;
        }
    }
/*
	deviceText[deviceCount] = new GuiText("nand", FontSize, (GXColor){0, 0, 0, 255});
	deviceText[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	deviceText[deviceCount]->SetPosition(0, 2);
	deviceImgs[deviceCount] = new GuiImage(&nand_ImgData);
	deviceImgs[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	deviceImgOver[deviceCount] = new GuiImage(&menu_select);
	deviceImgOver[deviceCount]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	deviceBtn[deviceCount] = new GuiButton(deviceImgs[deviceCount]->GetWidth(), deviceImgs[deviceCount]->GetHeight()+FontSize);
	deviceBtn[deviceCount]->SetLabel(deviceText[deviceCount]);
	deviceBtn[deviceCount]->SetSoundClick(&btnClick);
	deviceBtn[deviceCount]->SetIcon(deviceImgs[deviceCount]);
	deviceBtn[deviceCount]->SetImageOver(deviceImgOver[deviceCount]);
	deviceBtn[deviceCount]->SetTrigger(&trigA);
	deviceBtn[deviceCount]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	deviceBtn[deviceCount]->SetPosition(PositionX, PositionY);
	PositionX += deviceImgs[deviceCount]->GetWidth()+10;
	deviceSelection[deviceCount] = NAND;
	deviceCount++;
*/

    //! Set image position and tile
    tile = (PositionX-leftImg->GetWidth())/centerImg->GetWidth();

    leftImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    leftImg->SetPosition(0, 0);

    centerImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    centerImg->SetPosition(leftImg->GetLeft()+leftImg->GetWidth(), 0);
    centerImg->SetTileHorizontal(tile);

    rightImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    rightImg->SetPosition(leftImg->GetWidth() + tile * centerImg->GetWidth(), 0);

    width = leftImg->GetWidth() + tile * centerImg->GetWidth() + rightImg->GetWidth();
    height = leftImg->GetHeight();

    NoBtn = new GuiButton(screenwidth, screenheight);
    NoBtn->SetPosition(-x, -y);
    NoBtn->SetTrigger(trigA);
    NoBtn->SetTrigger(trigB);
    NoBtn->Clicked.connect(this, &DeviceMenu::OnButtonClick);

    Append(NoBtn);
    Append(leftImg);
    Append(centerImg);
    Append(rightImg);
    for(int i = 0; i < deviceCount; i++)
        Append(deviceBtn[i]);

    SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    SetPosition(x, y);
    SetEffect(EFFECT_FADE, 25);
}

DeviceMenu::~DeviceMenu()
{
    ResumeGui();
    SetEffect(EFFECT_FADE, -30);
    while(this->GetEffect() > 0) usleep(100);

    HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

	Resources::Remove(device_choose_right_Data);
	Resources::Remove(device_choose_left_Data);
	Resources::Remove(device_choose_center_Data);
    Resources::Remove(sd_ImgData);
    Resources::Remove(usb_ImgData);
    Resources::Remove(smb_ImgData);
    Resources::Remove(dvd_ImgData);

	Resources::Remove(btnClick);

    for(int i = 0; i < MAXDEVICES; i++)
    {
        if(deviceBtn[i]) {
            delete deviceBtn[i];
            deviceBtn[i] = NULL;
        }
        if(deviceImgs[i]) {
            delete deviceImgs[i];
            deviceImgs[i] = NULL;
        }
        if(deviceImgOver[i]) {
            delete deviceImgOver[i];
            deviceImgOver[i] = NULL;
        }
        if(deviceText[i]) {
            delete deviceText[i];
            deviceText[i] = NULL;
        }
    }

    delete centerImg;
    delete leftImg;
    delete rightImg;

    delete NoBtn;

    delete trigA;
    delete trigB;

    ResumeGui();
}

int DeviceMenu::GetChoice()
{
    return choice;
}

void DeviceMenu::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == NoBtn)
    {
        choice = -2;
        return;
    }

    int i = 0;
    for(i = 0; i < deviceCount; i++)
    {
        if(sender == deviceBtn[i])
        {
            choice = deviceSelection[i];
            break;
        }
    }
}
