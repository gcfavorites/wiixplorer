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
 * ArchiveProperties.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include <sys/statvfs.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#include "ArchiveProperties.h"
#include "sys.h"
#include "menu.h"
#include "Prompts/PromptWindows.h"

ArchiveProperties::ArchiveProperties(ArchiveFileStruct * ArcFile)
	:GuiFrame(0,0)
{
	int Position_X = 40;
	int Position_Y = 40;
	char temp[MAXPATHLEN];

	choice = -1;
	foldersize = 0;
	oldfoldersize = 0;
	filecount = 0;
	memset(&ArchiveFile, 0, sizeof(ArchiveFileStruct));

	if(!ArcFile)
		return;

	ArchiveFile.filename = new char[strlen(ArcFile->filename)+1];
	sprintf(ArchiveFile.filename, "%s",ArcFile->filename);
	ArchiveFile.length = ArcFile->length;
	ArchiveFile.comp_length = ArcFile->comp_length;
	ArchiveFile.isdir = ArcFile->isdir;
	ArchiveFile.fileindex = ArcFile->fileindex;
	ArchiveFile.ModTime = ArcFile->ModTime;
	ArchiveFile.archiveType = ArcFile->archiveType;


	dialogBox = Resources::GetImageData("bg_properties.png");
	titleData = Resources::GetImageData("icon_folder.png");
	arrowUp = Resources::GetImageData("close.png");
	arrowUpOver = Resources::GetImageData("close_over.png");

	btnClick = Resources::GetSound("button_click.wav");

	dialogBoxImg = new GuiImage(dialogBox);

	width = dialogBox->GetWidth();
	height = dialogBox->GetHeight();

	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigB = new GuiTrigger();
	trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);

	char * filename  = strrchr(ArcFile->filename, '/');
	if(filename)
		filename += 1;
	else
		filename = ArcFile->filename;

	int maxTxtWidth = dialogBox->GetWidth()-Position_X;
	TitleTxt = new GuiText(filename, 22, (GXColor){0, 0, 0, 255});
	TitleTxt->SetAlignment(ALIGN_CENTER | ALIGN_TOP);
	TitleTxt->SetPosition(0, Position_Y);
	TitleTxt->SetMaxWidth(maxTxtWidth, DOTTED);

	int ImgPos = (TitleTxt->GetTextWidth() > maxTxtWidth ? maxTxtWidth : TitleTxt->GetTextWidth());
	TitleImg = new GuiImage(titleData);
	TitleImg->SetAlignment(ALIGN_CENTER | ALIGN_TOP);
	int IconPosition = -(ImgPos/2+titleData->GetWidth())+10;
	if(IconPosition < (30-width/2))
		IconPosition = 30-width/2;
	TitleImg->SetPosition(IconPosition, Position_Y);
	Position_Y += 80;

	sprintf(temp, tr("Filepath:  %s"), ArcFile->filename);
	char * ptr = strrchr(temp, '/');
	if(ptr)
		ptr[0] = '\0';

	filepathTxt =  new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
	filepathTxt->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filepathTxt->SetPosition(Position_X, Position_Y);
	filepathTxt->SetMaxWidth(dialogBox->GetWidth()-Position_X-10, DOTTED);
	Position_Y += 30;

	filecountTxt = new GuiText(tr("Files:"), 20, (GXColor){0, 0, 0, 255});
	filecountTxt->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filecountTxt->SetPosition(Position_X, Position_Y);

	filecountTxtVal = new GuiText(tr("1"), 20, (GXColor){0, 0, 0, 255});
	filecountTxtVal->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filecountTxtVal->SetPosition(Position_X+180, Position_Y);
	Position_Y += 30;

	u32 filesize  = ArchiveFile.length;

	if(filesize > KBSIZE && filesize < MBSIZE)
		sprintf(temp, "%0.2fKB", filesize/KBSIZE);
	else if(filesize > MBSIZE && filesize < GBSIZE)
		sprintf(temp, "%0.2fMB", filesize/MBSIZE);
	else if(filesize > GBSIZE)
		sprintf(temp, "%0.2fGB", filesize/GBSIZE);
	else
		sprintf(temp, "%dB", filesize);

	filesizeTxt = new GuiText(tr("Size:"), 20, (GXColor){0, 0, 0, 255});
	filesizeTxt->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filesizeTxt->SetPosition(Position_X, Position_Y);

	filesizeTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
	filesizeTxtVal->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filesizeTxtVal->SetPosition(Position_X+180, Position_Y);
	Position_Y += 30;

	u32 comp_filesize  = ArchiveFile.comp_length;

	if(comp_filesize > KBSIZE && comp_filesize < MBSIZE)
		sprintf(temp, "%0.2fKB", comp_filesize/KBSIZE);
	else if(comp_filesize > MBSIZE && comp_filesize < GBSIZE)
		sprintf(temp, "%0.2fMB", comp_filesize/MBSIZE);
	else if(comp_filesize > GBSIZE)
		sprintf(temp, "%0.2fGB", comp_filesize/GBSIZE);
	else
		sprintf(temp, "%dB", comp_filesize);

	filesizeCompTxt = new GuiText(tr("Compressed Size:"), 20, (GXColor){0, 0, 0, 255});
	filesizeCompTxt->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filesizeCompTxt->SetPosition(Position_X, Position_Y);

	filesizeCompTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
	filesizeCompTxtVal->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filesizeCompTxtVal->SetPosition(Position_X+180, Position_Y);
	Position_Y += 30;

	char * pch = NULL;
	if(ArchiveFile.isdir)
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
	filetypeTxt->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filetypeTxt->SetPosition(Position_X, Position_Y);

	filetypeTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
	filetypeTxtVal->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	filetypeTxtVal->SetPosition(Position_X+180, Position_Y);
	Position_Y += 30;

	last_modifTxt = new GuiText(tr("Last modified:"), 20, (GXColor){0, 0, 0, 255});
	last_modifTxt->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	last_modifTxt->SetPosition(Position_X, Position_Y);

	TimeStruct ptm;
	if(ArchiveFile.archiveType == SZIP)
	{
		ConvertNTFSDate(ArchiveFile.ModTime, &ptm);
	}
	else
	{
		ConvertDosDate(ArchiveFile.ModTime, &ptm);
	}
	snprintf(temp, sizeof(temp), "%02d:%02d  %02d.%02d.%04d", ptm.tm_hour, ptm.tm_min, ptm.tm_mday, ptm.tm_mon, ptm.tm_year);
	last_modifTxtVal = new GuiText(temp, 20, (GXColor){0, 0, 0, 255});
	last_modifTxtVal->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	last_modifTxtVal->SetPosition(Position_X+180, Position_Y);
	Position_Y += 30;

	arrowUpImg = new GuiImage(arrowUp);
	arrowUpImgOver = new GuiImage(arrowUpOver);
	CloseBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	CloseBtn->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	CloseBtn->SetSoundClick(btnClick);
	CloseBtn->SetImage(arrowUpImg);
	CloseBtn->SetImageOver(arrowUpImgOver);
	CloseBtn->SetPosition(-20, 20);
	CloseBtn->SetEffectGrow();
	CloseBtn->SetTrigger(trigA);
	CloseBtn->SetTrigger(trigB);
	CloseBtn->Clicked.connect(this, &ArchiveProperties::OnButtonClick);

	Append(dialogBoxImg);
	Append(TitleTxt);
	if(ArchiveFile.isdir)
		Append(TitleImg);
	Append(filepathTxt);
	Append(filecountTxt);
	Append(filecountTxtVal);
	Append(filesizeTxt);
	Append(filesizeTxtVal);
	Append(filesizeCompTxt);
	Append(filesizeCompTxtVal);
	Append(filetypeTxt);
	Append(filetypeTxtVal);
	Append(last_modifTxt);
	Append(last_modifTxtVal);
	Append(CloseBtn);

	SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 40);
}

ArchiveProperties::~ArchiveProperties()
{
	SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 40);
	while(this->GetEffect() > 0)
		usleep(THREAD_SLEEP);

	if(parentElement)
		((GuiFrame *) parentElement)->Remove(this);

	RemoveAll();

	if(ArchiveFile.filename)
		delete [] ArchiveFile.filename;

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
	delete filesizeCompTxt;
	delete filesizeCompTxtVal;
	delete filetypeTxt;
	delete filetypeTxtVal;
	delete last_modifTxt;
	delete last_modifTxtVal;

	delete CloseBtn;
	delete trigA;
	delete trigB;
}

int ArchiveProperties::GetChoice()
{
	return choice;
}

void ArchiveProperties::OnButtonClick(GuiButton *sender, int pointer UNUSED, const POINT &p UNUSED)
{
	sender->ResetState();

	if(sender == CloseBtn)
	{
		choice = -2;
	}
}
