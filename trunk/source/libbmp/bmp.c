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
 * bmp.c
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "bmp.h"

BmpFile *BmpOpenMem(const u8 *img, int imgSize)
{
	u8 *ptr = (u8 *) img;

	BmpFile *fileType = (BmpFile *) malloc(sizeof(BmpFile));
	memset((void *) fileType, 0, sizeof(BmpFile));
	
	// Read first header...
	memcpy(&fileType->file_header, ptr, sizeof(bmp_file_header));
	ptr += sizeof(bmp_file_header);

	
	memcpy(&fileType->info_header, ptr, sizeof(bmp_info_header));
	ptr += sizeof(bmp_info_header);
	fileType->bottomUp = fileType->info_header.height > 0;
	if (fileType->info_header.height < 0) fileType->info_header.height *= -1;

	fileType->data = (u8*) malloc(fileType->info_header.sizeImage);
	memcpy(fileType->data, ptr, fileType->info_header.sizeImage);

	return fileType;
}

BmpData *BmpDecompress(BmpFile *bmpFile)
{
	BmpData *bmpData = (BmpData *) malloc(sizeof(BmpData));
	
	int width_useless = bmpFile->info_header.width & 3; // The width of a row must always dividable by four, so padding can occur.
	int i, j;
	
	u8 *p = (u8 *) bmpFile->data;
	
	// Allocate a buffer to contain the rows
	bmpData->data = (struct rgbQuad **) malloc(bmpFile->info_header.height * sizeof(struct rgbQuad *));
	
	for (i = 0; i < bmpFile->info_header.height; i++)
	{
		int line = bmpFile->bottomUp ? bmpFile->info_header.height - i - 1 : i;

		bmpData->data[line] = (struct rgbQuad *) malloc((bmpFile->info_header.width - width_useless) * sizeof(struct rgbQuad));
		if (!(bmpData->data[line])) {
			return NULL;
		}
		for (j = 0; j < bmpFile->info_header.width; j++)
		{
			bmpData->data[line][j].blue = *p++;
			bmpData->data[line][j].green = *p++;
			bmpData->data[line][j].red = *p++;
			bmpData->data[line][j].reserved = 0;
		}
		p += width_useless;	// Strip useless bytes off
	}	
	
	return bmpData;
}

int BmpDecodeTo4x4RGB8(BmpFile *bmpFile, BmpData *bmpData, void *dst)
{
    unsigned int vert, hor, col, row;
    u8 *p = (u8 *)dst;

	u32 height = bmpFile->info_header.height;
	u32 width = bmpFile->info_header.width;

	// Write to 4x4 blocks, so read from 4 lines at once
	// Read 4 pixels from each line, which is 16 pixels in a block
	// Every block contains 4 bytes for each colors, which is 64 bytes
	// The first 32 bytes will be alpha and red, the next 32 pixels will be green and blue

	// For gif, it's the most easy to calculate the color of a pixel, and write the components
	// at once...

	// If the image width is not 4 by 4 exactly, 
	// add the amount of extra cells in transparent color
	for (vert = 0; vert < height + (height & 3); vert += 4) // height & 3 == height % 4, but a lot faster!
	{
		for (hor = 0; hor < width + (width & 3); hor += 4)
		{
			for (row = 0; row < 4; row++)
			{
				struct rgbQuad *bmpRow = bmpData->data[vert + row];
				u8 *gb = (u8 *) (p + 32);
				for (col = 0; col < 4; col++)
				{
					if (vert + row >= height || // Transparent cell
						hor + col >= width)
					{
						*p++ = 0;
						*p++ = 255;
						*gb++ = 255;
						*gb++ = 255;
					}
					else
					{
						// Get the color of this pixel, which is at 
						// Line = vert + row, Col = hor + col
						struct rgbQuad *rgb = bmpRow + hor + col;
						*p++ = 255;
						*p++ = rgb->red;
						*gb++ = rgb->green;
						*gb++ = rgb->blue;
					}
				}
			}
			p += 32;
		}
	}

	return 0;
}

void BmpFreeData(BmpFile *bmpFile, BmpData *bmpData)
{
//	int i;
//	for (i = 0; i < bmpFile->info_header.height; i++)
//	{
//		free(bmpData->data[i]);
//	}
	free(bmpData->data);
}

void BmpCloseMem(BmpFile *file)
{
	free(file->data);
	free(file);
}
