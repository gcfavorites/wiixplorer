/****************************************************************************
 * ProgressWindow
 * WiiXplorer 2009
 * by dimok
 *
 * ProgressWindow.h
 ***************************************************************************/

#ifndef _PROGRESSWINDOW_H_
#define _PROGRESSWINDOW_H_

#include "libwiigui/gui.h"

enum
{
    PROGRESSBAR = 1,
    THROBBER,     //! Rotate throbber image only when calling ShowProgress
    AUTO_THROBBER //! Rotating throbber image without calling ShowProgress
};

class ProgressWindow : public GuiWindow
{
    public:
		static ProgressWindow * Instance();
		static void DestroyInstance();

        void StartProgress(const char *title, int mode = PROGRESSBAR);
        void ShowProgress(u64 done, u64 total, const char *filename);
        void StopProgress();
        const char * GetTitle() { return ProgressTitle.c_str(); };
        void SetMinimized(bool m) { Minimized = m; };
        void SetMinimizable(bool m) { Minimizable = m; };
    protected:
        ProgressWindow();
        ~ProgressWindow();
		static void * ThreadCallback(void *arg);
		void InternalUpdate();
		void SetupProgressbar();
		void SetupThrobber();
		void MainLoop();
		void LoadWindow();
		void UpdateValues();
		void ClearMemory();
        static ProgressWindow * instance;

        u64 progressDone;
        u64 progressTotal;
        int showProgress;
        bool Changed;
        bool Minimized;
        bool Minimizable;
        lwp_t ProgressThread;
		bool ExitRequested;

		Timer ProgressTimer;

		std::string ProgressTitle;
		char ProgressMsg[200];

		GuiImageData * dialogBox;
		GuiImageData * progressbarOutline;
		GuiImageData * throbber;
		GuiImageData * btnOutline;

		GuiImage * dialogBoxImg;
		GuiImage * progressbarOutlineImg;
		GuiImage * progressbarEmptyImg;
		GuiImage * progressbarImg;
		GuiImage * throbberImg;
		GuiImage * buttonImg;
		GuiImage * MinimizeImg;

        GuiText * titleTxt;
        GuiText * msgTxt;
        GuiText * prTxt;
        GuiText * prsTxt;
        GuiText * speedTxt;
        GuiText * sizeTxt;
        GuiText * AbortTxt;
        GuiText * MinimizeTxt;

        GuiButton * AbortBtn;
        GuiButton * MinimizeBtn;

        GuiSound * soundClick;
        GuiSound * soundOver;

		GuiTrigger * trigA;
};

void StartProgress(const char *title, int mode = PROGRESSBAR);
void ShowProgress(u64 done, u64 total, const char *filename);
void StopProgress();

#endif
