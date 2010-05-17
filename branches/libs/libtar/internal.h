/*
**  Copyright 2002-2003 University of Illinois Board of Trustees
**  Copyright 2002-2003 Mark D. Roth
**  All rights reserved.
**
**  internal.h - internal header file for libtar
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/
#ifndef _INTERNAL_TAR_H_
#define _INTERNAL_TAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <string.h>
#include "compat.h"
#include "libtar.h"


char * dirname(const char *path);
void strmode(mode_t mode, char * p);
char * basename(const char *path);

#endif
