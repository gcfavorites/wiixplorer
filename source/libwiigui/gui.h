#ifndef LIBWIIGUI_H
#define LIBWIIGUI_H

#include <gccore.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <exception>
#include <wchar.h>
#include <math.h>
#include <asndlib.h>
#include <wiiuse/wpad.h>
#include <gd.h>
#include "FreeTypeGX.h"
#include "Language/gettext.h"
#include "VideoOperations/video.h"
#include "SoundOperations/gui_sound.h"
#include "input.h"
#include "sigslot.h"
#include "Tools/gecko.h"
#include "Tools/wifi_gecko.h"
#include "Tools/tools.h"
#include "gui_trigger.h"

#define SCROLL_INITIAL_DELAY 	20
#define SCROLL_LOOP_DELAY 		3
#define PAGESIZE	 			8
#define MAX_KEYBOARD_DISPLAY	40
#define MAX_LINES_TO_DRAW	    12

extern FreeTypeGX * fontSystem;
extern GuiTrigger userInput[4];
extern bool shutdown;
extern bool reset;

#include "gui_element.h"
#include "gui_window.h"
#include "gui_imagedata.h"
#include "gui_image.h"
#include "gui_text.h"
#include "gui_button.h"


#endif
