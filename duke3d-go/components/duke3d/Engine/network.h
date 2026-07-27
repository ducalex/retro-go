//
//  network.h
//  Duke3D
//
//  Created by fabien sanglard on 12-12-22.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#ifndef Duke3D_network_h
#define Duke3D_network_h

#include "platform.h"

void Setup_UnstableNetworking();
void Setup_StableNetworking();

void callcommit(void);
void initcrc(void);
int32_t getcrc(uint8_t  *buffer, short bufleng);
void initmultiplayers(uint8_t  damultioption, uint8_t  dacomrateoption, uint8_t  dapriority);
void sendpacket(int32_t other, uint8_t  *bufptr, int32_t messleng);
void setpackettimeout(int32_t datimeoutcount, int32_t daresendagaincount);
void uninitmultiplayers(void);
void sendlogon(void);
void sendlogoff(void);
int  getoutputcirclesize(void);
void setsocket(short newsocket);
short getpacket(short *other, uint8_t  *bufptr);
void flushpackets(void);
void genericmultifunction(int32_t other, char  *bufptr, int32_t messleng, int32_t command);


#endif
