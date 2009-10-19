/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_optionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "gui_optionbrowser.h"

#define BROWSERSIZE        8


OptionList::OptionList(int size)
{
	name = new char * [size+1];
	value = new char * [size+1];
	for (int i = 0; i < size+1; i++)
	{
		name[i] = NULL;
		value[i] = NULL;
	}
	length = size+1;
}
OptionList::~OptionList()
{
	for (int i = 0; i < length; i++)
	{
	    if(name[i]) {
            free(name[i]);
            name[i] = NULL;
	    }
	    if(value[i]) {
            free(value[i]);
            name[i] = NULL;
	    }
	}
	delete [] name;
	delete [] value;
}

void OptionList::SetName(int i, const char *format, ...)
{
	if(i >= 0 && i < length)
	{
		if(name[i]) free(name[i]);
		name[i] = NULL;
		va_list va;
		va_start(va, format);
		vasprintf(&name[i], format, va);
		va_end(va);
		listChanged = true;
	}
}
void OptionList::SetValue(int i, const char *format, ...)
{
	if(i >= 0 && i < length)
	{
		if(value[i]) free(value[i]);
		value[i] = NULL;
		va_list va;
		va_start(va, format);
		vasprintf(&value[i], format, va);
		va_end(va);
		listChanged = true;
	}
}

