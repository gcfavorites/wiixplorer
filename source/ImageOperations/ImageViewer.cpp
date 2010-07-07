/***************************************************************************
 * Copyright (C) 2009
 * by r-win & Dimok
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
 * for WiiXplorer 2009
 ***************************************************************************/

#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "libwiigui/gui.h"
#include "Controls/MainWindow.h"
#include "Prompts/PromptWindows.h"
#include "FileOperations/fileops.h"
#include "FileStartUp/FileExtensions.h"
#include "ImageOperations/ImageViewer.h"
#include "sys.h"
#include "DirList.h"
#include "menu.h"

ImageViewer::ImageViewer(const char *filepath)
    : GuiWindow(0, 0)
{
	currentImage = 0;
	currentState = -1;
	slideshowDelay = Settings.SlideshowDelay;
	SlideShowStart = 0;
	imageDir = NULL;
	image = NULL;
	imageData = NULL;
	rotateLeft = 90;
	rotateRight = 90;
	buttonAlpha = 255;
	updateAlpha = false;
	isPointerVisible = true;

	for (int i = 0; i < 4; i++)
		isAButtonPressed[i] = false;

	width = 0;
	height = 0;

	this->LoadImageList(filepath);

	this->Setup();

	this->LoadImage(currentImage);
}

ImageViewer::~ImageViewer()
{
    MainWindow::Instance()->ResumeGui();
    SetEffect(EFFECT_FADE, -50);
    while(this->GetEffect() > 0) usleep(100);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    for(int i = 0; i < 4; i++)
        MainWindow::Instance()->ResetPointer(i);

    RemoveAll();

    if(imageDir)
        delete imageDir;

    if(image)
        delete image;

    if(imageData)
        delete imageData;

    delete nextButton;
    delete prevButton;
    delete stopSlideshowButton;
    delete slideshowButton;
    delete zoominButton;
    delete zoomoutButton;
    delete rotateLButton;
    delete rotateRButton;
    delete backButton;
	delete moveButton;
	delete trashButton;

    delete trigger;
    delete trigNext;
    delete trigPrev;
    delete trigB;
    delete trigA_Held;
    delete trigPlus_Held;
    delete trigMinus_Held;
	delete trigRotateL;
	delete trigRotateR;
	delete trigSlideshow;

	delete backGround;

    Resources::Remove(backButtonData);
    Resources::Remove(backButtonOverData);
    delete backButtonImage;
    delete backButtonOverImage;
    Resources::Remove(zoominButtonData);
    Resources::Remove(zoominButtonOverData);
    delete zoominButtonImage;
    delete zoominButtonOverImage;
    Resources::Remove(zoomoutButtonData);
    Resources::Remove(zoomoutButtonOverData);
    delete zoomoutButtonImage;
    delete zoomoutButtonOverImage;
    Resources::Remove(rotateRButtonData);
    Resources::Remove(rotateRButtonOverData);
    delete rotateRButtonImage;
    delete rotateRButtonOverImage;
    Resources::Remove(rotateLButtonData);
    Resources::Remove(rotateLButtonOverData);
    delete rotateLButtonImage;
    delete rotateLButtonOverImage;
    Resources::Remove(nextButtonData);
    Resources::Remove(nextButtonOverData);
    delete nextButtonImage;
    delete nextButtonOverImage;
    Resources::Remove(prevButtonData);
    Resources::Remove(prevButtonOverData);
    delete prevButtonImage;
    delete prevButtonOverImage;
    Resources::Remove(slideshowButtonData);
    Resources::Remove(slideshowButtonOverData);
    delete slideshowButtonImage;
    delete slideshowButtonOverImage;
    Resources::Remove(trashImgData);
    delete trashImg;

    MainWindow::Instance()->ResumeGui();
}

bool ImageViewer::LoadImageList(const char * filepath)
{
	char path[strlen(filepath)+1];
	snprintf(path, sizeof(path), "%s", filepath);

	char *ptr = strrchr(path, '/');
	if (!ptr)
	    return false;

    ptr++;
    ptr[0] = '\0';

    if(imageDir)
        delete imageDir;

    imageDir = new DirList(path, Settings.FileExtensions.GetImage());

    char * filename = strrchr(filepath, '/');
    if(filename)
        filename += 1;

    currentImage = imageDir->GetFileIndex(filename);
    if (currentImage < 0)
    {
        delete imageDir;
        imageDir = NULL;
        return false;
    }

	return true;
}

