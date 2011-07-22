/****************************************************************************
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
 * WiiXplorer 2010
 ***************************************************************************/
#ifndef _TASK_HPP_
#define _TASK_HPP_

#include "libwiigui/gui.h"
#include "Controls/Callback.hpp"

class Task : public GuiButton, public sigslot::has_slots<>
{
    public:
        Task(const char * title);
        virtual ~Task();
        void SetCallback(cCallback * CallbackClass) { Callback = CallbackClass; };
        void SetTitle(const char * title) { Title->SetText(title); };
        void Execute(int param = 0);
        void SetIcon(GuiImageData * img);
        void SetParameter(int p) { Parameter = p; };
    private:
        void OnButtonClick(GuiButton * sender, int channel, POINT point);

        cCallback * Callback;
        GuiText * Title;
        GuiImage * ButtonImg;
        GuiImage * ButtonIcon;
        SimpleGuiTrigger * trigA;
        int Parameter;
};

#endif
