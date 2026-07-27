//
//  network.c
//  Duke3D
//
//  Created by fabien sanglard on 12-12-22.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#include "network.h"

int nNetMode = 0;

//#include "mmulti_stable.h"
void Setup_UnstableNetworking()
{
	nNetMode = 0;
}

void Setup_StableNetworking()
{
	nNetMode = 1;
}


//TODO ( "[Fix this horrible networking mess. Function pointers not happy]" )
// I do not like this one bit.
// Figure out what was causing the problems with the function pointers.
// This mess is a direct result of my lack of time.. bleh
// This mess shouldn't even be in this file. /slap /slap
void callcommit(void)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:		
            unstable_callcommit();
            break;
        case 1:
            stable_callcommit();
            break;		
	}
#endif
}
void initcrc(void)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:	
            unstable_initcrc();
            break;
        case 1:	
            stable_initcrc();
            break;
	}
#endif
}
int32_t getcrc(uint8_t  *buffer, short bufleng)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            return unstable_getcrc(buffer, bufleng);
        case 1:
            return stable_getcrc(buffer, bufleng);
	}
#endif
	return 0;
}
void initmultiplayers(uint8_t  damultioption, uint8_t  dacomrateoption, uint8_t  dapriority)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_initmultiplayers(damultioption, dacomrateoption, dapriority);
            break;
        case 1:
            stable_initmultiplayers(damultioption, dacomrateoption, dapriority);
            break;
	}
#endif
}
void sendpacket(int32_t other, uint8_t  *bufptr, int32_t messleng)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_sendpacket(other, bufptr, messleng);
            break;
        case 1:
            stable_sendpacket(other, bufptr, messleng);
            break;
	}
#endif
}
void setpackettimeout(int32_t datimeoutcount, int32_t daresendagaincount)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_setpackettimeout(datimeoutcount, daresendagaincount);
            break;
        case 1:
            stable_setpackettimeout(datimeoutcount, daresendagaincount);
            break;
	}
#endif
}
void uninitmultiplayers(void)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_uninitmultiplayers();
            break;
        case 1:
            stable_uninitmultiplayers();
            break;
	}
#endif
}
void sendlogon(void)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_sendlogon();
            break;
        case 1:
            unstable_sendlogon();
            break;
	}
#endif
}
void sendlogoff(void)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_sendlogoff();
            break;
        case 1:
            stable_sendlogoff();
            break;
	}
#endif
}
int  getoutputcirclesize(void)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            return unstable_getoutputcirclesize();
        case 1:
            return stable_getoutputcirclesize();		
	}
#endif
	return 0;
}
void setsocket(short newsocket)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_setsocket(newsocket);
            break;
        case 1:
            stable_setsocket(newsocket);
            break;
	}
#endif
}
short getpacket(short *other, uint8_t  *bufptr)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            return unstable_getpacket(other, bufptr); // default
        case 1:
            return stable_getpacket(other, bufptr);
	}
#endif
	return 0;
}
void flushpackets(void)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_flushpackets();
            break;
        case 1:
            stable_flushpackets();
            break;
	}
#endif
}
void genericmultifunction(int32_t other, char  *bufptr, int32_t messleng, int32_t command)
{
#ifndef USER_DUMMY_NETWORK
	switch(nNetMode)
	{
        case 0:
            unstable_genericmultifunction(other, bufptr, messleng, command);
            break;
        case 1:
            stable_genericmultifunction(other, bufptr, messleng, command);
            break;
	}
#endif
}
