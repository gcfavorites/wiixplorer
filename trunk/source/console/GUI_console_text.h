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
#include "FreeTypeGX.h"
#include "video.h"
#include "filelist.h"
#include "input.h"
#include "oggplayer.h"
#include "sigslot.h"
#include "tools.h"

extern FreeTypeGX *fontSystem[];

#define SCROLL_INITIAL_DELAY 	20
#define SCROLL_LOOP_DELAY 		3
#define PAGESIZE	 			8
#define MAX_KEYBOARD_DISPLAY	40
#define MAX_LINES_TO_DRAW	    12

typedef void (*UpdateCallback)(void * e);

enum
{
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTRE,
	ALIGN_TOP,
	ALIGN_BOTTOM,
	ALIGN_MIDDLE
};

enum
{
	STATE_DEFAULT,
	STATE_SELECTED,
	STATE_CLICKED,
	STATE_HELD,
	STATE_DISABLED,
	STATE_CLOSED
};

enum
{
	SOUND_PCM,
	SOUND_OGG,
	SOUND_MP3
};

enum
{
	IMAGE_TEXTURE,
	IMAGE_COLOR,
	IMAGE_DATA
};

enum
{
	TRIGGER_SIMPLE,
	TRIGGER_HELD,
	TRIGGER_BUTTON_ONLY,
	TRIGGER_BUTTON_ONLY_IN_FOCUS
};

enum
{
	WRAP,
	LONGTEXT,
	DOTTED,
	SCROLL_HORIZONTAL,
	SCROLL_NONE
};


typedef struct _paddata {
	u16 btns_d;
	u16 btns_u;
	u16 btns_h;
	s8 stickX;
	s8 stickY;
	s8 substickX;
	s8 substickY;
	u8 triggerL;
	u8 triggerR;
} PADData;

typedef struct _POINT {
	s32 x;
	s32 y;
} POINT;

#define EFFECT_SLIDE_TOP			1
#define EFFECT_SLIDE_BOTTOM			2
#define EFFECT_SLIDE_RIGHT			4
#define EFFECT_SLIDE_LEFT			8
#define EFFECT_SLIDE_IN				16
#define EFFECT_SLIDE_OUT			32
#define EFFECT_FADE					64
#define EFFECT_SCALE				128
#define EFFECT_COLOR_TRANSITION		256


//!Display, manage, and manipulate text in the GUI
class GuiTextConsole 
{
	public:
		//!Constructor
		//!\param t Text
		//!\param s Font size
		//!\param c Font color
		GuiTextConsole(const char * t, int s, GXColor c);
		//!\overload
		//!\Assumes SetPresets() has been called to setup preferred text attributes
		//!\param t Text
		GuiTextConsole(const char * t);
		//!Destructor
		~GuiTextConsole();
		//!Sets the text of the GuiText element
		//!\param t Text
		void SetText(const char * t);
		void SetTextf(const char *format, ...) __attribute__((format(printf,2,3)));
		//!Sets up preset values to be used by GuiText(t)
		//!Useful when printing multiple text elements, all with the same attributes set
		//!\param sz Font size
		//!\param c Font color
		//!\param w Maximum width of texture image (for text wrapping)
		//!\param wrap Wrapmode when w>0
		//!\param s Font style
		//!\param h Text alignment (horizontal)
		//!\param v Text alignment (vertical)
		void SetPresets(int sz, GXColor c, int w, u16 s, int h, int v);
		//!Sets the font size
		//!\param s Font size
		void SetFontSize(int s);
		//!Sets the first line to draw (default = 0)
		//!\param line
		void SetFirstLine(int line);
		//!Sets max lines to draw
		//!\param lines
		void SetLinesToDraw(int lines);
		//!Gets the total line number
		int GetTotalLines();
		//!Sets the maximum width of the drawn texture image
		//!If the text exceeds this, it is wrapped to the next line
		//!\param w Maximum width
		//!\param m WrapMode
		void SetMaxWidth(int w = 0, int m = WRAP);
		//!Sets the font color
		//!\param c Font color
		void SetColor(GXColor c);
		//!Sets the FreeTypeGX style attributes
		//!\param s Style attributes
		//!\param m Style-Mask attributes
		void SetStyle(u16 s);
		//!Sets the text alignment
		//!\param hor Horizontal alignment (ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTRE)
		//!\param vert Vertical alignment (ALIGN_TOP, ALIGN_BOTTOM, ALIGN_MIDDLE)
		void SetAlignment(int hor, int vert);
		//!Sets the font
		//!\param f Font
		void SetFont(FreeTypeGX *f);
		//!Get the original text as char
        const char * GetOrigText();
		//!Get the Horizontal Size of Text
		int GetTextWidth();
        int GetTextWidth(int ind);
		//!Get the max textwidth
        int GetTextMaxWidth();
		//!Get current Textline (for position calculation)
        wchar_t * GetDynText();
        wchar_t * GetDynTextLine(int ind);
		//!Get the offset of a linebreak
		u32 GetLineBreakOffset(int line);
		//!Change the font
		//!\param font bufferblock
		//!\param font filesize
		bool SetFont(const u8 *font, const u32 filesize);
		//!Constantly called to draw the text
		void Draw();
	protected:
        wchar_t *text;
        wchar_t *textDyn;
		wchar_t *textDynRow[MAX_LINES_TO_DRAW]; //!< Wrapped lines text values
		char *origText; //!< Original text data
		int wrapMode; //!< Wrapping toggle
		int textScrollPos; //!< Current starting index of text string for scrolling
		int textScrollInitialDelay; //!< Delay to wait before starting to scroll
		int textScrollDelay; //!< Scrolling speed
		int size; //!< Font size
		int maxWidth; //!< Maximum width of the generated text object (for text wrapping)
		u16 style; //!< FreeTypeGX style attributes
		GXColor color; //!< Font color
		FreeTypeGX *font;
		bool widescreen; //added
		int firstLine;
		int linestodraw;
		int totalLines;
		int textWidth;
		int currentSize;
		u32 *LineBreak;
		u32 alpha;
};


#endif
