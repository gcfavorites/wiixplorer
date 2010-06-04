#ifndef GUI_KEYBOARD_H_
#define GUI_KEYBOARD_H_

#include "gui.h"
#include "TextOperations/TextPointer.h"
#include "TextOperations/wstring.hpp"

#define MAXKEYS 13
#define MAXROWS 4

typedef struct _keyrowtype
{
	wchar_t ch[MAXKEYS];
	wchar_t chShift[MAXKEYS];
} KeyboardRow;

//!On-screen keyboard
class GuiKeyboard : public GuiWindow
{
	public:
        GuiKeyboard(const char * t, u32 max);
		GuiKeyboard(const wchar_t * t, u32 max);
		~GuiKeyboard();
        const wchar_t * GetString();
		std::string GetUTF8String() const;
        void AddChar(int pos, wchar_t Char);
        void RemoveChar(int pos);
		void Update(GuiTrigger * t);
	protected:
        void SetupKeyboard(const wchar_t * t, u32 max);
        void MoveText(int n);
        const wchar_t * GetDisplayText(const wString * ws);
        void OnPointerHeld(GuiElement *sender, int pointer, POINT p);
        void OnPositionMoved(GuiElement *sender, int pointer, POINT p);

        u32 DeleteDelay;
        int CurrentFirstLetter;
		wString * kbtextstr;
		u32 kbtextmaxlen;
		KeyboardRow keys[MAXROWS];
		int ShiftChan;
		bool shift;
		bool caps;
		bool UpdateKeys;
		TextPointer * TextPointerBtn;
		GuiButton * GoLeft;
		GuiButton * GoRight;
		GuiText * kbText;
		GuiImage * keyTextboxImg;
		GuiText * keyCapsText;
		GuiImage * keyCapsImg;
		GuiImage * keyCapsOverImg;
		GuiButton * keyCaps;
		GuiText * keyShiftText;
		GuiImage * keyShiftImg;
		GuiImage * keyShiftOverImg;
		GuiButton * keyShift;
		GuiText * keyBackText;
		GuiImage * keyBackImg;
		GuiImage * keyBackOverImg;
		GuiButton * keyBack;
		GuiImage * keySpaceImg;
		GuiImage * keySpaceOverImg;
		GuiButton * keySpace;
		GuiText * keyLineBreakText;
		GuiImage * keyLineBreakImg;
		GuiImage * keyLineBreakOverImg;
		GuiButton * keyLineBreak;
		GuiText * keyClearText;
		GuiImage * keyClearImg;
		GuiImage * keyClearOverImg;
		GuiButton * keyClear;
		GuiButton * keyBtn[MAXROWS][MAXKEYS];
		GuiImage * keyImg[MAXROWS][MAXKEYS];
		GuiImage * keyImgOver[MAXROWS][MAXKEYS];
		GuiText * keyTxt[MAXROWS][MAXKEYS];
		GuiImageData * keyTextbox;
		GuiImageData * key;
		GuiImageData * keyOver;
		GuiImageData * keyMedium;
		GuiImageData * keyMediumOver;
		GuiImageData * keyLarge;
		GuiImageData * keyLargeOver;
		GuiSound * keySoundOver;
		GuiSound * keySoundClick;
		GuiTrigger * trigA;
		GuiTrigger * trigHeldA;
		GuiTrigger * trigLeft;
		GuiTrigger * trigRight;
};

#endif
