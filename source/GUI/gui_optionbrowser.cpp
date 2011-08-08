/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_optionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui_optionbrowser.h"
#include "Memory/Resources.h"
#include "menu.h"

#define BROWSERSIZE		8

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

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);

	btnSoundClick = Resources::GetSound("button_click.wav");

	bgOptions = Resources::GetImageData("bg_browser.png");
	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgOptionsEntry = Resources::GetImageData("bg_browser_selection.png");

	scrollbar = new Scrollbar(245);
	scrollbar->SetParent(this);
	scrollbar->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbar->SetPosition(-10, 5);
	scrollbar->SetScrollSpeed(Settings.ScrollSpeed);
	scrollbar->listChanged.connect(this, &GuiOptionBrowser::OnListChange);
	scrollbar->SetButtonScroll(WiiControls.OneButtonScroll | ClassicControls.OneButtonScroll << 16);

	for(int i=0; i<PAGESIZE; i++)
	{
		optionTxt[i] = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(15,0);

		optionVal[i] = new GuiText((char*) NULL, 20, (GXColor){0, 0, 0, 0xff});
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
		optionBtn[i]->StateChanged.connect(this, &GuiOptionBrowser::OnStateChange);
	}
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
	delete scrollbar;

	delete bgOptionsImg;

	Resources::Remove(bgOptions);
	Resources::Remove(bgOptionsEntry);
	Resources::Remove(btnSoundClick);

	delete trigA;

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

void GuiOptionBrowser::OnStateChange(GuiElement *sender, int s, int chan UNUSED)
{
	for(int i = 0; i < PAGESIZE; i++)
	{
		if(sender == optionBtn[i])
		{
			if(s == STATE_SELECTED)
			{
				optionVal[i]->SetMaxWidth(width-coL2-80, SCROLL_HORIZONTAL);
			}
			break;
		}
	}
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

	scrollbar->Draw();

	this->UpdateEffects();
}

void GuiOptionBrowser::TriggerUpdate()
{
	listChanged = true;
}

void GuiOptionBrowser::OnListChange(int selItem, int selIndex)
{
	selectedItem = selItem;
	listOffset = selIndex;
	int maxNameWidth = 0;
	int next = listOffset;

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

	for(int i = 0; i < PAGESIZE; i++)
	{
		optionVal[i]->SetPosition(coL2,0);
		optionVal[i]->SetMaxWidth(width-coL2-50, DOTTED);
	}

}

void GuiOptionBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;

	scrollbar->Update(t);

	if(options->IsChanged())
		OnListChange(selectedItem, listOffset);

	for(int i=0; i<PAGESIZE; i++)
	{
		if(i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED)
		{
			optionBtn[i]->ResetState();
			optionVal[i]->SetMaxWidth(width-coL2-50, DOTTED);
		}
		else if(i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT)
		{
			optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);
			optionVal[selectedItem]->SetMaxWidth(width-coL2-80, SCROLL_HORIZONTAL);
		}

		int currChan = t->chan;

		if(t->wpad->ir.valid && !optionBtn[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
			t->chan = -1;

		optionBtn[i]->Update(t);
		t->chan = currChan;

		if(optionBtn[i]->GetState() == STATE_SELECTED)
			selectedItem = i;
	}

	scrollbar->SetEntrieCount(options->GetLength());
	scrollbar->SetPageSize(PAGESIZE);
	scrollbar->SetRowSize(0);
	scrollbar->SetSelectedItem(selectedItem);
	scrollbar->SetSelectedIndex(listOffset);
}
