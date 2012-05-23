/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <malloc.h>
#include <string.h>
#include "gui_imagedata.h"
#include "ImageOperations/TextureConverter.h"
#include "ImageOperations/TplImage.h"
#include "Tools/tools.h"

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const u8 * img, int imgSize)
{
	data = NULL;
	AnimGif = NULL;
	width = 0;
	height = 0;
	format = GX_TF_RGBA8;

	if(img)
	{
		if (imgSize < 8)
		{
			return;
		}
		if (img[0] == 0xFF && img[1] == 0xD8)
		{
			// IMAGE_JPEG
			LoadJpeg(img, imgSize);
		}
		else if (img[0] == 0x49 && img[1] == 0x49)
		{
			// IMAGE_TIFF_PC
			LoadTIFF(img, imgSize);
		}
		else if (img[0] == 0x4D && img[1] == 0x4D)
		{
			// IMAGE_TIFF_MAC
			LoadTIFF(img, imgSize);
		}
		else if (img[0] == 'B' && img[1] == 'M')
		{
			// IMAGE_BMP
			LoadBMP(img, imgSize);
		}
		else if (img[0] == 'G' && img[1] == 'I' && img[2] == 'F')
		{
			// IMAGE_GIF
			LoadGIF(img, imgSize);
		}
		else if (img[0] == 0x89 && img[1] == 'P' && img[2] == 'N' && img[3] == 'G')
		{
			// IMAGE_PNG
			LoadPNG(img, imgSize);
		}
		else if ((img[0] == 0xFF && img[1] == 0xFF) || (img[0] == 0xFF && img[1] == 0xFE))
		{
			// IMAGE_GD
			LoadGD(img, imgSize);
		}
		else if (img[0] == 0x67 && img[1] == 0x64 && img[2] == 0x32 && img[3] == 0x00)
		{
			// IMAGE_GD2
			LoadGD2(img, imgSize);
		}
		else if (img[0] == 0x00 && img[1] == 0x20 && img[2] == 0xAF && img[3] == 0x30)
		{
			// IMAGE_TPL
			LoadTPL(img, imgSize);
		}
		//!This must be last since it can also intefere with outher formats
		else if(img[0] == 0x00)
		{
			// Try loading TGA image
			LoadTGA(img, imgSize);
		}
	}
}

/**
 * Destructor for the GuiImageData class.
 */
GuiImageData::~GuiImageData()
{
	if(data)
	{
		free(data);
		data = NULL;
	}

	if(AnimGif)
		delete AnimGif;
}

void GuiImageData::LoadPNG(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromPngPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadJpeg(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromJpegPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadGIF(const u8 *img, int imgSize)
{
	AnimGif = new GifImage(img, imgSize);
	if(AnimGif->GetFrameCount() > 1)
		return;

	delete AnimGif;
	AnimGif = NULL;

	gdImagePtr gdImg = gdImageCreateFromGifPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadGD(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromGdPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadGD2(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromGd2Ptr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadTIFF(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromTiffPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadBMP(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromBmpPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadTGA(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromTgaPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadTPL(const u8 *img, int imgSize)
{
	TplImage TplFile(img, imgSize);

	width = TplFile.GetWidth(0);
	height = TplFile.GetHeight(0);
	format = (u8) TplFile.GetFormat(0);

	const u8 * ImgPtr = TplFile.GetTextureBuffer(0);

	if(ImgPtr)
	{
		int len =  ALIGN32(TplFile.GetTextureSize(0));

		data = (u8 *) memalign(32, len);
		if(!data)
			return;

		memcpy(data, ImgPtr, len);
		DCFlushRange(data, len);
	}
}
