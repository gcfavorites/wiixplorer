/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_trigger.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "input.h"
#include "gui_element.h"
#include "gui_trigger.h"
#include "Settings.h"

#define SCROLL_INITIAL_DELAY 	20
#define SCROLL_LOOP_DELAY 		3

static int scrollDelay = 0;

/**
 * Constructor for the GuiTrigger class.
 */
GuiTrigger::GuiTrigger()
{
	chan = -1;
	memset(&wpaddata, 0, sizeof(WPADData));
	memset(&pad, 0, sizeof(PADData));
	wpad = &wpaddata;
}

/**
 * Destructor for the GuiTrigger class.
 */
GuiTrigger::~GuiTrigger()
{
}

/**
 * Sets a simple trigger. Requires:
 * - Element is selected
 * - Trigger button is pressed
 */
void GuiTrigger::SetSimpleTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_SIMPLE;
	chan = ch;
	wpaddata.btns_d = wiibtns;
	pad.btns_d = gcbtns;
}

/**
 * Sets a held trigger. Requires:
 * - Element is selected
 * - Trigger button is pressed and held
 */
void GuiTrigger::SetHeldTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_HELD;
	chan = ch;
	wpaddata.btns_d = wiibtns;
	wpaddata.btns_h = wiibtns;
	pad.btns_d = gcbtns;
	pad.btns_h = gcbtns;
}

/**
 * Sets a simple held trigger.
 */
void GuiTrigger::SetButtonOnlyHeldTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_BUTTON_ONLY_HELD;
	chan = ch;
	wpaddata.btns_d = wiibtns;
	wpaddata.btns_h = wiibtns;
	pad.btns_d = gcbtns;
	pad.btns_h = gcbtns;
}

/**
 * Sets a button trigger. Requires:
 * - Trigger button is pressed
 */
void GuiTrigger::SetButtonOnlyTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_BUTTON_ONLY;
	chan = ch;
	wpaddata.btns_d = wiibtns;
	pad.btns_d = gcbtns;
}

/****************************************************************************
 * WPAD_Stick
 *
 * Get X/Y value from Wii Joystick (classic, nunchuk) input
 ***************************************************************************/

s8 GuiTrigger::WPAD_Stick(u8 right, int axis)
{
	float mag = 0.0;
	float ang = 0.0;

	switch (wpad->exp.type)
	{
		case WPAD_EXP_NUNCHUK:
		case WPAD_EXP_GUITARHERO3:
			if (right == 0)
			{
				mag = wpad->exp.nunchuk.js.mag;
				ang = wpad->exp.nunchuk.js.ang;
			}
			break;

		case WPAD_EXP_CLASSIC:
			if (right == 0)
			{
				mag = wpad->exp.classic.ljs.mag;
				ang = wpad->exp.classic.ljs.ang;
			}
			else
			{
				mag = wpad->exp.classic.rjs.mag;
				ang = wpad->exp.classic.rjs.ang;
			}
			break;

		default:
			break;
	}

	/* calculate x/y value (angle need to be converted into radian) */
	if (mag > 1.0) mag = 1.0;
	else if (mag < -1.0) mag = -1.0;
	double val;

	if(axis == 0) // x-axis
		val = mag * sin((PI * ang)/180.0f);
	else // y-axis
		val = mag * cos((PI * ang)/180.0f);

	return (s8)(val * 128.0f);
}

bool GuiTrigger::Left()
{
	u32 wiibtn = WiiControls.LeftButton;

	if(((wpad->btns_d | wpad->btns_h) & (wiibtn | (ClassicControls.LeftButton << 16)))
			|| ((pad.btns_d | pad.btns_h) & GCControls.LeftButton)
			|| pad.stickX < -PADCAL
			|| WPAD_Stick(0,0) < -PADCAL)
	{
		if((wpad->btns_d & (wiibtn | (ClassicControls.LeftButton << 16)))
			|| (pad.btns_d & GCControls.LeftButton))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Right()
{
	u32 wiibtn = WiiControls.RightButton;

	if(((wpad->btns_d | wpad->btns_h) & (wiibtn | (ClassicControls.RightButton << 16)))
			|| ((pad.btns_d | pad.btns_h) & GCControls.RightButton)
			|| pad.stickX > PADCAL
			|| WPAD_Stick(0,0) > PADCAL)
	{
		if((wpad->btns_d & (wiibtn | (ClassicControls.RightButton << 16)))
			|| (pad.btns_d & GCControls.RightButton))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Up()
{
	u32 wiibtn = WiiControls.UpButton;

	if(((wpad->btns_d | wpad->btns_h) & (wiibtn | (ClassicControls.UpButton << 16)))
			|| ((pad.btns_d | pad.btns_h) & GCControls.UpButton)
			|| pad.stickY > PADCAL
			|| WPAD_Stick(0,1) > PADCAL)
	{
		if((wpad->btns_d & (wiibtn | (ClassicControls.UpButton << 16)))
			|| (pad.btns_d & GCControls.UpButton))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Down()
{
	u32 wiibtn = WiiControls.DownButton;

	if(((wpad->btns_d | wpad->btns_h) & (wiibtn | (ClassicControls.DownButton << 16)))
			|| ((pad.btns_d | pad.btns_h) & GCControls.DownButton)
			|| pad.stickY < -PADCAL
			|| WPAD_Stick(0,1) < -PADCAL)
	{
		if((wpad->btns_d & (wiibtn | (ClassicControls.DownButton << 16)))
			|| (pad.btns_d & GCControls.DownButton))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}
