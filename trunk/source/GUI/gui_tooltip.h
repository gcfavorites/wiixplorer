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
#ifndef GUI_TOOLTIP_H_
#define GUI_TOOLTIP_H_

#include "GUI/gui_element.h"
#include "GUI/gui_text.h"
#include "GUI/gui_image.h"
#include "GUI/gui_imagedata.h"

class GuiTooltip : public GuiElement
{
	public:
		//!Constructor
		GuiTooltip(const char *t);
		//!Destructor
		virtual ~GuiTooltip();
		//!Sets GuiTooltip text
		void SetText(const char * t);
		//!Set the fontsize of the text
		void SetFontSize(int size);
		//!Set the color of the text
		void SetColor(GXColor c);
		//!Set the min time before draw call
		void SetElapseTime(float t);
		//!Get the min time before draw call
		float GetElapseTime() { return ElapseTime; };
		//!Draw callback
		void Draw();
	protected:
		float ElapseTime;
		int FontSize;
		GXColor color;
		GuiImage * leftImage;
		GuiImage * tileImage;
		GuiImage * rightImage;
		GuiImageData * tooltipLeft;
		GuiImageData * tooltipTile;
		GuiImageData * tooltipRight;
		GuiText * text;
};

#endif
