/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * demo.h
 ***************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "FreeTypeGX.h"

enum {
	METHOD_SD,
	METHOD_USB,
	METHOD_SMB
};

struct SSettings {
	int		MountMethod;
};
extern struct SSettings Settings;

void ExitApp();
extern int ExitRequested;
extern FreeTypeGX *fontSystem;

#endif