int ImageViewer::MainUpdate()
{
	int pointerX = 0;
	int pointerY = 0;

	wasPointerVisible = isPointerVisible;
	isPointerVisible = false;

	for (int i = 3; i >= 0; i--)
	{
		if (userInput[i].wpad->ir.valid)
		{
			isPointerVisible = true;
 			pointerX = userInput[i].wpad->ir.x;
 			pointerY = userInput[i].wpad->ir.y;
		}

		if ((userInput[i].wpad->btns_u & WiiControls.ClickButton ||
             userInput[i].wpad->btns_u & (ClassicControls.ClickButton << 16))
             && isAButtonPressed[i])
		{
			MainWindow::Instance()->ResetPointer(i);
			isAButtonPressed[i] = false;
		}
	}

    if(SlideShowStart > 0)
    {
        time_t currentTime = time(0);
        if(currentTime-SlideShowStart >= slideshowDelay)
        {
            SlideShowStart = currentTime;
            NextImage(true);
        }
    }
    //!Has to be in main thread for image loading with progressbar
    else if(nextButton->GetState() == STATE_CLICKED)
    {
        nextButton->ResetState();
        NextImage();
    }
    else if(prevButton->GetState() == STATE_CLICKED)
    {
        prevButton->ResetState();
        PreviousImage();
    }
    else if (zoominButton->GetState() == STATE_HELD)
    {
        ZoomIn();
    }
    else if (zoomoutButton->GetState() == STATE_HELD)
    {
        ZoomOut();
    }
	else if (moveButton->GetState() == STATE_HELD)
	{
		if (image && isPointerVisible)
		{
			image->SetPosition(pointerX-clickPosX, pointerY-clickPosY);
		}
	}
	else if (trashButton->GetState() == STATE_CLICKED)
	{
	    if(imageDir && image)
	    {
            SetState(STATE_DISABLED);
            int choice = WindowPrompt(tr("Do you want to delete this file:"), imageDir->GetFilename(currentImage), tr("Yes"), tr("Cancel"), 0, 0, false);
            if (choice)
            {
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", imageDir->GetFilepath(currentImage), imageDir->GetFilename(currentImage));
                if(!RemoveFile(filepath))
                    ShowError(tr("File could not be deleted."));
                else
                {
                    NextImage(false);
                    snprintf(filepath, sizeof(filepath), "%s/%s", imageDir->GetFilepath(currentImage), imageDir->GetFilename(currentImage));
                    LoadImageList(filepath);
                }
            }
            SetState(STATE_DEFAULT);
	    }
		trashButton->ResetState();
	}

	if (wasPointerVisible != isPointerVisible)
	{
		updateAlpha = true;
	}

	if (updateAlpha)
	{
		nextButton->SetAlpha(buttonAlpha);
		prevButton->SetAlpha(buttonAlpha);

		zoominButton->SetAlpha(buttonAlpha);
		zoomoutButton->SetAlpha(buttonAlpha);
		backButton->SetAlpha(buttonAlpha);
		slideshowButton->SetAlpha(buttonAlpha);
		rotateLButton->SetAlpha(buttonAlpha);
		rotateRButton->SetAlpha(buttonAlpha);
		trashButton->SetAlpha(buttonAlpha);

		if (isPointerVisible)
		{
			if ((buttonAlpha+=5) >= 255)
				updateAlpha = false;
		}
		else
		{
			if ((buttonAlpha-=5) <= 0)
				updateAlpha = false;
		}
	}

	if (image && rotateRight < 90)
	{
		image->SetAngle(currentAngle+(rotateRight+=3));
	}

	if (image && rotateLeft < 90)
	{
		image->SetAngle(currentAngle-(rotateLeft+=3));
	}

    return currentState;
}

void ImageViewer::ZoomIn()
{
    if(!image)
        return;

    SetImageSize(image->GetScale()+0.01f);
}

