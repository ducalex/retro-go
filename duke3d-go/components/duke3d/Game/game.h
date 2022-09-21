#ifndef _GAME_H_
#define _GAME_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//extern uint8_t  game_dir[512];
char * getgamedir();
int gametextpal(int x,int y,char  *t,uint8_t  s,uint8_t  p);
int main(int argc,char  **argv);

#endif  // include-once header.

