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
 * Clipboard.h
 *
 * for Wii-FileXplorer 2010
 ***************************************************************************/
#ifndef _CLIPBOARD_H_
#define _CLIPBOARD_H_

#include "libwiigui/gui.h"
#include "FileOperations/filebrowser.h"
#include "FileOperations/ItemMarker.h"

class Clipboard : public ItemMarker
{
    public:
		static Clipboard * Instance();
        static void DestroyInstance();

        bool Cutted;
    private:
        Clipboard();
        ~Clipboard();
		static Clipboard *instance;

};

#endif
