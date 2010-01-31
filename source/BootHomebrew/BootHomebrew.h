#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

int BootHomebrew(const char * path, const char * filereference = NULL);
int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len);
void FreeHomebrewBuffer();
int LoadHomebrew(const char * filepath);

#endif
