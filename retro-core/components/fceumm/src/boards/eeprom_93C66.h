#ifndef _EEPROM_93C66_H
#define _EEPROM_93C66_H
#include "mapinc.h"

extern uint8* eeprom_93C66_storage;

void  eeprom_93C66_init  ();
uint8 eeprom_93C66_read  ();
void  eeprom_93C66_write (uint8 CS, uint8 CLK, uint8 DAT);
#endif
