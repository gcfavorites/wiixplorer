#include <unistd.h>

#include "HomeMenu.h"
#include "Controls/MainWindow.h"
#include "Prompts/PromptWindows.h"
#include "SoundOperations/gui_bgm.h"
#include "sys.h"

#include "Memory/Resources.h"

HomeMenu::HomeMenu()
	: GuiWindow(0, 0)
{
	choice = -1;

	this->SetPosition(0, 0);
	this->SetSize(screenwidth, screenheight);

	trigA = new SimpleGuiTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigHome = new GuiTrigger();
	trigHome->SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_TRIGGER_L);

	ButtonClickSnd = Resources::GetSound(button_click_wav, button_click_wav_size);
	ButtonOverSnd = Resources::GetSound(button_over_wav, button_over_wav_size);

	TopBtnImgData = new GuiImageData(homemenu_top_png, homemenu_top_png_size);
	TopBtnOverImgData = new GuiImageData(homemenu_top_over_png, homemenu_top_over_png_size);
	BottomBtnImgData = new GuiImageData(homemenu_bottom_png, homemenu_bottom_png_size);
	BottomBtnOverImgData = new GuiImageData(homemenu_bottom_over_png, homemenu_bottom_over_png_size);
	CloseBtnImgData = new GuiImageData(homemenu_close_png, homemenu_close_png_size);
	StandardBtnImgData = new GuiImageData(homemenu_button_png, homemenu_button_png_size);
	WiimoteBtnImgData = new GuiImageData(wiimote_png, wiimote_png_size);

	BatteryImgData = new GuiImageData(battery_png, battery_png_size);
	BatteryBarImgData = new GuiImageData(battery_bar_png, battery_bar_png_size);

	TopBtnImg = new GuiImage(TopBtnImgData);
	TopBtnOverImg = new GuiImage(TopBtnOverImgData);
	BottomBtnImg = new GuiImage(BottomBtnImgData);
	BottomBtnOverImg = new GuiImage(BottomBtnOverImgData);
	CloseBtnImg = new GuiImage(CloseBtnImgData);
	LoaderBtnImg = new GuiImage(StandardBtnImgData);
	MenuBtnImg = new GuiImage(StandardBtnImgData);
	WiimoteBtnImg = new GuiImage(WiimoteBtnImgData);

	TitleText = new GuiText(tr("Home Menu"), 40, (GXColor) {255, 255, 255, 255});
	TitleText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	TitleText->SetPosition(30, 40);
	TitleText->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	TopBtn = new GuiButton(TopBtnImg->GetWidth(), TopBtnImg->GetHeight());
	TopBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	TopBtn->SetImage(TopBtnImg);
	TopBtn->SetImageOver(TopBtnOverImg);
	TopBtn->SetLabel(TitleText);
	TopBtn->SetSoundClick(ButtonClickSnd);
	TopBtn->SetSoundOver(ButtonOverSnd);
	TopBtn->SetTrigger(trigA);
	TopBtn->SetTrigger(trigHome);
	TopBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	BottomBtn = new GuiButton(BottomBtnImg->GetWidth(), BottomBtnImg->GetHeight());
	BottomBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	BottomBtn->SetImage(BottomBtnImg);
	BottomBtn->SetImageOver(BottomBtnOverImg);
	BottomBtn->SetSoundClick(ButtonClickSnd);
	BottomBtn->SetSoundOver(ButtonOverSnd);
	BottomBtn->SetTrigger(trigA);
	BottomBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
	BottomBtn->SetSelectable(true);

	CloseBtnText = new GuiText(tr("Close"), 28, (GXColor) {0, 0, 0, 255});

	CloseBtn = new GuiButton(CloseBtnImg->GetWidth(), CloseBtnImg->GetHeight());
	CloseBtn->SetImage(CloseBtnImg);
	CloseBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	CloseBtn->SetPosition(-20, 30);
	CloseBtn->SetLabel(CloseBtnText);
	CloseBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	LoaderBtnText = new GuiText(tr("Loader"), 28, (GXColor) {0, 0, 0, 255});

	LoaderBtn = new GuiButton(LoaderBtnImg->GetWidth(), LoaderBtnImg->GetHeight());
	LoaderBtn->SetImage(LoaderBtnImg);
	LoaderBtn->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	LoaderBtn->SetPosition(-140, 0);
	LoaderBtn->SetLabel(LoaderBtnText);
	LoaderBtn->SetSoundClick(ButtonClickSnd);
	LoaderBtn->SetSoundOver(ButtonOverSnd);
	LoaderBtn->SetTrigger(trigA);
	LoaderBtn->SetClickable(true);
	LoaderBtn->SetEffectGrow();
	LoaderBtn->SetEffect(EFFECT_FADE, 50);

	MenuBtnText = new GuiText(tr("Menu"), 28, (GXColor) {0, 0, 0, 255});

	MenuBtn = new GuiButton(MenuBtnImg->GetWidth(), MenuBtnImg->GetHeight());
	MenuBtn->SetImage(MenuBtnImg);
	MenuBtn->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	MenuBtn->SetPosition(140, 0);
	MenuBtn->SetLabel(MenuBtnText);
	MenuBtn->SetSoundClick(ButtonClickSnd);
	MenuBtn->SetSoundOver(ButtonOverSnd);
	MenuBtn->SetClickable(true);
	MenuBtn->SetTrigger(trigA);
	MenuBtn->SetEffectGrow();
	MenuBtn->SetEffect(EFFECT_FADE, 50);

	WiimoteBtn = new GuiButton(WiimoteBtnImg->GetWidth(), WiimoteBtnImg->GetHeight());
	WiimoteBtn->SetImage(WiimoteBtnImg);
	WiimoteBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	WiimoteBtn->SetPosition(45, 232);
	WiimoteBtn->SetTrigger(trigA);
	WiimoteBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

	Append(BottomBtn);
	Append(TopBtn);
	Append(CloseBtn);
	Append(LoaderBtn);
	Append(MenuBtn);
	Append(WiimoteBtn);

	for (int i = 0; i < 4; i++)
	{
		char player[] = "P0";
		player[1] = i+'1';

		PlayerText[i] = new GuiText(player, 28, (GXColor) {255, 255, 255, 255});
		PlayerText[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		PlayerText[i]->SetPosition(178 + i*108, -76);
		PlayerText[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
		Append(PlayerText[i]);

		BatteryBarImg[i] = new GuiImage(BatteryBarImgData);
		BatteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);

		BatteryImg[i] = new GuiImage(BatteryImgData);
        BatteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		BatteryImg[i]->SetPosition(2, -4);

		BatteryBtn[i] = new GuiButton(0,0);
		BatteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		BatteryBtn[i]->SetPosition(214 + i*108, -80);
		BatteryBtn[i]->SetImage(BatteryBarImg[i]);
		BatteryBtn[i]->SetIcon(BatteryImg[i]);
		BatteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

		Append(BatteryBtn[i]);
	}

	GuiBGM::Instance()->Stop();
	MainWindow::Instance()->SetState(STATE_DISABLED);
    MainWindow::Instance()->SetDim(true);
}