void ImageViewer::ZoomOut()
{
    if(!image)
        return;

    SetImageSize(image->GetScale()-0.01f);
}

void ImageViewer::SetImageSize(float scale)
{
    if(!image)
        return;

    if(scale < 0.0f)
        scale = 0.000001f;

    image->SetScale(scale);
}

void ImageViewer::SetStartUpImageSize()
{
    if(!image)
        return;

    SetImageSize(1.0f);

    float newscale = image->GetScale();

    float imgwidth = (float) image->GetWidth() * 1.0f;
    float imgheight = (float) image->GetHeight() * 1.0f;
    int retries = 100;

    while(imgheight * newscale > height || imgwidth * newscale > width)
    {
        if(imgheight * newscale > height)
            newscale = height/imgheight;
        if(imgwidth * newscale > width)
            newscale = width/imgwidth;

        retries--;
        if(retries == 0)
        {
            newscale = 1.0f;
            break;
        }
    }

    if(newscale < 0.05f)
        newscale = 0.05f;

    int PositionX = (int) (GetLeft()+width/2.0f-imgwidth/2.0f);
    int PositionY = (int) (GetTop()+height/2.0f-imgheight/2.0f);
    image->SetScale(newscale);
    image->SetPosition(PositionX, PositionY);
}

bool ImageViewer::NextImage(bool silent)
{
    currentImage++;
    if(currentImage >= imageDir->GetFilecount())
        currentImage = 0;

	return LoadImage(currentImage, silent);
}

bool ImageViewer::PreviousImage(bool silent)
{
    currentImage--;
    if(currentImage < 0)
        currentImage = imageDir->GetFilecount()-1;

	return LoadImage(currentImage, silent);
}

void ImageViewer::StartSlideShow()
{
    //start a slideshow
    SlideShowStart = time(0);

	Remove(moveButton);
    Remove(backButton);
    Remove(slideshowButton);
    Remove(zoominButton);
    Remove(zoomoutButton);
    Remove(rotateLButton);
    Remove(rotateRButton);
    Remove(prevButton);
    Remove(nextButton);
    Append(stopSlideshowButton);

    Remove(backGround);
    if(backGround)
        delete backGround;

	backGround = new GuiImage(width, height, (GXColor){0, 0, 0, 255});
    Insert(backGround, 0);
}

void ImageViewer::StopSlideShow()
{
    //stop a slideshow
    SlideShowStart = 0;

	Append(moveButton);
    Append(backButton);
    Append(slideshowButton);
    Append(zoominButton);
    Append(zoomoutButton);
    Append(rotateLButton);
    Append(rotateRButton);
    Append(prevButton);
    Append(nextButton);

    Remove(stopSlideshowButton);

    Remove(backGround);
    if(backGround)
        delete backGround;

	backGround = new GuiImage(width, height, (GXColor){0, 0, 0, 0x50});
    Insert(backGround, 0);
}

void ImageViewer::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    if (sender == moveButton && image)
	{
		for (int i = 0; i < 4; i++)
		{
			if ((userInput[i].wpad->btns_h & WiiControls.ClickButton ||
                 userInput[i].wpad->btns_h & (ClassicControls.ClickButton << 16))
                 && !isAButtonPressed[i])
			{
				MainWindow::Instance()->SetGrabPointer(i);
				isAButtonPressed[i] = true;
			}
		}

		clickPosX = p.x-image->GetLeft()+moveButton->GetLeft();
		clickPosY = p.y-image->GetTop()+moveButton->GetTop();
		return;
	}

    sender->ResetState();

    if (sender == backButton)
    {
        currentState = 1;
    }
    else if (sender == slideshowButton)
    {
        StartSlideShow();
    }
    else if (sender == stopSlideshowButton)
    {
        StopSlideShow();
    }
    else if (sender == rotateRButton)
    {
		if (image && !((int)image->GetAngle()%90))
		{
			rotateRight = 0;
			currentAngle = image->GetAngle();
		}
    }
    else if (sender == rotateLButton)
    {
        if (image && !((int)image->GetAngle()%90))
		{
			rotateLeft = 0;
			currentAngle = image->GetAngle();
		}
    }
}

