/***************************************************************************
 * Copyright (C) 2009
 * by r-win
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
 * ImageViewer.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "libwiigui/gui.h"
#include "Prompts/PromptWindows.h"
#include "fileops.h"
#include "Language/gettext.h"
#include "sys.h"
#include "DirList.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

DirList *imageDir = NULL;

bool LoadImage(int index, GuiImageData **imageData)
{
	char *filename = imageDir->GetFilename(index);
	char *filedir = imageDir->GetFilepath(index);
	if (filename == NULL || filedir == NULL)
	{
		char buffer[30];
		sprintf((char *) &buffer, "No image found at index %d.", index);
		WindowPrompt("ImageViewer", (char *) &buffer, "OK");
		return false;
	}
	u64 filesize;
	u8 *file = NULL;
	
	char filepath[255];
	sprintf((char *) &filepath, "%s/%s", filedir, filename);

	int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &file, &filesize);
	if (!ret)
		return false;
	
	GuiImageData *newImage = new GuiImageData(file, filesize);
	if (!newImage->GetImage())
	{
		WindowPrompt("ImageViewer", "Cannot open image", "OK");
		return false;
	}
	if (*imageData != NULL)
	{
		delete *imageData;
	}
	*imageData = newImage;
	return true;
}

int NextImage(int currentImage, GuiImageData **imageData)
{
	return LoadImage(++currentImage, imageData) ? currentImage : -1;
}

int PreviousImage(int currentImage, GuiImageData **imageData)
{
	return LoadImage(--currentImage, imageData) ? currentImage : -1;
}

void ImageViewer(const char *filepath)
{
    bool exitwindow = false;
	int currentImage = -1;

    u8 *file = NULL;
    u64 filesize = 0;

	int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &file, &filesize);
	if (!ret)
		return;
		
	GuiImageData *imageData = new GuiImageData(file, filesize);

	if (file)
	{
		free(file);
		file = NULL;
	}

	if (!imageData->GetImage())
	{
		WindowPrompt(tr("Image"), tr("Cannot open the image file."), tr("OK"));
		return;
	}
		
	char path[255];
	char *ptr = strrchr(filepath, '/');
	if (ptr != NULL)
	{
		strncpy(path, filepath, ptr - filepath);
		imageDir = new DirList(path, ".jpg,.gif,.png");

		char *filename = ptr + 1;
		currentImage = imageDir->GetFileIndex(filename);
		if (currentImage == -1)
		{
			delete imageDir;
			imageDir = NULL;
		}
	}
	
	GuiImage image(imageData);

	GuiTrigger trigger;
	trigger.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	
	GuiWindow window(mainWindow->GetWidth(), mainWindow->GetHeight());

	GuiImage backGround(mainWindow->GetWidth(), mainWindow->GetHeight(), (GXColor){0, 0, 0, 255});

	float factor = (image.GetWidth() > image.GetHeight()) ? (1.0 * backGround.GetWidth()) / image.GetWidth() : (1.0 * backGround.GetHeight()) / image.GetHeight();
	image.SetScale(factor);
	
	image.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	
	window.Append(&backGround);
	
	window.Append(&image);
	
	GuiImageData backButtonData(back_png);
	GuiImage backButtonImage(&backButtonData);
	GuiImageData backButtonOverData(back_over_png);
	GuiImage backButtonOverImage(&backButtonOverData);
	
	GuiButton backButton(backButtonImage.GetWidth(), backButtonImage.GetHeight());
	backButton.SetImage(&backButtonImage);
	backButton.SetImageOver(&backButtonOverImage);
	backButton.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backButton.SetPosition(32, -16);
	backButton.SetTrigger(&trigger);

	window.Append(&backButton);

	GuiImageData zoominButtonData(zoomin_png);
	GuiImage zoominButtonImage(&zoominButtonData);
	GuiImageData zoominButtonOverData(zoomin_over_png);
	GuiImage zoominButtonOverImage(&zoominButtonOverData);
	
	GuiButton zoominButton(zoominButtonImage.GetWidth(), zoominButtonImage.GetHeight());
	zoominButton.SetImage(&zoominButtonImage);
	zoominButton.SetImageOver(&zoominButtonOverImage);
	zoominButton.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	zoominButton.SetPosition(152, -16);
	zoominButton.SetTrigger(&trigger);

	window.Append(&zoominButton);

	GuiImageData zoomoutButtonData(zoomout_png);
	GuiImage zoomoutButtonImage(&zoomoutButtonData);
	GuiImageData zoomoutButtonOverData(zoomout_over_png);
	GuiImage zoomoutButtonOverImage(&zoomoutButtonOverData);
	
	GuiButton zoomoutButton(zoomoutButtonImage.GetWidth(), zoomoutButtonImage.GetHeight());
	zoomoutButton.SetImage(&zoomoutButtonImage);
	zoomoutButton.SetImageOver(&zoomoutButtonOverImage);
	zoomoutButton.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	zoomoutButton.SetPosition(264, -16);
	zoomoutButton.SetTrigger(&trigger);
	zoomoutButton.SetState(STATE_DISABLED);

	window.Append(&zoomoutButton);

	GuiImageData rotateRButtonData(rotateR_png);
	GuiImage rotateRButtonImage(&rotateRButtonData);
	GuiImageData rotateRButtonOverData(rotateR_over_png);
	GuiImage rotateRButtonOverImage(&rotateRButtonOverData);
	
	GuiButton rotateRButton(rotateRButtonImage.GetWidth(), rotateRButtonImage.GetHeight());
	rotateRButton.SetImage(&rotateRButtonImage);
	rotateRButton.SetImageOver(&rotateRButtonOverImage);
	rotateRButton.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	rotateRButton.SetPosition(376, -16);
	rotateRButton.SetTrigger(&trigger);

	window.Append(&rotateRButton);

	GuiImageData rotateLButtonData(rotateL_png);
	GuiImage rotateLButtonImage(&rotateLButtonData);
	GuiImageData rotateLButtonOverData(rotateL_over_png);
	GuiImage rotateLButtonOverImage(&rotateLButtonOverData);
	
	GuiButton rotateLButton(rotateLButtonImage.GetWidth(), rotateLButtonImage.GetHeight());
	rotateLButton.SetImage(&rotateLButtonImage);
	rotateLButton.SetImageOver(&rotateLButtonOverImage);
	rotateLButton.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	rotateLButton.SetPosition(488, -16);
	rotateLButton.SetTrigger(&trigger);

	window.Append(&rotateLButton);

	GuiImageData nextButtonData(next_png);
	GuiImage nextButtonImage(&nextButtonData);
	GuiImageData nextButtonOverData(next_over_png);
	GuiImage nextButtonOverImage(&nextButtonOverData);
	
	GuiButton nextButton(nextButtonImage.GetWidth(), nextButtonImage.GetHeight());
	nextButton.SetImage(&nextButtonImage);
	nextButton.SetImageOver(&nextButtonOverImage);
	nextButton.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	nextButton.SetPosition(-16, 198);
	nextButton.SetVisible(imageDir != NULL && currentImage != -1 && currentImage < imageDir->GetFilecount() - 1);
	nextButton.SetTrigger(&trigger);

	if (imageDir != NULL) {
		window.Append(&nextButton);
	}

	GuiImageData prevButtonData(prev_png);
	GuiImage prevButtonImage(&prevButtonData);
	GuiImageData prevButtonOverData(prev_over_png);
	GuiImage prevButtonOverImage(&prevButtonOverData);
	
	GuiButton prevButton(prevButtonImage.GetWidth(), prevButtonImage.GetHeight());
	prevButton.SetImage(&prevButtonImage);
	prevButton.SetImageOver(&prevButtonOverImage);
	prevButton.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	prevButton.SetPosition(16, 198);
	prevButton.SetVisible(imageDir != NULL && currentImage > 0);
	prevButton.SetTrigger(&trigger);

	if (imageDir != NULL) {
		window.Append(&prevButton);
	}

	HaltGui();
	mainWindow->Append(&window);
	ResumeGui();
	
	while (!exitwindow)
	{
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();
			
		if (backButton.GetState() == STATE_CLICKED) {
			exitwindow = true;
			backButton.ResetState();
		}
		else if (zoominButton.GetState() == STATE_CLICKED) {
			image.SetScale(image.GetScale() + 1);
			zoominButton.ResetState();
			
			zoominButton.SetState(STATE_DISABLED);
			zoomoutButton.SetState(STATE_DEFAULT);
		}
		else if (zoomoutButton.GetState() == STATE_CLICKED) {
			image.SetScale(image.GetScale() - 1);
			zoomoutButton.ResetState();

			zoomoutButton.SetState(STATE_DISABLED);
			zoominButton.SetState(STATE_DEFAULT);
		}
		else if (rotateRButton.GetState() == STATE_CLICKED) {
			image.SetAngle(image.GetAngle() + 90);
			rotateRButton.ResetState();
		}
		else if (rotateLButton.GetState() == STATE_CLICKED) {
			image.SetAngle(image.GetAngle() - 90);
			rotateLButton.ResetState();
		}
		else if (nextButton.GetState() == STATE_CLICKED) {
			currentImage = NextImage(currentImage, &imageData);
			nextButton.ResetState();
			nextButton.SetVisible(currentImage != -1 && currentImage < imageDir->GetFilecount() - 1);
			prevButton.SetVisible(currentImage > 0);

			image.SetAngle(0);
			image.SetImage(imageData);
		}
		else if (prevButton.GetState() == STATE_CLICKED) {
			currentImage = PreviousImage(currentImage, &imageData);
			prevButton.ResetState();
			nextButton.SetVisible(currentImage != -1 && currentImage < imageDir->GetFilecount() - 1);
			prevButton.SetVisible(currentImage > 0);
			
			image.SetAngle(0);
			image.SetImage(imageData);
		}
	}
	
	HaltGui();
	mainWindow->Remove(&window);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	
	if (imageData != NULL)
	{
		delete imageData;
		imageData = NULL;
	}
	
	if (imageDir != NULL)
	{
		delete imageDir;
		imageDir = NULL;
	}
}
