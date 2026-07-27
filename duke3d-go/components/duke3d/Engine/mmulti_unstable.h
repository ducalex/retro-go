#ifndef _MMULTI_UNSTABLE_H_
#define _MMULTI_UNSTABLE_H_

void unstable_callcommit(void);
void unstable_initcrc(void);
long unstable_getcrc(char *buffer, short bufleng);
void unstable_initmultiplayers(char damultioption, char dacomrateoption, char dapriority);
void unstable_sendpacket(long other, char *bufptr, long messleng);
void unstable_setpackettimeout(long datimeoutcount, long daresendagaincount);
void unstable_uninitmultiplayers(void);
void unstable_sendlogon(void);
void unstable_sendlogoff(void);
int  unstable_getoutputcirclesize(void);
void unstable_setsocket(short newsocket);
short unstable_getpacket(short *other, char *bufptr);
void unstable_flushpackets(void);
void unstable_genericmultifunction(long other, char *bufptr, long messleng, long command);

#endif
