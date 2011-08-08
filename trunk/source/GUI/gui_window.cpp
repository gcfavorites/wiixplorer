/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_window.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui_window.h"
#include "Controls/Application.h"
#include "VideoOperations/video.h"

GuiWindow::GuiWindow(GuiWindow *p)
{
	parent = p;
	width = 0;
	height = 0;
	firstFrame = true;
	dim = false;
	if(parent) parent->Append(this);
}

GuiWindow::GuiWindow(int w, int h, GuiWindow *p)
{
	parent = p;
	width = w;
	height = h;
	firstFrame = true;
	dim = false;
	if(parent)
		parent->Append(this);
}

GuiWindow::~GuiWindow()
{
	Closing(this);

	if(parent)
		parent->Remove(this);
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

	for (u32 i = 0; i < _elements.size(); ++i)
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

void GuiWindow::Close()
{
	Application::Instance()->PushForDelete(this);
}

void GuiWindow::DimBackground(bool d)
{
	dim = d;
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

void GuiWindow::ResetState()
{
	if(state != STATE_DISABLED)
		state = STATE_DEFAULT;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try { _elements.at(i)->ResetState(); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetState(int s)
{
	state = s;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try { _elements.at(i)->SetState(s); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetVisible(bool v)
{
	visible = v;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try { _elements.at(i)->SetVisible(v); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMinWidth(int w)
{
	minwidth = w;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try { _elements.at(i)->SetMinWidth(w); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMaxWidth(int w)
{
	maxwidth = w;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try { _elements.at(i)->SetMaxWidth(w); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMinHeight(int h)
{
	minheight = h;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try { _elements.at(i)->SetMinHeight(h); }
		catch (const std::exception& e) { }
	}
}

void GuiWindow::SetMaxHeight(int h)
{
	maxheight = h;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try { _elements.at(i)->SetMaxHeight(h); }
		catch (const std::exception& e) { }
	}
}

int GuiWindow::GetSelected()
{
	// find selected element
	int found = -1;
	for (u32 i = 0; i < _elements.size(); ++i)
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

void GuiWindow::Draw()
{
	if(firstFrame)
	{
		firstFrame = false;
		return;
	}

	if(_elements.size() == 0 || !this->IsVisible())
		return;

	if(parentElement && dim == true)
	{
		GXColor dimColor = (GXColor){0, 0, 0, 0x70};
		Menu_DrawRectangle(0, 0, GetZPosition(), screenwidth,screenheight, &dimColor, false, true);
	}

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try	{ _elements.at(i)->Draw(); }
		catch (const std::exception& e) { }
	}

	this->UpdateEffects();
}

void GuiWindow::Update(GuiTrigger * t)
{
	if(firstFrame || (state == STATE_DISABLED && parentElement))
		return;

	for (u32 i = 0; i < _elements.size(); ++i)
	{
		try	{ _elements.at(i)->Update(t); }
		catch (const std::exception& e) { }
	}
}
