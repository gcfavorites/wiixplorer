/***************************************************************************
 * Copyright (C) 2010
 * Dimok
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
 * TplImage.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include "FileOperations/fileops.h"
#include "TextureConverter.h"
#include "TplImage.h"

#define TPL_FORMAT_I4		0
#define TPL_FORMAT_I8		1
#define TPL_FORMAT_IA4		2
#define TPL_FORMAT_IA8		3
#define TPL_FORMAT_RGB565	4
#define TPL_FORMAT_RGB5A3	5
#define TPL_FORMAT_RGBA8	6
#define TPL_FORMAT_CI4		8
#define TPL_FORMAT_CI8		9
#define TPL_FORMAT_CI14X2	10
#define TPL_FORMAT_CMP		14

TplImage::TplImage(const char * filepath)
{
    TPLBuffer = NULL;
    TPLSize = 0;

    u8 * buffer = NULL;
    u64 filesize = 0;
    LoadFileToMem(filepath, &buffer, &filesize);

    if(buffer)
    {
        LoadImage(buffer, filesize);
        free(buffer);
    }
}

TplImage::TplImage(const u8 * imgBuffer, u32 imgSize)
{
    TPLBuffer = NULL;
    TPLSize = 0;

    if(imgBuffer)
    {
        LoadImage(imgBuffer, imgSize);
    }
}

TplImage::~TplImage()
{
    if(TPLBuffer)
        free(TPLBuffer);
}

bool TplImage::LoadImage(const u8 * imgBuffer, u32 imgSize)
{
    if(TPLBuffer)
        free(TPLBuffer);

    TPLBuffer = NULL;
    TPLSize = 0;

    if(!imgBuffer)
        return false;

    TPLBuffer = (u8 *) malloc(imgSize);
    if(!TPLBuffer)
        return false;

    TPLSize = imgSize;

    memcpy(TPLBuffer, imgBuffer, imgSize);

    return ParseTplFile();
}

bool TplImage::ParseTplFile()
{
    if(!TPLBuffer)
        return false;

    TPLHeader = (const TPL_Header *) TPLBuffer;

    if(TPLHeader->magic != 0x0020AF30)
        return false;

    if(TPLHeader->head_size != 12)
        return false;

    const TPL_Texture * curTexture = (const TPL_Texture *) (TPLHeader+1);

    for(u32 i = 0; (i < TPLHeader->num_textures) && (i < MAX_TPL_TEXTURES); i++)
    {
        Texture[i] = curTexture;

        TextureHeader[i] = (const TPL_Texture_Header *) ((const u8 *) TPLBuffer+Texture[i]->text_header_offset);

        TplTextureBuffer[i] = TPLBuffer + TextureHeader[i]->offset;

        curTexture++;
    }

    return true;

}

int TplImage::GetWidth(int Texture)
{
    if(Texture < 0 || Texture >= MAX_TPL_TEXTURES || Texture >= (int) TPLHeader->num_textures)
    {
        return 0;
    }

    return TextureHeader[Texture]->width;
}

int TplImage::GetHeight(int Texture)
{
    if(Texture < 0 || Texture >= MAX_TPL_TEXTURES || Texture >= (int) TPLHeader->num_textures)
    {
        return 0;
    }

    return TextureHeader[Texture]->height;
}

const u8 * TplImage::GetTextureBuffer(int Texture)
{
    if(Texture < 0 || Texture >= MAX_TPL_TEXTURES || Texture >= (int) TPLHeader->num_textures)
    {
        return 0;
    }

    return TplTextureBuffer[Texture];
}

gdImagePtr TplImage::ConvertToGD(int Texture)
{
    if(Texture < 0 || Texture >= MAX_TPL_TEXTURES || Texture >= (int) TPLHeader->num_textures)
    {
        return 0;
    }

    gdImagePtr gdImg = 0;

    switch(TextureHeader[Texture]->format)
    {
        case TPL_FORMAT_RGB565:
            RGB565ToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        case TPL_FORMAT_RGB5A3:
            RGB565A3ToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        case TPL_FORMAT_RGBA8:
            RGBA8ToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        case TPL_FORMAT_I4:
            I4ToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        case TPL_FORMAT_I8:
            I8ToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        case TPL_FORMAT_IA4:
            IA4ToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        case TPL_FORMAT_IA8:
            IA8ToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        case TPL_FORMAT_CMP:
            CMPToGD(TplTextureBuffer[Texture], TextureHeader[Texture]->width, TextureHeader[Texture]->height, &gdImg);
            break;
        default:
            gdImg = 0;
            break;
    }

    return gdImg;
}
