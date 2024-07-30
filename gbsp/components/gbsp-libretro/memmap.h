#ifndef _MEMMAP_H
#define _MEMMAP_H

void *map_jit_block(unsigned size);
void unmap_jit_block(void *bufptr, unsigned size);

#endif
