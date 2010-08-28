/***************************************************************************
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
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"
#include "Prompts/ProgressWindow.h"
#include "Prompts/PromptWindows.h"
#include "TextOperations/wstring.hpp"
#include "PartitionFormatterGUI.hpp"
#include "PartitionFormatter.hpp"
#include "menu.h"
#include "sys.h"

PartitionFormatterGui::PartitionFormatterGui()
    : GuiWindow(0,0)
{
    CurPart = 0;
    CurDevice = SD;
    currentState = -1;
    FormatRequested = false;
    ActiveRequested = false;
    Device = NULL;

	btnClick = Resources::GetSound(button_click_wav, button_click_wav_size);
	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);

    bgWindow = Resources::GetImageData(bg_properties_png, bg_properties_png_size);
    bgWindowImg = new GuiImage(bgWindow);
    width = bgWindow->GetWidth();
    height = bgWindow->GetHeight();
    Append(bgWindowImg);

    btnOutline = Resources::GetImageData(small_button_png, small_button_png_size);
    CloseImgData = Resources::GetImageData(close_png, close_png_size);
    CloseImgOverData = Resources::GetImageData(close_over_png, close_over_png_size);

	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);

    TitleTxt = new GuiText(tr("Partition Formatter"), 22, (GXColor){0, 0, 0, 255});
    TitleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    TitleTxt->SetPosition(0, 25);
	Append(TitleTxt);

    /** Options **/
    DeviceOption.ClickedLeft.connect(this, &PartitionFormatterGui::OnOptionLeftClick);
    DeviceOption.ClickedRight.connect(this, &PartitionFormatterGui::OnOptionRightClick);
    DeviceOption.AddOption(tr("Device"), 150, 70);
    DeviceOption.AddOption(tr("Partition Nr."), 280, 70);
    DeviceOption.SetScale(1.25);
    Append(&DeviceOption);

    int PositionY = 150;
    int PositionX = 50;
    int PositionXVal = 260;

    MountNameTxt = new GuiText(tr("Mounted as:"), 20, (GXColor){0, 0, 0, 255});
    MountNameTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    MountNameTxt->SetPosition(PositionX, PositionY);
	Append(MountNameTxt);

    MountNameValTxt = new GuiText((char *) NULL, 20, (GXColor){0, 0, 0, 255});
    MountNameValTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    MountNameValTxt->SetPosition(PositionXVal, PositionY);
	Append(MountNameValTxt);

    PositionY += 30;

    PartitionTxt = new GuiText(tr("Partition Type:"), 20, (GXColor){0, 0, 0, 255});
    PartitionTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartitionTxt->SetPosition(PositionX, PositionY);
	Append(PartitionTxt);

    PartitionValTxt = new GuiText((char *) NULL, 20, (GXColor){0, 0, 0, 255});
    PartitionValTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartitionValTxt->SetPosition(PositionXVal, PositionY);
	Append(PartitionValTxt);

    PositionY += 30;

    PartActiveTxt = new GuiText(tr("Active (Bootable):"), 20, (GXColor){0, 0, 0, 255});
    PartActiveTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartActiveTxt->SetPosition(PositionX, PositionY);
	Append(PartActiveTxt);

    PartActiveValTxt = new GuiText((char *) NULL, 20, (GXColor){0, 0, 0, 255});
    PartActiveValTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartActiveValTxt->SetPosition(PositionXVal, PositionY);
	Append(PartActiveValTxt);

	PositionY += 30;

    PartTypeTxt = new GuiText(tr("File System:"), 20, (GXColor){0, 0, 0, 255});
    PartTypeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartTypeTxt->SetPosition(PositionX, PositionY);
	Append(PartTypeTxt);

    PartTypeValTxt = new GuiText((char *) NULL, 20, (GXColor){0, 0, 0, 255});
    PartTypeValTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartTypeValTxt->SetPosition(PositionXVal, PositionY);
	Append(PartTypeValTxt);

	PositionY += 30;

    PartSizeTxt = new GuiText(tr("Partition Size:"), 20, (GXColor){0, 0, 0, 255});
    PartSizeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartSizeTxt->SetPosition(PositionX, PositionY);
	Append(PartSizeTxt);

    PartSizeValTxt = new GuiText((char *) NULL, 20, (GXColor){0, 0, 0, 255});
    PartSizeValTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    PartSizeValTxt->SetPosition(PositionXVal, PositionY);
	Append(PartSizeValTxt);

    ActiveBtnTxt = new GuiText(tr("Set Active"), 16, (GXColor){0, 0, 0, 255});
    ActiveBtnImg = new GuiImage(btnOutline);
    ActiveBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
    ActiveBtn->SetLabel(ActiveBtnTxt);
    ActiveBtn->SetImage(ActiveBtnImg);
    ActiveBtn->SetSoundOver(btnSoundOver);
    ActiveBtn->SetSoundClick(btnClick);
    ActiveBtn->SetTrigger(trigA);
    ActiveBtn->SetPosition(80, 320);
    ActiveBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    ActiveBtn->SetEffectGrow();
	ActiveBtn->Clicked.connect(this, &PartitionFormatterGui::OnButtonClick);
	Append(ActiveBtn);

    FormatBtnTxt = new GuiText(tr("Format"), 16, (GXColor){0, 0, 0, 255});
    FormatBtnImg = new GuiImage(btnOutline);
    FormatBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
    FormatBtn->SetLabel(FormatBtnTxt);
    FormatBtn->SetImage(FormatBtnImg);
    FormatBtn->SetSoundOver(btnSoundOver);
    FormatBtn->SetSoundClick(btnClick);
    FormatBtn->SetTrigger(trigA);
    FormatBtn->SetPosition(-80, 320);
    FormatBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    FormatBtn->SetEffectGrow();
	FormatBtn->Clicked.connect(this, &PartitionFormatterGui::OnButtonClick);
	Append(FormatBtn);

    BackBtnImg = new GuiImage(CloseImgData);
    BackBtnImgOver = new GuiImage(CloseImgOverData);
    BackBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
    BackBtn->SetImage(BackBtnImg);
    BackBtn->SetImageOver(BackBtnImgOver);
    BackBtn->SetSoundOver(btnSoundOver);
    BackBtn->SetSoundClick(btnClick);
    BackBtn->SetTrigger(trigA);
    BackBtn->SetTrigger(trigB);
    BackBtn->SetPosition(370, 20);
    BackBtn->SetEffectGrow();
	BackBtn->Clicked.connect(this, &PartitionFormatterGui::OnButtonClick);
	Append(BackBtn);

    SetDevice();
    ListPartitions();

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
}

