/****************************************************************************
 * Copyright (C) 2009
 * by LilouMaster
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
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "TextOperations/TextEditor.h"
#include "libwiigui/gui_optionbrowser.h"
#include "Menus/Explorer.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "filelist.h"
#include "FileOperations/filebrowser.h"
#include "FileOperations/fileops.h"
#include "devicemounter.h"
#include "FileStartUp/FileStartUp.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "network/networkops.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Language/gettext.h"
#include "Language/LanguageBrowser.h"
#include "network/update.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "sys.h"


extern "C" void sgIP_dbgprint(char * txt, ...) ;

extern  void HaltGui();
extern void ResumeGui();

GuiText *ftpDebugText[19];
GuiText *ftpMainStatusText;
GuiText *WWWAdressText;
GuiText *NintendoAdressText ;

#define FONTSIZE    18
extern s32 master_stop;



extern s32 SetTextfGlobal(const char *format, ...);
extern u32 launchServer(const char *drive, int status, bool stop) ;

        DeviceMenu * Device_Menu;
		        GuiButton * deviceSwitchBtn;

void OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

 Device_Menu = new DeviceMenu(deviceSwitchBtn->GetLeft()-5, deviceSwitchBtn->GetTop()+deviceSwitchBtn->GetHeight());
}



s32 SetTextfGlobal(const char *format, ...)
{
        va_list ap;
        int ret;

        va_start(ap, format);
        // ptrMainFileTxt->SetTextf(format, ap);
        va_end(ap);
        return ret;

}


extern void debug(const char * text)
{
ftpDebugText[0]->SetText(text);
}


extern void dbgprintsyntesis(const char * text)
{
ftpMainStatusText->SetText(text);
}


extern void dbgprintclientIP(const char * text)
{
WWWAdressText->SetText(text);
}

extern void dbgprintWiiIP(const char * text)
{
NintendoAdressText->SetText(text);
}


void printWii(u8 *buf, u8 x, u8 y, u16 color, char c)
{
if (y>=10 || x>=47) return;
char line[48];
line[47]=0;
memcpy(line, buf+(47*y),47);
ftpDebugText[y]->SetText(line);
};

void Werror(char *p)
 {

		WindowPrompt(tr(p), 0, tr("OK"));

}

void CheckDeviceMenu()
{
 }

const char * GetCurrentDevice(int currentDevice)
{
	switch(currentDevice)
	{
	    case SD:
            return "sd:/";
            break;
	    case USB:
            return "usb:/";
            break;
	    case NTFS0:
            return "ntfs0:/";
            break;
	    case NTFS1:
            return "ntfs1:/";
            break;
	    case NTFS2:
            return "ntfs2:/";
            break;
	    case NTFS3:
            return "ntfs3:/";
            break;
	    case NTFS4:
            return "ntfs4:/";
            break;
	    case SMB1:
            return "smb1:/";
            break;
	    case SMB2:
            return "smb2:/";
            break;
	    case SMB3:
            return "smb3:/";
            break;
	    case SMB4:
            return "smb4:/";
            break;
		default :
			return "sd:/";
			break;
//		case ISFS:
//			sprintf(browser.rootdir, "isfs:/");
//			break;
//		case NAND:
//			sprintf(browser.rootdir, "nand:/");
//			break;
	}

}

#define TOP_OFFSET 40

int ServerFtpGui()
{
	int menu = MENU_NONE;
	int ret;

    GuiImageData * sdstorage;
    GuiImageData * usbstorage;
    GuiImageData * networkstorage;
	int currentDevice =0;


	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png, button_png_size);
	GuiImageData btnOutlineOver(button_over_png, button_over_png_size);
	sdstorage = Resources::GetImageData(sdstorage_png, sdstorage_png_size);
	usbstorage = Resources::GetImageData(usbstorage_png, usbstorage_png_size);
	networkstorage = Resources::GetImageData(networkstorage_png, networkstorage_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -65);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiText LaunchBtnTxt(tr("Launch Ftp"), 22, (GXColor){0, 0, 0, 255});
	GuiText StopBtnTxt(   tr("Stop Ftp "), 22, (GXColor){0, 0, 0, 255});
	GuiImage updateBtnImg(&btnOutline);
	GuiButton updateBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	updateBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	updateBtn.SetPosition(-50, -65);
	updateBtn.SetLabel(&LaunchBtnTxt);
	updateBtn.SetImage(&updateBtnImg);
	updateBtn.SetSoundOver(&btnSoundOver);
	updateBtn.SetTrigger(&trigA);
	updateBtn.SetEffectGrow();

	GuiImageData settingsimgData(bg_cftp_png,bg_cftp_png_size);
	GuiImage settingsimg(&settingsimgData);
	settingsimg.SetPosition(20, TOP_OFFSET);

	u32 cr=0;
	u32 y =20;
	u32 x=40;
	char currentdir[50];
    snprintf(currentdir, sizeof(currentdir), "%s", GetCurrentDevice(currentDevice));
    GuiText *AdressText = new GuiText(currentdir, 20, (GXColor) {0, 0, 0, 255});
	AdressText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	AdressText->SetPosition( x+40, y*1+TOP_OFFSET);

	char NintendoAdress[50];
    snprintf(NintendoAdress, sizeof(NintendoAdress), "%s", "?");
    NintendoAdressText = new GuiText(NintendoAdress, 20, (GXColor) {0, 0, 0, 255});
	NintendoAdressText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	NintendoAdressText->SetPosition(175, y*1+TOP_OFFSET);

	char WWWAdress[50];
    snprintf(WWWAdress, sizeof(WWWAdress), "%s", "?");
    WWWAdressText = new GuiText(WWWAdress, 20, (GXColor) {0, 0, 0, 255});
	WWWAdressText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	WWWAdressText->SetPosition(425, y*1+TOP_OFFSET);

	char ftpMainStatus[50];
    snprintf(ftpMainStatus, sizeof(ftpMainStatus), "%s", "Not connected");
    ftpMainStatusText = new GuiText(ftpMainStatus, 20, (GXColor) {0, 0, 0, 255});
	ftpMainStatusText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	ftpMainStatusText->SetPosition(x, y*3+TOP_OFFSET);

	char ftpDebugStatus[50];
    snprintf(ftpDebugStatus, sizeof(ftpDebugStatus), "%s", "");


	for (u32 i=0;i<10;i++)
	{
	ftpDebugText[i] = new GuiText(ftpDebugStatus, 20, (GXColor) {0, 0, 0, 255});
	ftpDebugText[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	ftpDebugText[i]->SetPosition(x, (y*(5+i))+TOP_OFFSET);
	}

	u32 font = 22;
	u32 linestodraw = 10;


	GuiImage deviceImg = GuiImage(sdstorage);

	if(currentDevice > SD && currentDevice < SMB1)
        deviceImg.SetImage(usbstorage);
    else if(currentDevice >= SMB1)
        deviceImg.SetImage(networkstorage);

	GuiButton deviceSwitchBtn = GuiButton(deviceImg.GetWidth(), deviceImg.GetHeight());
	deviceSwitchBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	deviceSwitchBtn.SetPosition(40, 20+TOP_OFFSET);
	deviceSwitchBtn.SetImage(&deviceImg);
	deviceSwitchBtn.SetTrigger(&trigA);
	deviceSwitchBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsimg);
	w.Append(AdressText);
	w.Append(NintendoAdressText);
	w.Append(WWWAdressText);
	w.Append(ftpMainStatusText);
	w.Append(&updateBtn);
	w.Append(&backBtn);
	w.Append(&deviceSwitchBtn);

	for (u32 i=0;i<10;i++)
	{
	w.Append(ftpDebugText[i]);
	}


	MainWindow::Instance()->Append(&w);
    w.SetEffect(EFFECT_FADE, 50);
	ResumeGui();

	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

        if(shutdown == 1)
            Sys_Shutdown();

        else if(reset == 1)
            Sys_Reboot();

		else if(deviceSwitchBtn.GetState() == STATE_CLICKED)
		{
		DeviceMenu Device_Menu = DeviceMenu(deviceSwitchBtn.GetLeft()-5, deviceSwitchBtn.GetTop()+deviceSwitchBtn.GetHeight());
        w.Append(&Device_Menu);

        int device_choice = -1;
        while(device_choice == -1 )
        {
            usleep(100);
            if(shutdown == 1)
                Sys_Shutdown();

            else if(reset == 1)
                Sys_Reboot();

            device_choice = Device_Menu.GetChoice();
        }
		currentDevice = device_choice;
		if(currentDevice > SD && currentDevice < SMB1)
			deviceImg.SetImage(usbstorage);
		else if(currentDevice >= SMB1)
			deviceImg.SetImage(networkstorage);
		else deviceImg.SetImage(sdstorage);
		AdressText->SetText(GetCurrentDevice(currentDevice));

		deviceSwitchBtn.ResetState();
		}
		else if(backBtn.GetState() == STATE_CLICKED)
		{
		menu = MENU_BROWSE_DEVICE;
		}

        else if(updateBtn.GetState() == STATE_CLICKED)
		{
		cr=0;
		updateBtn.SetLabel(&StopBtnTxt);
		updateBtn.ResetState();
		while(updateBtn.GetState() != STATE_CLICKED ) {
		cr = launchServer(GetCurrentDevice(currentDevice), cr, false);
		}
		cr = launchServer(GetCurrentDevice(currentDevice), cr, true);
		updateBtn.SetLabel(&LaunchBtnTxt);
		updateBtn.ResetState();
		WindowPrompt(tr("Ftp stopped"), 0, tr("OK"));
		menu = MENU_BROWSE_DEVICE;
		}

        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

	}

    w.SetEffect(EFFECT_FADE, -50);
	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	HaltGui();
	delete AdressText;
	MainWindow::Instance()->Remove(&w);


	ResumeGui();

	return menu;
}

