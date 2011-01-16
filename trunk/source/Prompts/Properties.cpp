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
 * Properties.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include <sys/statvfs.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#include "Properties.h"
#include "sys.h"
#include "menu.h"
#include "Controls/MainWindow.h"
#include "FileOperations/fileops.h"

bool sizegainrunning = false;

Properties::Properties(ItemMarker * IMarker)
    : GuiWindow(0,0)
{
    int Position_X = 40;
    int Position_Y = 40;

    choice = -1;
    folder = false;
    FileCount = 0;
    OldSize = 0;
    TotalSize = 0;
    devicefree = 0;
    devicesize = 0;
    Marker = IMarker;
    foldersizethread = LWP_THREAD_NULL;

    for(int i = 0; i < Marker->GetItemcount(); i++)
    {
        if(Marker->IsItemDir(i))
        {
            folder = true;
        }
        else
        {
            TotalSize += Marker->GetItemSize(i);
            ++FileCount;
        }
    }

    OldSize = TotalSize;

    if(folder)
        StartGetFolderSizeThread();

    dialogBox = Resources::GetImageData(bg_properties_png, bg_properties_png_size);
    titleData = Resources::GetImageData(icon_folder_png, icon_folder_png_size);
    arrowUp = Resources::GetImageData(close_png, close_png_size);
    arrowUpOver = Resources::GetImageData(close_over_png, close_over_png_size);

    btnClick = Resources::GetSound(button_click_wav, button_click_wav_size);

    dialogBoxImg = new GuiImage(dialogBox);

    width = dialogBox->GetWidth();
    height = dialogBox->GetHeight();

	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);

    const char * filename = Marker->GetItemName(Marker->GetItemcount()-1);

    TitleTxt = new GuiText(filename, 22, (GXColor){0, 0, 0, 255});
    TitleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    TitleTxt->SetPosition(0, Position_Y);
    TitleTxt->SetMaxWidth(dialogBox->GetWidth()-Position_X, DOTTED);

    TitleImg = new GuiImage(titleData);
    TitleImg->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    int IconPosition = -(TitleTxt->GetTextWidth()/2+titleData->GetWidth())+10;
    if(IconPosition < (30-width/2))
        IconPosition = 30-width/2;
    TitleImg->SetPosition(IconPosition, Position_Y);
    Position_Y += 50;

    filepathTxt =  new GuiText(tr("Filepath:"), 20, (GXColor){0, 0, 0, 255});
    filepathTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filepathTxt->SetPosition(Position_X, Position_Y);

    filepathvalTxt =  new GuiText(Marker->GetItemPath(Marker->GetItemcount()-1), 20, (GXColor){0, 0, 0, 255});
    filepathvalTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filepathvalTxt->SetPosition(Position_X+80, Position_Y);
    filepathvalTxt->SetMaxWidth(dialogBox->GetWidth()-Position_X-130, SCROLL_HORIZONTAL);
    Position_Y += 30;

    filecountTxt = new GuiText(tr("Files:"), 20, (GXColor){0, 0, 0, 255});
    filecountTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filecountTxt->SetPosition(Position_X, Position_Y);

    filecountTxtVal = new GuiText(fmt("%i", FileCount), 20, (GXColor){0, 0, 0, 255});
    filecountTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filecountTxtVal->SetPosition(Position_X+180, Position_Y);
    Position_Y += 30;

    char temp[MAXPATHLEN];

    if(OldSize > KBSIZE && OldSize < MBSIZE)
        sprintf(temp, "%0.2fKB", OldSize/KBSIZE);
    else if(OldSize > MBSIZE && OldSize < GBSIZE)
        sprintf(temp, "%0.2fMB", OldSize/MBSIZE);
    else if(OldSize > GBSIZE)
        sprintf(temp, "%0.2fGB", OldSize/GBSIZE);
    else
        sprintf(temp, "%LiB", OldSize);

    filesizeTxt = new GuiText(tr("Size:"), 20, (GXColor){0, 0, 0, 255});
    filesizeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filesizeTxt->SetPosition(Position_X, Position_Y);

    filesizeTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
    filesizeTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filesizeTxtVal->SetPosition(Position_X+180, Position_Y);
    Position_Y += 30;

    char * pch = NULL;
    if(Marker->GetItemcount() > 1)
    {
        snprintf(temp, sizeof(temp), tr("Multiple Items"));
    }
    else if(folder)
    {
        snprintf(temp, sizeof(temp), tr("Folder(s)"));
        TitleTxt->SetMaxWidth(dialogBox->GetWidth()-75, DOTTED);
    }
    else
    {
        if(filename)
            pch = strrchr(filename, '.');
        if(pch)
            pch += 1;
        else
            pch = (char *) filename;
        snprintf(temp, sizeof(temp), "%s", pch);
    }

    filetypeTxt = new GuiText(tr("Filetype:"), 20, (GXColor){0, 0, 0, 255});
    filetypeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filetypeTxt->SetPosition(Position_X, Position_Y);

    filetypeTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
    filetypeTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filetypeTxtVal->SetPosition(Position_X+180, Position_Y);
    Position_Y += 30;

    devicefreeTxt = NULL;
    devicefreeTxtVal = NULL;
    devicetotalTxt = NULL;
    devicetotalTxtVal = NULL;

    if(folder)
    {
        devicefreeTxt = new GuiText(tr("Free Space:"), 20, (GXColor){0, 0, 0, 255});
        devicefreeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        devicefreeTxt->SetPosition(Position_X, Position_Y);

        devicefreeTxtVal = new GuiText("0B", 20, (GXColor){0, 0, 0, 255});
        devicefreeTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        devicefreeTxtVal->SetPosition(Position_X+180, Position_Y);
        Position_Y += 30;

        devicetotalTxt = new GuiText(tr("Total Space:"), 20, (GXColor){0, 0, 0, 255});
        devicetotalTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        devicetotalTxt->SetPosition(Position_X, Position_Y);

        devicetotalTxtVal = new GuiText("0B", 20, (GXColor){0, 0, 0, 255});
        devicetotalTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        devicetotalTxtVal->SetPosition(Position_X+180, Position_Y);
        Position_Y += 30;
    }

    struct stat filestat;
    memset(&filestat, 0, sizeof(struct stat));
    if(Marker->GetItemcount() > 0)
        stat(Marker->GetItemPath(Marker->GetItemcount()-1), &filestat);

    last_accessTxt = new GuiText(tr("Last access:"), 20, (GXColor){0, 0, 0, 255});
    last_accessTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_accessTxt->SetPosition(Position_X, Position_Y);

    strftime(temp, sizeof(temp), "%H:%M  %d %b %Y", localtime(&filestat.st_atime));
    last_accessTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
    last_accessTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_accessTxtVal->SetPosition(Position_X+180, Position_Y);
    Position_Y += 30;

    last_modifTxt = new GuiText(tr("Last modified:"), 20, (GXColor){0, 0, 0, 255});
    last_modifTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_modifTxt->SetPosition(Position_X, Position_Y);

    strftime(temp, sizeof(temp), "%H:%M  %d %b %Y", localtime(&filestat.st_mtime));
    last_modifTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
    last_modifTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_modifTxtVal->SetPosition(Position_X+180, Position_Y);
    Position_Y += 30;

    last_changeTxt = new GuiText(tr("Last change:"), 20, (GXColor){0, 0, 0, 255});
    last_changeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_changeTxt->SetPosition(Position_X, Position_Y);

    strftime(temp, sizeof(temp), "%H:%M  %d %b %Y", localtime(&filestat.st_ctime));
    last_changeTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
    last_changeTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    last_changeTxtVal->SetPosition(Position_X+180, Position_Y);

    arrowUpImg = new GuiImage(arrowUp);
    arrowUpImgOver = new GuiImage(arrowUpOver);
    CloseBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
    CloseBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    CloseBtn->SetSoundClick(btnClick);
    CloseBtn->SetImage(arrowUpImg);
    CloseBtn->SetImageOver(arrowUpImgOver);
    CloseBtn->SetPosition(-20, 20);
    CloseBtn->SetEffectGrow();
    CloseBtn->SetTrigger(trigA);
    CloseBtn->SetTrigger(trigB);
    CloseBtn->Clicked.connect(this, &Properties::OnButtonClick);

    Append(dialogBoxImg);
    Append(TitleTxt);
    if(Marker->IsItemDir(Marker->GetItemcount()-1))
        Append(TitleImg);
    Append(filepathTxt);
    Append(filepathvalTxt);
    Append(filecountTxt);
    Append(filecountTxtVal);
    Append(filesizeTxt);
    Append(filesizeTxtVal);
    Append(filetypeTxt);
    Append(filetypeTxtVal);
    if(folder)
    {
        Append(devicefreeTxt);
        Append(devicefreeTxtVal);
        Append(devicetotalTxt);
        Append(devicetotalTxtVal);
    }
    Append(last_accessTxt);
    Append(last_accessTxtVal);
    Append(last_modifTxt);
    Append(last_modifTxtVal);
    Append(last_changeTxt);
    Append(last_changeTxtVal);
    Append(CloseBtn);

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 40);
}

