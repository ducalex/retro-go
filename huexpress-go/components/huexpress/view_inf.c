/**************************************************************************/
/*                                                                        */
/*                       View infos source file                           */
/*                                                                        */
/*  Allow to see a bunch of internal variables in a peep                  */
/*                                                                        */
/**************************************************************************/


/* Header section */

#include "view_inf.h"

#ifndef MY_EXCLUDE

/* Variable section */

#if defined(ALLEGRO)

static uchar page = 0;

static uchar out = 0;

void (*info_display[MAX_PAGES]) () =
{
  display_reg,
    display_cd_var,
    display_satb1,
    display_satb2,
    display_satb3,

    display_satb4,
    display_satb5, display_satb6, display_satb7, display_satb8};

#else
	
void (*info_display[MAX_PAGES])();	
	
#endif
	
uchar nb_choices[MAX_PAGES] = { 11,
  12
};

int pos_choices[MAX_PAGES][2][12] = {
  {
   {6 * 8,
    23 * 8},
   {20,
    20}
   },
  {
   }
};


/* Function section */

#ifdef ALLEGRO

void
display_cd_var ()
{
  char *tmp_buf = (char *) alloca (100);

  clear (screen);

  sprintf (tmp_buf, "        CD BIOS VARIABLES");
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y, 3, 2, 1, 1);

  sprintf (tmp_buf, "     AL = %02X         AH = %02X", Rd6502 (0x20F8),
	   Rd6502 (0x20F9));
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 20, 3, 2, 1, 1);
  sprintf (tmp_buf, "     BL = %02X         BH = %02X", Rd6502 (0x20FA),
	   Rd6502 (0x20FB));
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 30, 3, 2, 1, 1);
  sprintf (tmp_buf, "     CL = %02X         CH = %02X", Rd6502 (0x20FC),
	   Rd6502 (0x20FD));
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 40, 3, 2, 1, 1);
  sprintf (tmp_buf, "     DL = %02X         DH = %02X", Rd6502 (0x20FE),
	   Rd6502 (0x20FF));
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 50, 3, 2, 1, 1);

  sprintf (tmp_buf, "  $1800 = %02X      $1801 = %02X", cd_port_1800,
	   cd_port_1801);
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 70, 3, 2, 1, 1);

  sprintf (tmp_buf, "  $1802 = %02X      ----------", cd_port_1802);
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 80, 3, 2, 1, 1);

  sprintf (tmp_buf, "  $1804 = %02X      ----------", cd_port_1804);
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 90, 3, 2, 1, 1);

  sprintf (tmp_buf, "     AX = %04X       BX = %04X",
	   Rd6502 (0x20F8) + 256 * Rd6502 (0x20F9),
	   Rd6502 (0x20FA) + 256 * Rd6502 (0x20FB));
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 110, 3, 2, 1, 1);

  sprintf (tmp_buf, "     CX = %04X       DX = %04X",
	   Rd6502 (0x20FC) + 256 * Rd6502 (0x20FD),
	   Rd6502 (0x20FE) + 256 * Rd6502 (0x20FF));
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 120, 3, 2, 1, 1);

  sprintf (tmp_buf, "  REMAINING DATA : %d", pce_cd_read_datacnt);

  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 140, 3, 2, 1, 1);

  sprintf (tmp_buf, "  ADPCM READ PTR : %04X", io.adpcm_rptr);

  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 150, 3, 2, 1, 1);

  sprintf (tmp_buf, "  VRAM WRITE PTR : %04X", IO_VDC_00_MAWR.W * 2);

  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 160, 3, 2, 1, 1);

  sprintf (tmp_buf, "                 PAGE %d/%d", page + 1, MAX_PAGES);
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 10 * 19, 3, 2, 1, 1);


}

