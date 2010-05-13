/****************************************************************************
 * Copyright (C) 2009
 * by Tantric
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
 * Modifications by Dimok
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef GUI_BUTTON_H_
#define GUI_BUTTON_H_

#include "gui_element.h"
#include "gui_tooltip.h"

//!Display, manage, and manipulate buttons in the GUI. Buttons can have images, icons, text, and sound set (all of which are optional)
class GuiButton : public GuiElement
{
	public:
		//!Constructor
		//!\param w Width
		//!\param h Height
		GuiButton(int w, int h);
		//!Destructor
		~GuiButton();
		//!Sets the button's size
		void SetSize(int w, int h);
		//!Sets the button's image
		//!\param i Pointer to GuiImage object
		void SetImage(GuiImage* i);
		//!Sets the button's image on over
		//!\param i Pointer to GuiImage object
		void SetImageOver(GuiImage* i);
		//!Sets the button's image on hold
		//!\param i Pointer to GuiImage object
		void SetImageHold(GuiImage* i);
		//!Sets the button's image on click
		//!\param i Pointer to GuiImage object
		void SetImageClick(GuiImage* i);
		//!Sets the button's icon
		//!\param i Pointer to GuiImage object
		void SetIcon(GuiImage* i);
		//!Sets the button's icon on over
		//!\param i Pointer to GuiImage object
		void SetIconOver(GuiImage* i);
		//!Sets the button's icon on hold
		//!\param i Pointer to GuiImage object
		void SetIconHold(GuiImage* i);
		//!Sets the button's icon on click
		//!\param i Pointer to GuiImage object
		void SetIconClick(GuiImage* i);
		//!Sets the button's label
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabel(GuiText* t, int n = 0);
		//!Sets the button's label on over (eg: different colored text)
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabelOver(GuiText* t, int n = 0);
		//!Sets the button's label on hold
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabelHold(GuiText* t, int n = 0);
		//!Sets the button's label on click
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabelClick(GuiText* t, int n = 0);
		//!Sets the sound to play on over
		//!\param s Pointer to GuiSound object
		void SetSoundOver(GuiSound * s);
		//!Sets the sound to play on hold
		//!\param s Pointer to GuiSound object
		void SetSoundHold(GuiSound * s);
		//!Sets the sound to play on click
		//!\param s Pointer to GuiSound object
		void SetSoundClick(GuiSound * s);
		//!Sets a tooltip that should popup after a few seconds
		void SetTooltip(GuiTooltip * t);
		//!Constantly called to draw the GuiButton
		void Draw();
		//!Constantly called to allow the GuiButton to respond to updated input data
		//!\param t Pointer to a GuiTrigger, containing the current input data from PAD/WPAD
		void Update(GuiTrigger * t);
	protected:
		GuiImage * image; //!< Button image (default)
		GuiImage * imageOver; //!< Button image for STATE_SELECTED
		GuiImage * imageHold; //!< Button image for STATE_HELD
		GuiImage * imageClick; //!< Button image for STATE_CLICKED
		GuiImage * icon; //!< Button icon (drawn after button image)
		GuiImage * iconOver; //!< Button icon for STATE_SELECTED
		GuiImage * iconHold; //!< Button icon for STATE_HELD
		GuiImage * iconClick; //!< Button icon for STATE_CLICKED
		GuiText * label[3]; //!< Label(s) to display (default)
		GuiText * labelOver[3]; //!< Label(s) to display for STATE_SELECTED
		GuiText * labelHold[3]; //!< Label(s) to display for STATE_HELD
		GuiText * labelClick[3]; //!< Label(s) to display for STATE_CLICKED
		GuiSound * soundOver; //!< Sound to play for STATE_SELECTED
		GuiSound * soundHold; //!< Sound to play for STATE_HELD
		GuiSound * soundClick; //!< Sound to play for STATE_CLICKED
		GuiTooltip * tooltip; //!< Tooltip that is set
		Timer SelectTimer;
};

#endif
