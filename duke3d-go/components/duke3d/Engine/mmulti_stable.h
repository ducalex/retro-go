#ifndef _MMULTI_STABLE_H_
#define _MMULTI_STABLE_H_

void stable_callcommit(void);
void stable_initcrc(void);
long stable_getcrc(char *buffer, short bufleng);
void stable_initmultiplayers(char damultioption, char dacomrateoption, char dapriority);
void stable_sendpacket(long other, char *bufptr, long messleng);
void stable_setpackettimeout(long datimeoutcount, long daresendagaincount);
void stable_uninitmultiplayers(void);
void stable_sendlogon(void);
void stable_sendlogoff(void);
int  stable_getoutputcirclesize(void);
void stable_setsocket(short newsocket);
short stable_getpacket(short *other, char *bufptr);
void stable_flushpackets(void);
void stable_genericmultifunction(long other, char *bufptr, long messleng, long command);

#endif