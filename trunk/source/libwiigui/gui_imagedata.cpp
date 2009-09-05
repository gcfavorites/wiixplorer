/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_imagedata.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

#include "Prompts/PromptWindows.h"

#include "libgif/gif_lib.h"
#include "libbmp/EasyBMP.h"
#include "libtga/tga.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <jpeglib.h>

#ifdef __cplusplus
}
#endif

#define new_width 640
#define new_height 480

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const u8 * img)
{
	data = NULL;
	width = 0;
	height = 0;

	if (img)
	{
		LoadPNG(img);
	}
}

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
		}
		else if (img[0] == 0x4D && img[1] == 0x4D) { // IMAGE_TIFF_MAC
		}
		else if (img[0] == 'B' && img[1] == 'M') { // IMAGE_BMP
			LoadBMP(img, imgSize);
		}
		else if (img[0] == 'G' && img[1] == 'I' && img[2] == 'F') { // IMAGE_GIF
			LoadGIF(img, imgSize);
		}
		else if (img[0] == 0x89 && img[1] == 'P' && img[2] == 'N' && img[3] == 'G') { // IMAGE_PNG
			LoadPNG(img);
		}
		else {
			LoadTGA(img, imgSize); // Try loading TGA image
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

void GuiImageData::LoadPNG(const u8 *img)
{
	PNGUPROP imgProp;
	IMGCTX ctx = PNGU_SelectImageFromBuffer(img);

	if(!ctx)
		return;

	int res = PNGU_GetImageProperties(ctx, &imgProp);

	if(res == PNGU_OK)
	{
		int len = imgProp.imgWidth * imgProp.imgHeight * 4;
		if(len%32) len += (32-len%32);
		data = (u8 *)memalign (32, len);

		if(data)
		{
			res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

			if(res == PNGU_OK)
			{
				width = imgProp.imgWidth;
				height = imgProp.imgHeight;
				DCFlushRange(data, len);
			}
			else
			{
				free(data);
				data = NULL;
			}
		}
	}
	PNGU_ReleaseImageContext (ctx);
}

// This function finds it's origin in GRRLIB, which can be found here: http://code.google.com/p/grrlib/
void GuiImageData::LoadJpeg(const u8 *img, int imgSize)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	int n = imgSize;

	while (n > 1) {
		if (img[n - 1] == 0xff && img[n] == 0xd9) {
			break;
		}
		n--;
	}

	jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(&jerr);
    cinfo.progress = NULL;
    jpeg_memory_src(&cinfo, img, n);
    jpeg_read_header(&cinfo, TRUE);
	jpeg_calc_output_dimensions(&cinfo);

	if (cinfo.output_width > new_width || cinfo.output_height > new_height)
	{
		float factor = (cinfo.output_width > cinfo.output_height) ? (1.0 * cinfo.output_width) / new_width : (1.0 * cinfo.output_height) / new_height;
		cinfo.scale_num = 1;
		cinfo.scale_denom = factor;
		cinfo.do_fancy_upsampling = true;
		cinfo.do_block_smoothing = false;
		cinfo.dct_method = JDCT_IFAST;
	}

    jpeg_start_decompress(&cinfo);

	int rowsize = cinfo.output_width * cinfo.num_components;
	unsigned char *tempBuffer = (unsigned char *) malloc(rowsize * cinfo.output_height);

    JSAMPROW row_pointer[1];

	row_pointer[0] = (unsigned char*) malloc(rowsize);
    size_t location = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
		memcpy(tempBuffer + location, row_pointer[0], rowsize);
		location += rowsize;
    }

	int len = ((cinfo.output_width+3)>>2)*((cinfo.output_height+3)>>2)*32*2;

    data = (u8 *) memalign(32, len);

	RawTo4x4RGBA(tempBuffer, data, cinfo.output_width, cinfo.output_height);
	DCFlushRange(data, len);

	width = cinfo.output_width;
	height = cinfo.output_height;

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
	free(tempBuffer);
}

void GuiImageData::LoadBMP(const u8 *img, int imgSize)
{
	BMP bmpFile;
	if (!bmpFile.ReadFromMem(img, imgSize))
	{
		return;
	}
	
	width = bmpFile.TellWidth();
	height = bmpFile.TellHeight();
	
	int len = ((width+3)>>2)*((height+3)>>2)*32*2;	
    data = (u8 *) memalign(32, len);

	bmpFile.DecodeTo4x4RGB8(data);
	DCFlushRange(data, len);	
}

void GuiImageData::LoadGIF(const u8 *img, int imgSize)
{
	GifFileType *gifFile = DGifOpenMem(img, imgSize);
	if (gifFile == NULL) 
	{
		return;
	}
	
	short transparentColor;

	GifRowType *rows = (GifRowType *) DGifDecompress(gifFile, &transparentColor);
	if (rows == NULL)
	{
		return;
	}
	
	width = gifFile->Image.Width;
	height = gifFile->Image.Height;
	
//	int len = width * height * 4;
	int len = ((width+3)>>2)*((height+3)>>2)*32*2;
    data = (u8 *) memalign(32, len);

	DGifDecodeTo4x4RGB8(gifFile, rows, data, transparentColor);

	DGifCloseMem(gifFile);
	gifFile = NULL;
	
	free(rows);
	rows = NULL;
}

/**
 * Convert a raw bmp (RGB, no alpha) to 4x4RGBA.
 * @author DragonMinded
 * @param src
 * @param dst
 * @param width
 * @param height
*/
void GuiImageData::RawTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int width, const unsigned int height)
{
    unsigned int block;
    unsigned int i;
    unsigned int c;
    unsigned int ar;
    unsigned int gb;
    unsigned char *p = (unsigned char*)dst;

    for (block = 0; block < height; block += 4) {
        for (i = 0; i < width; i += 4) {
            /* Alpha and Red */
            for (c = 0; c < 4; ++c) {
                for (ar = 0; ar < 4; ++ar) {
                    /* Alpha pixels */
                    *p++ = 255;
                    /* Red pixels */
                    *p++ = src[((i + ar) + ((block + c) * width)) * 3];
                }
            }

            /* Green and Blue */
            for (c = 0; c < 4; ++c) {
                for (gb = 0; gb < 4; ++gb) {
                    /* Green pixels */
                    *p++ = src[(((i + gb) + ((block + c) * width)) * 3) + 1];
                    /* Blue pixels */
                    *p++ = src[(((i + gb) + ((block + c) * width)) * 3) + 2];
                }
            }
        } /* i */
    } /* block */
}

void GuiImageData::LoadTGA(const u8 *img, int imgSize)
{
	TGA *tga = TGAOpenMem(img, imgSize);
	if (tga == NULL)
	{
		return;
	}
	TGAData tgaData;
	
	if (TGAReadImage(tga, &tgaData) != TGA_OK)
	{
		delete tga;
		return;
	}
	
	width = tga->hdr.width;
	height = tga->hdr.height;
	
//	int len = width * height * 4;
	int len = ((width+3)>>2)*((height+3)>>2)*32*2;
    data = (u8 *) memalign(32, len);

	TGADecodeTo4x4RGB8(tga, &tgaData, data);

	tga = NULL;
}
