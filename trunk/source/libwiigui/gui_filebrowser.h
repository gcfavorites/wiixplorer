#ifndef __GUI_FILEBROWSER_H_
#define __GUI_FILEBROWSER_H_

#include "gui.h"
#include "FileOperations/filebrowser.h"


//!Display a list of files
class GuiFileBrowser : public GuiElement
{
	public:
		GuiFileBrowser(FileBrowser * filebrowser, int w, int h);
		~GuiFileBrowser();
		void DisableTriggerUpdate(bool set);
		void ResetState();
		void SetFocus(int f);
		void Draw();
		void TriggerUpdate();
		void Update(GuiTrigger * t);
		GuiButton * fileList[PAGESIZE];
	protected:
        void OnClicked(GuiElement *sender, int pointer, POINT p);
		int selectedItem;
		int numEntries;
		bool listChanged;
		bool triggerdisabled;

		FileBrowser * browser;

		GuiText * fileListText[PAGESIZE];
		GuiText * fileListTextOver[PAGESIZE];
		GuiImage * fileListBg[PAGESIZE];
		GuiImage * fileListArchives[PAGESIZE];
		GuiImage * fileListDefault[PAGESIZE];
		GuiImage * fileListFolder[PAGESIZE];
		GuiImage * fileListGFX[PAGESIZE];
		GuiImage * fileListPLS[PAGESIZE];
		GuiImage * fileListSFX[PAGESIZE];
		GuiImage * fileListTXT[PAGESIZE];
		GuiImage * fileListXML[PAGESIZE];

		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;

		GuiImage * bgFileSelectionImg;
		GuiImage * scrollbarImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowDownOverImg;
		GuiImage * arrowUpImg;
		GuiImage * arrowUpOverImg;
		GuiImage * scrollbarBoxImg;
		GuiImage * scrollbarBoxOverImg;

		GuiImageData * bgFileSelection;
		GuiImageData * bgFileSelectionEntry;
        GuiImageData * fileArchives;
		GuiImageData * fileDefault;
		GuiImageData * fileFolder;
        GuiImageData * fileGFX;
		GuiImageData * filePLS;
		GuiImageData * fileSFX;
        GuiImageData * fileTXT;
        GuiImageData * fileXML;
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

#endif
