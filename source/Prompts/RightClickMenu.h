/***************************************************************************
 * Copyright (C) 2009
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
 * PromptWindows.cpp
 *
 * All promptwindows
 * for WiiXplorer 2009
 ***************************************************************************/
 #ifndef __RIGHTCLICKMENU_H_
 #define __RIGHTCLICKMENU_H_

#include <gctypes.h>
#include <unistd.h>

#include "libwiigui/gui.h"
#include "Controls/Resources.h"

#define MAX_CLICKBUTTONS    7

enum
{
    CUT = 0,
    COPY,
    PASTE,
    RENAME,
    DELETE,
    NEWFOLDER,
    PROPERTIES,
    NoButton = -10
};

enum
{
    ArcOpen = 0,
    ArcExtractFile,
    ArcExtractAll
};

class RightClickMenu : public GuiWindow
{
    public:
        RightClickMenu(int x, int y,
                       const char *btn1Label = NULL, const char *btn2Label = NULL,
                       const char *btn3Label = NULL, const char *btn4Label = NULL,
                       const char *btn5Label = NULL, const char *btn6Label = NULL,
                       const char *btn7Label = NULL);
        ~RightClickMenu();
        int GetChoice();
    private:
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);

        int choice;
        int numItems;
        int buttonX;
        int buttonY;

        GuiImage * ClickMenuMiddleImg;
        GuiImage * ClickMenuUpperImg;
        GuiImage * ClickMenuLowerImg;
        GuiImage * btnMenuSelect[MAX_CLICKBUTTONS];

        GuiImageData * ClickMenuMiddle;
        GuiImageData * ClickMenuUpper;
        GuiImageData * menu_select;

        GuiSound * btnClick;

        GuiText * ButtonTxt[MAX_CLICKBUTTONS];
        GuiText * ButtonTxtOver[MAX_CLICKBUTTONS];

        GuiButton * Button[MAX_CLICKBUTTONS];
        GuiButton * NoBtn;

        GuiTrigger * trigA;
        GuiTrigger * trigB;
};

#endif
