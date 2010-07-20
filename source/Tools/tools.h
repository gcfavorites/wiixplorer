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
#ifndef __TOOLS_H
#define __TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uncompress.h"
#include "timer.h"
#include "encrypt.h"
#include "StringTools.h"
#include "ConvertUTF.h"

#define GXCOLORTORGBA(x) ((u32) (x.r << 24 | x.g << 16 | x.b << 8 | x.a))
#define RGBATOGXCOLOR(x) ((GXColor) {(x & 0xFF000000) >> 24, (x & 0x00FF0000) >> 16, (x & 0x0000FF00) >> 8, (x & 0x000000FF)})
#define cut_bounds(x, min, max) ( (x < min) ? min : (x > max) ? max : x )
#define ALIGN(x) (x + ((4 - x % 4) % 4))

void ShowError(const char * format, ...);
void ShowMsg(const char * title, const char * format, ...);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* SVNREV_H */
