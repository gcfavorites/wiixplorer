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
 * PictureButton.h
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#ifndef _PICTUREBUTTON_H
#define _PICTUREBUTTON_H

#include "libwiigui/gui.h"

class PictureButton : public GuiButton
{
	public:
		PictureButton(const u8 *img, u32 imgSize);
		PictureButton(const u8 *img, u32 imgSize, const u8 *imgOver, u32 imgOverSize);
		PictureButton(const u8 *img, u32 imgSize, const u8 *imgOver, u32 imgOverSize, GuiSound * sndClick);
		PictureButton(const u8 *img, u32 imgSize, const u8 *imgOver, u32 imgOverSize, GuiSound * sndClick, GuiSound * sndOver);

		PictureButton(GuiImageData *img);
		PictureButton(GuiImageData *img, GuiImageData *imgOver);
		PictureButton(GuiImageData *img, GuiImageData *imgOver, GuiSound * sndClick);
		PictureButton(GuiImageData *img, GuiImageData *imgOver, GuiSound * sndClick, GuiSound * sndOver);

		~PictureButton();
	private:
		void Init(const u8 *img, u32 imgSize, const u8 *imgOver, u32 imgOverSize, GuiSound * sndClick, GuiSound * sndOver);
		void Init(GuiImageData *img, GuiImageData *imgOver, GuiSound * sndClick, GuiSound * sndOver);
        GuiImageData *imgData;
        GuiImageData *imgOverData;
};

#endif //_PICTUREBUTTON_H
