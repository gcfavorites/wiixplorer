/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_button.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
/**
 * Constructor for the GuiButton class.
 */

GuiButton::GuiButton(int w, int h)
{
	width = w;
	height = h;
	image = NULL;
	imageOver = NULL;
	imageHold = NULL;
	imageClick = NULL;
	icon = NULL;
	iconOver = NULL;
	iconHold = NULL;
	iconClick = NULL;
	tooltip = NULL;

	for(int i=0; i < 3; i++)
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
}

/**
 * Destructor for the GuiButton class.
 */
GuiButton::~GuiButton()
{
}

void GuiButton::SetSize(int w, int h)
{
	width = w;
	height = h;
}

void GuiButton::SetImage(GuiImage* img)
{
    LOCK(this);
	image = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageOver(GuiImage* img)
{
    LOCK(this);
	imageOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageHold(GuiImage* img)
{
    LOCK(this);
	imageHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageClick(GuiImage* img)
{
    LOCK(this);
	imageClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIcon(GuiImage* img)
{
    LOCK(this);
	icon = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconOver(GuiImage* img)
{
    LOCK(this);
	iconOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconHold(GuiImage* img)
{
    LOCK(this);
	iconHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconClick(GuiImage* img)
{
    LOCK(this);
	iconClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetLabel(GuiText* txt, int n)
{
    LOCK(this);
	label[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelOver(GuiText* txt, int n)
{
    LOCK(this);
	labelOver[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelHold(GuiText* txt, int n)
{
    LOCK(this);
	labelHold[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelClick(GuiText* txt, int n)
{
    LOCK(this);
	labelClick[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetSoundOver(GuiSound * snd)
{
    LOCK(this);
	soundOver = snd;
}
void GuiButton::SetSoundHold(GuiSound * snd)
{
    LOCK(this);
	soundHold = snd;
}

void GuiButton::SetSoundClick(GuiSound * snd)
{
    LOCK(this);
	soundClick = snd;
}

void GuiButton::SetTooltip(GuiTooltip * t)
{
    LOCK(this);
    tooltip = t;
	if(t)
		tooltip->SetParent(this);
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

	}
	else if(tooltip)
	{
	    if(tooltip->IsVisible())
	    {
            tooltip->SetEffect(EFFECT_FADE, -20);
            tooltip->SetVisible(false);
	    }
	}

	this->UpdateEffects();
}

void GuiButton::Update(GuiTrigger * t)
{
	if(state == STATE_CLICKED || state == STATE_DISABLED || !t)
		return;
	else if(parentElement && parentElement->GetState() == STATE_DISABLED)
		return;

    LOCK(this);

	#ifdef HW_RVL
	// cursor
	if(t->wpad->ir.valid && t->chan >= 0)
	{
		if(this->IsInside(t->wpad->ir.x, t->wpad->ir.y))
		{
			if(state == STATE_DEFAULT) // we weren't on the button before!
			{
				this->SetState(STATE_SELECTED, t->chan);

				if(this->Rumble())
					rumbleRequest[t->chan] = 1;

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
		s32 wm_btns, wm_btns_trig, cc_btns, cc_btns_trig;
		for(int i=0; i<2; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				// higher 16 bits only (wiimote)
				wm_btns = t->wpad->btns_d << 16;
				wm_btns_trig = trigger[i]->wpad->btns_d << 16;

				// lower 16 bits only (classic controller)
				cc_btns = t->wpad->btns_d >> 16;
				cc_btns_trig = trigger[i]->wpad->btns_d >> 16;

				if(
					(t->wpad->btns_d > 0 &&
					(wm_btns == wm_btns_trig ||
					(cc_btns == cc_btns_trig && t->wpad->exp.type == EXP_CLASSIC))) ||
					(t->pad.btns_d == trigger[i]->pad.btns_d && t->pad.btns_d > 0))
				{
					if(t->chan == stateChan || stateChan == -1)
					{
						if(state == STATE_SELECTED)
						{
							if(!t->wpad->ir.valid ||	this->IsInside(t->wpad->ir.x, t->wpad->ir.y))
							{
								this->SetState(STATE_CLICKED, t->chan);

								if(soundClick)
									soundClick->Play();
							}
						}
						else if(trigger[i]->type == TRIGGER_BUTTON_ONLY)
						{
							this->SetState(STATE_CLICKED, t->chan);
						}
					}
				}
			}
		}
	}

	if(this->IsHoldable())
	{
		bool held = false;
		s32 wm_btns, wm_btns_h, wm_btns_trig, cc_btns, cc_btns_h, cc_btns_trig;

		for(int i=0; i<2; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				// higher 16 bits only (wiimote)
				wm_btns = t->wpad->btns_d << 16;
				wm_btns_h = t->wpad->btns_h << 16;
				wm_btns_trig = trigger[i]->wpad->btns_h << 16;

				// lower 16 bits only (classic controller)
				cc_btns = t->wpad->btns_d >> 16;
				cc_btns_h = t->wpad->btns_h >> 16;
				cc_btns_trig = trigger[i]->wpad->btns_h >> 16;

				if(
					(t->wpad->btns_d > 0 &&
					(wm_btns == wm_btns_trig ||
					(cc_btns == cc_btns_trig && t->wpad->exp.type == EXP_CLASSIC))) ||
					(t->pad.btns_d == trigger[i]->pad.btns_h && t->pad.btns_d > 0))
				{
					if(trigger[i]->type == TRIGGER_HELD && state == STATE_SELECTED &&
						(t->chan == stateChan || stateChan == -1))
						this->SetState(STATE_CLICKED, t->chan);
				}

				if(
					(t->wpad->btns_h > 0 &&
					(wm_btns_h == wm_btns_trig ||
					(cc_btns_h == cc_btns_trig && t->wpad->exp.type == EXP_CLASSIC))) ||
					(t->pad.btns_h == trigger[i]->pad.btns_h && t->pad.btns_h > 0))
				{
					if(trigger[i]->type == TRIGGER_HELD)
						held = true;
				}

				if(!held && state == STATE_HELD && stateChan == t->chan)
				{
					this->ResetState();
				}
				else if(held && state == STATE_CLICKED && stateChan == t->chan)
				{
					this->SetState(STATE_HELD, t->chan);
				}
				else if(held && state == STATE_HELD && stateChan == t->chan)
				{
                    POINT p = {0, 0};

                    if (userInput[stateChan].wpad)
                    {
                        if (userInput[stateChan].wpad->ir.valid)
                        {
                            p.x = userInput[stateChan].wpad->ir.x;
                            p.y = userInput[stateChan].wpad->ir.y;
                        }
                    }
                    Held(this, stateChan, PtrToControl(p));
				}
			}
		}
	}
}
