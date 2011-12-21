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
#include <unistd.h>
#include "TextOperations/TextEditor.h"
#include "FileOperations/fileops.h"
#include "Prompts/PromptWindows.h"
#include "Controls/Application.h"
#include "Memory/Resources.h"
#include "FreeTypeGX.h"
#include "input.h"
#include "menu.h"

#define FONTSIZE	18

/**
 * Constructor for the TextEditor class.
 */
TextEditor::TextEditor(const wchar_t *intext, int LinesToDraw, const char *path)
{
	ExitEditor = false;
	FileEdited = false;
	linestodraw = LinesToDraw;
	filesize = (u32) FileSize(path);

	filepath = new char[strlen(path)+1];
	sprintf(filepath, "%s", path);

	char * filename = strrchr(filepath, '/')+1;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WiiControls.EditTextLine | ClassicControls.EditTextLine << 16, GCControls.EditTextLine);

	btnSoundOver = Resources::GetSound("button_over.wav");
	btnSoundClick = Resources::GetSound("button_click.wav");

	bgTexteditorData = Resources::GetImageData("textreader_box.png");
	bgTexteditorImg = new GuiImage(bgTexteditorData);

	closeImgData = Resources::GetImageData("close.png");
	closeImgOverData = Resources::GetImageData("close_over.png");
	closeImg = new GuiImage(closeImgData);
	closeImgOver = new GuiImage(closeImgOverData);
	maximizeImgData = Resources::GetImageData("maximize_dis.png");
	maximizeImg = new GuiImage(maximizeImgData);
	minimizeImgData = Resources::GetImageData("minimize_dis.png");
	minimizeImg = new GuiImage(minimizeImgData);

	scrollbar = new Scrollbar(230, Scrollbar::LISTMODE);
	scrollbar->SetParent(this);
	scrollbar->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	scrollbar->SetPosition(-25, 60);
	scrollbar->SetScrollSpeed(Settings.ScrollSpeed);
	scrollbar->listChanged.connect(this, &TextEditor::OnListChange);

	closeBtn = new GuiButton(closeImg->GetWidth(), closeImg->GetHeight());
	closeBtn->SetImage(closeImg);
	closeBtn->SetImageOver(closeImgOver);
	closeBtn->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	closeBtn->SetPosition(-30, 30);
	closeBtn->SetSoundOver(btnSoundOver);
	closeBtn->SetSoundClick(btnSoundClick);
	closeBtn->SetTrigger(trigA);
	closeBtn->SetTrigger(trigB);
	closeBtn->SetEffectGrow();
	closeBtn->Clicked.connect(this, &TextEditor::OnButtonClick);

	maximizeBtn = new GuiButton(maximizeImg->GetWidth(), maximizeImg->GetHeight());
	maximizeBtn->SetImage(maximizeImg);
	maximizeBtn->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	maximizeBtn->SetPosition(-60, 30);
	maximizeBtn->SetSoundClick(btnSoundClick);

	minimizeBtn = new GuiButton(minimizeImg->GetWidth(), minimizeImg->GetHeight());
	minimizeBtn->SetImage(minimizeImg);
	minimizeBtn->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	minimizeBtn->SetPosition(-90, 30);
	minimizeBtn->SetSoundClick(btnSoundClick);

	filenameTxt = new GuiText(filename, 22, (GXColor){0, 0, 0, 255});
	filenameTxt->SetAlignment(ALIGN_CENTER | ALIGN_TOP);
	filenameTxt->SetPosition(-30,30);
	filenameTxt->SetMaxWidth(340, DOTTED);

	MainFileTxt = new GuiLongText(intext, FONTSIZE, (GXColor){0, 0, 0, 255});
	MainFileTxt->SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	MainFileTxt->SetPosition(0, 0);
	MainFileTxt->SetLinesToDraw(linestodraw);
	MainFileTxt->SetMaxWidth(330);

	TextPointerBtn = new TextPointer(MainFileTxt, linestodraw);
	TextPointerBtn->SetPosition(43, 75);
	TextPointerBtn->SetHoldable(true);
	TextPointerBtn->SetTrigger(trigHeldA);
	TextPointerBtn->PositionChanged(0, 0, 0);
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
	this->Append(scrollbar);
	this->Append(closeBtn);
	this->Append(maximizeBtn);
	this->Append(minimizeBtn);

	SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	SetPosition(0,0);
	SetAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
}

/**
 * Destructor for the TextEditor class.
 */
TextEditor::~TextEditor()
{
	SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(this->GetEffect() > 0)
		Application::Instance()->updateEvents();

	if(parentElement)
		((GuiFrame *) parentElement)->Remove(this);

	this->RemoveAll();

	delete scrollbar;

	/** Buttons **/
	delete maximizeBtn;
	delete minimizeBtn;
	delete closeBtn;
	delete TextPointerBtn;

	/** Images **/
	delete bgTexteditorImg;
	delete closeImg;
	delete closeImgOver;
	delete maximizeImg;
	delete minimizeImg;

	/** ImageDatas **/
	Resources::Remove(bgTexteditorData);
	Resources::Remove(closeImgData);
	Resources::Remove(closeImgOverData);
	Resources::Remove(maximizeImgData);
	Resources::Remove(minimizeImgData);

	/** Sounds **/
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnSoundClick);

	/** Triggers **/
	delete trigHeldA;
	delete trigA;
	delete trigB;
	delete PlusBtn;

	/** Texts **/
	delete filenameTxt;
	delete MainFileTxt;
	delete [] filepath;
}

