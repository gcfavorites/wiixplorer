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
 * Properties.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/
 #ifndef __PROPERTIES_H_
 #define __PROPERTIES_H_

#include <gctypes.h>
#include <unistd.h>

#include "libwiigui/gui.h"
#include "Memory/Resources.h"
#include "FileOperations/ItemMarker.h"

class Properties : public GuiWindow
{
    public:
        Properties(ItemMarker * IMarker);
        ~Properties();
        int GetChoice();
    private:
        void UpdateSizeValue();
        void StartGetFolderSizeThread();
        void StopSizeThread();
        void InternalFolderSizeGain();
		static void * FolderSizeThread(void *arg);
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);

        ItemMarker * Marker;
        int choice;
        bool folder;
        u64 TotalSize;
        u32 FileCount;
        u64 OldSize;
        u64 devicefree;
        u64 devicesize;
        lwp_t foldersizethread;

        GuiImage * dialogBoxImg;
        GuiImage * TitleImg;
        GuiImage * arrowUpImg;
        GuiImage * arrowUpImgOver;

        GuiImageData * dialogBox;
        GuiImageData * titleData;
        GuiImageData * arrowUp;
        GuiImageData * arrowUpOver;

        GuiSound * btnClick;

        GuiText * TitleTxt;
        GuiText * filepathTxt;
        GuiText * filepathvalTxt;
        GuiText * filecountTxt;
        GuiText * filecountTxtVal;
        GuiText * filesizeTxt;
        GuiText * filesizeTxtVal;
        GuiText * filetypeTxt;
        GuiText * filetypeTxtVal;
        GuiText * devicefreeTxt;
        GuiText * devicefreeTxtVal;
        GuiText * devicetotalTxt;
        GuiText * devicetotalTxtVal;
        GuiText * last_accessTxt;
        GuiText * last_accessTxtVal;
        GuiText * last_modifTxt;
        GuiText * last_modifTxtVal;
        GuiText * last_changeTxt;
        GuiText * last_changeTxtVal;

        GuiButton * CloseBtn;

        SimpleGuiTrigger * trigA;
        GuiTrigger * trigB;
};

#endif
