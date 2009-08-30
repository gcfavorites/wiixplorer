/***************************************************************************
 * Copyright (C) 2009
 * by r-win
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
 * ImageViewer.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#include <gctypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "libwiigui/gui.h"
#include "Prompts/PromptWindows.h"
#include "fileops.h"
#include "gettext.h"
#include "sys.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

void ImageViewer(const char *filepath)
{
    bool exitwindow = false;

    u8 *file = NULL;
    u64 filesize = 0;

	int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &file, &filesize);
	if (!ret)
		return;
		
	GuiImageData imageData(file, filesize);
	GuiImage image(&imageData);

	if (file)
	{
		free(file);
		file = NULL;
	}

	if (!imageData.GetImage())
	{
		WindowPrompt(tr("Image"), tr("Cannot open the image file."), tr("OK"));
		return;
	}
		
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiWindow window(mainWindow->GetWidth(), mainWindow->GetHeight());

	GuiImage backGround(mainWindow->GetWidth(), mainWindow->GetHeight(), (GXColor){0, 0, 0, 255});

	float factor = (image.GetWidth() > image.GetHeight()) ? (1.0 * backGround.GetWidth()) / image.GetWidth() : (1.0 * backGround.GetHeight()) / image.GetHeight();
	image.SetScale(factor);
	
	image.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	
	window.Append(&backGround);
	
	GuiButton backButton(backGround.GetWidth(), backGround.GetHeight());
	backButton.SetImage(&image);
	backButton.SetTrigger(&trigB);

	window.Append(&backButton);

	HaltGui();
	mainWindow->Append(&window);
	ResumeGui();
	
	while (!exitwindow)
	{
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();
        else if(reset == 1)
            Sys_Reboot();
			
		if (backButton.GetState() == STATE_CLICKED) {
			exitwindow = true;
			backButton.ResetState();
		}
	}
	
	HaltGui();
	mainWindow->Remove(&window);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}
