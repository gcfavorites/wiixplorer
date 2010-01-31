/***************************************************************************
 * Copyright (C) 2009
 * by Dimok & r-win
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
 * for WiiXplorer 2009
 ***************************************************************************/
#include "gui.h"

#define MAXWIDTH 1024.0f
#define MAXHEIGHT 768.0f

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const u8 * img, int imgSize)
{
	data = NULL;
	width = 0;
	height = 0;

	if(img)
	{
		if (imgSize < 8) {
			return;
		}
		if (img[0] == 0xFF && img[1] == 0xD8) { // IMAGE_JPEG
			LoadJpeg(img, imgSize);
		}
		else if (img[0] == 0x49 && img[1] == 0x49) { // IMAGE_TIFF_PC
            LoadTIFF(img, imgSize);
		}
		else if (img[0] == 0x4D && img[1] == 0x4D) { // IMAGE_TIFF_MAC
            LoadTIFF(img, imgSize);
		}
		else if (img[0] == 'B' && img[1] == 'M') { // IMAGE_BMP
			LoadBMP(img, imgSize);
		}
		else if (img[0] == 'G' && img[1] == 'I' && img[2] == 'F') { // IMAGE_GIF
			LoadGIF(img, imgSize);
		}
		else if (img[0] == 0x89 && img[1] == 'P' && img[2] == 'N' && img[3] == 'G') { // IMAGE_PNG
			LoadPNG(img, imgSize);
		}
		else if ((img[0] == 0xFF && img[1] == 0xFF) || (img[0] == 0xFF && img[1] == 0xFE)) { // IMAGE_GD
			LoadGD(img, imgSize);
		}
		else if (img[0] == 0x67 && img[1] == 0x64 && img[2] == 0x32 && img[3] == 0x00) { // IMAGE_GD2
			LoadGD2(img, imgSize);
		}
		//!This must be last since it can also intefere with outher formats
		else if(img[0] == 0x00) { // Try loading TGA image
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
}

u8 * GuiImageData::GetImage()
{
	return data;
}

int GuiImageData::GetWidth()
{
	return width;
}

int GuiImageData::GetHeight()
{
	return height;
}

void GuiImageData::LoadPNG(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromPngPtr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

// This function finds it's origin in GRRLIB, which can be found here: http://code.google.com/p/grrlib/
void GuiImageData::LoadJpeg(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromJpegPtr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

void GuiImageData::LoadGIF(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromGifPtr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

void GuiImageData::LoadGD(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromGdPtr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

void GuiImageData::LoadGD2(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromGd2Ptr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

void GuiImageData::LoadTIFF(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromTiffPtr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

void GuiImageData::LoadBMP(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromBmpPtr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

void GuiImageData::LoadTGA(const u8 *img, int imgSize)
{
    gdImagePtr gdImg = gdImageCreateFromTgaPtr(imgSize, (u8*) img);
    if(gdImg == 0)
        return;

    GDImageToRGBA8(gdImg);
    gdImageDestroy(gdImg);
}

static inline u32 coordsRGBA8(u32 x, u32 y, u32 w)
{
	return ((((y >> 2) * (w >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
}

void GuiImageData::GDImageToRGBA8(gdImagePtr & gdImg)
{
	width = gdImageSX(gdImg);
	height = gdImageSY(gdImg);
    int origwidth = width;
    int origheight = height;

    int newwidth = width;
    int newheight = height;
    float scale = 1.0f;
    int retries = 100;  //shouldn't need that long but to be sure

    while(newwidth*scale > MAXWIDTH || newheight*scale > MAXHEIGHT)
    {
        if(newwidth*scale > MAXWIDTH)
            scale = MAXWIDTH/newwidth;
        if(newheight*scale > MAXHEIGHT)
            scale = MAXHEIGHT/newheight;

        retries--;

        if(!retries)
            break;
    }

    newwidth = (int) newwidth * scale;
    newheight = (int) newheight * scale;

    if(newwidth % 4)
        newwidth -= newwidth % 4;

    if(newheight % 4)
        newheight -= newheight % 4;

    if(newwidth != width || newheight != height)
    {
        gdImagePtr dst = gdImageCreateTrueColor(newwidth, newheight);
        gdImageCopyResized(dst, gdImg, 0, 0, 0, 0, newwidth, newheight, width, height);

        gdImageDestroy(gdImg);
        gdImg = dst;

        width = gdImageSX(gdImg);
        height = gdImageSY(gdImg);
    }

    int len =  ((width+3)>>2)*((height+3)>>2)*32*2;
    if(len%32)
        len += (32-len%32);

    data = (u8 *) memalign (32, len);
    if(!data)
        return;

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            u32 p;
            if(x >= origwidth || y >= origheight)
                p = 0;
            else
                p = gdImageGetPixel(gdImg, x, y);

            u8 a = 254 - 2*((u8)gdImageAlpha(gdImg, p));
            if(a == 254) a++;
            u8 r = (u8)gdImageRed(gdImg, p);
            u8 g = (u8)gdImageGreen(gdImg, p);
            u8 b = (u8)gdImageBlue(gdImg, p);

            u32 offset = coordsRGBA8(x, y, width);
            data[offset] = a;
            data[offset+1] = r;
            data[offset+32] = g;
            data[offset+33] = b;
        }
    }

    DCFlushRange(data, len);
}
