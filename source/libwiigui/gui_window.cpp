/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_window.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

GuiWindow::GuiWindow()
{
	width = 0;
	height = 0;
	dim = false;
}

GuiWindow::GuiWindow(int w, int h)
{
	width = w;
	height = h;
	dim = false;
}

GuiWindow::~GuiWindow()
{
}

void GuiWindow::Append(GuiElement* e)
{
	if (e == NULL)
		return;

	Remove(e);
	_elements.push_back(e);
	e->SetParent(this);
}

void GuiWindow::Insert(GuiElement* e, u32 index)
{
	if (e == NULL || index > (_elements.size() - 1))
		return;

	Remove(e);
	_elements.insert(_elements.begin()+index, e);
	e->SetParent(this);
}

void GuiWindow::Remove(GuiElement* e)
{
	if (e == NULL)
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		if(e == _elements.at(i))
		{
			_elements.erase(_elements.begin()+i);
			break;
		}
	}
}

void GuiWindow::RemoveAll()
{
	_elements.clear();
}

void GuiWindow::SetDim(bool d)
{
    dim = d;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->SetDim(d); }
		catch (const std::exception& e) { }
	}
}
GuiElement* GuiWindow::GetGuiElementAt(u32 index) const
{
	if (index >= _elements.size())
		return NULL;
	return _elements.at(index);
}

u32 GuiWindow::GetSize()
{
	return _elements.size();
}

void GuiWindow::Draw()
{
	if(_elements.size() == 0 || !this->IsVisible())
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->Draw(); }
		catch (const std::exception& e) { }
	}

	this->UpdateEffects();

	if(parentElement && dim == true)
	{
		GXColor dimColor = (GXColor){0, 0, 0, 0x70};
		Menu_DrawRectangle(0, 0, GetZPosition(), screenwidth,screenheight, &dimColor, false, true);
	}
}

void GuiWindow::ResetState()
{
	if(state != STATE_DISABLED)
		state = STATE_DEFAULT;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->ResetState(); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetState(int s)
{
	state = s;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->SetState(s); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetVisible(bool v)
{
	visible = v;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->SetVisible(v); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMinWidth(int w)
{
	minwidth = w;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->SetMinWidth(w); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMaxWidth(int w)
{
	maxwidth = w;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->SetMaxWidth(w); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMinHeight(int h)
{
	minheight = h;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->SetMinHeight(h); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMaxHeight(int h)
{
	maxheight = h;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try { _elements.at(i)->SetMaxHeight(h); }
		catch (const std::exception& e) { }
	}
}

int GuiWindow::GetSelected()
{
	// find selected element
	int found = -1;
	for (u8 i = 0; i < _elements.size(); i++)
	{
		try
		{
			if(_elements.at(i)->GetState() == STATE_SELECTED)
			{
				found = i;
				break;
			}
		}
		catch (const std::exception& e) { }
	}
	return found;
}

void GuiWindow::Update(GuiTrigger * t)
{
	if(_elements.size() == 0 || (state == STATE_DISABLED && parentElement))
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->Update(t); }
		catch (const std::exception& e) { }
	}
}
