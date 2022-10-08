//////////////////////////////////////////////////////////////////////////////
// Lynx 3wire EEPROM Class                                                  //
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "eeprom.h"

CEEPROM::CEEPROM(UBYTE typ)
{
   *filename=0;
   memset(romdata, 0xff, sizeof(romdata));
   Reset();

   SetEEPROMType(type);
}

void CEEPROM::Reset(void)
{
   busy_count=0;
   state=EE_NONE;
   readdata=0;

   data=0;
   addr=0;
   sendbits=0;
   readonly=true;

   counter=0;
   iodir=0;
   iodat=0;

   mAUDIN_ext=0;
}

CEEPROM::~CEEPROM()
{
}

void CEEPROM::Load(void)
{
   if(!Available()) return;
   FILE *fe;
   if((fe=fopen(filename,"rb"))!=NULL){
      log_printf("EEPROM: Loading from '%s'\n",filename);
      fread(romdata,1,1024,fe);
      fclose(fe);
   }
}

void CEEPROM::Save(void)
{
   if(!Available()) return;
   FILE *fe;
   if((fe=fopen(filename,"wb+"))!=NULL){
      log_printf("EEPROM: Saving to '%s'\n",filename);
      fwrite(romdata,1,Size(),fe);
      fclose(fe);
   }
}