PartitionFormatterGui::~PartitionFormatterGui()
{
    MainWindow::Instance()->ResumeGui();

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);

    while(GetEffect() > 0)
        usleep(100);

    MainWindow::Instance()->HaltGui();

    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    MainWindow::Instance()->ResumeGui();

    RemoveAll();

    delete ActiveBtn;
    delete BackBtn;
    delete FormatBtn;

    delete TitleTxt;
    delete ActiveBtnTxt;
    delete FormatBtnTxt;
    delete MountNameTxt;
    delete PartitionTxt;
    delete PartActiveTxt;
    delete PartTypeTxt;
    delete PartSizeTxt;
    delete MountNameValTxt;
    delete PartitionValTxt;
    delete PartActiveValTxt;
    delete PartTypeValTxt;
    delete PartSizeValTxt;

    delete bgWindowImg;
    delete ActiveBtnImg;
    delete FormatBtnImg;
    delete BackBtnImg;
    delete BackBtnImgOver;

    Resources::Remove(btnClick);
    Resources::Remove(btnSoundOver);

    Resources::Remove(bgWindow);
    Resources::Remove(btnOutline);
    Resources::Remove(CloseImgData);
    Resources::Remove(CloseImgOverData);

    delete trigA;
    delete trigB;
}

void PartitionFormatterGui::ResetMainDim()
{
    MainWindow::Instance()->HaltGui();
    MainWindow::Instance()->Remove(this);
    MainWindow::Instance()->SetDim(true);
    MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->Append(this);
    this->SetDim(false);
    this->SetState(STATE_DEFAULT);
    MainWindow::Instance()->ResumeGui();
}

