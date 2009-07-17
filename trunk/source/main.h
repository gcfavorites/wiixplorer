/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * demo.h
 ***************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "FreeTypeGX.h"
#include "Settings.h"

enum {
	METHOD_SD,
	METHOD_USB,
	METHOD_SMB
};

extern Settings Settings;
extern FreeTypeGX *fontSystem;

#endif
