#ifndef _LISTFILES_H_
#define _LISTFILES_H_

bool findfile(const char * filename, const char * path);
char * GetFileName(int i);
int GetAllDirFiles(char * filespath);
bool subfoldercreate(char * fullpath);
bool checkfile(char * path);

#endif
