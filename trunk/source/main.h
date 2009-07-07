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
struct SMBData {
	char	Host[50];
	char	User[50];
	char	Password[50];
	char	SMBName[50];
};
struct SSettings {
	int		MountMethod;
	int		CurrentUser;
	struct  SMBData SMBUser[3];
};
extern struct SSettings Settings;
extern FreeTypeGX *fontSystem;

#endif
