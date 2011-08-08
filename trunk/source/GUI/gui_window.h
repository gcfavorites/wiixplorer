/****************************************************************************
 * Copyright (C) 2009-2011
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
 ***************************************************************************/
#ifndef GUI_WINDOW_H_
#define GUI_WINDOW_H_

#include "gui_element.h"

//!Allows GuiElements to be grouped together into a "window"
class GuiWindow : public GuiElement
{
	public:
		//!Constructor
		GuiWindow(GuiWindow *parent = 0);
		//!\overload
		//!\param w Width of window
		//!\param h Height of window
		GuiWindow(int w, int h, GuiWindow *parent = 0);
		//!Destructor
		virtual ~GuiWindow();
		//!Appends a GuiElement to the GuiWindow
		//!\param e The GuiElement to append. If it is already in the GuiWindow, it is removed first
		void Append(GuiElement* e);
		//!Inserts a GuiElement into the GuiWindow at the specified index
		//!\param e The GuiElement to insert. If it is already in the GuiWindow, it is removed first
		//!\param i Index in which to insert the element
		void Insert(GuiElement* e, u32 i);
		//!Removes the specified GuiElement from the GuiWindow
		//!\param e GuiElement to be removed
		void Remove(GuiElement* e);
		//!Removes all GuiElements
		void RemoveAll();
		//!Returns the GuiElement at the specified index
		//!\param index The index of the element
		//!\return A pointer to the element at the index, NULL on error (eg: out of bounds)
		GuiElement* GetGuiElementAt(u32 index) const;
		//!Returns the size of the list of elements
		//!\return The size of the current element list
		u32 GetSize();
		//!Sets the visibility of the window
		//!\param v visibility (true = visible)
		void SetVisible(bool v);
		//!Resets the window's state to STATE_DEFAULT
		void ResetState();
		//!Sets the window's state
		//!\param s State
		void SetState(int s);
		//!Gets the index of the GuiElement inside the window that is currently selected
		//!\return index of selected GuiElement
		int GetSelected();
		//!Window cutoff bounds
		void SetMinWidth(int w);
		void SetMaxWidth(int w);
		void SetMinHeight(int h);
		void SetMaxHeight(int h);
		//!Dim the Window's background
		void DimBackground(bool d);
		//!Draws all the elements in this GuiWindow
		void Draw();
		//!Updates the window and all elements contains within
		//!Allows the GuiWindow and all elements to respond to the input data specified
		//!\param t Pointer to a GuiTrigger, containing the current input data from PAD/WPAD
		void Update(GuiTrigger * t);
		//!virtual Close Window - this will put the object on the delete queue in MainWindow
		virtual void Close();
		//!virtual show window function
		virtual void show() {}
		//!virtual hide window function
		virtual void hide() {}
		//!virtual enter main loop function (blocking)
		virtual void exec() {}
		//!virtual eventUpdate which is called by the main loop
		virtual void updateEvents() {}
		//! Signals
		//! On Closing
		sigslot::signal1<GuiWindow *> Closing;
	protected:
		bool firstFrame; //! Skip first frame
		bool dim;   //! Enable/disable dim of a window only
		GuiWindow *parent; //!< Parent Window
		std::vector<GuiElement*> _elements; //!< Contains all elements within the GuiWindow
};

#endif