/**
 * Constructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l)
{
	width = w;
	height = h;
	options = l;
	coL2 = 50;
	selectable = true;
	listOffset = this->FindMenuItem(-1, 1);
	listChanged = true; // trigger an initial list update
	selectedItem = 0;
	focus = 0; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	bgOptions = new GuiImageData(bg_browser_png);
	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	
	bgOptionsEntry = new GuiImageData(bg_browser_selection_png);

	scrollbar = new GuiImageData(scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(-10, 30);

	arrowDown = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOver = new GuiImageData(scrollbar_arrowdown_over_png);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUp = new GuiImageData(scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData(scrollbar_arrowup_over_png);
	arrowUpOverImg = new GuiImage(arrowUpOver);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	arrowUpBtn->SetPosition(-10, 0);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetTrigger(trigA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	arrowDownBtn->SetPosition(-10, 0);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetTrigger(trigA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	scrollbarBox = new GuiImageData(scrollbar_box_png);
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOver = new GuiImageData(scrollbar_box_over_png);
	scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarBoxBtn->SetPosition(-10, 0);
	scrollbarBoxBtn->SetMinY(0);
	scrollbarBoxBtn->SetMaxY(136);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetClickable(false);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);

	for(int i=0; i<PAGESIZE; i++)
	{
		optionTxt[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(15,0);

		optionVal[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionVal[i]->SetPosition(250,0);
		optionVal[i]->SetMaxWidth(width-optionTxt[i]->GetLeft()-50,DOTTED);

		optionBg[i] = new GuiImage(bgOptionsEntry);

		optionBtn[i] = new GuiButton(512,30);
		optionBtn[i]->SetParent(this);
		optionBtn[i]->SetLabel(optionTxt[i], 0);
		optionBtn[i]->SetLabel(optionVal[i], 1);
		optionBtn[i]->SetImageOver(optionBg[i]);
		optionBtn[i]->SetPosition(2,30*i+3);
		optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;

	delete bgOptionsImg;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;

	delete bgOptions;
	delete bgOptionsEntry;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;
	delete scrollbarBox;
	delete scrollbarBoxOver;

	delete trigA;
	delete trigHeldA;
	delete btnSoundOver;
	delete btnSoundClick;

	for(int i=0; i<PAGESIZE; i++)
	{
		delete optionTxt[i];
		delete optionVal[i];
		delete optionBg[i];
		delete optionBtn[i];
	}
}

void GuiOptionBrowser::SetCol2Position(int x)
{
	for(int i=0; i<PAGESIZE; i++)
		optionVal[i]->SetPosition(x,0);
}

void GuiOptionBrowser::SetFocus(int f)
{
	focus = f;

	for(int i=0; i<PAGESIZE; i++)
		optionBtn[i]->ResetState();

	if(f == 1)
		optionBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiOptionBrowser::ResetState()
{
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i=0; i<PAGESIZE; i++)
	{
		optionBtn[i]->ResetState();
	}
}

int GuiOptionBrowser::GetClickedOption()
{
	int found = -1;
	for(int i=0; i<PAGESIZE; i++)
	{
		if(optionBtn[i]->GetState() == STATE_CLICKED)
		{
			optionBtn[i]->SetState(STATE_SELECTED);
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiOptionBrowser::FindMenuItem(int currentItem, int direction)
{
	int nextItem = currentItem + direction;

	if(nextItem < 0 || nextItem >= options->GetLength())
		return -1;

	if(options->GetName(nextItem))
		return nextItem;
	else
		return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiOptionBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	bgOptionsImg->Draw();

	int next = listOffset;

	for(int i=0; i<PAGESIZE; i++)
	{
		if(next >= 0)
		{
			optionBtn[i]->Draw();
			next = this->FindMenuItem(next, 1);
		}
		else
			break;
	}

	if(options->GetLength() > PAGESIZE) {
        scrollbarImg->Draw();
        arrowUpBtn->Draw();
        arrowDownBtn->Draw();
        scrollbarBoxBtn->Draw();
	}

	this->UpdateEffects();
}

void GuiOptionBrowser::TriggerUpdate()
{
	listChanged = true;
}

void GuiOptionBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;

	int next, prev;
	int position = 0;
	int positionWiimote = 0;
	int optionslength = options->GetLength();

    if(optionslength > PAGESIZE) {
        arrowUpBtn->Update(t);
        arrowDownBtn->Update(t);
        scrollbarBoxBtn->Update(t);
    }

    // move the file listing to respond to wiimote cursor movement
	if(scrollbarBoxBtn->GetState() == STATE_HELD &&
		scrollbarBoxBtn->GetStateChan() == t->chan &&
		t->wpad.ir.valid &&
		optionslength > PAGESIZE)
	{
		scrollbarBoxBtn->SetPosition(-10,0);
		positionWiimote = t->wpad.ir.y - 60 - scrollbarBoxBtn->GetTop();

		if(positionWiimote < scrollbarBoxBtn->GetMinY())
			positionWiimote = scrollbarBoxBtn->GetMinY();
		else if(positionWiimote > scrollbarBoxBtn->GetMaxY())
			positionWiimote = scrollbarBoxBtn->GetMaxY();

		listOffset = (positionWiimote * optionslength)/136.0 - selectedItem;

		if(listOffset <= 0)
		{
			listOffset = 0;
		}
		else if(listOffset+PAGESIZE >= optionslength)
		{
			listOffset= optionslength-PAGESIZE;
		}
		listChanged = true;
		focus = false;
	}

	if(arrowDownBtn->GetState() == STATE_HELD && arrowDownBtn->GetStateChan() == t->chan)
	{
		t->wpad.btns_h |= WPAD_BUTTON_DOWN;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
	}
	else if(arrowUpBtn->GetState() == STATE_HELD && arrowUpBtn->GetStateChan() == t->chan)
	{
		t->wpad.btns_h |= WPAD_BUTTON_UP;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
	}

	next = listOffset;

	if(options->IsChanged())
        listChanged = true;

	if(listChanged)
	{
		listChanged = false;
		int maxNameWidth = 0;

		for(int i=0; i<PAGESIZE; i++)
		{
			if(next >= 0)
			{
				if(optionBtn[i]->GetState() == STATE_DISABLED)
				{
					optionBtn[i]->SetVisible(true);
					optionBtn[i]->SetState(STATE_DEFAULT);
				}

				optionTxt[i]->SetText(options->GetName(next));
				optionVal[i]->SetText(options->GetValue(next));

                if(maxNameWidth < optionTxt[i]->GetTextWidth())
                    maxNameWidth = optionTxt[i]->GetTextWidth();

                if(coL2 < (24+maxNameWidth+16))
                    coL2 = 24+maxNameWidth+16;

				optionIndex[i] = next;
				next = this->FindMenuItem(next, 1);
			}
			else
			{
				optionBtn[i]->SetVisible(false);
				optionBtn[i]->SetState(STATE_DISABLED);
			}
		}

		for(int i = 0; i < PAGESIZE; i++) {
            optionVal[i]->SetPosition(coL2,0);
            optionVal[i]->SetMaxWidth(width-coL2-50,DOTTED);
		}
	}

	for(int i=0; i<PAGESIZE; i++)
	{
		if(i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED)
			optionBtn[i]->ResetState();
		else if(focus && i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT)
			optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->wpad.ir.valid && !optionBtn[i]->IsInside(t->wpad.ir.x, t->wpad.ir.y))
			t->chan = -1;

		optionBtn[i]->Update(t);
		t->chan = currChan;

		if(optionBtn[i]->GetState() == STATE_SELECTED)
			selectedItem = i;
	}

	// pad/joystick navigation
	//if(!focus)
		//return; // skip navigation

	if(t->Down() || arrowDownBtn->GetState() == STATE_CLICKED)
	{
		next = this->FindMenuItem(optionIndex[selectedItem], 1);

		if(next >= 0)
		{
			if(selectedItem == PAGESIZE-1)
			{
				// move list down by 1
				listOffset = this->FindMenuItem(listOffset, 1);
				listChanged = true;
			}
			else if(optionBtn[selectedItem+1]->IsVisible())
			{
				optionBtn[selectedItem]->ResetState();
				optionBtn[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
				selectedItem++;
			}
		}
		arrowDownBtn->ResetState();
	}
	else if(t->Up() || arrowUpBtn->GetState() == STATE_CLICKED)
	{
		prev = this->FindMenuItem(optionIndex[selectedItem], -1);

		if(prev >= 0)
		{
			if(selectedItem == 0)
			{
				// move list up by 1
				listOffset = prev;
				listChanged = true;
			}
			else
			{
				optionBtn[selectedItem]->ResetState();
				optionBtn[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
				selectedItem--;
			}
		}
		arrowUpBtn->ResetState();
	}

	// update the location of the scroll box based on the position in the file list
	if(positionWiimote > 0)
	{
		position = positionWiimote; // follow wiimote cursor
	}
	else
	{
		position = 136*(listOffset + PAGESIZE/2.0) / (optionslength*1.0);

		if(listOffset/(PAGESIZE/2.0) < 1)
			position = 0;
		else if((listOffset+PAGESIZE)/(PAGESIZE*1.0) >= (optionslength)/(PAGESIZE*1.0))
			position = 136;
	}

	scrollbarBoxBtn->SetPosition(-10,position+36);

	if(updateCB)
		updateCB(this);
}
