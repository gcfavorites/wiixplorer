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
#include "GUI/gui_window.h"
#include "GUI/gui_image.h"

//! Explorer class, forward declaration
class Explorer;

class Application : public GuiWindow
{
	public:
		static Application * Instance() { if(!instance) instance = new Application(); return instance; }
		static void DestroyInstance() { delete instance; instance = NULL; }

		void quit();
		void show();
		void hide();
		void exec();
		void updateEvents();

		void SetGrabPointer(int i);
		void ResetPointer(int i);

		void PushForDelete(GuiElement *e);
		void UpdateOnly(GuiElement *e) { updateOnlyElement = e; }
		GuiElement *GetUpdateOnly() const { return updateOnlyElement; }

		GXColor * GetBGColorPtr() { return bgImg->GetColorPtr(); };
	private:
		Application();
		virtual ~Application();

		static Application *instance;

		bool exitApplication;

		GuiImage *bgImg;
		GuiImageData *pointer[4];
		GuiImageData *standardPointer[4];
		GuiImageData *grabPointer[4];
		GuiElement *updateOnlyElement;
		Explorer *mainExplorer;
		std::vector<GuiElement *> elements;

		struct ElementList
		{
			GuiElement *element;
			ElementList *next;
		};

		ElementList *DeleteQueue;
};

#endif //_APPLICATION_H
