/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "Controls/Application.h"
#include "Prompts/PromptWindows.h"

extern "C" void ShowError(const char * format, ...)
{
	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		WindowPrompt(tr("Error:"), tmp, tr("OK"));
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

extern "C" void ShowMsg(const char * title, const char * format, ...)
{
	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		WindowPrompt(title, tmp, tr("OK"));
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

extern "C" void ThrowMsg(const char * title, const char * format, ...)
{
	if(!title && !format)
	{
		return;
	}
	else if(title && !format)
	{
		PromptWindow *prompt = new PromptWindow(title, 0, tr("OK"));
		prompt->DimBackground(true);
		prompt->SetAutoClose(true);
		Application::Instance()->Append(prompt);
		Application::Instance()->SetUpdateOnly(prompt);
		return;
	}

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		PromptWindow *prompt = new PromptWindow(title, tmp, tr("OK"));
		prompt->DimBackground(true);
		prompt->SetAutoClose(true);
		Application::Instance()->Append(prompt);
		Application::Instance()->SetUpdateOnly(prompt);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}
