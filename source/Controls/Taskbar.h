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
 * Taskbar.h
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#ifndef _TASKBAR_H
#define _TASKBAR_H

#include "libwiigui/gui.h"
#include "libwiigui/PictureButton.h"

class Taskbar : public GuiWindow
{
	public:
		static Taskbar *Instance();
        static void DestroyInstance();

        void SetMenu(int m);
        int GetMenu();

		virtual void SetState(int s, int c = -1);
        void ResetState();
		virtual void SetDim(bool d);
	protected:
        void Update(GuiTrigger * t);
		void Draw();
		void OnExitClick(GuiElement *sender, int pointer, POINT p);
		void OnSettingsClick(GuiElement *sender, int pointer, POINT p);
	private:
		Taskbar();
		~Taskbar();

		static Taskbar *instance;

        int menu;

		GuiImageData *taskbarImgData;
		GuiImage *taskbarImg;
		GuiText *timeTxt;
		PictureButton *settingsBtn;
		PictureButton *exitBtn;
		GuiTrigger *trigA;
		GuiSound *soundClick, *soundOver;
};

#endif // _TASKBAR_H
