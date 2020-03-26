#ifndef __LOADER_H__
#define __LOADER_H__


typedef struct loader_s
{
	char *rom;
	char *base;
	char *sram;
	char *state;
} loader_t;


extern loader_t loader;

void loader_init(char *s);
void loader_unload();
int rom_loadbank(short);
int rom_load();
int sram_load();
int sram_save();
int state_load(char *s);
int state_save(char *s);


#endif
