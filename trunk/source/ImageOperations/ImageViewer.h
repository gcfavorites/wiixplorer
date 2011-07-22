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
 * ImageViewer.h
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef __IMAGEVIEWER_H
#define __IMAGEVIEWER_H

#include "libwiigui/gui.h"
#include "DirList.h"

class ImageViewer : public GuiWindow, public sigslot::has_slots<>
{
    public:
        //!Constructor
        //!\param path Path from where to load the filelist of all images or the image itself
        ImageViewer(const char *filepath);
        //!Destructor
        ~ImageViewer();
        //!Zoom into the image (currently 20% till a limit of screenwidth or screenheight)
        virtual void ZoomIn();
        //!Zoom out of the image (currently 20% till a limit of MIN_IMAGE_WIDTH or MIN_IMAGE_HEIGHT)
        virtual void ZoomOut();
        //!Set image size  (limits described above)
        //!\param scale of the image (%/100)
        virtual void SetImageSize(float scale);
        //!Set the startup image size when loading an image
        //!Sets up imagesize at 1.0 scale and limits it to screenwidth/screenheight if it's over size
        virtual void SetStartUpImageSize();
        //!Load NextImage in the Directory
        virtual bool NextImage(bool silent = false);
        //!Load PreviousImage in the directory
        virtual bool PreviousImage(bool silent = false);
        //!Start a Slideshow of the images in the directory (slidespeed: TIME_BETWEEN_IMAGES)
        virtual void StartSlideShow();
        //!Stop a Slideshow
        virtual void StopSlideShow();
        //!MainUpdate function. Callback for the main thread.
        virtual int MainUpdate();
    protected:
        //!Setup the needed images/buttons/texts for the ImageViewer
        virtual void Setup();
        //!Intern image loading funtion.
        //!\param index of image in the directory
        virtual bool LoadImage(int index, bool silent = false);
        //!Intern function to seperate from constuctor
        //!\param path Path from where to load the filelist of all images or the image itself
        virtual bool LoadImageList(const char * filepath);
        //!OnButtonClick intern callback for buttonclicks.
        virtual void OnButtonClick(GuiButton *sender, int pointer, POINT p);
		//!Variables of the ImageViewer
        DirList * imageDir;
        int currentImage;
        int currentState;
		int slideshowDelay;
		int clickPosX;
		int clickPosY;
		int buttonAlpha;
		int rotateRight;
		int rotateLeft;
		bool wasPointerVisible;
		bool isPointerVisible;

		bool isAButtonPressed[4];
		bool updateAlpha;
		float currentAngle;

        time_t SlideShowStart;

        GuiButton * prevButton;
        GuiButton * nextButton;
        GuiButton * stopSlideshowButton;
        GuiButton * slideshowButton;
        GuiButton * zoominButton;
        GuiButton * zoomoutButton;
        GuiButton * rotateLButton;
        GuiButton * rotateRButton;
        GuiButton * backButton;
		GuiButton * moveButton;
		GuiButton * trashButton;

        GuiImage * image;
        GuiImageData *imageData;

        GuiImageData * nextButtonData;
        GuiImageData * nextButtonOverData;
        GuiImageData * prevButtonData;
        GuiImageData * prevButtonOverData;
        GuiImageData * slideshowButtonData;
        GuiImageData * slideshowButtonOverData;
        GuiImageData * backButtonData;
        GuiImageData * backButtonOverData;
        GuiImageData * zoominButtonData;
        GuiImageData * zoominButtonOverData;
        GuiImageData * zoomoutButtonData;
        GuiImageData * zoomoutButtonOverData;
        GuiImageData * rotateRButtonData;
        GuiImageData * rotateRButtonOverData;
        GuiImageData * rotateLButtonData;
        GuiImageData * rotateLButtonOverData;
        GuiImageData * trashImgData;

        GuiImage * backGround;
        GuiImage * nextButtonImage;
        GuiImage * nextButtonOverImage;
        GuiImage * prevButtonImage;
        GuiImage * prevButtonOverImage;
        GuiImage * slideshowButtonImage;
        GuiImage * slideshowButtonOverImage;
        GuiImage * backButtonImage;
        GuiImage * backButtonOverImage;
        GuiImage * zoominButtonImage;
        GuiImage * zoominButtonOverImage;
        GuiImage * zoomoutButtonImage;
        GuiImage * zoomoutButtonOverImage;
        GuiImage * rotateRButtonImage;
        GuiImage * rotateRButtonOverImage;
        GuiImage * rotateLButtonImage;
        GuiImage * rotateLButtonOverImage;
        GuiImage * trashImg;

        GuiTrigger * trigger;
        GuiTrigger * trigNext;
        GuiTrigger * trigPrev;
        GuiTrigger * trigB;
        GuiTrigger * trigA_Held;
        GuiTrigger * trigPlus_Held;
        GuiTrigger * trigMinus_Held;
		GuiTrigger * trigRotateL;
		GuiTrigger * trigRotateR;
		GuiTrigger * trigSlideshow;
};

#endif
