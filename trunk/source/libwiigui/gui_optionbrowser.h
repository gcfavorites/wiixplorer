#include "gui.h"
#include "OptionList.hpp"
#include "Controls/Scrollbar.hpp"


//!Display a list of menu options
class GuiOptionBrowser : public GuiElement
{
	public:
		GuiOptionBrowser(int w, int h, OptionList * l);
		~GuiOptionBrowser();
		void SetCol2Position(int x);
		int FindMenuItem(int c, int d);
		int GetClickedOption();
		void ResetState();
		void Draw();
		void TriggerUpdate();
		void Update(GuiTrigger * t);
	protected:
        void OnStateChange(GuiElement *sender, int state, int chan);
		int selectedItem;
		int listOffset;
		int coL2;
		bool listChanged;

		OptionList *options;
		int optionIndex[PAGESIZE];
		GuiButton * optionBtn[PAGESIZE];
		GuiText * optionTxt[PAGESIZE];
		GuiImage * optionBg[PAGESIZE];
		GuiText * optionVal[PAGESIZE];

        Scrollbar * scrollbar;

		GuiImage * bgOptionsImg;

		GuiImageData * bgOptions;
		GuiImageData * bgOptionsEntry;

		GuiSound * btnSoundClick;
		GuiTrigger * trigA;
};