Properties::~Properties()
{
    MainWindow::Instance()->ResumeGui();

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 40);
    while(this->GetEffect() > 0)
        usleep(THREAD_SLEEP);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

    if(foldersizethread != LWP_THREAD_NULL)
        StopSizeThread();

	Resources::Remove(dialogBox);
	Resources::Remove(titleData);
	Resources::Remove(arrowUp);
	Resources::Remove(arrowUpOver);
	Resources::Remove(btnClick);

	delete dialogBoxImg;
	delete TitleImg;
	delete arrowUpImg;
	delete arrowUpImgOver;
	delete TitleTxt;
	delete filepathTxt;
	delete filepathvalTxt;
	delete filecountTxt;
	delete filecountTxtVal;
	delete filesizeTxt;
	delete filesizeTxtVal;
	delete filetypeTxt;
	delete filetypeTxtVal;
	if(devicefreeTxt)
        delete devicefreeTxt;
	if(devicefreeTxtVal)
        delete devicefreeTxtVal;
	if(devicetotalTxt)
        delete devicetotalTxt;
	if(devicetotalTxtVal)
        delete devicetotalTxtVal;
	delete last_accessTxt;
	delete last_accessTxtVal;
	delete last_modifTxt;
	delete last_modifTxtVal;
	delete last_changeTxt;
	delete last_changeTxtVal;

	delete CloseBtn;
	delete trigA;
	delete trigB;

    MainWindow::Instance()->SetDim(false);
    MainWindow::Instance()->SetState(STATE_DEFAULT);
	MainWindow::Instance()->ResumeGui();
}

