/****************************************************************************
 * fileops.h
 * by dimok
 *
 * File operations for the WiiXplorer
 * Handling all the needed file operations
 ***************************************************************************/

#ifndef _FILEOPS_H_
#define _FILEOPS_H_

bool CreateSubfolder(const char * fullpath);
bool FindFile(const char * filename, const char * path);
bool CheckFile(const char * filepath);
u64 FileSize(const char * filepath);
int LoadFileToMem(const char * filepath, u8 **buffer, u64 *size);
int LoadFileToMemWithProgress(const char *progressText, const char *filePath, u8 **buffer, u64 *size);
int CopyFile(const char * src, const char * dest);
int CopyDirectory(const char * src, const char * dest);
int MoveDirectory(char * src, const char * dest);
int RemoveDirectory(char * dirpath);
bool RemoveFile(char * filepath);
void GetFolderSize(const char * folderpath, u64 &foldersize, u32 &filenumber);

#endif
