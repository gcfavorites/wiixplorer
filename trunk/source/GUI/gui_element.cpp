/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_element.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui_element.h"

/**
 * Constructor for the Object class.
 */
GuiElement::GuiElement()
{
	xoffset = 0;
	yoffset = 0;
	zoffset = 0;
	xmin = 0;
	xmax = 0;
	ymin = 0;
	ymax = 0;
	width = 0;
	height = 0;
	alpha = 255;
	scaleX = 1.0f;
	scaleY = 1.0f;
	minwidth = -100;
	maxwidth = (screenwidth+100);
	minheight = -100;
	maxheight = (screenheight+100);
	state = STATE_DEFAULT;
	stateChan = -1;
	parentElement = NULL;
	rumble = true;
	selectable = false;
	clickable = false;
	holdable = false;
	visible = true;
	yoffsetDyn = 0;
	xoffsetDyn = 0;
	alphaDyn = -1;
	scaleDyn = 1;
	effects = 0;
	effectAmount = 0;
	effectTarget = 0;
	effectsOver = 0;
	effectAmountOver = 0;
	effectTargetOver = 0;

	// default alignment - align to top left
	alignmentVert = ALIGN_TOP;
	alignmentHor = ALIGN_LEFT;
}

/**
 * Destructor for the GuiElement class.
 */
GuiElement::~GuiElement()
{
}

void GuiElement::SetParent(GuiElement * e)
{
	parentElement = e;
}

/**
 * Get the left position of the GuiElement.
 * @see SetLeft()
 * @return Left position in pixel.
 */
int GuiElement::GetLeft()
{
	int x = 0;
	int pWidth = 0;
	int pLeft = 0;

	if(parentElement)
	{
		pWidth = parentElement->GetWidth();
		pLeft = parentElement->GetLeft();
	}

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT))
		pLeft += xoffsetDyn;

	switch(alignmentHor)
	{
		case ALIGN_LEFT:
			x = pLeft;
			break;
		case ALIGN_CENTRE:
			x = pLeft + (pWidth/2) - (width/2);
			break;
		case ALIGN_RIGHT:
			x = pLeft + pWidth - width;
			break;
	}
	return x + xoffset;
}

/**
 * Get the top position of the GuiElement.
 * @see SetTop()
 * @return Top position in pixel.
 */
int GuiElement::GetTop()
{
	int y = 0;
	int pHeight = 0;
	int pTop = 0;

	if(parentElement)
	{
		pHeight = parentElement->GetHeight();
		pTop = parentElement->GetTop();
	}

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT))
		pTop += yoffsetDyn;

	switch(alignmentVert)
	{
		case ALIGN_TOP:
			y = pTop;
			break;
		case ALIGN_MIDDLE:
			y = pTop + (pHeight/2) - (height/2);
			break;
		case ALIGN_BOTTOM:
			y = pTop + pHeight - height;
			break;
	}
	return y + yoffset;
}

void GuiElement::SetMinX(int x)
{
	xmin = x;
}

void GuiElement::SetMaxX(int x)
{
	xmax = x;
}

void GuiElement::SetMinY(int y)
{
	ymin = y;
}

void GuiElement::SetMaxY(int y)
{
	ymax = y;
}


/**
 * Set the width and height of the GuiElement.
 * @param[in] Width Width in pixel.
 * @param[in] Height Height in pixel.
 * @see SetWidth()
 * @see SetHeight()
 */
void GuiElement::SetSize(int w, int h)
{
	width = w;
	height = h;
}

/**
 * Set visible.
 * @param[in] Visible Set to true to show GuiElement.
 * @see IsVisible()
 */
void GuiElement::SetVisible(bool v)
{
	visible = v;
	VisibleChanged(this, v);
}

void GuiElement::SetAlpha(int a)
{
	alpha = a;
}

int GuiElement::GetAlpha()
{
	int a;

	if(alphaDyn >= 0)
		a = alphaDyn;
	else
		a = alpha;

	if(parentElement)
		a *= parentElement->GetAlpha()/255.0;

	return a;
}

void GuiElement::SetScale(float s)
{
	scaleX = s;
	scaleY = s;
}

void GuiElement::SetScaleX(float s)
{
	scaleX = s;
}

void GuiElement::SetScaleY(float s)
{
	scaleY = s;
}

float GuiElement::GetScale()
{
	float s = (scaleX+scaleY)/2 * scaleDyn;

	if(parentElement)
		s *= parentElement->GetScale();

	return s;
}

float GuiElement::GetScaleX()
{
	float s = scaleX * scaleDyn;

	if(parentElement)
		s *= parentElement->GetScaleX();

	return s;
}

float GuiElement::GetScaleY()
{
	float s = scaleY * scaleDyn;

	if(parentElement)
		s *= parentElement->GetScaleY();

	return s;
}

void GuiElement::SetState(int s, int c)
{
	state = s;
	stateChan = c;
	StateChanged(this, s, c);
}

void GuiElement::ResetState()
{
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}
}

void GuiElement::SetClickable(bool c)
{
	clickable = c;
}

void GuiElement::SetSelectable(bool s)
{
	selectable = s;
}

void GuiElement::SetHoldable(bool d)
{
	holdable = d;
}

bool GuiElement::IsSelectable()
{
	if(state == STATE_DISABLED || state == STATE_CLICKED)
		return false;
	else
		return selectable;
}