int Properties::GetChoice()
{
    if(shutdown)
        Sys_Shutdown();
    else if(reset)
        Sys_Reboot();

    if(folder)
    {
        if(OldSize != TotalSize)
        {
            UpdateSizeValue();
        }
    }

    return choice;
}

void Properties::OnButtonClick(GuiElement *sender, int pointer UNUSED, POINT p UNUSED)
{
    sender->ResetState();

    if(sender == CloseBtn)
    {
        choice = -2;
    }
}

void Properties::UpdateSizeValue()
{
    OldSize = TotalSize;
    char sizetext[20];
    char filecounttext[20];

    if(OldSize > KBSIZE && OldSize < MBSIZE)
        snprintf(sizetext, sizeof(sizetext), "%0.2fKB", OldSize/KBSIZE);
    else if(OldSize > MBSIZE && OldSize < GBSIZE)
        snprintf(sizetext, sizeof(sizetext), "%0.2fMB", OldSize/MBSIZE);
    else if(OldSize > GBSIZE)
        snprintf(sizetext, sizeof(sizetext), "%0.2fGB", OldSize/GBSIZE);
    else
        snprintf(sizetext, sizeof(sizetext), "%LiB", OldSize);

    filesizeTxtVal->SetText(sizetext);
    snprintf(filecounttext, sizeof(filecounttext), "%i", FileCount);
    filecountTxtVal->SetText(filecounttext);

    if(devicefree > 0 && devicesize > 0)
    {
        if(devicefree > KBSIZE && devicefree < MBSIZE)
            snprintf(sizetext, sizeof(sizetext), "%0.2fKB", devicefree/KBSIZE);
        else if(devicefree > MBSIZE && devicefree < GBSIZE)
            snprintf(sizetext, sizeof(sizetext), "%0.2fMB", devicefree/MBSIZE);
        else if(devicefree > GBSIZE)
            snprintf(sizetext, sizeof(sizetext), "%0.2fGB", devicefree/GBSIZE);
        else
            snprintf(sizetext, sizeof(sizetext), "%LiB", devicefree);

        devicefreeTxtVal->SetText(sizetext);

        if(devicesize > KBSIZE && devicesize < MBSIZE)
            snprintf(sizetext, sizeof(sizetext), "%0.2fKB", devicesize/KBSIZE);
        else if(devicesize > MBSIZE && devicesize < GBSIZE)
            snprintf(sizetext, sizeof(sizetext), "%0.2fMB", devicesize/MBSIZE);
        else if(devicesize > GBSIZE)
            snprintf(sizetext, sizeof(sizetext), "%0.2fGB", devicesize/GBSIZE);
        else
            snprintf(sizetext, sizeof(sizetext), "%LiB", devicesize);

        devicetotalTxtVal->SetText(sizetext);
    }
}

/****************************************************************************
 * GetCurrentFolderSize in the background
 ***************************************************************************/
void Properties::InternalFolderSizeGain()
{
    bool gotDeviceSize = false;

    for(int i = 0; i < Marker->GetItemcount(); i++)
    {
        if(!Marker->IsItemDir(i))
            continue;

        char folderpath[1024];
        snprintf(folderpath, sizeof(folderpath), "%s/", Marker->GetItemPath(i));

        if(!gotDeviceSize)
        {
            //background capacity getting because it is very slow
            struct statvfs stats;
            memset(&stats, 0, sizeof(stats));
            memcpy(&stats.f_flag, "SCAN", 4);
            statvfs(folderpath, &stats);

            devicefree = (u64)stats.f_frsize * (u64)stats.f_bfree;
            devicesize = (u64)stats.f_frsize * (u64)stats.f_blocks;

            gotDeviceSize = true;
        }

        GetFolderSize(folderpath, TotalSize, FileCount);
    }

    UpdateSizeValue();
}

void * Properties::FolderSizeThread(void *arg)
{
	((Properties *) arg)->InternalFolderSizeGain();
    return NULL;
}

void Properties::StartGetFolderSizeThread()
{
    sizegainrunning = true;

    //!Initialize GetSizeThread for Properties
	LWP_CreateThread(&foldersizethread, FolderSizeThread, this, NULL, 32768, 60);
}

void Properties::StopSizeThread()
{
    sizegainrunning = false;
	LWP_JoinThread(foldersizethread, NULL);
	foldersizethread = LWP_THREAD_NULL;
}