bool ImageViewer::LoadImage(int index, bool silent)
{
    if(!imageDir)
        return false;

    if(index < 0 || index >= imageDir->GetFilecount())
        return false;

	char * filename = imageDir->GetFilename(index);
	char * filedir = imageDir->GetFilepath(index);

	if (filename == NULL || filedir == NULL)
	{
		char buffer[100];
		snprintf(buffer, sizeof(buffer), tr("No image found at index %d."), index);
		WindowPrompt(tr("Image Viewer"), buffer, tr("OK"));
		return false;
	}

	u64 filesize;
	u8 * file = NULL;

	char filepath[MAXPATHLEN];
	snprintf(filepath, sizeof(filepath), "%s/%s", filedir, filename);

	int ret = -1;
	if(silent)
        ret = LoadFileToMem(filepath, &file, &filesize);
    else
        ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &file, &filesize);

	if (ret < 0)
		return false;

	GuiImageData *newImage = new GuiImageData(file, filesize);

    if(file)
    {
        free(file);
        file = NULL;
    }

	if (newImage->GetImage() == NULL && !silent)
	{
	    delete newImage;
		ShowError(tr("Cannot open image"));
		return false;
	}

    if(image && SlideShowStart > 0)
    {
        image->SetEffect(EFFECT_FADE, -Settings.ImageFadeSpeed);
        while(image->GetEffect() > 0) usleep(100);
    }

	MainWindow::Instance()->HaltGui();

	if(image != NULL)
	{
        Remove(image);
	    delete image;
	}

	if (imageData != NULL)
	{
		delete imageData;
	}

	imageData = newImage;

	image = new GuiImage(imageData);

    //!Set original size first if not over the limits
    SetStartUpImageSize();

    //!Insert after background image and before Buttons
	Insert(image, 1);

    if(SlideShowStart > 0)
        image->SetEffect(EFFECT_FADE, Settings.ImageFadeSpeed);

	MainWindow::Instance()->ResumeGui();

	//!Substract loading time from timer for slideshow
	if(SlideShowStart > 0)
        SlideShowStart = time(0);

	return true;
}