void TextEditor::SetText(const wchar_t *intext)
{
	if(TextPointerBtn)
	{
		delete TextPointerBtn;
		TextPointerBtn = NULL;
	}

	MainFileTxt->SetText(intext);

	TextPointerBtn = new TextPointer(MainFileTxt, 0);
	TextPointerBtn->SetPosition(43, 75);
	TextPointerBtn->SetHoldable(true);
	TextPointerBtn->SetTrigger(trigHeldA);
	TextPointerBtn->Held.connect(this, &TextEditor::OnPointerHeld);
}

void TextEditor::WriteTextFile(const char * path)
{
	FILE * f = fopen(path, "wb");
	if(!f)
	{
		ShowError(tr("Cannot write to the file."));
		return;
	}

	const std::string &FullText = MainFileTxt->toUTF8();

	fwrite(FullText.c_str(), 1, strlen(FullText.c_str())+1, f);

	fclose(f);
}

int TextEditor::EditLine()
{
	int currentline = TextPointerBtn->GetCurrentLine();

	if(currentline < 0 || currentline >= linestodraw)
		return -1;

	u32 LetterNumInLine = TextPointerBtn->GetCurrentLetter();

	wString * wText = MainFileTxt->GetwString();
	if(!wText)
		return -1;

	const wchar_t * lineText = MainFileTxt->GetTextLine(currentline);
	if(!lineText)
		return -1;

	wchar_t temptxt[150];
	memset(temptxt, 0, sizeof(temptxt));

	int LineOffset = MainFileTxt->GetLineOffset(currentline+MainFileTxt->GetCurrPos());

	wcsncpy(temptxt, lineText, LetterNumInLine);
	temptxt[LetterNumInLine] = 0;

	int result = OnScreenKeyboard(temptxt, 150);
	if(result == 1)
	{
		wText->replace(LineOffset, LetterNumInLine, temptxt);
		MainFileTxt->Refresh();
		FileEdited = true;
		return 1;
	}

	return -1;
}

void TextEditor::OnButtonClick(GuiButton *sender, int pointer UNUSED, const POINT &p UNUSED)
{
	if(sender == closeBtn)
	{
		int choice = 1;
		if(FileEdited)
		{
			choice = WindowPrompt(tr("File was edited."), tr("Do you want to save changes?"), tr("Yes"), tr("No"), tr("Cancel"));
			if(choice == 1)
				WriteTextFile(filepath);
		}
		if(choice)
		{
			Application::Instance()->UnsetUpdateOnly(this);
			Application::Instance()->PushForDelete(this);
		}
	}
	else if(sender == PlusBtn)
	{
		EditLine();
	}
}

void TextEditor::OnPointerHeld(GuiButton *sender UNUSED, int pointer, const POINT &p)
{
	if(!userInput[pointer].wpad->ir.valid)
		return;

	TextPointerBtn->PositionChanged(pointer, p.x - TextPointerBtn->GetLeft(), p.y - TextPointerBtn->GetTop());
}

void TextEditor::OnListChange(int selItem, int selIndex)
{
	MainFileTxt->SetTextLine(selItem+selIndex);
}

void TextEditor::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;

	scrollbar->Update(t);
	maximizeBtn->Update(t);
	closeBtn->Update(t);
	minimizeBtn->Update(t);
	TextPointerBtn->Update(t);
	PlusBtn->Update(t);

	scrollbar->SetEntrieCount(MainFileTxt->GetTotalLinesCount());
	scrollbar->SetPageSize(linestodraw);
	scrollbar->SetRowSize(0);
	scrollbar->SetSelectedItem(0);
	scrollbar->SetSelectedIndex(MainFileTxt->GetCurrPos());
}

void TextEditor::LoadFile(const char *filepath)
{
	u8 *file = NULL;
	u32 filesize = 0;

	int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &file, &filesize);
	if(ret < 0)
	{
		ShowError(tr("Could not load text file."));
		return;
	}
	else if(filesize > (u32) (4.5*MBSIZE))
	{
		free(file);
		ShowError(tr("File is too big."));
		return;
	}

	u8 * tmp = (u8 *) realloc(file, filesize+1);
	if(!tmp)
	{
		free(file);
		ShowError(tr("Not enough memory."));
		return;
	}
	file = tmp;
	file[filesize] = 0;
	filesize++;

	wString * filetext = NULL;

	//To check if text is UTF8 or not
	if(utf8Len((char*) file) > 0)
	{
		filetext = new (std::nothrow) wString();
		if(!filetext)
		{
			free(file);
			file = NULL;
			ShowError(tr("Not enough memory."));
			return;
		}

		filetext->fromUTF8((char*) file);
		free(file);
		file = NULL;
	}
	else
	{
		wchar_t * tmptext = charToWideChar((char*) file);

		free(file);
		file = NULL;

		if(!tmptext)
		{
			ShowError(tr("Not enough memory."));
			return;
		}

		filetext = new (std::nothrow) wString(tmptext);

		delete [] tmptext;

		if(!filetext)
		{
			ShowError(tr("Not enough memory."));
			return;
		}
	}

	TextEditor * Editor = new TextEditor(filetext->c_str(), 9, filepath);
	Editor->SetAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
	Editor->SetPosition(0, 0);
	Editor->DimBackground(true);

	delete filetext;
	filetext = NULL;

	Application::Instance()->SetUpdateOnly(Editor);
	Application::Instance()->Append(Editor);
}
