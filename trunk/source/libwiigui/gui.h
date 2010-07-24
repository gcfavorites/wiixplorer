/*!\mainpage libwiigui Documentation
 *
 * \section Introduction
 * libwiigui is a GUI library for the Wii, created to help structure the
 * design of a complicated GUI interface, and to enable an author to create
 * a sophisticated, feature-rich GUI. It was originally conceived and written
 * after I started to design a GUI for Snes9x GX, and found libwiisprite and
 * GRRLIB inadequate for the purpose. It uses GX for drawing, and makes use
 * of PNGU for displaying images and FreeTypeGX for text. It was designed to
 * be flexible and is easy to modify - don't be afraid to change the way it
 * works or expand it to suit your GUI's purposes! If you do, and you think
 * your changes might benefit others, please share them so they might be
 * added to the project!
 *
 * \section Quickstart
 * Start from the supplied template example. For more advanced uses, see the
 * source code for Snes9x GX, FCE Ultra GX, and Visual Boy Advance GX.

 * \section Contact
 * If you have any suggestions for the library or documentation, or want to
 * contribute, please visit the libwiigui website:
 * http://code.google.com/p/libwiigui/

 * \section Credits
 * This library was wholly designed and written by Tantric. Thanks to the
 * authors of PNGU and FreeTypeGX, of which this library makes use. Thanks
 * also to the authors of GRRLIB and libwiisprite for laying the foundations.
 *
*/

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
#include "filelist.h"
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
