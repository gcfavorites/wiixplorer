/****************************************************************************
 * Copyright (C) 2009
 * by Tantric
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
 * Modifications by Dimok
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef GUI_ELEMENT_H_
#define GUI_ELEMENT_H_

#define EFFECT_SLIDE_TOP			1
#define EFFECT_SLIDE_BOTTOM			2
#define EFFECT_SLIDE_RIGHT			4
#define EFFECT_SLIDE_LEFT			8
#define EFFECT_SLIDE_IN				16
#define EFFECT_SLIDE_OUT			32
#define EFFECT_SLIDE_FROM			64
#define EFFECT_FADE					128
#define EFFECT_SCALE				256
#define EFFECT_COLOR_TRANSITION		512

enum
{
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTRE,
	ALIGN_TOP,
	ALIGN_BOTTOM,
	ALIGN_MIDDLE
};

enum
{
	STATE_DEFAULT,
	STATE_SELECTED,
	STATE_CLICKED,
	STATE_HELD,
	STATE_DISABLED,
	STATE_CLOSED
};

enum
{
	WRAP,
	DOTTED,
	SCROLL_HORIZONTAL,
	SCROLL_NONE
};

typedef struct _POINT {
	s32 x;
	s32 y;
} POINT;

//!Primary GUI class. Most other classes inherit from this class.
class GuiElement : public sigslot::has_slots<>
{
	public:
		//!Constructor
		GuiElement();
		//!Destructor
		~GuiElement();
		//!Set the element's parent
		//!\param e Pointer to parent element
		void SetParent(GuiElement * e);
		//!Gets the element's parent
		//!\return Pointer to parent element
		GuiElement * GetParent() { return parentElement; };
		//!Gets the current leftmost coordinate of the element
		//!Considers horizontal alignment, x offset, width, and parent element's GetLeft() / GetWidth() values
		//!\return left coordinate
		int GetLeft();
		//!Gets the current topmost coordinate of the element
		//!Considers vertical alignment, y offset, height, and parent element's GetTop() / GetHeight() values
		//!\return top coordinate
		int GetTop();
		//!Gets the current Z coordinate of the element
		//!\return Z coordinate
		int GetZPosition();
		//!Sets the minimum y offset of the element
		//!\param y Y offset
		void SetMinY(int y);
		//!Gets the minimum y offset of the element
		//!\return Minimum Y offset
		int GetMinY() { return ymin; };
		//!Sets the maximum y offset of the element
		//!\param y Y offset
		void SetMaxY(int y);
		//!Gets the maximum y offset of the element
		//!\return Maximum Y offset
		int GetMaxY() { return ymax; };
		//!Sets the minimum x offset of the element
		//!\param x X offset
		void SetMinX(int x);
		//!Gets the minimum x offset of the element
		//!\return Minimum X offset
		int GetMinX() { return xmin; };
		//!Sets the maximum x offset of the element
		//!\param x X offset
		void SetMaxX(int x);
		//!Gets the maximum x offset of the element
		//!\return Maximum X offset
		int GetMaxX() { return xmax; };
		//!Gets the current width of the element. Does not currently consider the scale
		//!\return width
		int GetWidth() { return width; };
		//!Gets the height of the element. Does not currently consider the scale
		//!\return height
		int GetHeight() { return height; };
		//!Sets the size (width/height) of the element
		//!\param w Width of element
		//!\param h Height of element
		void SetSize(int w, int h);
		//!Checks whether or not the element is visible
		//!\return true if visible, false otherwise
		bool IsVisible() { return visible; };
		//!Checks whether or not the element is selectable
		//!\return true if selectable, false otherwise
		bool IsSelectable();
		//!Checks whether or not the element is clickable
		//!\return true if clickable, false otherwise
		bool IsClickable();
		//!Checks whether or not the element is holdable
		//!\return true if holdable, false otherwise
		bool IsHoldable();
		//!Sets whether or not the element is selectable
		//!\param s Selectable
		void SetSelectable(bool s);
		//!Sets whether or not the element is clickable
		//!\param c Clickable
		void SetClickable(bool c);
		//!Sets whether or not the element is holdable
		//!\param c Holdable
		void SetHoldable(bool d);
		//!Gets the element's current state
		//!\return state
		int GetState() { return state; };
		//!Gets the controller channel that last changed the element's state
		//!\return Channel number (0-3, -1 = no channel)
		int GetStateChan() { return stateChan; };
		//!Sets the element's alpha value
		//!\param a alpha value
		void SetAlpha(int a);
		//!Gets the element's alpha value
		//!Considers alpha, alphaDyn, and the parent element's GetAlpha() value
		//!\return alpha
		int GetAlpha();
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		void SetScale(float s);
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		void SetScaleX(float s);
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		void SetScaleY(float s);
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's GetScale() value
		float GetScale();
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's GetScale() value
		float GetScaleX();
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's GetScale() value
		float GetScaleY();
		//!Set a new GuiTrigger for the element
		//!\param t Pointer to GuiTrigger
		void SetTrigger(GuiTrigger * t);
		//!\overload
		//!\param i Index of trigger array to set
		//!\param t Pointer to GuiTrigger
		void SetTrigger(u8 i, GuiTrigger * t);
		//!Checks whether rumble was requested by the element
		//!\return true is rumble was requested, false otherwise
		bool Rumble() { return rumble; };
		//!Sets whether or not the element is requesting a rumble event
		//!\param r true if requesting rumble, false if not
		void SetRumble(bool r);
		//!Set an effect for the element
		//!\param e Effect to enable
		//!\param a Amount of the effect (usage varies on effect)
		//!\param t Target amount of the effect (usage varies on effect)
		void SetEffect(int e, int a, int t=0);
		//!Sets an effect to be enabled on wiimote cursor over
		//!\param e Effect to enable
		//!\param a Amount of the effect (usage varies on effect)
		//!\param t Target amount of the effect (usage varies on effect)
		void SetEffectOnOver(int e, int a, int t=0);
		//!Shortcut to SetEffectOnOver(EFFECT_SCALE, 4, 110)
		void SetEffectGrow();
		//!Reset all applied effects
		void ResetEffects();
		//!Gets the current element effects
		//!\return element effects
		int GetEffect() { return effects; };
		//!The Element's cutoff bounds
		virtual void SetMinWidth(int w) { minwidth = w; };
		virtual void SetMaxWidth(int w) { maxwidth = w; };
		virtual void SetMinHeight(int h) { minheight = h; };
		virtual void SetMaxHeight(int h) { maxheight = h; };
		//!Checks whether the specified coordinates are within the element's boundaries
		//!\param x X coordinate
		//!\param y Y coordinate
		//!\return true if contained within, false otherwise
		bool IsInside(int x, int y);
		//!Sets the element's position
		//!\param x X coordinate
		//!\param y Y coordinate
		void SetPosition(int x, int y);
		//!Sets the element's position
		//!\param x X coordinate
		//!\param y Y coordinate
		//!\param z Z coordinate
		void SetPosition(int x, int y, int z);
		//!Updates the element's effects (dynamic values)
		//!Called by Draw(), used for animation purposes
		void UpdateEffects();
		//!Sets the element's visibility
		//!\param v Visibility (true = visible)
		virtual void SetVisible(bool v);
		//!Sets the element's state
		//!\param s State (STATE_DEFAULT, STATE_SELECTED, STATE_CLICKED, STATE_DISABLED)
		//!\param c Controller channel (0-3, -1 = none)
		virtual void SetState(int s, int c = -1);
		//!Resets the element's state to STATE_DEFAULT
		virtual void ResetState();
		//!Gets whether or not the element is in STATE_SELECTED
		//!\return true if selected, false otherwise
		virtual int GetSelected() { return -1; };
		//!Sets the element's alignment respective to its parent element
		//!\param hor Horizontal alignment (ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTRE)
		//!\param vert Vertical alignment (ALIGN_TOP, ALIGN_BOTTOM, ALIGN_MIDDLE)
		virtual void SetAlignment(int hor, int vert);
		//!Dim the Element and its child elements
		virtual void SetDim(bool d) { };
		//!Check if element is already
		virtual bool IsDimmed() { return dim; };
		//!Called constantly to allow the element to respond to the current input data
		//!\param t Pointer to a GuiTrigger, containing the current input data from PAD/WPAD
		virtual void Update(GuiTrigger * t) { };
		//!Called constantly to redraw the element
		virtual void Draw() { };

		POINT PtrToScreen(POINT p);
		POINT PtrToControl(POINT p);

		sigslot::signal3<GuiElement *, int, POINT> Clicked;
		sigslot::signal3<GuiElement *, int, POINT> Held;
		sigslot::signal2<GuiElement *, int> Released;
		sigslot::signal2<GuiElement *, bool> VisibleChanged;
		sigslot::signal3<GuiElement *, int, int> StateChanged;
	protected:
		void Lock();
		void Unlock();
		static mutex_t mutex;
		friend class SimpleLock;

		bool visible; //!< Visibility of the element. If false, Draw() is skipped
		int width; //!< Element width
		int height; //!< Element height
		int xoffset; //!< Element X offset
		int yoffset; //!< Element Y offset
		int zoffset; //!< Element Z offset
		int ymin; //!< Element's min Y offset allowed
		int ymax; //!< Element's max Y offset allowed
		int xmin; //!< Element's min X offset allowed
		int xmax; //!< Element's max X offset allowed
		int xoffsetDyn; //!< Element X offset, dynamic (added to xoffset value for animation effects)
		int yoffsetDyn; //!< Element Y offset, dynamic (added to yoffset value for animation effects)
		int minwidth; //!< Element's minwidth cutoff limit
		int minheight; //!< Element's minheight cutoff limit
		int maxwidth; //!< Element's maxwidth cutoff limit
		int maxheight; //!< Element's maxheight cutoff limit
		int alpha; //!< Element alpha value (0-255)
		int alphaDyn; //!< Element alpha, dynamic (multiplied by alpha value for blending/fading effects)
		f32 scaleX; //!< Element scale (1 = 100%)
		f32 scaleY; //!< Element scale (1 = 100%)
		f32 scaleDyn; //!< Element scale, dynamic (multiplied by alpha value for blending/fading effects)
		bool rumble; //!< Wiimote rumble (on/off) - set to on when this element requests a rumble event
		int effects; //!< Currently enabled effect(s). 0 when no effects are enabled
		int effectAmount; //!< Effect amount. Used by different effects for different purposes
		int effectTarget; //!< Effect target amount. Used by different effects for different purposes
		int effectsOver; //!< Effects to enable when wiimote cursor is over this element. Copied to effects variable on over event
		int effectAmountOver; //!< EffectAmount to set when wiimote cursor is over this element
		int effectTargetOver; //!< EffectTarget to set when wiimote cursor is over this element
		int alignmentHor; //!< Horizontal element alignment, respective to parent element (LEFT, RIGHT, CENTRE)
		int alignmentVert; //!< Horizontal element alignment, respective to parent element (TOP, BOTTOM, MIDDLE)
		int state; //!< Element state (DEFAULT, SELECTED, CLICKED, DISABLED)
		int stateChan; //!< Which controller channel is responsible for the last change in state
		bool selectable; //!< Whether or not this element selectable (can change to SELECTED state)
		bool clickable; //!< Whether or not this element is clickable (can change to CLICKED state)
		bool holdable; //!< Whether or not this element is holdable (can change to HELD state)
        bool dim;   //! Enable/disable dim of a window only
		GuiTrigger * trigger[4]; //!< GuiTriggers (input actions) that this element responds to
		GuiElement * parentElement; //!< Parent element
};

class SimpleLock
{
    public:
        SimpleLock(GuiElement *e);
        ~SimpleLock();
    private:
        GuiElement *element;
};

#define LOCK(e) SimpleLock MutexLock(e)

#endif
