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
#include "Controls/MainWindow.h"
#include "Prompts/PromptWindows.h"
#include "FileOperations/fileops.h"
#include "Language/gettext.h"
#include "sys.h"
#include "DirList.h"

#define TIME_BETWEEN_IMAGES		5
#define THREAD_SLEEP    		100

#define FILETYPESFILTER			".jpg,.bmp,.gif,.png,.tga"

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

DirList *imageDir = NULL;
lwp_t slideShowThread = LWP_THREAD_NULL;
int currentImage = -1;
GuiImageData *imageData = NULL;
bool stopSlideShow = false;

void *SlideShowCallback(void *arg)
{
	// We should sleep for a total of TIME_BETWEEN_IMAGES
	// but substract the loading time first (maybe it's longer then that!)
	// The remainder of the time should be divided by THREAD_SLEEP
	// to remain responsive.

	GuiImage *image = (GuiImage *) arg;
	time_t startloading, stoploading;
	while (!stopSlideShow)
	{
		bool fileLoaded = false;
		startloading = time(NULL);
		GuiImageData *newImage;

		while (!fileLoaded && !stopSlideShow) {
			// Load the next image...
			int index = currentImage + 1;

			if (index == imageDir->GetFilecount())
			{
				index = 0;
				currentImage = -1;
			}

			char *filename = imageDir->GetFilename(index);
			char *filedir = imageDir->GetFilepath(index);
			if (filename == NULL || filedir == NULL) {
				currentImage++;
				continue;
			}
			u64 filesize;
			u8 *file = NULL;

			char filepath[255];
			sprintf((char *) &filepath, "%s/%s", filedir, filename);
			if (!LoadFileToMem(filepath, &file, &filesize)) {
				currentImage++;
				continue;
			}

			newImage = new GuiImageData(file, filesize);

			if(file) {
				free(file);
				file = NULL;
			}

			if (!newImage->GetImage())
			{
				delete newImage;
				newImage = NULL;
				currentImage++;
				continue;
			}

			fileLoaded = true;
			stoploading = time(NULL);
		}
		if (stopSlideShow) {
			return NULL;
		}

		int seconds_to_wait = TIME_BETWEEN_IMAGES - difftime(stoploading, startloading);
		int amount_of_sleep = seconds_to_wait * 1000000 / THREAD_SLEEP;

//		char buf[255];
//		sprintf((char *) &buf, "Sec2wait: %d, cycles: %d", seconds_to_wait, amount_of_sleep);
//		WindowPrompt("Slide", (char *) &buf, "OK");

		for (int i = 0; i < amount_of_sleep && !stopSlideShow; i++) {
			usleep(THREAD_SLEEP);
		}
		if (stopSlideShow) {
			return NULL;
		}

		// Load the next image...
		if (imageData != NULL)
		{
			delete imageData;
		}
		imageData = newImage;
		image->SetImage(imageData);

		currentImage++;
	}
	return NULL;
}

bool LoadImage(int index, GuiImageData **imageData)
{
	char *filename = imageDir->GetFilename(index);
	char *filedir = imageDir->GetFilepath(index);
	if (filename == NULL || filedir == NULL)
	{
		char buffer[30];
		sprintf((char *) &buffer, tr("No image found at index %d."), index);
		WindowPrompt(tr("ImageViewer"), (char *) &buffer, tr("OK"));
		return false;
	}
	u64 filesize;
	u8 *file = NULL;

	char filepath[255];
	sprintf((char *) &filepath, "%s/%s", filedir, filename);

	int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &file, &filesize);
	if (ret < 0)
		return false;

	GuiImageData *newImage = new GuiImageData(file, filesize);

    if(file) {
        free(file);
        file = NULL;
    }

	if (!newImage->GetImage())
	{
		WindowPrompt(tr("ImageViewer"), tr("Cannot open image"), tr("OK"));
		delete newImage;
		return false;
	}
	if (*imageData != NULL)
	{
		delete *imageData;
	}
	*imageData = newImage;
	return true;
}

void UpdateImage(GuiImage *image, GuiImageData *imageData)
{
	image->SetAngle(0);
	image->SetImage(imageData);

	image->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
}

int NextImage(int index, GuiImageData **imageData)
{
	return LoadImage(++index, imageData) ? index : -1;
}

int PreviousImage(int index, GuiImageData **imageData)
{
	return LoadImage(--index, imageData) ? index : -1;
}

