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
 * bmp.h
 *
 * for Wii-FileXplorer 2009
 *
 * File specification taken from: http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
 ***************************************************************************/

#ifndef _BMP_H
#define _BMP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gctypes.h>

typedef struct bmp_file_header
{
	u8 type[2];			// must always be set to 'BM' to declare that this is a .bmp-file.
	u32 size;				// specifies the size of the file in bytes.
	u16 reserved1;		// must always be set to zero.
	u16 reserved2;		// must always be set to zero.
	u32 offBits;			// specifies the offset from the beginning of the file to the bitmap data.
} bmp_file_header;

typedef struct bmp_info_header
{
	u32 size;				// specifies the size of the BITMAPINFOHEADER structure, in bytes.
	u32 width;			// specifies the width of the image, in pixels.
	u32 height;			// specifies the height of the image, in pixels.
	u16 planes;			// specifies the number of planes of the target device, must be set to zero.
	u16 bitCount;		// specifies the number of bits per pixel.
	u32 compression;		// Specifies the type of compression, usually set to zero (no compression).
	u32 sizeImage;		// specifies the size of the image data, in bytes. If there is no compression, it is valid to set this member to zero.
	u32 xPelsPerMeter;	// specifies the the horizontal pixels per meter on the designated target device, usually set to zero.
	u32 yPelsPerMeter;	// specifies the the vertical pixels per meter on the designated target device, usually set to zero.
	u32 clrUsed;			// specifies the number of colors used in the bitmap, if set to zero the number of colors is calculated using the biBitCount member.
	u32 clrImportant;		// specifies the number of color that are 'important' for the bitmap, if set to zero, all colors are important.
} bmp_info_header;

typedef struct BmpFile
{
	struct bmp_file_header file_header;
	struct bmp_info_header info_header;
	u8 bottomUp;
	u8 *data;
} BmpFile;

typedef struct BmpData
{
	struct rgbQuad **data;
} BmpData;

typedef struct rgbQuad
{
	u8 blue;				// specifies the blue part of the color.
	u8 green;			// specifies the green part of the color.
	u8 red;				// specifies the red part of the color.
	u8 reserved;			// must always be set to zero.
} rgbQuad;

BmpFile *BmpOpenMem(const u8 *img, int imgSize);
BmpData *BmpDecompress(BmpFile *bmpFile);
int BmpDecodeTo4x4RGB8(BmpFile *bmpFile, BmpData *bmpData, void *dst);
void BmpFreeData(BmpFile *bmpFile, BmpData *bmpData);
void BmpCloseMem(BmpFile *file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _BMP_H