void ImageViewer::Setup()
{
    int DefaultButtonWidth = 96;

	width = MainWindow::Instance()->GetWidth();
	height = MainWindow::Instance()->GetHeight();

	trigger = new GuiTrigger();
	trigA_Held = new GuiTrigger();
	trigPlus_Held = new GuiTrigger();
	trigMinus_Held = new GuiTrigger();
	trigNext = new GuiTrigger();
	trigPrev = new GuiTrigger();
	trigB = new GuiTrigger();
	trigRotateL = new GuiTrigger();
	trigRotateR = new GuiTrigger();
	trigSlideshow = new GuiTrigger();

	trigA_Held->SetHeldTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigPlus_Held->SetButtonOnlyHeldTrigger(-1, WiiControls.ZoomIn | ClassicControls.ZoomIn << 16, GCControls.ZoomIn);
	trigMinus_Held->SetButtonOnlyHeldTrigger(-1, WiiControls.ZoomOut | ClassicControls.ZoomOut << 16, GCControls.ZoomOut);
	trigger->SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);
	trigPrev->SetButtonOnlyTrigger(-1, WiiControls.LeftButton | ClassicControls.LeftButton << 16, GCControls.LeftButton);
	trigNext->SetButtonOnlyTrigger(-1, WiiControls.RightButton | ClassicControls.RightButton << 16, GCControls.RightButton);
	trigRotateL->SetButtonOnlyTrigger(-1, WiiControls.UpButton | ClassicControls.UpButton << 16, GCControls.UpButton);
	trigRotateR->SetButtonOnlyTrigger(-1, WiiControls.DownButton | ClassicControls.DownButton << 16, GCControls.DownButton);
	trigSlideshow->SetButtonOnlyTrigger(-1, WiiControls.SlideShowButton | ClassicControls.SlideShowButton << 16, GCControls.SlideShowButton);

	backGround = new GuiImage(width, height, (GXColor){0, 0, 0, 0x50});

	Append(backGround);

	backButtonData = Resources::GetImageData(back_png, back_png_size);
	backButtonOverData = Resources::GetImageData(back_over_png, back_over_png_size);
	backButtonImage = new GuiImage(backButtonData);
	backButtonOverImage = new GuiImage(backButtonOverData);

	backButton = new GuiButton(backButtonImage->GetWidth(), backButtonImage->GetHeight());
	backButton->SetImage(backButtonImage);
	backButton->SetImageOver(backButtonOverImage);
	backButton->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	backButton->SetPosition(-2*DefaultButtonWidth-DefaultButtonWidth/2-2, -16);
	backButton->SetTrigger(trigger);
	backButton->SetTrigger(trigB);
    backButton->Clicked.connect(this, &ImageViewer::OnButtonClick);
	Append(backButton);

	zoominButtonData = Resources::GetImageData(zoomin_png, zoomin_png_size);
	zoominButtonOverData = Resources::GetImageData(zoomin_over_png, zoomin_over_png_size);
	zoominButtonImage = new GuiImage(zoominButtonData);
	zoominButtonOverImage = new GuiImage(zoominButtonOverData);

	zoominButton = new GuiButton(zoominButtonImage->GetWidth(), zoominButtonImage->GetHeight());
	zoominButton->SetImage(zoominButtonImage);
	zoominButton->SetImageOver(zoominButtonOverImage);
	zoominButton->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	zoominButton->SetPosition(-DefaultButtonWidth/2, -16);
	zoominButton->SetHoldable(true);
	zoominButton->SetTrigger(trigA_Held);
	zoominButton->SetTrigger(trigPlus_Held);
	Append(zoominButton);

	zoomoutButtonData = Resources::GetImageData(zoomout_png, zoomout_png_size);
	zoomoutButtonOverData = Resources::GetImageData(zoomout_over_png, zoomout_over_png_size);
	zoomoutButtonImage = new GuiImage(zoomoutButtonData);
	zoomoutButtonOverImage = new GuiImage(zoomoutButtonOverData);

	zoomoutButton = new GuiButton(zoomoutButtonImage->GetWidth(), zoomoutButtonImage->GetHeight());
	zoomoutButton->SetImage(zoomoutButtonImage);
	zoomoutButton->SetImageOver(zoomoutButtonOverImage);
	zoomoutButton->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	zoomoutButton->SetPosition(DefaultButtonWidth/2, -16);
	zoomoutButton->SetHoldable(true);
	zoomoutButton->SetTrigger(trigA_Held);
	zoomoutButton->SetTrigger(trigMinus_Held);
	Append(zoomoutButton);

	rotateRButtonData = Resources::GetImageData(rotateR_png, rotateR_png_size);
	rotateRButtonOverData = Resources::GetImageData(rotateR_over_png, rotateR_over_png_size);
	rotateRButtonImage = new GuiImage(rotateRButtonData);
	rotateRButtonOverImage = new GuiImage(rotateRButtonOverData);

	rotateRButton = new GuiButton(rotateRButtonImage->GetWidth(), rotateRButtonImage->GetHeight());
	rotateRButton->SetImage(rotateRButtonImage);
	rotateRButton->SetImageOver(rotateRButtonOverImage);
	rotateRButton->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	rotateRButton->SetPosition(DefaultButtonWidth+DefaultButtonWidth/2, -16);
	rotateRButton->SetTrigger(trigger);
	rotateRButton->SetTrigger(trigRotateR);
    rotateRButton->Clicked.connect(this, &ImageViewer::OnButtonClick);

	Append(rotateRButton);

	rotateLButtonData = Resources::GetImageData(rotateL_png, rotateL_png_size);
	rotateLButtonOverData = Resources::GetImageData(rotateL_over_png, rotateL_over_png_size);
	rotateLButtonImage = new GuiImage(rotateLButtonData);
	rotateLButtonOverImage = new GuiImage(rotateLButtonOverData);

	rotateLButton = new GuiButton(rotateLButtonImage->GetWidth(), rotateLButtonImage->GetHeight());
	rotateLButton->SetImage(rotateLButtonImage);
	rotateLButton->SetImageOver(rotateLButtonOverImage);
	rotateLButton->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	rotateLButton->SetPosition(2*DefaultButtonWidth+DefaultButtonWidth/2+2, -16);
	rotateLButton->SetTrigger(trigger);
	rotateLButton->SetTrigger(trigRotateL);
    rotateLButton->Clicked.connect(this, &ImageViewer::OnButtonClick);
	Append(rotateLButton);

    nextButtonData = Resources::GetImageData(next_png, next_png_size);
    nextButtonOverData = Resources::GetImageData(next_over_png, next_over_png_size);
    nextButtonImage = new GuiImage(nextButtonData);
    nextButtonOverImage = new GuiImage(nextButtonOverData);

    nextButton = new GuiButton(nextButtonImage->GetWidth(), nextButtonImage->GetHeight());
    nextButton->SetImage(nextButtonImage);
    nextButton->SetImageOver(nextButtonOverImage);
    nextButton->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    nextButton->SetPosition(-16, 198);
    nextButton->SetTrigger(trigger);
    nextButton->SetTrigger(trigNext);
    Append(nextButton);

    prevButtonData = Resources::GetImageData(prev_png, prev_png_size);
    prevButtonOverData = Resources::GetImageData(prev_over_png, prev_over_png_size);
    prevButtonImage = new GuiImage(prevButtonData);
    prevButtonOverImage = new GuiImage(prevButtonOverData);

    prevButton = new GuiButton(prevButtonImage->GetWidth(), prevButtonImage->GetHeight());
    prevButton->SetImage(prevButtonImage);
    prevButton->SetImageOver(prevButtonOverImage);
    prevButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    prevButton->SetPosition(16, 198);
    prevButton->SetTrigger(trigger);
    prevButton->SetTrigger(trigPrev);
    Append(prevButton);

    slideshowButtonData = Resources::GetImageData(slideshow_png, slideshow_png_size);
    slideshowButtonOverData = Resources::GetImageData(slideshow_over_png, slideshow_over_png_size);
    slideshowButtonImage = new GuiImage(slideshowButtonData);
    slideshowButtonOverImage = new GuiImage(slideshowButtonOverData);

    slideshowButton = new GuiButton(slideshowButtonImage->GetWidth(), slideshowButtonImage->GetHeight());
    slideshowButton->SetImage(slideshowButtonImage);
    slideshowButton->SetImageOver(slideshowButtonOverImage);
    slideshowButton->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    slideshowButton->SetPosition(-DefaultButtonWidth-DefaultButtonWidth/2, -16);
    slideshowButton->SetTrigger(trigger);
	slideshowButton->SetTrigger(trigSlideshow);
    slideshowButton->Clicked.connect(this, &ImageViewer::OnButtonClick);

    Append(slideshowButton);

    stopSlideshowButton = new GuiButton(MainWindow::Instance()->GetWidth(), MainWindow::Instance()->GetHeight());
    stopSlideshowButton->SetTrigger(trigger);
    stopSlideshowButton->SetTrigger(trigB);
    stopSlideshowButton->Clicked.connect(this, &ImageViewer::OnButtonClick);

	moveButton = new GuiButton(screenwidth-(prevButton->GetLeft()+prevButton->GetWidth())*2, screenheight-backButton->GetHeight()-16);
	moveButton->SetPosition(prevButton->GetLeft()+prevButton->GetWidth(), 0);
	moveButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	moveButton->SetHoldable(true);
	moveButton->SetTrigger(trigA_Held);
	moveButton->Clicked.connect(this, &ImageViewer::OnButtonClick);

	Append(moveButton);

    trashImgData = Resources::GetImageData(trash_png, trash_png_size);
    trashImg = new GuiImage(trashImgData);
    trashImg->SetAlpha(120);
    trashImg->SetScale(1.1);
    trashButton = new GuiButton(trashImgData->GetWidth(), trashImgData->GetHeight());
    trashButton->SetImage(trashImg);
    trashButton->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    trashButton->SetPosition(-30, 30);
    trashButton->SetTrigger(trigger);
    trashButton->SetEffectGrow();

    Append(trashButton);

    SetEffect(EFFECT_FADE, 50);
}