void
display_reg ()
{
  char *tmp_buf = (char *) alloca (100);
  uint32 i, j;
  uchar page1, page2;
  uint16 stack = Rd6502 (0x2000) + 256 * Rd6502 (0x2001);
  // __stack is now at 0x2000 but it's not absolutely compulsory

  clear (screen);

  sprintf (tmp_buf, "        GENERAL REGISTERS       ");
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y, 3, 2, 1, 1);

  sprintf (tmp_buf, " A = %02X  X = %02X  Y = %02X  S = %02X", reg_a, reg_x, reg_y,
           reg_s);
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 10, 3, 2, 1, 1);

  sprintf (tmp_buf, " A:X = %04X   [__stack] = %04X",
           (uint16) (reg_a * 256 + reg_x),
	   (uint16) (Rd6502 (stack) + Rd6502 (stack + 1) * 256));
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 20, 3, 2, 1, 1);

  for (i = 0; i < 8; i += 2)
    {
      sprintf (tmp_buf, "  MMR%d = %02X       MMR%d = %02X", i, mmr[i], i + 1,
               mmr[i+1]);
      textoutshadow (screen, font, tmp_buf, blit_x,
		     blit_y + 10 * (i / 2) + 40, 3, 2, 1, 1);
    }

  textoutshadow (screen, font, " ZERO FLAG :", blit_x, blit_y + 90, 3, 2, 1,
		 1);

  if (reg_p & FL_Z)
    textoutshadow (screen, font, "ON", blit_x + 27 * 8, blit_y + 90, 255, 2,
		   1, 1)
    else
    textoutshadow (screen, font, "OFF", blit_x + 27 * 8, blit_y + 90, 3, 2, 1,
		   1);


  textoutshadow (screen, font, " NEGATIVE FLAG :", blit_x, blit_y + 100, 3, 2,
		 1, 1);

  if (reg_p & FL_N)
    textoutshadow (screen, font, "ON", blit_x + 27 * 8, blit_y + 100, 255, 2,
		   1, 1)
    else
    textoutshadow (screen, font, "OFF", blit_x + 27 * 8, blit_y + 100, 3, 2,
		   1, 1);


  textoutshadow (screen, font, " OVERFLOW FLAG :", blit_x, blit_y + 110, 3, 2,
		 1, 1);

  if (reg_p & FL_V)
    textoutshadow (screen, font, "ON", blit_x + 27 * 8, blit_y + 110, 255, 2,
		   1, 1)
    else
    textoutshadow (screen, font, "OFF", blit_x + 27 * 8, blit_y + 110, 3, 2,
		   1, 1);


  textoutshadow (screen, font, " INTERRUPT DISABLED FLAG :", blit_x,
		 blit_y + 120, 3, 2, 1, 1);

  if (reg_p & FL_I)
    textoutshadow (screen, font, "ON", blit_x + 27 * 8, blit_y + 120, 255, 2,
		   1, 1)
    else
    textoutshadow (screen, font, "OFF", blit_x + 27 * 8, blit_y + 120, 3, 2,
		   1, 1);


  textoutshadow (screen, font, " DECIMAL MODE FLAG :", blit_x, blit_y + 130,
		 3, 2, 1, 1)

  if (reg_p & FL_D)
    textoutshadow (screen, font, "ON", blit_x + 27 * 8, blit_y + 130, 255, 2,
		   1, 1)
    else
    textoutshadow (screen, font, "OFF", blit_x + 27 * 8, blit_y + 130, 3, 2,
		   1, 1);


  textoutshadow (screen, font, " CARRY FLAG :", blit_x, blit_y + 140, 3, 2, 1,
		 1)

  if (reg_p & FL_C)
    textoutshadow (screen, font, "ON", blit_x + 27 * 8, blit_y + 140, 255, 2,
		   1, 1)
    else
    textoutshadow (screen, font, "OFF", blit_x + 27 * 8, blit_y + 140, 3, 2,
		   1, 1);


  sprintf (tmp_buf, "                 PAGE %d/%d", page + 1, MAX_PAGES);
  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 10 * 19, 3, 2, 1, 1);

  return;
}

