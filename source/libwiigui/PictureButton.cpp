/***************************************************************************
 * Copyright (C) 2010
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
 * PictureButton.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/

#include "PictureButton.h"
#include "Memory/Resources.h"

PictureButton::PictureButton(const char *img, const char *imgOver, GuiSound * sndClick, GuiSound *sndOver)
	: GuiButton(0, 0)
{
	Init(img, imgOver, sndClick, sndOver);
}

PictureButton::PictureButton(GuiImageData *img)
	: GuiButton(img->GetWidth(), img->GetHeight())
{
	Init(img, NULL, NULL, NULL);
}

PictureButton::PictureButton(GuiImageData *img, GuiImageData *imgOver)
	: GuiButton(img->GetWidth(), img->GetHeight())
{
	Init(img, imgOver, NULL, NULL);
}

PictureButton::PictureButton(GuiImageData *img, GuiImageData *imgOver, GuiSound * sndClick)
	: GuiButton(img->GetWidth(), img->GetHeight())
{
	Init(img, imgOver, sndClick, NULL);
}

PictureButton::PictureButton(GuiImageData *img, GuiImageData *imgOver, GuiSound * sndClick, GuiSound * sndOver)
	: GuiButton(img->GetWidth(), img->GetHeight())
{
	Init(img, imgOver, sndClick, sndOver);
}

PictureButton::~PictureButton()
{
    if(image)
        Resources::Remove(imgData);
    if(imageOver)
        Resources::Remove(imgOverData);

	image = NULL;
	imageOver = NULL;

}

void PictureButton::Init(const char *img, const char *imgOver, GuiSound * sndClick, GuiSound * sndOver)
{
	imgData = Resources::GetImageData(img);
	imgOverData = Resources::GetImageData(imgOver);

	width = imgData->GetWidth();
	height = imgData->GetHeight();

	Init(imgData, imgOverData, sndClick, sndOver);
}

void PictureButton::Init(GuiImageData *img, GuiImageData *imgOver, GuiSound * sndClick, GuiSound * sndOver)
{
	image = new GuiImage(img);
	SetImage(image);

	if (imgOver != NULL)
	{
		imageOver = new GuiImage(imgOver);
		SetImageOver(imageOver);

		SetEffectGrow();
	}

	if (sndClick != NULL)
	{
		SetSoundClick(sndClick);
	}
	if (sndOver != NULL)
	{
		SetSoundOver(sndOver);
	}
}