void PartitionFormatterGui::MainUpdate()
{
    while(currentState == -1)
    {
        usleep(100);

		if(shutdown)
			Sys_Shutdown();
		else if(reset)
			Sys_Reboot();

        if(FormatRequested && Device)
        {
            int ret = WindowPrompt(tr("Format Partition?"), fmt("%s %i: %s (%0.2fGB)", tr("Partition"), CurPart, Device->GetFSName(CurPart), Device->GetSize(CurPart)/GBSIZE), tr("Yes"), tr("Cancel"));
            ResetMainDim();
            if(ret)
            {
                ret = WindowPrompt(tr("WARNING"), tr("This will delete all the data on this partition. Do you want to continue?"), tr("Yes"), tr("Cancel"));
                ResetMainDim();
                if(ret)
                {
                    StartProgress(tr("Formating partition..."), AUTO_THROBBER);
                    ShowProgress(0, 2, tr("Please wait..."));
                    ret = PartitionFormatter::FormatToFAT32(Device->GetDiscInterface(), Device->GetLBAStart(CurPart), Device->GetSecCount(CurPart));
                    if(ret >= 0)
                    {
                        if(Device->GetEBRSector(CurPart) > 0)
                            ret = PartitionFormatter::WriteEBR_FAT32(Device->GetDiscInterface(), Device->GetEBRSector(CurPart), Device->GetLBAStart(CurPart));
                        else
                            ret = PartitionFormatter::WriteMBR_FAT32(Device->GetDiscInterface(), Device->GetLBAStart(CurPart));
                    }
                    ShowProgress(1, 2, tr("Remounting devices..."));
                    DeviceHandler::Instance()->UnMountAll();
                    DeviceHandler::Instance()->MountAll();
                    StopProgress();
                    ResetMainDim();
                    if(ret >= 0)
                        WindowPrompt(tr("Successfully formated the partition."), 0, tr("OK"));
                    ResetMainDim();
                }
            }
            SetDevice();
            ListPartitions();
            FormatRequested = false;
        }
        else if(ActiveRequested && Device)
        {
            ActiveRequested = false;
            if(Device->GetEBRSector(CurPart) > 0)
            {
                ShowError(tr("You can't set a logical partition as active."));
                ResetMainDim();
                continue;
            }
            else if(Device->GetPartitionType(CurPart) == 0x05 || Device->GetPartitionType(CurPart) == 0x0F ||
                    Device->GetPartitionType(CurPart) == 0x00)
            {
                ShowError(tr("You can't set this partition as active."));
                ResetMainDim();
                continue;
            }

            int ret = WindowPrompt(tr("Set this Partition as Active?"), fmt("%s %i: %s (%0.2fGB)", tr("Partition"), CurPart, Device->GetFSName(CurPart), Device->GetSize(CurPart)/GBSIZE), tr("Yes"), tr("Cancel"));
            ResetMainDim();
            if(ret)
            {
                StartProgress(tr("Setting partition as active."), AUTO_THROBBER);
                ShowProgress(0, 1, tr("Please wait..."));
                ret = PartitionFormatter::SetActive(Device->GetDiscInterface(), CurPart);
                Device->GetPartitionRecord(CurPart)->Bootable = true;
                StopProgress();
                ResetMainDim();
            }

            SetDevice();
            ListPartitions();
        }
    }
}

void PartitionFormatterGui::ListPartitions()
{
    if(!Device)
        return;

    if(CurPart < 0 || CurPart >= Device->GetPartitionCount())
        return;

    char MountName[10];
    if(Device->MountName(CurPart))
        sprintf(MountName, "%s:/", Device->MountName(CurPart));

    MountNameValTxt->SetText(Device->MountName(CurPart) ? MountName : tr("Not mounted"));
    PartitionValTxt->SetText(Device->GetEBRSector(CurPart) > 0 ? tr("Logical") : tr("Primary"));
    PartActiveValTxt->SetText(Device->IsActive(CurPart) ? tr("Yes"): tr("No"));
    PartTypeValTxt->SetText(fmt("%s (0x%02X)", Device->GetFSName(CurPart), Device->GetPartitionType(CurPart)));
    PartSizeValTxt->SetTextf("%0.2fGB", Device->GetSize(CurPart)/GBSIZE);
}