void 
display_satb(uchar number_page, uchar index_low, uchar index_mid, uchar index_hi)
{
 int i;
 char* tmp_buf=(char*)alloca(100);
 clear(screen);
 sprintf(tmp_buf,"        DISPLAY SATB #%d",number_page);
 textoutshadow(screen,font,tmp_buf,blit_x,blit_y,3,2,1,1);
 for (i=index_low; i<index_mid; i++)
    {
 	sprintf(tmp_buf,"#%02d: X=%4d : Y=%4d :PTN@0X%04X",
                i,
                (((SPR*)SPRAM)[i].x&1023)-32,
                (((SPR*)SPRAM)[i].y&1023)-64,
                ((SPR*)SPRAM)[i].no << 5);
 	textoutshadow(screen,font,tmp_buf,blit_x,blit_y+8+16*(i+1-index_low),(i&1?3:255),2,1,1); 
 	sprintf(tmp_buf,"PRI=%c:PAL=%02d: FLX=%c: FLY=%c:", 
                ((((SPR*)SPRAM)[i].atr >> 7) & 1?'X':'O'), 
                ((SPR*)SPRAM)[i].atr & 0xF, 
                ((((SPR*)SPRAM)[i].atr >> 11) & 1?'X':'O'), 
                ((((SPR*)SPRAM)[i].atr >> 15) & 1?'X':'O')); 
        switch ((((SPR*)SPRAM)[i].atr>>12) & 3) 
          { 
           case 00: 
                strcat(tmp_buf,"16x"); 
                break; 
           case 01: 
                strcat(tmp_buf,"32x"); 
                break; 
           default: 
                strcat(tmp_buf,"64x"); 
         } 
         if ((((SPR*)SPRAM)[i].atr>>8)&1) 
           strcat(tmp_buf,"32"); 
         else 
           strcat(tmp_buf,"16"); 
 	textoutshadow(screen,font,tmp_buf,blit_x,blit_y+16+16*(i+1-index_low),(i&1?3:255),2,1,1); 
    } 
 for (i=index_mid; i<index_hi; i++) 
    { 
 	sprintf(tmp_buf,"#%02d: X=%4d : Y=%4d :PTN@0X%04X", 
                i, 
                (((SPR*)SPRAM)[i].x&1023)-32, 
                (((SPR*)SPRAM)[i].y&1023)-64, 
                ((SPR*)SPRAM)[i].no << 5); 
 	textoutshadow(screen,font,tmp_buf,blit_x,blit_y+16+8+16*(i+1-index_low),(i&1?3:255),2,1,1); 
 	sprintf(tmp_buf,"PRI=%c:PAL=%02d: FLX=%c: FLY=%c:", 
                ((((SPR*)SPRAM)[i].atr >> 7) & 1?'X':'O'), 
                ((SPR*)SPRAM)[i].atr & 0xF, 
                ((((SPR*)SPRAM)[i].atr >> 11) & 1?'X':'O'), 
                ((((SPR*)SPRAM)[i].atr >> 15) & 1?'X':'O')); 
        switch ((((SPR*)SPRAM)[i].atr>>12) & 3) 
          { 
           case 00: 
                strcat(tmp_buf,"16x"); 
                break; 
           case 01: 
                strcat(tmp_buf,"32x"); 
                break; 
           default: 
                strcat(tmp_buf,"64x"); 
         } 
         if ((((SPR*)SPRAM)[i].atr>>8)&1) 
           strcat(tmp_buf,"32"); 
         else 
           strcat(tmp_buf,"16"); 
 	textoutshadow(screen,font,tmp_buf,blit_x,blit_y+32+16*(i+1-index_low),(i&1?3:255),2,1,1); 
    } 
 sprintf(tmp_buf,"                 PAGE %d/%d",page+1,MAX_PAGES); 
 textoutshadow(screen,font,tmp_buf,blit_x,blit_y+10*19,3,2,1,1); 
 }

void
display_satb1()
 { display_satb(1,  0,  4,  8); }

void
display_satb2()
 { display_satb( 2,  8, 12, 16); }

void
display_satb3()
{ display_satb( 3, 16, 20, 24); }

void
display_satb4()
{ display_satb( 4, 24, 28, 32); }

void
display_satb5()
{ display_satb( 5, 32, 36, 40); }

void
display_satb6()
{ display_satb( 6, 40, 44, 48); }

void
display_satb7()
{ display_satb( 7, 48, 52, 56); }

void
display_satb8()
{ display_satb( 8, 56, 60, 64); }


/* TODO : make this function display things so that it will be easier
   to detect bugs in hu-go! rendering routines and own developped roms */
void
display_pattern ()
{
  uchar no = 0;
  uchar inc = 2, h = 16, j = 0;
  uchar *R = &SPal[(((SPR *) SPRAM)[0].atr & 15) * 16];
  uchar *C;
  char *tmp_buf = (char *) alloca (100);
  uchar* C2;
/*
 clear(screen);

 for (no=0; no<20; no++)
    {

 	C = &VRAM[no*128];
 	C2 = &VRAMS[no*32];

 	PutSprite(screen->line[16*no] + 50,C+j*128,C2+j*32,R,h,inc);
   }

 for (; no<40; no++)
    {

 	C = &VRAM[no*128];
 	C2 = &VRAMS[no*32];

 	PutSprite(screen->line[16*(no-20)] + 100,C+j*128,C2+j*32,R,h,inc);
   }

 for (; no<64; no++)
    {

 	C = &VRAM[no*128];
 	C2 = &VRAMS[no*32];

 	PutSprite(screen->line[16*(no-40)] + 150,C+j*128,C2+j*32,R,h,inc);
   }

 sprintf(tmp_buf,"                 PAGE %d/%d",page+1,MAX_PAGES);
 textoutshadow(screen,font,tmp_buf,blit_x,blit_y+10*19,3,2,1,1);
*/
}

void
key_info ()
{
  int ch = osd_readkey ();

  switch (ch >> 8)
    {
    case KEY_RIGHT:
      if (++page >= MAX_PAGES)
	page = 0;
      break;
    case KEY_LEFT:
      if (page)
	page--;
      else
	page = MAX_PAGES - 1;
      break;
    case KEY_F12:
    case KEY_ESC:
      out = 1;
      break;
    default:
    }
}

#endif

void
view_info ()
{
#if defined(ALLEGRO)	
  out = 0;

  do
    {
      (*info_display[page]) ();
       key_info ();
    }
  while (!out);
#endif
  return;
}

#endif