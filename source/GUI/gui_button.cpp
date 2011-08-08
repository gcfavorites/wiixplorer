/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_button.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui_button.h"
#include "input.h"

//! Standard inputs from the 4 WiiMotes/GCPads
extern GuiTrigger userInput[4];

/**
 * Constructor for the GuiButton class.
 */

GuiButton::GuiButton(int w, int h)
{
	width = w;
	height = h;
	Init();
}

/**
 * Destructor for the GuiButton class.
 */
GuiButton::~GuiButton()
{
}

void GuiButton::Init()
{
	image = NULL;
	imageOver = NULL;
	imageHold = NULL;
	imageClick = NULL;
	icon = NULL;
	iconOver = NULL;
	iconHold = NULL;
	iconClick = NULL;
	tooltip = NULL;

	for(int i = 0; i < 3; i++)
	{
		label[i] = NULL;
		labelOver[i] = NULL;
		labelHold[i] = NULL;
		labelClick[i] = NULL;
	}

	soundOver = NULL;
	soundHold = NULL;
	soundClick = NULL;
	selectable = true;
	holdable = false;
	clickable = true;

	for(int i = 0; i < 4; ++i)
		trigger[i] = NULL;
}

void GuiButton::SetSize(int w, int h)
{
	width = w;
	height = h;
}

