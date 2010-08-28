/***************************************************************************
 * Copyright (C) 2009
 * by r-win
 *
 * Copyright (C) 2010
 * by dude
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
 * for WiiXplorer 2010
 ***************************************************************************/

#ifndef _TASKBAR_H
#define _TASKBAR_H

#include <queue>
#include "libwiigui/gui.h"
#include "libwiigui/PictureButton.h"
#include "Prompts/PopUpMenu.h"
#include "Controls/Callback.hpp"
#include "Controls/Task.hpp"

enum
{
    APPS = 0,
    CHANNELS,
    URLS,
    FORMATTER,
    SETTINGS,
    FTPSERVER,
    RELOAD,
    RESTART,
    EXIT,
};

class Taskbar : public GuiWindow
{
	public:
		static Taskbar *Instance();
        static void DestroyInstance();

        void SetMenu(int m);
        int GetMenu();

		virtual void SetState(int s, int c = -1);
        void ResetState();
        void SetTriggerUpdate(bool b) { triggerupdate = b; };
		virtual void SetDim(bool d);
		void AddTask(Task * t);
		void RemoveTask(Task * t);
	protected:
        void Update(GuiTrigger * t);
		void Draw();

	private:
		Taskbar();
		~Taskbar();

		int CheckHomeButton();
		int CheckStartMenu();
		void CheckAppsMenu();
		void CheckChannelsMenu();
		void OpenLinksMenu();

		static Taskbar *instance;

        int menu;
		int menuWidth;
        bool triggerupdate;

		GuiImageData *taskbarImgData;
		GuiImageData *HeadPhonesData;
		GuiImageData *WifiData;
		GuiImage *taskbarImg;
		GuiImage *HeadPhonesImg;
		GuiImage *WifiImg;
		GuiText *timeTxt;
		PictureButton *startBtn;
		SimpleGuiTrigger *trigA;
		GuiTrigger *trigHome;
		GuiSound *soundClick;
		GuiSound *soundOver;
		GuiButton *homeBtn;
		GuiButton *Musicplayer;
        std::vector<Task *> Tasks;
        std::queue<Task *> TasksDeleteQueue;
};

#endif // _TASKBAR_H
