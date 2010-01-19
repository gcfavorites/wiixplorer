 /****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * TextEditor.cpp
 * for WiiXplorer 2009
 ***************************************************************************/

#include <unistd.h>
#include "TextOperations/TextEditor.h"
#include "Prompts/PromptWindows.h"
#include "Language/gettext.h"
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"

#define FONTSIZE    18

/**
 * Constructor for the TextEditor class.
 */
TextEditor::TextEditor(char *intext, int LinesToDraw, const char *path)
{
	focus = 0; // allow focus
	triggerdisabled = false;
	ExitEditor = false;
	LineEditing = false;
	FileEdited = false;
	linestodraw = LinesToDraw;
	currentLine = 0;

	filepath = new char[strlen(path)+1];
	snprintf(filepath, strlen(path)+1, "%s", path);

	char * filename = strrchr(filepath, '/')+1;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	bgTexteditorData = new GuiImageData(textreader_box_png);
	bgTexteditorImg = new GuiImage(bgTexteditorData);

	scrollbar = new GuiImageData(scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(-25, 80);

	arrowDown = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOver = new GuiImageData(scrollbar_arrowdown_over_png);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUp = new GuiImageData(scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData(scrollbar_arrowup_over_png);
	arrowUpOverImg = new GuiImage(arrowUpOver);
	scrollbarBox = new GuiImageData(scrollbar_box_png);
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOver = new GuiImageData(scrollbar_box_over_png);
	scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);
	closeImgData = new GuiImageData(close_png);
	closeImgOverData = new GuiImageData(close_over_png);
    closeImg = new GuiImage(closeImgData);
    closeImgOver = new GuiImage(closeImgOverData);
	maximizeImgData = new GuiImageData(maximize_dis_png);
    maximizeImg = new GuiImage(maximizeImgData);
	minimizeImgData = new GuiImageData(minimize_dis_png);
    minimizeImg = new GuiImage(minimizeImgData);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	arrowUpBtn->SetPosition(-25, 60);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetClickable(false);
	arrowUpBtn->SetHoldable(true);
	arrowUpBtn->SetTrigger(trigHeldA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	arrowDownBtn->SetPosition(-25, -30);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetClickable(false);
	arrowDownBtn->SetHoldable(true);
	arrowDownBtn->SetTrigger(trigHeldA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarBoxBtn->SetPosition(-25, 55+36);
	scrollbarBoxBtn->SetMinY(0);
	scrollbarBoxBtn->SetMaxY(120);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetClickable(false);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);

    closeBtn = new GuiButton(closeImg->GetWidth(), closeImg->GetHeight());
    closeBtn->SetImage(closeImg);
    closeBtn->SetImageOver(closeImgOver);
    closeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    closeBtn->SetPosition(-30, 30);
    closeBtn->SetSoundOver(btnSoundOver);
    closeBtn->SetSoundClick(btnSoundClick);
    closeBtn->SetTrigger(trigA);
    closeBtn->SetTrigger(trigB);
    closeBtn->SetEffectGrow();
    closeBtn->Clicked.connect(this, &TextEditor::OnButtonClick);

    maximizeBtn = new GuiButton(maximizeImg->GetWidth(), maximizeImg->GetHeight());
    maximizeBtn->SetImage(maximizeImg);
    maximizeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    maximizeBtn->SetPosition(-60, 30);
    maximizeBtn->SetSoundClick(btnSoundClick);

    minimizeBtn = new GuiButton(minimizeImg->GetWidth(), minimizeImg->GetHeight());
    minimizeBtn->SetImage(minimizeImg);
    minimizeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    minimizeBtn->SetPosition(-90, 30);
    minimizeBtn->SetSoundClick(btnSoundClick);

    filenameTxt = new GuiText(filename, 22, (GXColor){0, 0, 0, 255});
    filenameTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    filenameTxt->SetPosition(-30,30);
    filenameTxt->SetMaxWidth(340, DOTTED);

    MainFileTxt = new GuiText(intext, FONTSIZE, (GXColor){0, 0, 0, 255});
    MainFileTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    MainFileTxt->SetPosition(0, 0);
    MainFileTxt->SetMaxWidth(343, LONGTEXT);
    MainFileTxt->SetFirstLine(0);
    MainFileTxt->SetLinesToDraw(linestodraw);
    TotalLines = MainFileTxt->GetTotalLines();

    TextPointerBtn = new TextPointer(MainFileTxt, FONTSIZE, linestodraw);
    TextPointerBtn->SetPosition(43, 75);
    TextPointerBtn->SetClickable(false);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->Held.connect(this, &TextEditor::OnPointerHeld);

    PlusBtn = new GuiButton(0, 0);
    PlusBtn->SetTrigger(trigPlus);
    PlusBtn->SetSoundClick(btnSoundClick);
    PlusBtn->Clicked.connect(this, &TextEditor::OnButtonClick);

	width = bgTexteditorImg->GetWidth();
	height = bgTexteditorImg->GetHeight();

    this->Append(PlusBtn);
    this->Append(bgTexteditorImg);
    this->Append(filenameTxt);
    this->Append(TextPointerBtn);
    this->Append(scrollbarImg);
    this->Append(scrollbarBoxBtn);
    this->Append(arrowUpBtn);
    this->Append(arrowDownBtn);
    this->Append(closeBtn);
    this->Append(maximizeBtn);
    this->Append(minimizeBtn);

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    SetPosition(0,0);
    SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
}

/**
 * Destructor for the TextEditor class.
 */
TextEditor::~TextEditor()
{
    MainWindow::Instance()->ResumeGui();
    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(this->GetEffect() > 0) usleep(50);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    this->RemoveAll();

    /** Buttons **/
	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;
	delete maximizeBtn;
	delete minimizeBtn;
	delete closeBtn;
	delete TextPointerBtn;

    /** Images **/
	delete bgTexteditorImg;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;
	delete closeImg;
	delete closeImgOver;
	delete maximizeImg;
	delete minimizeImg;

    /** ImageDatas **/
	delete bgTexteditorData;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;
	delete scrollbarBox;
	delete scrollbarBoxOver;
	delete closeImgData;
	delete closeImgOverData;
	delete maximizeImgData;
	delete minimizeImgData;

    /** Sounds **/
	delete btnSoundOver;
	delete btnSoundClick;

    /** Triggers **/
	delete trigHeldA;
	delete trigA;
	delete trigB;
	delete PlusBtn;

    /** Texts **/
    delete filenameTxt;
    delete MainFileTxt;
    delete [] filepath;

    MainWindow::Instance()->ResumeGui();
}

void TextEditor::SetText(const char *intext)
{
    LOCK(this);
    if(MainFileTxt)
    {
        delete MainFileTxt;
        MainFileTxt = NULL;
    }
    if(TextPointerBtn)
    {
        delete TextPointerBtn;
        TextPointerBtn = NULL;
    }

    MainFileTxt = new GuiText(intext, FONTSIZE, (GXColor){0, 0, 0, 255});
    MainFileTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    MainFileTxt->SetPosition(0, 0);
    MainFileTxt->SetMaxWidth(343, LONGTEXT);
    MainFileTxt->SetFirstLine(0);
    MainFileTxt->SetLinesToDraw(linestodraw);
    TotalLines = MainFileTxt->GetTotalLines();

    TextPointerBtn = new TextPointer(MainFileTxt, FONTSIZE, linestodraw);
    TextPointerBtn->SetPosition(43, 75);
    TextPointerBtn->SetClickable(false);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->Held.connect(this, &TextEditor::OnPointerHeld);
}

void TextEditor::DisableTriggerUpdate(bool set)
{
    LOCK(this);
	triggerdisabled = set;
}

void TextEditor::WriteTextFile(const char * path)
{
    FILE * f = fopen(path, "wb");
    fwrite(MainFileTxt->GetOrigText(), 1, strlen(MainFileTxt->GetOrigText())+1, f);
    fclose(f);
}

void TextEditor::ResetState()
{
    LOCK(this);
	state = STATE_DEFAULT;
	stateChan = -1;

	arrowUpBtn->ResetState();
	arrowDownBtn->ResetState();
	scrollbarBoxBtn->ResetState();
	maximizeBtn->ResetState();
	minimizeBtn->ResetState();
	closeBtn->ResetState();
}

int TextEditor::GetState()
{
    if(LineEditing)
    {
        DisableTriggerUpdate(true);
        if(TextPointerBtn->EditLine() > 0)
        {
            FileEdited = true;
        }
        DisableTriggerUpdate(false);

        MainWindow::Instance()->SetState(STATE_DISABLED);
        MainWindow::Instance()->SetDim(true);
        MainWindow::Instance()->ChangeFocus(this);
        this->SetDim(false);
        this->SetState(STATE_DEFAULT);
        LineEditing = false;
    }

    if(state == STATE_CLOSED && FileEdited)
    {
        int choice = WindowPrompt(tr("File was edited."), tr("Do you want to save changes?"), tr("Yes"), tr("Cancel"));
        if(choice)
            WriteTextFile(filepath);

        //to revert the state reset
        state = STATE_CLOSED;
    }

    return GuiWindow::GetState();
}

void TextEditor::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == closeBtn)
        SetState(STATE_CLOSED);

    else if(sender == PlusBtn)
    {
        LineEditing = true;
    }
}