HomeMenu::~HomeMenu()
{
	MainWindow::Instance()->ResumeGui();
	SetEffect(EFFECT_FADE, -50);
	while(this->GetEffect() > 0) usleep(100);

	MainWindow::Instance()->HaltGui();
	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	RemoveAll();

	delete WiimoteBtn;
	delete MenuBtn;
	delete LoaderBtn;
	delete CloseBtn;
	delete TopBtn;
	delete BottomBtn;

	delete TopBtnImg;
	delete TopBtnOverImg;
	delete BottomBtnImg;
	delete BottomBtnOverImg;
	delete CloseBtnImg;
	delete LoaderBtnImg;
	delete MenuBtnImg;
	delete WiimoteBtnImg;

	delete TopBtnImgData;
	delete TopBtnOverImgData;
	delete BottomBtnImgData;
	delete BottomBtnOverImgData;
	delete CloseBtnImgData;
	delete StandardBtnImgData;
	delete WiimoteBtnImgData;
	delete BatteryImgData;
	delete BatteryBarImgData;

	delete TitleText;
	delete MenuBtnText;
	delete LoaderBtnText;
	delete CloseBtnText;

	for (int i = 0; i < 4; i++)
	{
		delete PlayerText[i];
		delete BatteryBarImg[i];
		delete BatteryImg[i];
		delete BatteryBtn[i];
	}

	delete trigA;
	delete trigHome;

	Resources::Remove(ButtonClickSnd);
	Resources::Remove(ButtonOverSnd);

	GuiBGM::Instance()->Play();
	MainWindow::Instance()->SetState(STATE_DEFAULT);
	MainWindow::Instance()->SetDim(false);
	MainWindow::Instance()->ResumeGui();
}

void HomeMenu::FadeOut()
{
	TitleText->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	TopBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	CloseBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	LoaderBtn->SetEffect(EFFECT_FADE, -50);
	MenuBtn->SetEffect(EFFECT_FADE, -50);
	BottomBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	WiimoteBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);

	for (int i = 0; i < 4; i++)
	{
		PlayerText[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
		BatteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	}
}

int HomeMenu::GetChoice()
{
	for (int i = 0; i < 4; i++)
	{
		if (WPAD_Probe(i, NULL) == WPAD_ERR_NONE)
		{
			int level = (WPAD_BatteryLevel(i) / 100.0) * 4;
			if (level > 4) level = 4;

			BatteryImg[i]->SetTileHorizontal(level);
			BatteryBtn[i]->SetAlpha(255);
			PlayerText[i]->SetAlpha(255);
		}
		else
		{
			BatteryImg[i]->SetTileHorizontal(0);
			BatteryBtn[i]->SetAlpha(130);
			PlayerText[i]->SetAlpha(100);
		}
	}

	if (TopBtn->GetState() == STATE_CLICKED)
	{
		TopBtn->ResetState();
		FadeOut();
		choice = 0; // return to WiiXplorer
	}
	else if (BottomBtn->GetState() == STATE_CLICKED)
	{
		BottomBtn->ResetState();
		FadeOut();
		choice = 0; // return to WiiXplorer
	}
	else if (LoaderBtn->GetState() == STATE_CLICKED)
	{
		LoaderBtn->ResetState();

		this->SetState(STATE_DISABLED);
		if (WindowPrompt(tr("Do you want to exit WiiXplorer?"), 0, tr("Yes"), tr("Cancel"), 0, 0, false))
		{
			Sys_BackToLoader();
		}
		this->SetState(STATE_DEFAULT);
	}
	else if (MenuBtn->GetState() == STATE_CLICKED)
	{
		MenuBtn->ResetState();

		this->SetState(STATE_DISABLED);
		if (WindowPrompt(tr("Do you want to exit to the system menu?"), 0, tr("Yes"), tr("Cancel"), 0, 0, false))
		{
			Sys_LoadMenu();
		}
		this->SetState(STATE_DEFAULT);
	}
	else if (BottomBtn->GetState() == STATE_SELECTED)
	{
		WiimoteBtn->SetPosition(WiimoteBtn->GetLeft(), 210);
	}
	else if (BottomBtn->GetState() != STATE_SELECTED)
	{
		WiimoteBtn->SetPosition(WiimoteBtn->GetLeft(), 232);
	}

	return choice;
}