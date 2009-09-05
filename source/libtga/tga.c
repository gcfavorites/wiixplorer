/*
 *  tga.c
 *
 *  Copyright (C) 2001-2002  Matthias Brueckner <matbrc@gmx.de>
 *  This file is part of the TGA library (libtga).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <stdio.h>
#include <stdlib.h>

#include "libtga/tga.h"

/* text strings corresponding to the error codes */
static char*
tga_error_strings[] = {
	"Success",
	"Error",
	"Out of memory",
	"Failed to open file",
	"Seek failed",
	"Read failed",
	"Write failed",
	"Unknown sub-format"
};
TGA*
TGAOpen(char *file, 
	char *mode)
{
 	TGA *tga;
	FILE *fd;

	tga = (TGA*)malloc(sizeof(TGA));
	if (!tga) {
		TGA_ERROR(tga, TGA_OOM);
		return NULL;
	}
	memset(tga, 0, sizeof(TGA));
	
	fd = fopen(file, mode);
	if (!fd) {
		TGA_ERROR(tga, TGA_OPEN_FAIL);
		free(tga);
		return NULL;
	}
	tga->fd = fd;
	tga->last = TGA_OK;
	return tga;
}

TGA*
TGAOpenFd(FILE *fd)
{
	TGA *tga;

	tga = (TGA*)malloc(sizeof(TGA));
	if (!tga) {
		TGA_ERROR(tga, TGA_OOM);
		return NULL;
	}
	memset(tga, 0, sizeof(TGA));

	if (!fd) {
		TGA_ERROR(tga, TGA_OPEN_FAIL);
		free(tga);
		return NULL;
	}

	tga->off = ftell(fd);
	if(tga->off == -1) {
		TGA_ERROR(tga, TGA_OPEN_FAIL);
		free(tga);
		return NULL;
	}
	
	tga->fd = fd;
	tga->last = TGA_OK;
	return tga;
}

TGA*
TGAOpenMem(const unsigned char *img, 
	int imgSize)
{
 	TGA *tga;

	tga = (TGA*)malloc(sizeof(TGA));
	if (!tga) {
		TGA_ERROR(tga, TGA_OOM);
		return NULL;
	}
	memset(tga, 0, sizeof(TGA));

	tga->imgData = img;
	tga->imgSize = imgSize;
	tga->last = TGA_OK;
	return tga;
}

void 
TGAClose(TGA *tga)
{
	if (tga) {
		fclose(tga->fd);
		free(tga);
	}
}


char*
TGAStrError(tuint8 code)
{
	if (code >= TGA_ERRORS) code = TGA_ERROR;
	return tga_error_strings[code];
}


tlong
__TGASeek(TGA  *tga, 
	  tlong off, 
	  int   whence)
{
	if (tga->fd) {
		fseek(tga->fd, off, whence);
		tga->off = ftell(tga->fd);
	} else {
		switch(whence) {
			case SEEK_SET:
				tga->off = off;
				break;
			case SEEK_CUR:
				tga->off += off;
				break;
			case SEEK_END:
				tga->off = tga->imgSize + off;
				break;
		}
	}
	return tga->off;
}


void
__TGAbgr2rgb(tbyte  *data, 
	     size_t  size, 
	     size_t  bytes)
{
	size_t i;
	tbyte tmp;
	
	for (i = 0; i < size; i += bytes) {
		tmp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = tmp;
	}
}

int TGADecodeTo4x4RGB8(TGA *tga, TGAData *tgaData, unsigned char *dst)
{
    unsigned int vert, hor, col, row;
    unsigned char *p = (unsigned char*)dst;

	unsigned int height = tga->hdr.height;
	unsigned int width = tga->hdr.width;

	// Write to 4x4 blocks, so read from 4 lines at once
	// Read 4 pixels from each line, which is 16 pixels in a block
	// Every block contains 4 bytes for each colors, which is 64 bytes
	// The first 32 bytes will be alpha and red, the next 32 pixels will be green and blue

	// For gif, it's the most easy to calculate the color of a pixel, and write the components
	// at once...
	unsigned char colorSize = 3;
	switch(tga->hdr.depth) {
		case 32: colorSize = 4; break;
		case 8: colorSize = 1; break;
	}

	// If the image width is not 4 by 4 exactly, 
	// add the amount of extra cells in transparent color
	for (vert = 0; vert < height + (height & 3); vert += 4) // height & 3 == height % 4, but a lot faster!
	{
		for (hor = 0; hor < width + (width & 3); hor += 4)
		{
			for (row = 0; row < 4; row++)
			{
				unsigned char *gb = (unsigned char *) (p + 32);
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
						
						int pixel = (vert + row) * (hor + col) * colorSize;
//						int pixel = (((vert + row) * width) + hor + col) * colorSize;
						unsigned char *srcData = tgaData->img_data + pixel;

						*p++ = 255;
						switch(tga->hdr.depth) {
							case 32:
								*p++ = *srcData;
								*gb++ = *(srcData + 1);
								*gb++ = *(srcData + 2);
								break;
							case 8:
								*p++ = *srcData > 0 ? 255 : 0;
								*gb++ = *srcData > 0 ? 255 : 0;
								*gb++ = *srcData > 0 ? 255 : 0;
								break;
							default:
								*p++ = *srcData * 255 / 31;
								*gb++ = *(srcData + 1) * 255 / 31;
								*gb++ = *(srcData + 2) * 255 / 31;
						}
					}
				}
			}
			p += 32;
		}
	}

	return TGA_OK;	
}
