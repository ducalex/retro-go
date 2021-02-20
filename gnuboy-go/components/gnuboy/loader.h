#ifndef __LOADER_H__
#define __LOADER_H__

int rom_loadbank(int);
int rom_load(const char *file);
void rom_unload(void);

int sram_load(const char *file);
int sram_save(const char *file);
int sram_update(const char *file);
int state_load(const char *file);
int state_save(const char *file);

#endif