bool CEEPROM::ContextSave(LSS_FILE *fp)
{
   if(!lss_printf(fp,"CEEPROM::ContextSave")) return 0;

   if(!lss_write(&busy_count,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&state,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&readdata,sizeof(UWORD),1,fp)) return 0;

   if(!lss_write(&data,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&addr,sizeof(UWORD),1,fp)) return 0;
   if(!lss_write(&sendbits,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&readonly,sizeof(UBYTE),1,fp)) return 0;

   if(!lss_write(&counter,sizeof(UWORD),1,fp)) return 0;
   if(!lss_write(&iodir,sizeof(UBYTE),1,fp)) return 0;
   if(!lss_write(&iodat,sizeof(UBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIN_ext,sizeof(UBYTE),1,fp)) return 0;

   if(!lss_write(&romdata,sizeof(UWORD),1024,fp)) return 0;
   return 1;
}

bool CEEPROM::ContextLoad(LSS_FILE *fp)
{
   char teststr[32]="XXXXXXXXXXXXXXXXXXXX";
   if(!lss_read(teststr,sizeof(char),20,fp)) return 0;
   teststr[20]=0;
   if(strcmp(teststr,"CEEPROM::ContextSave")!=0) return 0;

   if(!lss_read(&busy_count,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&state,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&readdata,sizeof(UWORD),1,fp)) return 0;

   if(!lss_read(&data,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&addr,sizeof(UWORD),1,fp)) return 0;
   if(!lss_read(&sendbits,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&readonly,sizeof(UBYTE),1,fp)) return 0;

   if(!lss_read(&counter,sizeof(UWORD),1,fp)) return 0;
   if(!lss_read(&iodir,sizeof(UBYTE),1,fp)) return 0;
   if(!lss_read(&iodat,sizeof(UBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIN_ext,sizeof(UBYTE),1,fp)) return 0;

   if(!lss_read(&romdata,sizeof(UWORD),1024,fp)) return 0;
   return 1;
}

void CEEPROM::SetEEPROMType(UBYTE b)
{
   type=b;
   const char *type = "none";
   switch(b&0x7) {
      case 1: // 93C46 , 8 bit mode
         ADDR_MASK =  0x7F;
         CMD_BITS  =  10;
         ADDR_BITS =  7;
         type = "93C46";
         break;
      case 2: // 93C56 , 8 bit mode
         ADDR_MASK =  0xFF;
         CMD_BITS  =  12;
         ADDR_BITS =  9;
         type = "93C56";
         break;
      case 3: // 93C66 , 8 bit mode
         ADDR_MASK =  0x1FF;
         CMD_BITS  =  12;
         ADDR_BITS =  9;
         type = "93C66";
         break;
      case 4: // 93C76 , 8 bit mode
         ADDR_MASK =  0x3FF;
         CMD_BITS  =  14;
         ADDR_BITS =  11;
         type = "93C76";
         break;
      case 5: // 93C86 , 8 bit mode
         ADDR_MASK =  0x7FF;
         CMD_BITS  =  14;
         ADDR_BITS =  11;
         type = "93C86";
         break;
      case 0: // NONE, fallthrou
      default:
         ADDR_MASK =  0;
         CMD_BITS  =  1;
         ADDR_BITS =  1;
         break;
   }
   if(b&0x80) { // 8 bit access
      DONE_MASK =  0x100;
      log_printf("EEPROM: Type: %s 8bit\n", type);
   } else { // 16 bit access
      ADDR_MASK>>=1;
      CMD_BITS--;
      ADDR_BITS--;
      DONE_MASK = 0x10000;
      log_printf("EEPROM: Type: %s 16bit\n", type);
   }
}

int CEEPROM::Size(void)
{
    int m=ADDR_MASK+1;
    if(type&0x80) return m; else return m*2;
}

void CEEPROM::ProcessEepromBusy(void)
{
   if(state==EE_BUSY || state==EE_NONE) {
      if(busy_count<2) {
         busy_count++;
         readdata=0x0000;// RDY
         mAUDIN_ext=0;
      } else {
         readdata=0xFFFF;// RDY
         mAUDIN_ext=1;
         state=EE_WAIT;
      }
//       printf("(%d)",busy_count);
   }
}

void CEEPROM::ProcessEepromCounter(UWORD cnt)
{
   // Update if either counter strobed or AUDIN changed
   UpdateEeprom( cnt);
}

void CEEPROM::ProcessEepromIO(UBYTE iodir_loc,UBYTE iodat_loc)
{
   // Update if either counter strobed or AUDIN changed
   iodat=iodat_loc;
   iodir=iodir_loc;
}

void CEEPROM::UpdateEeprom(UWORD cnt)
{
   // Update if either counter strobed or AUDIN changed
   bool CLKp, CLKn;
   CLKp=counter&0x02;
   counter=cnt;
   CLKn=counter&0x02;

   if( CLKp!=CLKn && CLKn) { // Rising edge
      bool CS, DI;
      mAUDIN_ext=(readdata&(DONE_MASK>>1)) ? 1 : 0 ;
      readdata<<=1;
      CS=cnt&0x80;
      DI=false;
      if(iodir&0x10) {
         DI=iodat&0x10;
      }
      if(!CS) state=EE_NONE;
      switch(state) {
         case EE_NONE:
            data=0;
            if( CS) {
               if(DI && (iodir&0x10)) {
                  //if( state!=EE_START) printf("EE Start...\n");
                  mAUDIN_ext=0;
                  state=EE_START;
                  data=0;
                  sendbits=CMD_BITS-1;
               } else if(!(iodir&0x10)) {
                  state=EE_BUSY;
                  //printf("BUSY\n");
                  readdata=0x0000;// RDY
                  mAUDIN_ext=0;
                  busy_count=0;
               }
            }
            break;
         case EE_START:
            data<<=1;
            if(DI) data++;
            sendbits--;
            if( sendbits>0) break;

            state=EE_NONE;
            // if(data!=(0xFFFF&((ADDR_MASK<<2)|0x3))) printf("EE Byte $%02X .. ",(int)data);
            addr=data&ADDR_MASK;
            switch(data>>ADDR_BITS) {
               case 0x3:
                  if(!readonly) {
//                      printf("ERASE ADD $%02X RO %d\n",(int)addr,readonly);
                     romdata[addr]=0xFFFF;
                  }
                  break;
               case 0x2:
                  if(type&0x80) readdata=((unsigned char *)romdata)[addr]; else readdata=romdata[addr];
                  mAUDIN_ext=0;
//                   printf("Read ADD $%02X $%04X\n",(int)addr,readdata);
                  state=EE_WAIT;
                  break;
               case 0x1:
//                   printf("Write ADD $%02X RO %d\n",(int)addr,readonly);
                  data=0x1;
                  state=EE_DATA;
                  break;
               case 0x00:
                  if((data>>(ADDR_BITS-2))==0x0) {
//                      printf("EWDS\n");
                     readonly=true;
                     break;
                  };
                  if((data>>(ADDR_BITS-2))==0x3) {
//                      printf("EWEN\n");
                     readonly=false;
                     break;
                  };
                  if((data>>(ADDR_BITS-2))==0x1) {
//                      printf("WRAL\n");
                     break;
                  };
                  if((data>>(ADDR_BITS-2))==0x2) {
//                      printf("ERAL\n");
                     break;
                  };
               // falltrhou
               default:
//                   printf("Unknown $%03X\n",(int)data);
                  break;
            }
            break;
         case EE_DATA:
            data<<=1;
            if(DI) data++;
            if(data&DONE_MASK) {
               state=EE_NONE;
//                printf("EE Written Data $%04X ",(unsigned int)data&0xFFFF);
               if(readonly) {
//                   printf("WRITE PROT!\n");
               } else {
                  if(type &0x80){
                     ((unsigned char *)romdata)[addr]=(data&0xFF);
               } else {
                  romdata[addr]=(data&0xFFFF);
                  }
//                   printf("done\n");
               }
               busy_count=0;
               readdata=0x0000;// RDY
               mAUDIN_ext=0;
               state=EE_WAIT;
            }
            break;
         case EE_WAIT:
//             printf(".%d.",mAUDIN_ext);
            break;
      }
   }
}
