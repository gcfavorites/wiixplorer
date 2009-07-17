#ifndef _NETWORKOPS_H_
#define _NETWORKOPS_H_

bool ConnectSMBShare();
void SMB_Reconnect();
void CloseSMBShare();
void Initialize_Network(void);
bool IsNetworkInit(void);
bool IsNetworkInitiating(void);
char * GetNetworkIP(void);
void HaltNetworkThread();
void ResumeNetworkThread();
void InitNetworkThread();
void ShutdownNetworkThread();

#endif
