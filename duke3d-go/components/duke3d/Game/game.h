#ifndef _GAME_H_
#define _GAME_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//extern uint8_t  game_dir[512];
char* getGameDir(void);
int gametextpal(int x,int y,char  *t,uint8_t  s,uint8_t  p);
int main(int argc,char  **argv);

bool rg_system_should_exit(void);
void rg_system_request_exit(void);

#endif  // include-once header.