void ImageViewer(const char *filepath)
{
    bool exitwindow = false;
	currentImage = -1;

	char path[255];
	memset((void *) &path, 0, 255);

	char *ptr = strrchr(filepath, '/');
	if (ptr != NULL)
	{
		strncpy(path, filepath, ptr - filepath);
		imageDir = new DirList(path, FILETYPESFILTER);

//		char buf[255];
//		sprintf((char *) &buf, "Images: %d", imageDir->GetFilecount());
//		WindowPrompt("ImageViewer", (char *) &buf, "OK");

		char *filename = ptr + 1;

//		sprintf((char *) &buf, "Filename: %s", filename);
//		WindowPrompt("ImageViewer", (char *) &buf, "OK");

		currentImage = imageDir->GetFileIndex(filename);
		if (currentImage == -1)
		{
			delete imageDir;
			imageDir = NULL;
			return;
		}
	}

	imageData = NULL;
	LoadImage(currentImage, &imageData);

	GuiImage image(imageData);

	GuiTrigger trigger, trigNext, trigPrev, trigB;
	trigger.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigPrev.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, 0);
	trigNext.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, 0);

	GuiWindow window(MainWindow::Instance()->GetWidth(), MainWindow::Instance()->GetHeight());

	GuiImage backGround(MainWindow::Instance()->GetWidth(), MainWindow::Instance()->GetHeight(), (GXColor){0, 0, 0, 0x50});

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
	backButton.SetPosition(24, -16);
	backButton.SetTrigger(&trigger);
	backButton.SetTrigger(&trigB);

	window.Append(&backButton);

	GuiImageData slideshowButtonData(slideshow_png);
	GuiImage slideshowButtonImage(&slideshowButtonData);
	GuiImageData slideshowButtonOverData(slideshow_over_png);
	GuiImage slideshowButtonOverImage(&slideshowButtonOverData);

	GuiButton slideshowButton(slideshowButtonImage.GetWidth(), slideshowButtonImage.GetHeight());
	slideshowButton.SetImage(&slideshowButtonImage);
	slideshowButton.SetImageOver(&slideshowButtonOverImage);
	slideshowButton.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	slideshowButton.SetPosition(128, -16);
	slideshowButton.SetTrigger(&trigger);

	window.Append(&slideshowButton);

	GuiImageData zoominButtonData(zoomin_png);
	GuiImage zoominButtonImage(&zoominButtonData);
	GuiImageData zoominButtonOverData(zoomin_over_png);
	GuiImage zoominButtonOverImage(&zoominButtonOverData);

	GuiButton zoominButton(zoominButtonImage.GetWidth(), zoominButtonImage.GetHeight());
	zoominButton.SetImage(&zoominButtonImage);
	zoominButton.SetImageOver(&zoominButtonOverImage);
	zoominButton.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	zoominButton.SetPosition(224, -16);
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
	zoomoutButton.SetPosition(320, -16);
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
	rotateRButton.SetPosition(416, -16);
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
	rotateLButton.SetPosition(512, -16);
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
	nextButton.SetTrigger(&trigNext);

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
	prevButton.SetTrigger(&trigPrev);

	if (imageDir != NULL) {
		window.Append(&prevButton);
	}

	GuiButton stopSlideshowButton(MainWindow::Instance()->GetWidth(), MainWindow::Instance()->GetHeight());
	stopSlideshowButton.SetTrigger(&trigger);

	HaltGui();
	MainWindow::Instance()->SetState(STATE_DISABLED);
	MainWindow::Instance()->Append(&window);
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
		else if (slideshowButton.GetState() == STATE_CLICKED) {
			//start a slideshow
			backButton.SetVisible(false);
			slideshowButton.SetVisible(false);
			zoominButton.SetVisible(false);
			zoomoutButton.SetVisible(false);
			rotateLButton.SetVisible(false);
			rotateRButton.SetVisible(false);
			prevButton.SetVisible(false);
			nextButton.SetVisible(false);

			window.Append(&stopSlideshowButton);
			slideshowButton.ResetState();

			// Launch the slideshow thread
			stopSlideShow = false;
			LWP_CreateThread(&slideShowThread, SlideShowCallback, &image, NULL, 0, 60);
		}
		else if (stopSlideshowButton.GetState() == STATE_CLICKED) {
			// stop the slideshow thread
			stopSlideShow = true;

			backButton.SetVisible(true);
			slideshowButton.SetVisible(true);
			zoominButton.SetVisible(true);
			zoomoutButton.SetVisible(true);
			rotateLButton.SetVisible(true);
			rotateRButton.SetVisible(true);
			prevButton.SetVisible(true);
			nextButton.SetVisible(true);

			// Wait for the thread to stop
			LWP_JoinThread(slideShowThread, NULL);
			slideShowThread = LWP_THREAD_NULL;

			stopSlideshowButton.ResetState();
			window.Remove(&stopSlideshowButton);
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

			UpdateImage(&image, imageData);
		}
		else if (prevButton.GetState() == STATE_CLICKED) {
			currentImage = PreviousImage(currentImage, &imageData);
			prevButton.ResetState();
			nextButton.SetVisible(currentImage != -1 && currentImage < imageDir->GetFilecount() - 1);
			prevButton.SetVisible(currentImage > 0);

			UpdateImage(&image, imageData);
		}
	}

	HaltGui();
	MainWindow::Instance()->Remove(&window);
	MainWindow::Instance()->SetState(STATE_DEFAULT);
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