void PartitionFormatterGui::SetDevice()
{
    if(CurDevice == SD)
    {
        DeviceOption.SetOptionValue(0, tr("SD-Card"));
        Device = (PartitionHandle *) DeviceHandler::Instance()->GetSDHandle();
    }
    else if(CurDevice == GCSDA)
    {
        DeviceOption.SetOptionValue(0, tr("GC-SD A"));
        Device = (PartitionHandle *) DeviceHandler::Instance()->GetGCAHandle();
    }
    else if(CurDevice == GCSDB)
    {
        DeviceOption.SetOptionValue(0, tr("GC-SD B"));
        Device = (PartitionHandle *) DeviceHandler::Instance()->GetGCBHandle();
    }
    else if(CurDevice >= USB1)
    {
        DeviceOption.SetOptionValue(0, tr("USB Device"));
        Device = (PartitionHandle *) DeviceHandler::Instance()->GetUSBHandle();
    }

    DeviceOption.SetOptionValue(1, fmt("%i", CurPart+1));
}

void PartitionFormatterGui::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == BackBtn)
    {
        currentState = -2;
    }
    else if(sender == FormatBtn)
    {
        FormatRequested = true;
    }
    else if(sender == ActiveBtn)
    {
        ActiveRequested = true;
    }
}

void PartitionFormatterGui::OnOptionLeftClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == DeviceOption.GetButtonLeft(0))
    {
        if(CurDevice == SD)
        {
            if(DeviceHandler::Instance()->GCB_Inserted())
                CurDevice = GCSDB;
            else if(DeviceHandler::Instance()->GCA_Inserted())
                CurDevice = GCSDA;
            else if(DeviceHandler::Instance()->USB_Inserted())
                CurDevice = USB1;
        }
        else if(CurDevice == GCSDB)
        {
            if(DeviceHandler::Instance()->GCA_Inserted())
                CurDevice = GCSDA;
            else if(DeviceHandler::Instance()->USB_Inserted())
                CurDevice = USB1;
            else if(DeviceHandler::Instance()->SD_Inserted())
                CurDevice = SD;
        }
        else if(CurDevice == GCSDA)
        {
            if(DeviceHandler::Instance()->USB_Inserted())
                CurDevice = USB1;
            else if(DeviceHandler::Instance()->SD_Inserted())
                CurDevice = SD;
            else if(DeviceHandler::Instance()->GCB_Inserted())
                CurDevice = GCSDB;
        }
        else if(CurDevice >= USB1)
        {
            if(DeviceHandler::Instance()->SD_Inserted())
                CurDevice = SD;
            else if(DeviceHandler::Instance()->GCB_Inserted())
                CurDevice = GCSDB;
            else if(DeviceHandler::Instance()->GCA_Inserted())
                CurDevice = GCSDA;
        }
    }
    else if(sender == DeviceOption.GetButtonLeft(1))
    {
        if(CurPart > 0)
            --CurPart;
    }

    SetDevice();
    ListPartitions();
}

void PartitionFormatterGui::OnOptionRightClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == DeviceOption.GetButtonRight(0))
    {
        if(CurDevice == SD)
        {
            if(DeviceHandler::Instance()->USB_Inserted())
                CurDevice = USB1;
            else if(DeviceHandler::Instance()->GCA_Inserted())
                CurDevice = GCSDA;
            else if(DeviceHandler::Instance()->GCB_Inserted())
                CurDevice = GCSDB;
        }
        else if(CurDevice >= USB1)
        {
            if(DeviceHandler::Instance()->GCA_Inserted())
                CurDevice = GCSDA;
            else if(DeviceHandler::Instance()->GCB_Inserted())
                CurDevice = GCSDB;
            else if(DeviceHandler::Instance()->SD_Inserted())
                CurDevice = SD;
        }
        else if(CurDevice == GCSDA)
        {
            if(DeviceHandler::Instance()->GCB_Inserted())
                CurDevice = GCSDB;
            else if(DeviceHandler::Instance()->SD_Inserted())
                CurDevice = SD;
            else if(DeviceHandler::Instance()->USB_Inserted())
                CurDevice = USB1;
        }
        else if(CurDevice == GCSDB)
        {
            if(DeviceHandler::Instance()->SD_Inserted())
                CurDevice = SD;
            else if(DeviceHandler::Instance()->USB_Inserted())
                CurDevice = USB1;
            else if(DeviceHandler::Instance()->GCA_Inserted())
                CurDevice = GCSDA;
        }
    }
    else if(sender == DeviceOption.GetButtonRight(1))
    {
        if(Device && CurPart+1 < Device->GetPartitionCount())
            ++CurPart;
    }

    SetDevice();
    ListPartitions();
}

