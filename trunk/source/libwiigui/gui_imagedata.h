/****************************************************************************
 * Copyright (C) 2010
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
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef GUI_IMAGEDATA_H_
#define GUI_IMAGEDATA_H_

#include <gctypes.h>
#include <gd.h>

//!Converts image data into GX-useable RGBA8. Currently designed for use only with PNG files
class GuiImageData
{
	public:
		//!Constructor
		//!Converts the image data to RGBA8 - expects PNG format
		//!\param img Image data
		//!\param imgSize The image size
		GuiImageData(const u8 * img, int imgSize);
		//!Destructor
		~GuiImageData();
		//!Gets a pointer to the image data
		//!\return pointer to image data
		u8 * GetImage();
		//!Gets the image width
		//!\return image width
		int GetWidth();
		//!Gets the image height
		//!\return image height
		int GetHeight();
		//!Gets the texture format
		u8 GetTextureFormat();
	protected:
		void LoadPNG(const u8 *img, int imgSize);
		void LoadBMP(const u8 *img, int imgSize);
		void LoadJpeg(const u8 *img, int imgSize);
		void LoadGIF(const u8 *img, int imgSize);
        void LoadGD(const u8 *img, int imgSize);
        void LoadGD2(const u8 *img, int imgSize);
        void LoadTIFF(const u8 *img, int imgSize);
		void LoadTGA(const u8 *img, int imgSize);
		void LoadTPL(const u8 *img, int imgSize);
        void GDImageToRGBA8(gdImagePtr & gdImg);

		u8 * data; //!< Image data
		int height; //!< Height of image
		int width; //!< Width of image
		u8 format; //!< Texture format
};

#endif
