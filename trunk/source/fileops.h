#ifndef _FILEOPS_H_
#define _FILEOPS_H_

bool findfile(const char * filename, const char * path);
bool subfoldercreate(char * fullpath);
bool checkfile(char * path);
int CopyFile(const char * src, const char * dest);
int CopyDirectory(char * src, char * dest);

#endif
