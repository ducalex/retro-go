#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
#include "eeprom_93C66.h"

uint8* eeprom_93C66_storage;
uint8  eeprom_93C66_opcode;
uint8  eeprom_93C66_data;
uint16 eeprom_93C66_address;
uint8  eeprom_93C66_state;
uint8  eeprom_93C66_lastCLK;
uint8  eeprom_93C66_writeEnabled;
uint8  eeprom_93C66_output;

#define OPCODE_MISC         0
#define OPCODE_WRITE        1
#define OPCODE_READ         2
#define OPCODE_ERASE        3
#define OPCODE_WRITEDISABLE 10
#define OPCODE_WRITEALL     11
#define OPCODE_ERASEALL     12
#define OPCODE_WRITEENABLE  13

#define STATE_STANDBY   0
#define STATE_STARTBIT  1
#define STATE_OPCODE    3
#define STATE_ADDRESS   12
#define STATE_DATA      20
#define STATE_FINISHED  99

void eeprom_93C66_init ()
{
   eeprom_93C66_address      =0;
   eeprom_93C66_state        =STATE_STANDBY;
   eeprom_93C66_lastCLK      =0;
   eeprom_93C66_writeEnabled =0;
}

uint8 eeprom_93C66_read ()
{
   return eeprom_93C66_output;
}

void eeprom_93C66_write (uint8 CS, uint8 CLK, uint8 DAT)
{
   if (!CS && eeprom_93C66_state <= STATE_ADDRESS)
      eeprom_93C66_state =STATE_STANDBY;
   else
   if (eeprom_93C66_state ==STATE_STANDBY && CS  && CLK)
   {
      eeprom_93C66_state =STATE_STARTBIT;
      eeprom_93C66_opcode  =0;
      eeprom_93C66_address =0;
      eeprom_93C66_output  =1;
   }
   else
   if (CLK && !eeprom_93C66_lastCLK)
   {
      if (eeprom_93C66_state >=STATE_STARTBIT && eeprom_93C66_state <STATE_OPCODE) 
         eeprom_93C66_opcode  =(eeprom_93C66_opcode  <<1) | (DAT? 1: 0);
      else
      if (eeprom_93C66_state >=STATE_OPCODE   && eeprom_93C66_state <STATE_ADDRESS)
         eeprom_93C66_address =(eeprom_93C66_address <<1) | (DAT? 1: 0);
      else
      if (eeprom_93C66_state >=STATE_ADDRESS  && eeprom_93C66_state <STATE_DATA)
      {
         if (eeprom_93C66_opcode ==OPCODE_WRITE || eeprom_93C66_opcode ==OPCODE_WRITEALL)
            eeprom_93C66_data =(eeprom_93C66_data    <<1) | (DAT? 1: 0);
         else
         if (eeprom_93C66_opcode ==OPCODE_READ)
         {
            eeprom_93C66_output =!!(eeprom_93C66_data &0x80);
            eeprom_93C66_data   =   eeprom_93C66_data <<1;
         }
      }
      eeprom_93C66_state++;
      if (eeprom_93C66_state ==STATE_ADDRESS) {
         switch (eeprom_93C66_opcode)
         {
            case OPCODE_MISC:
               eeprom_93C66_opcode =(eeprom_93C66_address >>7) + 10;
               switch (eeprom_93C66_opcode)
               {
                  case OPCODE_WRITEDISABLE:
                     eeprom_93C66_writeEnabled = 0;
                     eeprom_93C66_state = STATE_FINISHED;
                     break;
                  case OPCODE_WRITEENABLE: 
                     eeprom_93C66_writeEnabled = 1;
                     eeprom_93C66_state = STATE_FINISHED;
                     break;
                  case OPCODE_ERASEALL:
                     if (eeprom_93C66_writeEnabled)
                     {
                        int i;
                        for (i =0; i <512; i++)
                           eeprom_93C66_storage[i] = 0xFF;
                     }
                     eeprom_93C66_state = STATE_FINISHED;
                     break;
                  case OPCODE_WRITEALL:
                     eeprom_93C66_address = 0;
                     break;
               }
               break;
            case OPCODE_ERASE:
               if (eeprom_93C66_writeEnabled) eeprom_93C66_storage[eeprom_93C66_address] = 0xFF;
               eeprom_93C66_state = STATE_FINISHED;
               break;
            case OPCODE_READ:
               eeprom_93C66_data = eeprom_93C66_storage[eeprom_93C66_address++];
               break;
         }
      }
      else
      if (eeprom_93C66_state ==STATE_DATA)
      {
         if (eeprom_93C66_opcode ==OPCODE_WRITE)
	 {
            eeprom_93C66_storage[eeprom_93C66_address++] = eeprom_93C66_data;
            eeprom_93C66_state = STATE_FINISHED;
         }
	 else
         if (eeprom_93C66_opcode ==OPCODE_WRITEALL)
	 {
            eeprom_93C66_storage[eeprom_93C66_address++] = eeprom_93C66_data;
            eeprom_93C66_state = CS && eeprom_93C66_address <512? STATE_ADDRESS: STATE_FINISHED;
         }
	 else
         if (eeprom_93C66_opcode ==OPCODE_READ)
	 {
            if (eeprom_93C66_address <512) eeprom_93C66_data = eeprom_93C66_storage[eeprom_93C66_address];
            eeprom_93C66_state = CS && ++eeprom_93C66_address <=512? STATE_ADDRESS: STATE_FINISHED;
         }
      }
      if (eeprom_93C66_state == STATE_FINISHED)
      {
         eeprom_93C66_output = 0;
         eeprom_93C66_state = STATE_STANDBY;
      }
   }
   eeprom_93C66_lastCLK = CLK;
}
#endif