void TextEditor::OnPointerHeld(GuiElement *sender, int pointer, POINT p)
{
    TextPointerBtn->PositionChanged(pointer, p.x, p.y);
}

void TextEditor::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t || triggerdisabled)
		return;

	int position = 0;
	int positionWiimote = 0;

	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);
	scrollbarBoxBtn->Update(t);
	maximizeBtn->Update(t);
	closeBtn->Update(t);
	minimizeBtn->Update(t);
	TextPointerBtn->Update(t);
	PlusBtn->Update(t);

	if(TotalLines < linestodraw)
        return;

	if(scrollbarBoxBtn->GetState() == STATE_HELD &&
		scrollbarBoxBtn->GetStateChan() == t->chan &&
		t->wpad->ir.valid)
	{
		scrollbarBoxBtn->SetPosition(-25, 55);
		positionWiimote = t->wpad->ir.y - 60 - scrollbarBoxBtn->GetTop();

		if(positionWiimote < scrollbarBoxBtn->GetMinY())
			positionWiimote = scrollbarBoxBtn->GetMinY();
		else if(positionWiimote > scrollbarBoxBtn->GetMaxY())
			positionWiimote = scrollbarBoxBtn->GetMaxY();

		currentLine = (positionWiimote * TotalLines)/120.0;

        if(currentLine+linestodraw > TotalLines)
            currentLine = TotalLines-linestodraw;
        if(currentLine < 0)
            currentLine = 0;

		MainFileTxt->SetFirstLine(currentLine);
	}

	if(arrowDownBtn->GetState() == STATE_HELD && arrowDownBtn->GetStateChan() == t->chan)
	{
        currentLine++;
        if(currentLine+linestodraw > TotalLines)
            currentLine = TotalLines-linestodraw;
        if(currentLine < 0)
            currentLine = 0;

        MainFileTxt->SetFirstLine(currentLine);
	}
	else if(arrowUpBtn->GetState() == STATE_HELD && arrowUpBtn->GetStateChan() == t->chan)
	{
		currentLine--;
        if(currentLine < 0)
            currentLine = 0;
        MainFileTxt->SetFirstLine(currentLine);
	}

	if(t->Right())
	{
		currentLine += 8;
        if(currentLine+linestodraw > TotalLines)
            currentLine = TotalLines-linestodraw;
        if(currentLine < 0)
            currentLine = 0;

        MainFileTxt->SetFirstLine(currentLine);
	}
	else if(t->Left())
	{
		currentLine -= 8;
        if(currentLine < 0)
            currentLine = 0;

        MainFileTxt->SetFirstLine(currentLine);
	}
	else if(t->Down())
	{
		currentLine++;
        if(currentLine+linestodraw > TotalLines)
            currentLine = TotalLines-linestodraw;
        if(currentLine < 0)
            currentLine = 0;

        MainFileTxt->SetFirstLine(currentLine);
	}
	else if(t->Up())
	{
		currentLine--;
        if(currentLine < 0)
            currentLine = 0;
        MainFileTxt->SetFirstLine(currentLine);
	}

	// update the location of the scroll box based on the position in the file list
	if(positionWiimote > 0)
	{
		position = positionWiimote; // follow wiimote cursor
	}
	else
	{
		position = 120*(currentLine + linestodraw/2.0) / (TotalLines*1.0);

		if(currentLine < 1)
			position = 0;
		else if((currentLine+linestodraw) >= TotalLines)
			position = 120;
	}

	scrollbarBoxBtn->SetPosition(-25,position+36+55);

	if(updateCB)
		updateCB(this);
}