bool GuiElement::IsClickable()
{
	if(state == STATE_DISABLED ||
		state == STATE_CLICKED ||
		state == STATE_HELD)
		return false;
	else
		return clickable;
}

bool GuiElement::IsHoldable()
{
	if(state == STATE_DISABLED)
		return false;
	else
		return holdable;
}

void GuiElement::SetRumble(bool r)
{
	rumble = r;
}

void GuiElement::SetEffect(int eff, int amount, int target)
{
	if(eff & EFFECT_SLIDE_IN)
	{
		// these calculations overcompensate a little
		if(eff & EFFECT_SLIDE_TOP)
		{
			if(eff & EFFECT_SLIDE_FROM)
				yoffsetDyn = (int) -GetHeight()*scaleY;
			else
				yoffsetDyn = -screenheight;
		}
		else if(eff & EFFECT_SLIDE_LEFT)
		{
			if(eff & EFFECT_SLIDE_FROM)
				xoffsetDyn = (int) -GetWidth()*scaleX;
			else
				xoffsetDyn = -screenwidth;
		}
		else if(eff & EFFECT_SLIDE_BOTTOM)
		{
			if(eff & EFFECT_SLIDE_FROM)
				yoffsetDyn = (int) GetHeight()*scaleY;
			else
				yoffsetDyn = screenheight;
		}
		else if(eff & EFFECT_SLIDE_RIGHT)
		{
			if(eff & EFFECT_SLIDE_FROM)
				xoffsetDyn = (int) GetWidth()*scaleX;
			else
				xoffsetDyn = screenwidth;
		}
	}
	if((eff & EFFECT_FADE) && amount > 0)
	{
		alphaDyn = 0;
	}
	else if((eff & EFFECT_FADE) && amount < 0)
	{
		alphaDyn = alpha;
	}

	effects |= eff;
	effectAmount = amount;
	effectTarget = target;
}

void GuiElement::SetEffectOnOver(int eff, int amount, int target)
{
	effectsOver |= eff;
	effectAmountOver = amount;
	effectTargetOver = target;
}

void GuiElement::SetEffectGrow()
{
	SetEffectOnOver(EFFECT_SCALE, 4, 110);
}

void GuiElement::ResetEffects()
{
	yoffsetDyn = 0;
	xoffsetDyn = 0;
	alphaDyn = -1;
	scaleDyn = 1;
	effects = 0;
	effectAmount = 0;
	effectTarget = 0;
	effectsOver = 0;
	effectAmountOver = 0;
	effectTargetOver = 0;
}

void GuiElement::UpdateEffects()
{
	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_SLIDE_FROM))
	{
		if(effects & EFFECT_SLIDE_IN)
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
		}
		else
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= -screenwidth)
					effects = 0; // shut off effect
				else if((effects & EFFECT_SLIDE_FROM) && xoffsetDyn <= -GetWidth())
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= screenwidth)
					effects = 0; // shut off effect
				else if((effects & EFFECT_SLIDE_FROM) && xoffsetDyn >= GetWidth()*scaleX)
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= -screenheight)
					effects = 0; // shut off effect
				else if((effects & EFFECT_SLIDE_FROM) && yoffsetDyn <= -GetHeight())
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= screenheight)
					effects = 0; // shut off effect
				else if((effects & EFFECT_SLIDE_FROM) && yoffsetDyn >= GetHeight())
					effects = 0; // shut off effect
			}
		}
	}
	if(effects & EFFECT_FADE)
	{
		alphaDyn += effectAmount;

		if(effectAmount < 0 && alphaDyn <= 0)
		{
			alphaDyn = 0;
			effects = 0; // shut off effect
		}
		else if(effectAmount > 0 && alphaDyn >= alpha)
		{
			alphaDyn = alpha;
			effects = 0; // shut off effect
		}
	}
	if(effects & EFFECT_SCALE)
	{
		scaleDyn += effectAmount/100.0;

		if((effectAmount < 0 && scaleDyn <= effectTarget/100.0)
			|| (effectAmount > 0 && scaleDyn >= effectTarget/100.0))
		{
			scaleDyn = effectTarget/100.0;
			effects = 0; // shut off effect
		}
	}
}

void GuiElement::SetPosition(int xoff, int yoff)
{
	xoffset = xoff;
	yoffset = yoff;
}

void GuiElement::SetPosition(int xoff, int yoff, int zoff)
{
	xoffset = xoff;
	yoffset = yoff;
	zoffset = zoff;
}

void GuiElement::SetAlignment(int hor, int vert)
{
	alignmentHor = hor;
	alignmentVert = vert;
}

int GuiElement::GetZPosition()
{
	int zParent = 0;

	if(parentElement)
		zParent = parentElement->GetZPosition();

	return zParent+zoffset;
}

/**
 * Check if a position is inside the GuiElement.
 * @param[in] x X position in pixel.
 * @param[in] y Y position in pixel.
 */
bool GuiElement::IsInside(int x, int y)
{
	if(x > this->GetLeft() && x < (this->GetLeft()+width)
	&& y > this->GetTop() && y < (this->GetTop()+height))
		return true;
	return false;
}

POINT GuiElement::PtrToScreen(POINT p)
{
	POINT r = { p.x + GetLeft(), p.y + GetTop() };
	return r;
}

POINT GuiElement::PtrToControl(POINT p)
{
	POINT r = { p.x - GetLeft(), p.y - GetTop() };
	return r;
}

