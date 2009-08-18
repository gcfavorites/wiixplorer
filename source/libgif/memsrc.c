
#include <string.h>
#include <stdlib.h>

#include "gif_lib.h"

#define GIF_TRANSPARENT 	0x01
#define GIF_NOT_TRANSPARENT	-1

typedef struct GifMemoryType {
	void *imgData;
	int imgSize;
	int currentPosition;
} GifMemoryType;

int DGifCurrentPos(GifFileType *gifFile)
{
	GifMemoryType *gifMemoryType = (GifMemoryType *) gifFile->UserData;
	return gifMemoryType == NULL ? -1 : gifMemoryType->currentPosition;
}

int DGifOpenFromMemoryCallback(GifFileType *gifFile, GifByteType *gifByteType, int length)
{
	GifMemoryType *gifMemoryType = (GifMemoryType *) gifFile->UserData;

	if (length + gifMemoryType->currentPosition > gifMemoryType->imgSize)
	{
		return D_GIF_ERR_READ_FAILED;
	}

	void *start = (gifMemoryType->imgData + gifMemoryType->currentPosition);
	memcpy(gifByteType, start, length);
	gifMemoryType->currentPosition += length;

	return length;
}

int DGifCloseMem(GifFileType *gifFile)
{
	// First, free the GifMemoryType
	if (gifFile->UserData != NULL)
	{
		GifMemoryType *gifMemoryType = (GifMemoryType *)gifFile->UserData;
		free(gifMemoryType);
	}
	return DGifCloseFile(gifFile);
}

GifFileType *DGifOpenMem(const unsigned char *img, int imgSize)
{
	GifMemoryType *gifMemoryType = (GifMemoryType *) malloc(sizeof(GifMemoryType));
	memset((void *) gifMemoryType, 0, sizeof(GifMemoryType));
	gifMemoryType->imgData = (void *) img;
	gifMemoryType->imgSize = imgSize;

	return DGifOpen(gifMemoryType, DGifOpenFromMemoryCallback);
}

int DGifDecodeTo4x4RGB8(GifFileType *gifFile, GifRowType *rowType, void *dst, short transparentColor)
{
    unsigned int vert, hor, col, row;
    unsigned char *p = (unsigned char*)dst;

	unsigned int height = gifFile->Image.Height;
	unsigned int width = gifFile->Image.Width;

    ColorMapObject *colorMap = (gifFile->Image.ColorMap
		? gifFile->Image.ColorMap
		: gifFile->SColorMap);

	if (colorMap == NULL)
	{
		return GIF_ERROR;
	}

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
				GifRowType gifRow = rowType[vert + row];
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
						int color = *(char*) (gifRow + hor + col);
						GifColorType *colorMapEntry = &colorMap->Colors[color];
						*p++ = color == transparentColor ? 0 : 255;
						*p++ = colorMapEntry->Red;
						*gb++ = colorMapEntry->Green;
						*gb++ = colorMapEntry->Blue;
					}
				}
			}
			p += 32;
		}
	}

	return GIF_OK;
}

GifRowType *DGifDecompress(GifFileType *gifFile, short *transparentColor)
{
	static int     
		InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
		InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

	int i,j, row, col, width, height = 0;
	GifRowType *ScreenBuffer = NULL;    

	int rowSize = gifFile->SWidth * sizeof(GifPixelType); /* Size in bytes one row.*/
	
	// The size of the screenbuffer should be width * height, every pixel contains a number,
	// which is a mapping in the colormap.

    ScreenBuffer = (GifRowType *) malloc(gifFile->SHeight * sizeof(GifRowType *));
	if (ScreenBuffer == NULL) {
	    return NULL;
	}

    int Size = gifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    ScreenBuffer[0] = (GifRowType) malloc(Size);
	if (ScreenBuffer[0] == NULL) {
	    goto cleanup;
	}

	for (i = 0; i < gifFile->SWidth; i++) {  /* Set its color to BackGround. */
		ScreenBuffer[0][i] = gifFile->SBackGroundColor;
	}
    for (i = 1; i < gifFile->SHeight; i++) {
		/* Allocate the other rows, and set their color to background too: */
		ScreenBuffer[i] = (GifRowType) malloc(Size);
		if (ScreenBuffer[i] == NULL) {
			goto cleanup;
		}
		memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
    }

	*transparentColor = GIF_NOT_TRANSPARENT;

	GifRecordType recordType;
	do
	{
		if (DGifGetRecordType(gifFile, &recordType) == GIF_ERROR)
		{
			goto cleanup;
		}
		switch(recordType)
		{
			case IMAGE_DESC_RECORD_TYPE:
			{				
				if (DGifGetImageDesc(gifFile) == GIF_ERROR)
				{
					goto cleanup;
				}
			
				row = gifFile->Image.Top; /* Image Position relative to Screen. */
				col = gifFile->Image.Left;
				width = gifFile->Image.Width;
				height = gifFile->Image.Height;
	
				GifPixelType *gifPixelType;
				int location = 0;
				if (gifFile->Image.Interlace)
				{
					// Read interlaced image
					/* Need to perform 4 passes on the images: */
					for (i = 0; i < 4; i++)
					{
						for (j = row + InterlacedOffset[i]; j < row + height; j += InterlacedJumps[i]) 
						{
							gifPixelType = &ScreenBuffer[j][col];
							if (DGifGetLine(gifFile, gifPixelType, width) == GIF_ERROR) {
								goto cleanup;
							}
							location += rowSize;
						}
					}
				}
				else
				{
					// Read non-interlaced image
					for (i = 0; i < height; i++) {
						if (DGifGetLine(gifFile, &ScreenBuffer[row++][col], width) == GIF_ERROR) {
							goto cleanup;
						}
					}
				}
				break;
			}
			case EXTENSION_RECORD_TYPE:
			{
				GifByteType *extension;
				int extCode;
				if (DGifGetExtension(gifFile, &extCode, &extension) == GIF_ERROR)
				{
					goto cleanup;
				}
				switch(extCode)
				{
					case GRAPHICS_EXT_FUNC_CODE:
					{
						int flag = extension[1];
						*transparentColor = (flag & GIF_TRANSPARENT) ? extension[4] : GIF_NOT_TRANSPARENT;
						break;
					}
				}
				while(extension != NULL)
				{
					if (DGifGetExtensionNext(gifFile, &extension) == GIF_ERROR)
					{
						goto cleanup;
					}
				}
				break;
			}
			case TERMINATE_RECORD_TYPE:
			case UNDEFINED_RECORD_TYPE:
			default:
				break;
		}
	} while (recordType != TERMINATE_RECORD_TYPE);
	
	goto finish;

cleanup:
	if (ScreenBuffer != NULL)
	{
		free(ScreenBuffer);
		ScreenBuffer = NULL;
	}	
finish:
	return ScreenBuffer;
}
