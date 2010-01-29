#include "gui.h"
#include "TextOperations/TextPointer.h"

typedef struct _keytype {
	char ch, chShift;
} Key;

//!On-screen keyboard
class GuiKeyboard : public GuiWindow
{
	public:
		GuiKeyboard(char * t, u32 m);
		~GuiKeyboard();
        void AddChar(int pos, char Char);
        void RemoveChar(int pos);
		void Update(GuiTrigger * t);
		char kbtextstr[256];
	protected:
        void MoveText(int n);
        const char * GetDisplayText(const char * t);
        void OnPointerHeld(GuiElement *sender, int pointer, POINT p);
        void OnPositionMoved(GuiElement *sender, int pointer, POINT p);

        int CurrentFirstLetter;
		u32 kbtextmaxlen;
		Key keys[4][13];
		int shift;
		int caps;
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
		GuiButton * keyBtn[4][13];
		GuiImage * keyImg[4][13];
		GuiImage * keyImgOver[4][13];
		GuiText * keyTxt[4][13];
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
		GuiTrigger * trigB;
		GuiTrigger * trigLeft;
		GuiTrigger * trigRight;
};
