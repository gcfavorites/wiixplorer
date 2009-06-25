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
bool CreateSubfolder(char * fullpath);
bool CheckFile(char * filepath);
int CopyFile(const char * src, const char * dest);
int CopyDirectory(char * src, char * dest);
bool RemoveDirectory(char * dirpath);
bool RemoveFile(char * filepath);

#endif
