/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_image.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
/**
 * Constructor for the GuiImage class.
 */
GuiImage::GuiImage()
{
	image = NULL;
	width = 0;
	height = 0;
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = false;
	imgType = IMAGE_DATA;
}

GuiImage::GuiImage(GuiImageData * img)
{
	image = NULL;
	width = 0;
	height = 0;
	widescreen = false;
	if(img)
	{
		image = img->GetImage();
		width = img->GetWidth();
		height = img->GetHeight();
	}
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	imgType = IMAGE_DATA;
}

GuiImage::GuiImage(u8 * img, int w, int h)
{
	image = img;
	width = w;
	height = h;
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = false;
	imgType = IMAGE_TEXTURE;
}

GuiImage::GuiImage(int w, int h, GXColor c)
{
	int len = w*h*4;
	if(len%32)
        len += (32-len%32);

	image = (u8 *)memalign (32, len);
	width = w;
	height = h;
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	imgType = IMAGE_COLOR;
	widescreen = false;

	if(!image)
		return;

	int x, y;

	for(y=0; y < h; y++)
	{
		for(x=0; x < w; x++)
		{
			this->SetPixel(x, y, c);
		}
	}
	DCFlushRange(image, len);
}

/**
 * Destructor for the GuiImage class.
 */
GuiImage::~GuiImage()
{
	if(imgType == IMAGE_COLOR && image)
		free(image);
}

u8 * GuiImage::GetImage()
{
	return image;
}

void GuiImage::SetImage(GuiImageData * img)
{
    LOCK(this);
	image = NULL;
	width = 0;
	height = 0;
	if(img)
	{
		image = img->GetImage();
		width = img->GetWidth();
		height = img->GetHeight();
	}
	imgType = IMAGE_DATA;
}

void GuiImage::SetImage(u8 * img, int w, int h)
{
    LOCK(this);
	image = img;
	width = w;
	height = h;
	imgType = IMAGE_TEXTURE;
}

float GuiImage::GetAngle()
{
	return imageangle;
}

void GuiImage::SetAngle(float a)
{
    LOCK(this);
	imageangle = a;
}

void GuiImage::SetTileHorizontal(int t)
{
    LOCK(this);
	tileHorizontal = t;
}

void GuiImage::SetTileVertical(int t)
{
    LOCK(this);
	tileVertical = t;
}

GXColor GuiImage::GetPixel(int x, int y)
{
	if(!image || this->GetWidth() <= 0 || x < 0 || y < 0)
		return (GXColor){0, 0, 0, 0};

	u32 offset = (((y >> 2)<<4)*this->GetWidth()) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
	GXColor color;
	color.a = *(image+offset);
	color.r = *(image+offset+1);
	color.g = *(image+offset+32);
	color.b = *(image+offset+33);
	return color;
}

void GuiImage::SetPixel(int x, int y, GXColor color)
{
    LOCK(this);
	if(!image || this->GetWidth() <= 0 || x < 0 || y < 0)
		return;

	u32 offset = (((y >> 2)<<4)*this->GetWidth()) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
	*(image+offset) = color.a;
	*(image+offset+1) = color.r;
	*(image+offset+32) = color.g;
	*(image+offset+33) = color.b;
}

void GuiImage::SetStripe(int s)
{
    LOCK(this);
	stripe = s;
}

void GuiImage::SetWidescreen(bool w)
{
    LOCK(this);
    widescreen = w;
}

void GuiImage::ColorStripe(int shift)
{
	int x, y;
	GXColor color;
	int alt = 0;

	for(y=0; y < this->GetHeight(); y++)
	{
		if(y % 3 == 0)
			alt ^= 1;

		for(x=0; x < this->GetWidth(); x++)
		{
			color = GetPixel(x, y);

			if(alt)
			{
				if(color.r < 255-shift)
					color.r += shift;
				else
					color.r = 255;
				if(color.g < 255-shift)
					color.g += shift;
				else
					color.g = 255;
				if(color.b < 255-shift)
					color.b += shift;
				else
					color.b = 255;

				color.a = 255;
			}
			else
			{
				if(color.r > shift)
					color.r -= shift;
				else
					color.r = 0;
				if(color.g > shift)
					color.g -= shift;
				else
					color.g = 0;
				if(color.b > shift)
					color.b -= shift;
				else
					color.b = 0;

				color.a = 255;
			}
			SetPixel(x, y, color);
		}
	}
}

/**
 * Draw the button on screen
 */
void GuiImage::Draw()
{
	if(!image || !this->IsVisible() || tileVertical == 0 || tileHorizontal == 0)
		return;

    LOCK(this);

	float currScale = this->GetScale();
	int currLeft = this->GetLeft();
	int currTop = this->GetTop();

    if(tileHorizontal > 0 && tileVertical > 0)
    {
        for(int n=0; n<tileVertical; n++)
            for(int i=0; i<tileHorizontal; i++)
                Menu_DrawImg(currLeft+width*i, currTop+width*n, width, height, image, imageangle, currScale, currScale, this->GetAlpha());
    }
    else if(tileHorizontal > 0)
    {
        for(int i=0; i<tileHorizontal; i++)
            Menu_DrawImg(currLeft+width*i, currTop, width, height, image, imageangle, currScale, currScale, this->GetAlpha());
    }
    else if(tileVertical > 0)
    {
        for(int i=0; i<tileVertical; i++)
            Menu_DrawImg(currLeft, currTop+height*i, width, height, image, imageangle, currScale, currScale, this->GetAlpha());
    }
	else
	{
		// temporary (maybe), used to correct offset for scaled images
		if(scale != 1)
			currLeft = currLeft - width/2 + (width*scale)/2;

		Menu_DrawImg(currLeft, currTop, width, height, image, imageangle, ((widescreen) ? (currScale*screenwidth/VI_MAX_WIDTH_PAL) : currScale), currScale, this->GetAlpha());
	}

	if(stripe > 0)
		for(int y=0; y < this->GetHeight(); y+=6)
			Menu_DrawRectangle(currLeft,this->GetTop()+y,this->GetWidth(),3,(GXColor){0, 0, 0, stripe},1);

	this->UpdateEffects();
}
