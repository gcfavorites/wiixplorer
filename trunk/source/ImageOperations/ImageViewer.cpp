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
#include "ImageOperations/ImageViewer.h"
#include "Language/gettext.h"
#include "sys.h"
#include "DirList.h"

#define TIME_BETWEEN_IMAGES		5
#define FILETYPESFILTER			".jpg,.bmp,.gif,.png,.tga,.tif,.tiff,.jfif,.jpe,.gd,.gd2"
#define MIN_IMAGE_WIDTH         4.0f
#define MIN_IMAGE_HEIGHT        4.0f
#define MAX_IMAGE_WIDTH         1024.0f
#define MAX_IMAGE_HEIGHT        768.0f

extern u8 shutdown;
extern u8 reset;


ImageViewer::ImageViewer(const char *filepath)
    : GuiWindow(0, 0)
{
	currentImage = 0;
	currentState = -1;
    SlideShowStart = 0;
	imageDir = NULL;
	image = NULL;
	imageData = NULL;

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

    delete trigger;
    delete trigNext;
    delete trigPrev;
    delete trigB;
    delete trigA_Held;

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

    imageDir = new DirList(path, FILETYPESFILTER);

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
    if(shutdown == 1)
        Sys_Shutdown();
    else if(reset == 1)
        Sys_Reboot();

    if(SlideShowStart > 0)
    {
        time_t currentTime = time(0);
        if(currentTime-SlideShowStart >= TIME_BETWEEN_IMAGES)
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

    float imgwidth = (float) image->GetWidth() * 1.0f;
    float imgheight = (float) image->GetHeight() * 1.0f;

    float newscale = scale;

    int retries = 100;

    while(imgheight * newscale > MAX_IMAGE_HEIGHT || imgwidth * newscale > MAX_IMAGE_WIDTH ||
          imgheight * newscale < MIN_IMAGE_HEIGHT ||  imgwidth * newscale < MIN_IMAGE_WIDTH)
    {
        if(imgheight * newscale > MAX_IMAGE_HEIGHT)
            newscale = MAX_IMAGE_HEIGHT/imgheight;
        if(imgwidth * newscale > MAX_IMAGE_WIDTH)
            newscale = MAX_IMAGE_WIDTH/imgwidth;
        if(imgheight * newscale < MIN_IMAGE_HEIGHT)
            newscale = MIN_IMAGE_HEIGHT/imgheight;
        if(imgwidth * newscale < MIN_IMAGE_WIDTH)
            newscale = MIN_IMAGE_WIDTH/imgwidth;

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
        if(image)
            image->SetAngle(image->GetAngle() + 90);
    }
    else if (sender == rotateLButton)
    {
        if(image)
            image->SetAngle(image->GetAngle() - 90);
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

	if (newImage->GetImage() == NULL)
	{
		WindowPrompt(tr("ImageViewer"), tr("Cannot open image"), tr("OK"));
		delete newImage;
		return false;
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

	MainWindow::Instance()->ResumeGui();

	//!Substract loading time from timer for slideshow
	if(SlideShowStart > 0)
        SlideShowStart = time(0);

	return true;
}

void ImageViewer::Setup()
{
	width = MainWindow::Instance()->GetWidth();
	height = MainWindow::Instance()->GetHeight();

	trigger = new GuiTrigger();
	trigA_Held = new GuiTrigger();
	trigNext = new GuiTrigger();
	trigPrev = new GuiTrigger();
	trigB = new GuiTrigger();

	trigA_Held->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigger->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigPrev->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigNext->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

	backGround = new GuiImage(width, height, (GXColor){0, 0, 0, 0x50});

	Append(backGround);

	backButtonData = Resources::GetImageData(back_png, back_png_size);
	backButtonOverData = Resources::GetImageData(back_over_png, back_over_png_size);
	backButtonImage = new GuiImage(backButtonData);
	backButtonOverImage = new GuiImage(backButtonOverData);

	backButton = new GuiButton(backButtonImage->GetWidth(), backButtonImage->GetHeight());
	backButton->SetImage(backButtonImage);
	backButton->SetImageOver(backButtonOverImage);
	backButton->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backButton->SetPosition(24, -16);
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
	zoominButton->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	zoominButton->SetPosition(224, -16);
	zoominButton->SetSelectable(false);
	zoominButton->SetClickable(false);
	zoominButton->SetHoldable(true);
	zoominButton->SetTrigger(trigA_Held);

	Append(zoominButton);

	zoomoutButtonData = Resources::GetImageData(zoomout_png, zoomout_png_size);
	zoomoutButtonOverData = Resources::GetImageData(zoomout_over_png, zoomout_over_png_size);
	zoomoutButtonImage = new GuiImage(zoomoutButtonData);
	zoomoutButtonOverImage = new GuiImage(zoomoutButtonOverData);

	zoomoutButton = new GuiButton(zoomoutButtonImage->GetWidth(), zoomoutButtonImage->GetHeight());
	zoomoutButton->SetImage(zoomoutButtonImage);
	zoomoutButton->SetImageOver(zoomoutButtonOverImage);
	zoomoutButton->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	zoomoutButton->SetPosition(320, -16);
	zoomoutButton->SetSelectable(false);
	zoomoutButton->SetClickable(false);
	zoomoutButton->SetHoldable(true);
	zoomoutButton->SetTrigger(trigA_Held);

	Append(zoomoutButton);

	rotateRButtonData = Resources::GetImageData(rotateR_png, rotateR_png_size);
	rotateRButtonOverData = Resources::GetImageData(rotateR_over_png, rotateR_over_png_size);
	rotateRButtonImage = new GuiImage(rotateRButtonData);
	rotateRButtonOverImage = new GuiImage(rotateRButtonOverData);

	rotateRButton = new GuiButton(rotateRButtonImage->GetWidth(), rotateRButtonImage->GetHeight());
	rotateRButton->SetImage(rotateRButtonImage);
	rotateRButton->SetImageOver(rotateRButtonOverImage);
	rotateRButton->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	rotateRButton->SetPosition(416, -16);
	rotateRButton->SetTrigger(trigger);
    rotateRButton->Clicked.connect(this, &ImageViewer::OnButtonClick);

	Append(rotateRButton);

	rotateLButtonData = Resources::GetImageData(rotateL_png, rotateL_png_size);
	rotateLButtonOverData = Resources::GetImageData(rotateL_over_png, rotateL_over_png_size);
	rotateLButtonImage = new GuiImage(rotateLButtonData);
	rotateLButtonOverImage = new GuiImage(rotateLButtonOverData);

	rotateLButton = new GuiButton(rotateLButtonImage->GetWidth(), rotateLButtonImage->GetHeight());
	rotateLButton->SetImage(rotateLButtonImage);
	rotateLButton->SetImageOver(rotateLButtonOverImage);
	rotateLButton->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	rotateLButton->SetPosition(512, -16);
	rotateLButton->SetTrigger(trigger);
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
    slideshowButton->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    slideshowButton->SetPosition(128, -16);
    slideshowButton->SetTrigger(trigger);
    slideshowButton->Clicked.connect(this, &ImageViewer::OnButtonClick);

    Append(slideshowButton);

    stopSlideshowButton = new GuiButton(MainWindow::Instance()->GetWidth(), MainWindow::Instance()->GetHeight());
    stopSlideshowButton->SetTrigger(trigger);
    stopSlideshowButton->SetTrigger(trigB);
    stopSlideshowButton->Clicked.connect(this, &ImageViewer::OnButtonClick);

    SetEffect(EFFECT_FADE, 50);
}
