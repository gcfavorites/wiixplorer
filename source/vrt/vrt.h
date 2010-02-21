/*

Copyright (C) 2010 masterlilou derived from  Joseph Jordan <joe.ftpii@psychlaw.com.au>
This work is derived from Daniel Ehlers' <danielehlers@mindeye.net> srg_vrt branch.

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1.The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software in a
product, an acknowledgment in the product documentation would be
appreciated but is not required.

2.Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3.This notice may not be removed or altered from any source distribution.

*/
#ifndef _VRT_H_
#define _VRT_H_

#include <stdio.h>
#include <sys/dir.h>


FILE *vrt_fopen(char *cwd, char *mode);
int vrt_stat(char *cwd,  struct stat *st);
int vrt_chdir(char *cwd);
int vrt_unlink(char *cwd);
int vrt_mkdir(char *cwd, mode_t mode);
int vrt_rename(char *cwd, char *to_path);
DIR_ITER *vrt_diropen(const char *cwd);
int vrt_dirnext(DIR_ITER *iter, char *filename, struct stat *st);
int vrt_dirclose(DIR_ITER *iter);

void initialise_virtual_path(void) ;


#endif /* _VRT_H_ */
