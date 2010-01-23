#ifndef _NETWORKOPS_H_
#define _NETWORKOPS_H_

int DownloadFileToMem(const char *url, u8 **inbuffer, u32 *size);
int DownloadFileToPath(const char *url, const char *dest);
bool ConnectSMBShare();
void SMB_Reconnect();
void CloseSMBShare();
bool IsSMB_Mounted(int smb);
void Initialize_Network(void);
void DeInit_Network(void);
bool IsNetworkInit(void);
bool IsNetworkInitiating(void);
char * GetNetworkIP(void);
void HaltNetworkThread();
void ResumeNetworkThread();
void InitNetworkThread();
void ShutdownNetworkThread();

#endif
