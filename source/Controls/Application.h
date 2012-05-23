/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <queue>
#include "GUI/gui_frame.h"
#include "GUI/gui_image.h"

class Application : public GuiFrame
{
	public:
		static Application * Instance() { if(!instance) instance = new Application(); return instance; }
		static void DestroyInstance() { delete instance; instance = NULL; }

		void init();
		void quit();
		void exec();
		void show();
		void hide();
		void updateEvents();
		static bool isClosing() { return exitApplication; }

		void SetGrabPointer(int i);
		void ResetPointer(int i);

		void SetDrawOnly(GuiElement *e) { drawOnlyElement = e; }

		void Append(GuiElement *e)
		{
			LWP_MutexLock(m_mutex);
			GuiFrame::Append(e);
			LWP_MutexUnlock(m_mutex);
		}

		void PushForDelete(GuiElement *e);
		void SetUpdateOnly(GuiElement *e)
		{
			UnsetUpdateOnly(e);
			LWP_MutexLock(m_mutex);
			updateOnlyElement.push_back(e);
			LWP_MutexUnlock(m_mutex);
		}

		void UnsetUpdateOnly(GuiElement *e)
		{
			for(u32 i = 0; i < updateOnlyElement.size(); ++i)
				if(updateOnlyElement[i] == e) {
					LWP_MutexLock(m_mutex);
					updateOnlyElement.erase(updateOnlyElement.begin()+i);
					LWP_MutexUnlock(m_mutex);
				}
		}

		GXColor * GetBGColorPtr() { return bgImg->GetColorPtr(); };
	private:
		Application();
		virtual ~Application();

		static Application *instance;
		static bool exitApplication;

		GuiImage *bgImg;
		GuiImageData *pointer[4];
		GuiImageData *standardPointer[4];
		GuiImageData *grabPointer[4];
		GuiElement *drawOnlyElement;
		std::vector<GuiElement *> updateOnlyElement;
		std::vector<GuiElement *> elements;

		struct ElementList
		{
			GuiElement *element;
			ElementList *next;
		};

		ElementList *DeleteQueue;
		mutex_t m_mutex;
};

#endif //_APPLICATION_H
