/***************************************************************************
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
#ifndef GUI_ARROWOPTION_H_
#define GUI_ARROWOPTION_H_

#include "libwiigui/gui.h"

using namespace std;

class GuiArrowOption : public GuiWindow
{
    public:
        GuiArrowOption();
        ~GuiArrowOption();
        void AddOption(const char * name, int PositionX, int PositionY);
        void SetOptionName(int i, const char * text);
        void SetOptionValue(int i, const char * text);
        int GetOptionCount();
        void RemoveOption(int i);
        GuiButton * GetButton(int i);
        GuiButton * GetButtonLeft(int i);
        GuiButton * GetButtonRight(int i);
        void ClearList();

		sigslot::signal3<GuiElement *, int, POINT> ClickedLeft;
		sigslot::signal3<GuiElement *, int, POINT> ClickedRight;
		sigslot::signal3<GuiElement *, int, POINT> ClickedButton;
    protected:
		void OnButtonClick(GuiElement *sender, int pointer, POINT p);
		void OnRightButtonClick(GuiElement *sender, int pointer, POINT p);
		void OnLeftButtonClick(GuiElement *sender, int pointer, POINT p);

        GuiSound * btnClick;
        GuiSound * btnSoundOver;
        GuiImageData * ArrowImgData;

        vector<GuiText *> OptionsName;
        vector<GuiText *> OptionsText;
        vector<GuiButton *> OptionsBtn;
        vector<GuiButton *> OptionsBtnRight;
        vector<GuiButton *> OptionsBtnLeft;
        vector<GuiImage *> OptionsImgLeft;
        vector<GuiImage *> OptionsImgRight;

        GuiTrigger * trigA;
};

#endif
