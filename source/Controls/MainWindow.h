/***************************************************************************
 * Copyright (C) 2009
 * by r-win & Dimok
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
 * MainWindow.h
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include "libwiigui/gui.h"
#include <list>

class MainWindow : public GuiWindow
{
	public:
		static MainWindow * Instance();

		static void DestroyInstance();

		void Append(GuiElement *w);
		void Append(GuiWindow *w);
		void HaltGui();
		void ResumeGui();

		void Quit();
		void Show();
		void ChangeVolume(int v);
		void LoadMusic(const u8 * file, u32 filesize, int mode);
	private:
		MainWindow();
		~MainWindow();
		static void * UpdateGUI(void *arg);
		void InternalUpdateGUI();

		void AddWindow(GuiWindow *window);

		static MainWindow *instance;

		bool guihalt;
		bool exitApplication;
		lwp_t guithread;

		GuiImageData *bgImgData;
		GuiSound *bgMusic;
		GuiImageData *pointer[4];
		GuiImage *bgImg;

		std::list<GuiWindow *> windows;
};

#endif //_MAINWINDOW_H
