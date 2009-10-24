#include "gui.h"

class OptionList {
	public:
		OptionList(int size);
		~OptionList();
		void SetName(int i, const char *format, ...) __attribute__((format (printf, 3, 4)));
		const char *GetName(int i)
		{
			if(i >= 0 && i < length && name[i])
				return name[i];
			else
				return NULL;
		}
		void SetValue(int i, const char *format, ...) __attribute__((format (printf, 3, 4)));
		const char *GetValue(int i)
		{
			if(i >= 0 && i < length && value[i])
				return value[i];
			else
				return NULL;
		}
		int GetLength()	{ return length; }
		bool IsChanged() { bool ret = listChanged; listChanged = false; return ret;}
	private:
		int length;
		char ** name;
		char ** value;
		bool listChanged;
};


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
		void SetFocus(int f);
		void Draw();
		void TriggerUpdate();
		void Update(GuiTrigger * t);
		GuiText * optionVal[PAGESIZE];
	protected:
		int selectedItem;
		int listOffset;
		int coL2;
		bool listChanged;

		OptionList *options;
		int optionIndex[PAGESIZE];
		GuiButton * optionBtn[PAGESIZE];
		GuiText * optionTxt[PAGESIZE];
		GuiImage * optionBg[PAGESIZE];

		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;

		GuiImage * bgOptionsImg;
		GuiImage * scrollbarImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowDownOverImg;
		GuiImage * arrowUpImg;
		GuiImage * arrowUpOverImg;
		GuiImage * scrollbarBoxImg;
		GuiImage * scrollbarBoxOverImg;

		GuiImageData * bgOptions;
		GuiImageData * bgOptionsEntry;
		GuiImageData * scrollbar;
		GuiImageData * arrowDown;
		GuiImageData * arrowDownOver;
		GuiImageData * arrowUp;
		GuiImageData * arrowUpOver;
		GuiImageData * scrollbarBox;
		GuiImageData * scrollbarBoxOver;

		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;
		GuiTrigger * trigA;
		GuiTrigger * trigHeldA;
};
