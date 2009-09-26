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
 * Window.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#include "Window.h"

Window::Window(int w, int h)
	: GuiWindow(w, h)
{
	tlData= new GuiImageData((const u8 *) "");
	trData= new GuiImageData((const u8 *)"");
	blData= new GuiImageData((const u8 *)"");
	brData= new GuiImageData((const u8 *)"");
	lData= new GuiImageData((const u8 *)"");
	rData= new GuiImageData((const u8 *)"");
	tData= new GuiImageData((const u8 *)"");
	bData= new GuiImageData((const u8 *)"");
	bckData= new GuiImageData((const u8 *)"");
	titleData= new GuiImageData((const u8 *)"");
	tl= new GuiImage(tlData);
	tr= new GuiImage(trData);
	bl= new GuiImage(blData);
	br= new GuiImage(brData);
	l= new GuiImage(lData);
	r= new GuiImage(rData);
	t= new GuiImage(tData);
	b= new GuiImage(bData);
	bck= new GuiImage(bckData);
	title= new GuiImage(titleData);
	
	// Calculate new width and height
	width += l->GetWidth() + r->GetWidth();
	height += t->GetHeight() + b->GetHeight();
}

Window::~Window()
{
	delete tlData;
	delete trData;
	delete blData;
	delete brData;
	delete lData;
	delete rData;
	delete tData;
	delete bData;
	delete bckData;
	delete titleData;
	delete tl;
	delete tr;
	delete bl;
	delete br;
	delete l;
	delete r;
	delete t;
	delete b;
	delete bck;
	delete title;
}

void Window::Draw()
{
	// Get close button click
	// Get minimize button click
	
	GuiWindow::Draw();
	
	// Get the clicks
	
}

void Window::OnClose(CloseEvent close)
{
	
}
