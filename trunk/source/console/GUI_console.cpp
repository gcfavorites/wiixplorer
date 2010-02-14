#include <errno.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/dir.h>
#include <sys/fcntl.h>
#include "gui_guidef.h"
#include "GUI_console.h"
#include "GUI_console_text.h"

#include <unistd.h> //usleep


Console myConsole;


const u32 delai = 20000*10;
void Console::print(const char * str, u16 color ) {
	int x;
	//ScrollUp();
	x=curx;
	while(*str) {
		if(x==47) {x=0; ScrollUp();}
		u16 pos = 9*47+x;
		mText[pos]=(*str) ; 
		mColor[pos]= color;
		printWii(mText,x, 9, color, (*str)); 
		x++;
		str++;
	}
	curx=x;
	    usleep(delai);
}



void Console::print(const char * str) {
	int x;
	//ScrollUp();
	x=curx;
	while(*str) {
		if (*str =='\n') {x=0; ScrollUp();str++; continue;}
		if(x==47) {x=0; ScrollUp();}
		u16 pos = 9*47+x;
		mText[pos]=(*str);
		mColor[pos]= mTextColor;
		printWii(mText,x, 9, mTextColor, (*str)); 
		x++;
		str++;
	}
	curx=x;
	          //usleep(delai);
}


Console::Console () {
	curx=cury=0;
	mTextColor = TEXT_BLACK;

	mState = WND_IDLE;
	mVisible = true;
	mFreeze = false;

	drawConsole();
}



void Console::setFreeze(bool state) {
	mFreeze = state;
}

void Console::setVisible(bool visible) {

	mVisible = visible;

	if(visible)
		drawConsole();


	if(!visible)
		hideConsole();

}

void Console::drawConsole (void) {
}


void Console::hideConsole() {
	clearText(0,0,47,10);

}


Console::~Console() {

	hideConsole();

}

void Console::printat(int x, int y, u16 color, const char * str) {
	while(*str) {
		if(x==47) {x=0; y++;}
		if(y==10) return;
		u16 pos = y*47+x;
		mText[pos]=(*str) ;
		mColor[pos]= color;
		printWii(mText,x, y, color, (*str)); 
		x++;
		str++;
	}
}


void Console::printat(int x, int y, const char * str)
{
   printat(x, y, mTextColor, str);
}   


void Console::printalt(int alt, int x, int y, u16 color, const char * str) {
	while(*str) {
		if(x==47) {x=0; y++;}
		if(y==10) return;
		u16 pos = y*47+x;
		mText[pos]=(*str) ;
		mColor[pos]= color;
		printWii(mText,x, y, color, (*str)); 
		x++;
		str++;
	}
}

void Console::printalt(int alt, int x, int y, const char * str) {
   printalt(alt, x, y, mTextColor, str);
}


void Console::printnat(int x, int y, const char * str, int n) {
   printnat(x, y, mTextColor, str, n);
}
   
void Console::printnat(int x, int y, u16 color, const char * str, int n) {
	while(*str && 0<=--n) {
		if(x==47) {x=0; y++;}
		if(y==10) return;
		u16 pos = y*47+x;
		mText[pos]=(*str) ;
		mColor[pos]= color;
		printWii(mText,x, y, color, (*str)); 
		x++;
		str++;
	}
}


char buffer[256];
void sgIP_dbgprint(const char * txt, ...) {
return ;
	va_list args;
	va_start(args,txt);
	vsprintf(buffer,txt,args);
	myConsole.print(buffer);		
}

void sgIP_appprint(const char * txt, ...) {
	va_list args;
	va_start(args,txt);
	vsprintf(buffer,txt,args);
	myConsole.print(buffer);		
}

void Console::Cls(void) {
	int i;
	for(i=0;i<10*47;i++) {
		mText[i]=0x20;
		printWii(mText,i%47, i/47, mTextColor, 0x20); 
	}
}


void Console::dbgprint(char * txt, ...) {
	char buffer[256];
	va_list args;
	va_start(args,txt);
	vsprintf(buffer,txt,args);
	print(buffer);		
}

void Console::dbgprintat(int x, int y, char * txt, ...) {
	char buffer[256];
	va_list args;
	va_start(args,txt);
	vsprintf(buffer,txt,args);
	printat(x, y, buffer);		
}

void Console::ScrollUp(void) {
	int i;
	for(i=0;i<9*47;i++) {
		u16 pos = i+47;
		mText[i]=mText[pos];
		printWii(mText,pos%47, i/47, mTextColor, mText[i]); 
	}
	for(i=0;i<47;i++) {
		u16 pos = 9*47+i;
		mText[pos]=0x20;
		printWii(mText,i, 9, mTextColor, 0x20); 
	}
}

void Console::clearText(u8 x1, u8 y1, u8 x2, u8 y2) {
	u8 x,y;
	for(y = y1; y < y2; y++) {
		for(x = x1; x < x2; x++) {
			u16 pos = y*47+x;
			mText[pos]=0;
			printWii(mText,x, y, mTextColor, 0x20); 
		}
	}
}


