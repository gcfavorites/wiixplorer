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
#include "PictureButton.h"

class Taskbar : public GuiWindow
{
	public:
		static Taskbar *Instance();

		static void DestroyInstance();
	protected:
        void Update(GuiTrigger * t);
		void Draw();
	private:
		Taskbar();
		~Taskbar();

		static Taskbar *instance;

		GuiImageData *taskbarImgData;
		GuiImage *taskbarImg;
		GuiText *timeTxt;
		PictureButton *settingsBtn;
		PictureButton *exitBtn;
		GuiSound *soundClick;
		GuiSound *soundOver;
		GuiTrigger *trigA;
};

#endif // _TASKBAR_H
