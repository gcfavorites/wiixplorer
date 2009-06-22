#ifndef _WPAD_H_
#define _WPAD_H_

#include <wiiuse/wpad.h>

s32  Wpad_Init(void);
void Wpad_Disconnect(void);
u32  ButtonsPressed(void);
u32  ButtonsHold(void);
bool IsWpadConnected();

#endif
