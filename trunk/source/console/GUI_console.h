#ifndef Console_H
#define Console_H


//#include "GUI_guiDef.h"
//#include "GUI_console_text.h"

// Standard print
void sgIP_dbgprint(const char * txt, ...) ;


class Console 
{
private:
	u8 mText[23*32];
	u16 mColor[23*32];
	u8 curx, cury;
	bool mVisible;
	bool mFreeze;
	u16 mTextColor;
	u16 mState;
	
public:
	Console(u16* map, u16* text);
	Console();
	~Console();
	

	// standard debug purpose
	void print(const char * str) ;
	void print(const char * str, u16 color) ;
	void dbgprint(char * txt, ...) ;
	void dbgprintat(int x, int y, char * txt, ...) ;

	// with default Console text color
	void setTextMiddleColor(u16 color) { mTextColor = color; }
	void Cls(void) ;
	void printalt(int alt, int x, int y, const char * str) ;
	void printat(int x, int y, const char * str) ;
	void printnat(int x, int y, const char * str, int n) ;
		
	// with color
	void printalt(int alt, int x, int y, u16 color, const char * str) ;
	void printat(int x, int y, u16 color, const char * str) ;
	void printnat(int x, int y, u16 color, const char * str, int n) ;
	void setFreeze(bool state);
	void setVisible(bool visible);
	
private:
	void drawConsole();
	void hideConsole();
	void ScrollUp(void) ;
	void clearText(u8 x1, u8 y1, u8 x2, u8 y2);
};

extern	void printWii(u8 *, u8 x, u8 y, u16 color, char c);

#endif


