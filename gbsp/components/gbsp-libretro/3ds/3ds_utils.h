#ifndef _3DS_UTILS_H
#define _3DS_UTILS_H

void ctr_invalidate_ICache(void);
void ctr_flush_DCache(void);
void ctr_flush_invalidate_cache(void);

extern __attribute((weak)) unsigned int __ctr_svchax;

void check_rosalina();
void ctr_clear_cache(void);

void wait_for_input();
#define DEBUG_HOLD() do{printf("%s@%s:%d.\n",__FUNCTION__, __FILE__, __LINE__);fflush(stdout);wait_for_input();}while(0)
#define DEBUG_VAR(X) printf( "%-20s: 0x%08X\n", #X, (u32)(X))
#define DEBUG_VAR64(X) printf( #X"\r\t\t\t\t : 0x%016llX\n", (u64)(X))

#endif // _3DS_UTILS_H
