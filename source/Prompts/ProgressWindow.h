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

extern bool actioncanceled;

enum
{
    PROGRESSBAR = 1,
    MULTI_PROGRESSBAR, //! Two progress bars running
    THROBBER,     //! Rotate throbber image only when calling ShowProgress
    AUTO_THROBBER //! Rotating throbber image without calling ShowProgress
};

class ProgressWindow : public GuiWindow, public sigslot::has_slots<>
{
    public:
		static ProgressWindow * Instance();
		static void DestroyInstance();

        void StartProgress(const char *title, int mode = PROGRESSBAR, bool reset = true);
        void ShowProgress(u64 done, u64 total, const char *filename);
        void StopProgress();
        const char * GetTitle() { return ProgressTitle.c_str(); };
        void SetMinimized(bool m) { Minimized = m; };
        void SetMinimizable(bool m) { Minimizable = m; };
        void SetTotalValues(u64 Size, u32 FileCount UNUSED) { TotalSize = Size; };
        void ResetValues() { progressTotal = 0; TotalDone = 0; TotalSize = 0; };
        int GetProgressMode() { return showProgress; };
    protected:
        ProgressWindow();
        virtual ~ProgressWindow();
		static void * ThreadCallback(void *arg);
		void InternalUpdate();
		void SetupProgressbar();
		void SetupMultiProgressbar();
		void SetupThrobber();
		void MainLoop();
		void LoadWindow();
		void UpdateValues();
		void ClearMemory();
        static ProgressWindow * instance;

        u64 progressDone;
        u64 progressTotal;
        u64 TotalDone;
        u64 TotalSize;
        u64 TimerSize;
        int showProgress;
        bool Changed;
        bool Minimized;
        bool Minimizable;
        u8 * ThreadStack;
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
		GuiImage * progressbarTotalOutlineImg;
		GuiImage * progressbarTotalEmptyImg;
		GuiImage * progressbarTotalImg;
		GuiImage * throbberImg;
		GuiImage * buttonImg;
		GuiImage * MinimizeImg;

        GuiText * titleTxt;
        GuiText * msgTxt;
        GuiText * prTxt;
        GuiText * prTotalTxt;
        GuiText * prsTxt;
        GuiText * prsTotalTxt;
        GuiText * speedTxt;
        GuiText * sizeTxt;
        GuiText * sizeTotalTxt;
        GuiText * TimeTxt;;
        GuiText * AbortTxt;
        GuiText * MinimizeTxt;

        GuiButton * AbortBtn;
        GuiButton * MinimizeBtn;

        GuiSound * soundClick;
        GuiSound * soundOver;

		GuiTrigger * trigA;
};

void StartProgress(const char *title, int mode = PROGRESSBAR, bool reset = true);
void ShowProgress(u64 done, u64 total, const char *filename);
void StopProgress();

#endif