void GuiButton::SetImage(GuiImage* img)
{
	image = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageOver(GuiImage* img)
{
	imageOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageHold(GuiImage* img)
{
	imageHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageClick(GuiImage* img)
{
	imageClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIcon(GuiImage* img)
{
	icon = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconOver(GuiImage* img)
{
	iconOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconHold(GuiImage* img)
{
	iconHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconClick(GuiImage* img)
{
	iconClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetLabel(GuiText* txt, int n)
{
	label[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelOver(GuiText* txt, int n)
{
	labelOver[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelHold(GuiText* txt, int n)
{
	labelHold[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelClick(GuiText* txt, int n)
{
	labelClick[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetSoundOver(GuiSound * snd)
{
	soundOver = snd;
}
void GuiButton::SetSoundHold(GuiSound * snd)
{
	soundHold = snd;
}

void GuiButton::SetSoundClick(GuiSound * snd)
{
	soundClick = snd;
}

void GuiButton::SetTrigger(GuiTrigger * t)
{
	if(!trigger[0])
		trigger[0] = t;
	else if(!trigger[1])
		trigger[1] = t;
	else if(!trigger[2])
		trigger[2] = t;
	else if(!trigger[3])
		trigger[3] = t;
	else // both were assigned, so we'll just overwrite the first one
		trigger[0] = t;
}

void GuiButton::SetTrigger(u8 i, GuiTrigger * t)
{
	trigger[i] = t;
}

void GuiButton::SetTooltip(GuiTooltip * t)
{
	tooltip = t;
	if(t)
		tooltip->SetParent(this);
}

void GuiButton::SetMinWidth(int w)
{
	if(image)
		image->SetMinWidth(w);
	if(imageOver)
		imageOver->SetMinWidth(w);
	if(imageHold)
		imageHold->SetMinWidth(w);
	if(imageClick)
		imageClick->SetMinWidth(w);
	if(icon)
		icon->SetMinWidth(w);
	if(iconOver)
		iconOver->SetMinWidth(w);
	if(iconHold)
		iconHold->SetMinWidth(w);
	if(iconClick)
		iconClick->SetMinWidth(w);
	if(tooltip)
		tooltip->SetMinWidth(w);
}

void GuiButton::SetMaxWidth(int w)
{
	if(image)
		image->SetMaxWidth(w);
	if(imageOver)
		imageOver->SetMaxWidth(w);
	if(imageHold)
		imageHold->SetMaxWidth(w);
	if(imageClick)
		imageClick->SetMaxWidth(w);
	if(icon)
		icon->SetMaxWidth(w);
	if(iconOver)
		iconOver->SetMaxWidth(w);
	if(iconHold)
		iconHold->SetMaxWidth(w);
	if(iconClick)
		iconClick->SetMaxWidth(w);
	if(tooltip)
		tooltip->SetMaxWidth(w);
}

void GuiButton::SetMinHeight(int h)
{
	if(image)
		image->SetMinHeight(h);
	if(imageOver)
		imageOver->SetMinHeight(h);
	if(imageHold)
		imageHold->SetMinHeight(h);
	if(imageClick)
		imageClick->SetMinHeight(h);
	if(icon)
		icon->SetMinHeight(h);
	if(iconOver)
		iconOver->SetMinHeight(h);
	if(iconHold)
		iconHold->SetMinHeight(h);
	if(iconClick)
		iconClick->SetMinHeight(h);
	if(tooltip)
		tooltip->SetMinHeight(h);
}

void GuiButton::SetMaxHeight(int h)
{
	if(image)
		image->SetMaxHeight(h);
	if(imageOver)
		imageOver->SetMaxHeight(h);
	if(imageHold)
		imageHold->SetMaxHeight(h);
	if(imageClick)
		imageClick->SetMaxHeight(h);
	if(icon)
		icon->SetMaxHeight(h);
	if(iconOver)
		iconOver->SetMaxHeight(h);
	if(iconHold)
		iconHold->SetMaxHeight(h);
	if(iconClick)
		iconClick->SetMaxHeight(h);
	if(tooltip)
		tooltip->SetMaxHeight(h);
}

/**
 * Draw the button on screen
 */
void GuiButton::Draw()
{
	if(!this->IsVisible())
		return;

	// draw image
	if((state == STATE_SELECTED || state == STATE_HELD) && imageOver)
		imageOver->Draw();
	else if(image)
		image->Draw();
	// draw icon
	if((state == STATE_SELECTED || state == STATE_HELD) && iconOver)
		iconOver->Draw();
	else if(icon)
		icon->Draw();
	// draw text
	for(int i=0; i<3; i++)
	{
		if((state == STATE_SELECTED || state == STATE_HELD) && labelOver[i])
			labelOver[i]->Draw();
		else if(label[i])
			label[i]->Draw();
	}

	if(state == STATE_SELECTED && tooltip)
	{
		if(!tooltip->IsVisible() && SelectTimer.elapsed() > tooltip->GetElapseTime())
		{
			tooltip->SetEffect(EFFECT_FADE, 20);
			tooltip->SetVisible(true);
		}
		if(parentElement && parentElement->GetState() != STATE_DISABLED)
			tooltip->Draw();

	}
	else if(tooltip)
	{
		if(tooltip->IsVisible())
		{
			tooltip->SetEffect(EFFECT_FADE, -20);
			tooltip->SetVisible(false);
		}
		if(state != STATE_DISABLED && parentElement && parentElement->GetState() != STATE_DISABLED)
			tooltip->Draw();
	}

	this->UpdateEffects();
}

void GuiButton::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;
	else if(parentElement && parentElement->GetState() == STATE_DISABLED)
		return;

	#ifdef HW_RVL
	// cursor
	if(t->wpad->ir.valid && t->chan >= 0)
	{
		if(this->IsInside(t->wpad->ir.x, t->wpad->ir.y))
		{
			if(state == STATE_DEFAULT || (state == STATE_SELECTED && t->chan != stateChan)) // we weren't on the button before!
			{
				this->SetState(STATE_SELECTED, t->chan);

				if(this->Rumble())
					RequestRumble(t->chan);

				if(soundOver)
					soundOver->Play();

				if(tooltip)
					SelectTimer.reset();

				if(effectsOver && !effects)
				{
					// initiate effects
					effects = effectsOver;
					effectAmount = effectAmountOver;
					effectTarget = effectTargetOver;
				}
			}
		}
		else
		{
			if(state == STATE_SELECTED && (stateChan == t->chan || stateChan == -1))
				this->ResetState();

			if(effectTarget == effectTargetOver && effectAmount == effectAmountOver)
			{
				// initiate effects (in reverse)
				effects = effectsOver;
				effectAmount = -effectAmountOver;
				effectTarget = 100;
			}
		}
	}
	#endif

	// button triggers
	if(this->IsClickable())
	{
		u32 wm_btns = t->wpad->btns_d;

		for(int i = 0; i < 4; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				if((wm_btns & trigger[i]->wpad->btns_d) || (t->pad.btns_d & trigger[i]->pad.btns_d))
				{
					if(state == STATE_SELECTED)
					{
						if(!t->wpad->ir.valid || this->IsInside(t->wpad->ir.x, t->wpad->ir.y))
						{
							if(soundClick)
								soundClick->Play();

							POINT p = {0, 0};
							if (userInput[t->chan].wpad)
							{
								if (userInput[t->chan].wpad->ir.valid)
								{
									p.x = userInput[t->chan].wpad->ir.x;
									p.y = userInput[t->chan].wpad->ir.y;
								}
							}
							Clicked(this, t->chan, p);
							return;
						}
					}
					else if(trigger[i]->type == TRIGGER_BUTTON_ONLY)
					{
						POINT p = {0, 0};
						if (userInput[t->chan].wpad)
						{
							if (userInput[t->chan].wpad->ir.valid)
							{
								p.x = userInput[t->chan].wpad->ir.x;
								p.y = userInput[t->chan].wpad->ir.y;
							}
						}
						Clicked(this, t->chan, p);
						return;
					}
				}
			}
		}
	}

	if(this->IsHoldable())
	{
		bool held = false;

		for(int i = 0; i < 4; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				if((t->wpad->btns_h & trigger[i]->wpad->btns_h) || (t->pad.btns_h & trigger[i]->pad.btns_h))
				{
					if(trigger[i]->type == TRIGGER_HELD && (state == STATE_CLICKED || state == STATE_HELD) && stateChan == t->chan)
					{
						held = true;
					}
					else if(trigger[i]->type == TRIGGER_BUTTON_ONLY_HELD)
					{
						held = true;
					}
				}

				if(held && state != STATE_HELD)
				{
					this->SetState(STATE_HELD, t->chan);
				}
				else if(held && state == STATE_HELD && stateChan == t->chan)
				{
					POINT p = {0, 0};

					if (userInput[t->chan].wpad && userInput[t->chan].wpad->ir.valid)
					{
						p.x = userInput[t->chan].wpad->ir.x;
						p.y = userInput[t->chan].wpad->ir.y;
					}
					Held(this, t->chan, PtrToControl(p));
					return;
				}
				else if(!held && state == STATE_HELD && stateChan == t->chan)
				{
					this->ResetState();
					Released(this, t->chan);
					return;
				}
			}
		}
	}
}
