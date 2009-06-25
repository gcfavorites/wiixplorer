/****************************************************************************
 * fileops.h
 * by dimok
 *
 * File operations for the WiiXplorer
 * Handling all the needed file operations
 ***************************************************************************/

#ifndef _FILEOPS_H_
#define _FILEOPS_H_

bool FindFile(const char * filename, const char * path);
bool CreateSubfolder(const char * fullpath);
bool CheckFile(const char * filepath);
int CopyFile(const char * src, const char * dest);
int CopyDirectory(const char * src, const char * dest);
int RemoveDirectory(char * dirpath);
bool RemoveFile(char * filepath);

#endif
