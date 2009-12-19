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

enum
{
    CUT,
    COPY,
    PASTE,
    RENAME,
    DELETE,
    NEWFOLDER,
    PROPERTIES
};

class RightClickMenu : public GuiWindow
{
    public:
        RightClickMenu(int x, int y);
        ~RightClickMenu();
        int GetChoice();
    private:
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);

        int choice;
        int numItems;
        int buttonY;

        GuiImage * dialogBoxImg;
        GuiImage * btnCutMenuSelect;
        GuiImage * CopybtnMenuSelect;
        GuiImage * PastebtnMenuSelect;
        GuiImage * RenamebtnMenuSelect;
        GuiImage * DeletebtnMenuSelect;
        GuiImage * NewFolderbtnMenuSelect;
        GuiImage * PropertiesbtnMenuSelect;

        GuiImageData * dialogBox;
        GuiImageData * menu_select;

        GuiSound * btnClick;

        GuiText * cutTxt;
        GuiText * cutTxtOver;
        GuiText * copyTxt;
        GuiText * copyTxtOver;
        GuiText * pasteTxt;
        GuiText * PasteTxtOver;
        GuiText * RenameTxt;
        GuiText * RenameTxtOver;
        GuiText * DeleteTxt;
        GuiText * DeleteTxtOver;
        GuiText * NewFolderTxt;
        GuiText * NewFolderTxtOver;
        GuiText * PropertiesTxt;
        GuiText * PropertiesTxtOver;

        GuiButton * btnCut;
        GuiButton * Copybtn;
        GuiButton * Pastebtn;
        GuiButton * Renamebtn;
        GuiButton * Deletebtn;
        GuiButton * NewFolderbtn;
        GuiButton * Propertiesbtn;
        GuiButton * NoBtn;

        GuiTrigger * trigA;
        GuiTrigger * trigB;
};

#endif
