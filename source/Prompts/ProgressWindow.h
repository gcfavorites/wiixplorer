/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef _PROGRESSWINDOW_H_
#define _PROGRESSWINDOW_H_

#include "GUI/gui.h"
#include "PromptWindow.h"
#include "ProgressBar.h"

class ProgressWindow : public GuiFrame, public sigslot::has_slots<>
{
	public:
		static ProgressWindow * Instance() { if(!instance) instance = new ProgressWindow; return instance; }
		static void DestroyInstance() { delete instance; instance = NULL; }

		void StartProgress(const char *title, int mode = SINGLE);
		void ShowProgress(float done, float total, const char *filename);
		void StopProgress();
		const char * GetTitle() { return ProgressTitle.c_str(); };
		void SetMinimized(bool m) { Minimized = m; };
		void SetTotalValues(float Size, u32 FileCount UNUSED) { TotalSize = Size; };
		bool IsCanceled() const { return Canceled; }
		bool IsMinimized() const { return Minimized; }
		void Draw();
		enum
		{
			SINGLE = 1,	//! One progress bar
			MULTI = 2, //! Two progress bar
		};
	private:
		ProgressWindow();
		virtual ~ProgressWindow();
		void OnMinimizeClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED) { Minimized = true; }
		void OnCancelClick(GuiButton *sender UNUSED, int pointer UNUSED, const POINT &p UNUSED) { Canceled = true; }
		void SetupProgressbar();
		void SetupMultiProgressbar();
		void LoadWindow();
		void ClearMemory();
		static ProgressWindow * instance;

		int ProgressType;
		float progressDone;
		float progressTotal;
		float TotalDone;
		float TotalSize;
		bool Changed;
		bool Canceled;
		bool Minimized;
		bool WindowClosed;

		Timer delayTimer;
		Timer ProgressTimer;

		std::string ProgressTitle;
		const char *ProgressMsg;

		ProgressBar *progressBar;
		ProgressBar *totalProgressBar;

		GuiImageData *dialogBox;
		GuiImageData *btnOutline;

		GuiImage * dialogBoxImg;
		GuiImage * buttonImg;
		GuiImage * MinimizeImg;

		GuiText * titleTxt;
		GuiText * msgTxt;
		GuiText * speedTxt;
		GuiText * sizeTxt;
		GuiText * sizeTotalTxt;
		GuiText * TimeTxt;;
		GuiText * AbortBtnTxt;
		GuiText * MinimizeTxt;

		GuiButton * AbortBtn;
		GuiButton * MinimizeBtn;

		GuiSound * soundClick;
		GuiSound * soundOver;

		GuiTrigger * trigA;
};

#define StartProgress	ProgressWindow::Instance()->StartProgress
#define ShowProgress	ProgressWindow::Instance()->ShowProgress
#define StopProgress	ProgressWindow::Instance()->StopProgress

#endif
