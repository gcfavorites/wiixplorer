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

Properties::Properties(const char * filepath)
    :GuiWindow(0,0)
{
    int Position_X = 40;
    int Position_Y = 40;

    choice = -1;
    folder = false;
    foldersize = 0;
    oldfoldersize = 0;
    filecount = 0;
    devicefree = 0;
    devicesize = 0;

    if(!filepath)
        return;

    char temp[MAXPATHLEN];
    foldersizethread = LWP_THREAD_NULL;
    folderpath = new char[strlen(filepath)+2];
    sprintf(folderpath, "%s/", filepath);

    struct stat filestat;
    stat(filepath, &filestat);

    filesize = filestat.st_size;
    folder = (filestat.st_mode & _IFDIR) == 0 ? false : true;

    if(folder)
    {
        StartGetFolderSizeThread();
    }

    dialogBox = Resources::GetImageData(bg_properties_png, bg_properties_png_size);
    titleData = Resources::GetImageData(icon_folder_png, icon_folder_png_size);
    arrowUp = Resources::GetImageData(close_png, close_png_size);
    arrowUpOver = Resources::GetImageData(close_over_png, close_over_png_size);

    btnClick = Resources::GetSound(button_click_wav, button_click_wav_size);

    dialogBoxImg = new GuiImage(dialogBox);

    width = dialogBox->GetWidth();
    height = dialogBox->GetHeight();

	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger();
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    char * filename = strrchr(filepath, '/')+1;
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

    sprintf(temp, tr("Filepath:  %s"), filepath);
    filepathTxt =  new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
    filepathTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filepathTxt->SetPosition(Position_X, Position_Y);
    filepathTxt->SetMaxWidth(dialogBox->GetWidth()-Position_X-10, DOTTED);
    Position_Y += 30;

    filecountTxt = new GuiText(tr("Files:"), 20, (GXColor){0, 0, 0, 255});
    filecountTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filecountTxt->SetPosition(Position_X, Position_Y);

    filecountTxtVal = new GuiText(tr("1"), 20, (GXColor){0, 0, 0, 255});
    filecountTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filecountTxtVal->SetPosition(Position_X+180, Position_Y);
    Position_Y += 30;

    if(filesize > KBSIZE && filesize < MBSIZE)
        sprintf(temp, "%0.2fKB", filesize/KBSIZE);
    else if(filesize > MBSIZE && filesize < GBSIZE)
        sprintf(temp, "%0.2fMB", filesize/MBSIZE);
    else if(filesize > GBSIZE)
        sprintf(temp, "%0.2fGB", filesize/GBSIZE);
    else
        sprintf(temp, "%LiB", filesize);

    filesizeTxt = new GuiText(tr("Size:"), 20, (GXColor){0, 0, 0, 255});
    filesizeTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filesizeTxt->SetPosition(Position_X, Position_Y);

    filesizeTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
    filesizeTxtVal->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    filesizeTxtVal->SetPosition(Position_X+180, Position_Y);
    Position_Y += 30;

    char * pch = NULL;
    if(folder)
    {
        snprintf(temp, sizeof(temp), tr("Folder"));
        TitleTxt->SetMaxWidth(dialogBox->GetWidth()-75, DOTTED);
    }
    else
    {
        pch = strrchr(filename, '.');
        if(pch)
            pch += 1;
        else
            pch = filename;
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
    if(folder)
        Append(TitleImg);
    Append(filepathTxt);
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
    if(folderpath)
        delete [] folderpath;

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
        if(oldfoldersize != foldersize)
        {
            UpdateSizeValue();
        }
    }

    return choice;
}

void Properties::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == CloseBtn)
    {
        choice = -2;
    }
}

void Properties::UpdateSizeValue()
{
    oldfoldersize = foldersize;
    char sizetext[20];
    char filecounttext[20];

    if(foldersize > KBSIZE && foldersize < MBSIZE)
        snprintf(sizetext, sizeof(sizetext), "%0.2fKB", foldersize/KBSIZE);
    else if(foldersize > MBSIZE && foldersize < GBSIZE)
        snprintf(sizetext, sizeof(sizetext), "%0.2fMB", foldersize/MBSIZE);
    else if(foldersize > GBSIZE)
        snprintf(sizetext, sizeof(sizetext), "%0.2fGB", foldersize/GBSIZE);
    else
        snprintf(sizetext, sizeof(sizetext), "%LiB", foldersize);

    filesizeTxtVal->SetText(sizetext);
    snprintf(filecounttext, sizeof(filecounttext), "%i", filecount);
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
    //background capacity getting because it is very slow
    struct statvfs stats;
    statvfs(folderpath, &stats);

    devicefree = (u64)stats.f_frsize * (u64)stats.f_bfree;
	devicesize = (u64)stats.f_frsize * (u64)stats.f_blocks;

    GetFolderSize(folderpath, foldersize, filecount);
    UpdateSizeValue();
}

void * Properties::FolderSizeThread(void *arg)
{
	((Properties *) arg)->InternalFolderSizeGain();
    return NULL;
}

void Properties::StartGetFolderSizeThread()
{
    foldersize = 0;
    oldfoldersize = 0;
    filecount = 0;
    sizegainrunning = true;

    //!Initialize GetSizeThread for Properties
	LWP_CreateThread(&foldersizethread, FolderSizeThread, this, NULL, 0, 60);
}

void Properties::StopSizeThread()
{
    sizegainrunning = false;
	LWP_JoinThread(foldersizethread, NULL);
	foldersizethread = LWP_THREAD_NULL;
}
