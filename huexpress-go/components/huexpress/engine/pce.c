/*
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU Library General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/************************************************************************/
/*   'Portable' PC-Engine Emulator Source file							*/
/*                                                                      */
/*	1998 by BERO bero@geocities.co.jp                                   */
/*                                                                      */
/*  Modified 1998 by hmmx hmmx@geocities.co.jp                          */
/*	Modified 1999-2005 by Zeograd (Olivier Jolly) zeograd@zeograd.com   */
/*	Modified 2011-2013 by Alexander von Gluck kallisti5@unixzen.com     */
/************************************************************************/

/* Header section */

#include "pce.h"
#include "utils.h"

#include "romdb.h"

#define LOG_NAME "huexpress.log"

#define CD_FRAMES 75
#define CD_SECS 60

/* Variable section */

struct host_machine host;
struct hugo_options option;

uchar can_write_debug = 0;

uchar *cd_buf = NULL;

uchar *PopRAM;
// Now dynamicaly allocated
// ( size of popRAMsize bytes )
// If someone could explain me why we need it
// the version I have works well without this trick

const uint32 PopRAMsize = 0x8000;
// I don't really know if it must be 0x8000 or 0x10000

#define ZW			64
//byte ZBuf[ZW*256];
//BOOL IsROM[8];

uchar *ROM = NULL;
// IOAREA = a pointer to the emulated IO zone
// vchange = array of boolean to know whether bg tiles have changed (i.e.
//      vchanges[5]==1 means the 6th tile have changed and VRAM2 should be updated)
//      [to check !]
// vchanges IDEM for sprites
// ROM = the same thing as the ROM file (w/o header)

uchar CDBIOS_replace[0x4d][2];
// Used to know what byte do we have replaced to hook bios functions so that
// we can restore them if needed

int ROM_size;
// obvious, no ?
// actually, the number of block of 0x2000 bytes in the rom

extern int32 vmode;
// What is the favorite video mode to use

int32 smode;
// what sound card type should we use? (0 means the silent one,
// my favorite : the fastest!!! ; and -1 means AUTODETECT;
// later will avoid autodetection if wanted)

char silent = 1;
// a bit different from the previous one, even if asked to
// use a card, we could not be able to make sound...

/*
 * nb_joy no more used
 * uchar nb_joy = 1;
 * number of input to poll
 */

int Country;
/* Is this^ initialised anywhere ?
 * You may try to play with if some games don't want to start
 * it could be useful on some cases
 */

int IPeriod;
// Number of cycle between two interruption calls

uint32 TimerCount;
// int CycleOld;
// int TimerPeriod;
int scanlines_per_frame = 263;

//int MinLine = 0,MaxLine = 255;
//#define MAXDISP 227

char cart_name[PCE_PATH_MAX];
// Name of the file containing the ROM

char short_cart_name[PCE_PATH_MAX];
// Just the filename without the extension (with a dot)
// you just have to add your own extension...

uchar hook_start_cd_system = 0;
// Do we hook CD system to avoid pressing start on main screen

char rom_file_name[PCE_PATH_MAX];
// the name of the file containing the ROM (with path, ext)
// Now needed 'coz of ZIP archiving...

uchar force_header = 1;
// Force the first sector of the code track to be the correct header

char *server_hostname = NULL;

char effectively_played = 0;
// Well, the name is enough I think...

uchar populus = 0;
// no more hasardous detection
// thanks to the CRC detection
// now, used to know whether the save file
// must contain extra info

uchar US_encoded_card = 0;
// Do we have to swap bit order in the rom

uint16 NO_ROM;
// Number of the ROM in the database or 0xFFFF if unknown

uchar debug_on_beginning = 0;
// Do we have to set a bp on the reset IP

uchar builtin_system_used = 0;
// Have we used the .dat included rom or no ?

int scroll = 0;

signed char snd_vol[6][32];
// cooked volume for each channel

volatile char key_delay = 0;
// delay to avoid too many key strokes

static volatile uchar can_blit = 1;
// used to sync screen to 60 image/sec.

volatile uint32 message_delay = 0;
// if different of zero, we must display the message pointed by pmessage

char exit_message[256];
// What we must display at the end


// const
uchar binbcd[0x100] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
};

// const
uchar bcdbin[0x100] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0, 0, 0, 0,
		0, 0,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0, 0, 0, 0,
		0, 0,
	0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0, 0, 0, 0,
		0, 0,
	0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0, 0, 0, 0,
		0, 0,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0, 0, 0, 0,
		0, 0,
	0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0, 0, 0, 0,
		0, 0,
	0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0, 0, 0, 0,
		0, 0,
	0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0, 0, 0, 0,
		0, 0,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0, 0, 0, 0,
		0, 0,
	0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0, 0, 0, 0,
		0, 0,
};

const char *joymap_reverse[J_MAX] = {
	"UP", "DOWN", "LEFT", "RIGHT",
	"I", "II", "SELECT", "RUN",
	"AUTOI", "AUTOII", "PI", "PII",
	"PSELECT", "PRUN", "PAUTOI", "PAUTOII",
	"PXAXIS", "PYAXIS"
};

//extern char    *pCartName;

//extern char snd_bSound;

uint32 timer_60 = 0;
// how many times do the interrupt have been called

int UPeriod = 0;
// Number of frame to skip

uchar video_driver = 0;
/* 0 => Normal driver, normal display
 * 1 => Eagle graphism
 * 2 => Scanline graphism
 */

static char *
check_char(char *s, char c)
{
	while ((*s) && (*s != c))
		s++;

	return *s == c ? s : NULL;
}


uint32
interrupt_60hz(uint32 interval, void *param)
{
	/* Make the system understand it can blit */
	can_blit = 1;

	/* If we've displayed a message recently, make it less recent */
	if (message_delay)
		message_delay--;

	/* number of call of this function */
	timer_60++;

	return interval;
};

/*****************************************************************************

		Function: init_log_file

		Description: destroy the current log file, and create another
		Parameters: none
		Return: nothin

*****************************************************************************/
void
init_log_file()
{
//	Log("Creating log file %s\n", __DATE__);
}


extern int op6502_nb;

IRAM_ATTR void
IO_write(uint16 A, uchar V)
{
    //printf("w%04x,%02x ",A&0x3FFF,V);

    if ((A >= 0x800) && (A < 0x1800))   // We keep the io buffer value
        io.io_buffer = V;

#ifndef FINAL_RELEASE
    if ((A & 0x1F00) == 0x1A00)
        Log("AC Write %02x at %04x\n", V, A);
#endif

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            IO_VDC_active_set(V & 31)
            return;
        case 1:
            return;
        case 2:
            //printf("vdc_l%d,%02x ",io.vdc_reg,V);
            switch (io.vdc_reg) {
            case VWR:           /* Write to video */
                io.vdc_ratch = V;
                return;
            case HDR:           /* Horizontal Definition */
                {
                    typeof(io.screen_w) old_value = io.screen_w;
                    io.screen_w = (V + 1) * 8;

                    if (io.screen_w == old_value)
                        break;

                    // (*init_normal_mode[video_driver]) ();
                    gfx_need_video_mode_change = 1;
                    {
                        uint32 x, y =
                            (WIDTH - io.screen_w) / 2 - 512 * WIDTH;
                        for (x = 0; x < 1024; x++) {
                            // spr_init_pos[x] = y;
                            y += WIDTH;
                        }
                    }
                }
                break;

            case MWR:           /* size of the virtual background screen */
                {
                    static uchar bgw[] = { 32, 64, 128, 128 };
                    io.bg_h = (V & 0x40) ? 64 : 32;
                    io.bg_w = bgw[(V >> 4) & 3];
                }
                break;

            case BYR:           /* Vertical screen offset */
                /*
                   if (IO_VDC_08_BYR.B.l == V)
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_08_BYR.B.l = V;
                scroll = 1;
                ScrollYDiff = scanline - 1;
                ScrollYDiff -= IO_VDC_0C_VPR.B.h + IO_VDC_0C_VPR.B.l;

#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (l), ", ScrollY);
#endif
                return;
            case BXR:           /* Horizontal screen offset */

                /*
                   if (IO_VDC_07_BXR.B.l == V)
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_07_BXR.B.l = V;
                scroll = 1;
                return;

            case CR:
                if (IO_VDC_active.B.l == V)
                    return;
                save_gfx_context(0);
                IO_VDC_active.B.l = V;
                return;

            case VCR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case HSR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case VPR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case VDW:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;
            }

            IO_VDC_active.B.l = V;
            // all others reg just need to get the value, without additional stuff

#if ENABLE_TRACING_DEEP_GFX
            TRACE("VDC[%02x]=0x%02x, ", io.vdc_reg, V);
#endif

#ifndef FINAL_RELEASE
            if (io.vdc_reg > 19) {
                fprintf(stderr, "ignore write lo vdc%d,%02x\n", io.vdc_reg,
                        V);
            }
#endif

            return;
        case 3:
            switch (io.vdc_reg) {
            case VWR:           /* Write to mem */
                /* Writing to hi byte actually perform the action */
                VRAM[IO_VDC_00_MAWR.W * 2] = io.vdc_ratch;
                VRAM[IO_VDC_00_MAWR.W * 2 + 1] = V;

                vchange[IO_VDC_00_MAWR.W / 16] = 1;
                vchanges[IO_VDC_00_MAWR.W / 64] = 1;

                IO_VDC_00_MAWR.W += io.vdc_inc;

                /* vdc_ratch shouldn't be reset between writes */
                // io.vdc_ratch = 0;
                return;

            case VCR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case HSR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case VPR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case VDW:           /* screen height */
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case LENR:          /* DMA transfert */

                IO_VDC_12_LENR.B.h = V;

                {               // black-- 's code

                    int sourcecount = (IO_VDC_0F_DCR.W & 8) ? -1 : 1;
                    int destcount = (IO_VDC_0F_DCR.W & 4) ? -1 : 1;

                    int source = IO_VDC_10_SOUR.W * 2;
                    int dest = IO_VDC_11_DISTR.W * 2;

                    int i;

                    for (i = 0; i < (IO_VDC_12_LENR.W + 1) * 2; i++) {
                        *(VRAM + dest) = *(VRAM + source);
                        dest += destcount;
                        source += sourcecount;
                    }

                    /*
                       IO_VDC_10_SOUR.W = source;
                       IO_VDC_11_DISTR.W = dest;
                     */
                    // Erich Kitzmuller fix follows
                    IO_VDC_10_SOUR.W = source / 2;
                    IO_VDC_11_DISTR.W = dest / 2;

                }

                IO_VDC_12_LENR.W = 0xFFFF;

                memset(vchange, 1, VRAMSIZE / 32);
                memset(vchanges, 1, VRAMSIZE / 128);


                /* TODO: check whether this flag can be ignored */
                io.vdc_status |= VDC_DMAfinish;

                return;

            case CR:            /* Auto increment size */
                {
                    static uchar incsize[] = { 1, 32, 64, 128 };
                    /*
                       if (IO_VDC_05_CR.B.h == V)
                       return;
                     */
                    save_gfx_context(0);

                    io.vdc_inc = incsize[(V >> 3) & 3];
                    IO_VDC_05_CR.B.h = V;
                }
                break;
            case HDR:           /* Horizontal display end */
                /* TODO : well, maybe we should implement it */
                //io.screen_w = (io.VDC_ratch[HDR]+1)*8;
                //TRACE0("HDRh\n");
#if ENABLE_TRACING_DEEP_GFX
                TRACE("VDC[HDR].h = %d, ", V);
#endif
                break;

            case BYR:           /* Vertical screen offset */

                /*
                   if (IO_VDC_08_BYR.B.h == (V & 1))
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_08_BYR.B.h = V & 1;
                scroll = 1;
                ScrollYDiff = scanline - 1;
                ScrollYDiff -= IO_VDC_0C_VPR.B.h + IO_VDC_0C_VPR.B.l;
#if ENABLE_TRACING_GFX
                if (ScrollYDiff < 0)
                    TRACE("ScrollYDiff went negative when substraction VPR.h/.l (%d,%d)\n",
                        IO_VDC_0C_VPR.B.h, IO_VDC_0C_VPR.B.l);
#endif

#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (h), ", ScrollY);
#endif

                return;

            case SATB:          /* DMA from VRAM to SATB */
                IO_VDC_13_SATB.B.h = V;
                io.vdc_satb = 1;
                io.vdc_status &= ~VDC_SATBfinish;
                return;

            case BXR:           /* Horizontal screen offset */

                if (IO_VDC_07_BXR.B.h == (V & 3))
                    return;

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }

                IO_VDC_07_BXR.B.h = V & 3;
                scroll = 1;
                return;
            }
            IO_VDC_active.B.h = V;

#ifndef FINAL_RELEASE
            if (io.vdc_reg > 19) {
                fprintf(stderr, "ignore write hi vdc%d,%02x\n", io.vdc_reg,
                        V);
            }
#endif

            return;
        }
        break;

    case 0x0400:                /* VCE */
        switch (A & 7) {
        case 0:
            /*TRACE("VCE 0, V=%X\n", V); */
            return;

            /* Choose color index */
        case 2:
            io.vce_reg.B.l = V;
            return;
        case 3:
            io.vce_reg.B.h = V & 1;
            return;

            /* Set RGB components for current choosen color */
        case 4:
            io.VCE[io.vce_reg.W].B.l = V;
            {
                uchar c;
                int i, n;
                n = io.vce_reg.W;
                c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (i = 0; i < 256; i += 16)
                        Pal[i] = c;
                } else if (n & 15)
                    Pal[n] = c;
            }
            return;

        case 5:
            io.VCE[io.vce_reg.W].B.h = V;
            {
                uchar c;
                int i, n;
                n = io.vce_reg.W;
                c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (i = 0; i < 256; i += 16)
                        Pal[i] = c;
                } else if (n & 15)
                    Pal[n] = c;
            }
            io.vce_reg.W = (io.vce_reg.W + 1) & 0x1FF;
            return;
        }
        break;


    case 0x0800:                /* PSG */

        switch (A & 15) {

            /* Select PSG channel */
        case 0:
            io.psg_ch = V & 7;
            return;

            /* Select global volume */
        case 1:
            io.psg_volume = V;
            return;

            /* Frequency setting, 8 lower bits */
        case 2:
            io.PSG[io.psg_ch][2] = V;
            break;

            /* Frequency setting, 4 upper bits */
        case 3:
            io.PSG[io.psg_ch][3] = V & 15;
            break;

        case 4:
            io.PSG[io.psg_ch][4] = V;
#if ENABLE_TRACING_AUDIO
            if ((V & 0xC0) == 0x40)
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = 0;
#endif
            break;

            /* Set channel specific volume */
        case 5:
            io.PSG[io.psg_ch][5] = V;
            break;

            /* Put a value into the waveform or direct audio buffers */
        case 6:
            if (io.PSG[io.psg_ch][PSG_DDA_REG] & PSG_DDA_DIRECT_ACCESS) {
                io.psg_da_data[io.psg_ch][io.psg_da_index[io.psg_ch]] = V;
                io.psg_da_index[io.psg_ch] =
                    (io.psg_da_index[io.psg_ch] + 1) & 0x3FF;
                if (io.psg_da_count[io.psg_ch]++ >
                    (PSG_DIRECT_ACCESS_BUFSIZE - 1)) {
                    if (!io.psg_channel_disabled[io.psg_ch])
                        MESSAGE_INFO
                            ("Audio being put into the direct access buffer faster than it's being played.\n");
                    io.psg_da_count[io.psg_ch] = 0;
                }
            } else {
                io.wave[io.psg_ch][io.PSG[io.psg_ch][PSG_DATA_INDEX_REG]] =
                    V;
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] =
                    (io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] + 1) & 0x1F;
            }
            break;

        case 7:
            io.PSG[io.psg_ch][7] = V;
            break;

        case 8:
            io.psg_lfo_freq = V;
            break;

        case 9:
            io.psg_lfo_ctrl = V;
            break;

#ifdef EXTRA_CHECKING
        default:
            fprintf(stderr, "ignored PSG write\n");
#endif
        }
        return;

    case 0x0c00:                /* timer */
        //TRACE("Timer Access: A=%X,V=%X\n", A, V);
        switch (A & 1) {
        case 0:
            io.timer_reload = V & 127;
            return;
        case 1:
            V &= 1;
            if (V && !io.timer_start)
                io.timer_counter = io.timer_reload;
            io.timer_start = V;
            return;
        }
        break;

    case 0x1000:                /* joypad */
//              TRACE("V=%02X\n", V);
        io.joy_select = V & 1;
        //io.joy_select = V;
        if (V & 2)
            io.joy_counter = 0;
        return;

    case 0x1400:                /* IRQ */
        switch (A & 15) {
        case 2:
            io.irq_mask = V;    /*TRACE("irq_mask = %02X\n", V); */
            return;
        case 3:
            io.irq_status = (io.irq_status & ~TIRQ) | (V & 0xF8);
            return;
        }
        break;

    case 0x1A00:
		Log("Arcade Card not supported : %d into 0x%04X\n", V, A);
        break;

    case 0x1800:                /* CD-ROM extention */
		// Log("CD Emulation not implemented : %d 0x%04X\n", V, A);
        break;
    }
#ifndef FINAL_RELEASE
    fprintf(stderr,
            "ignore I/O write %04x,%02x\tBase adress of port %X\nat PC = %04X\n",
            A, V, A & 0x1CC0,
            reg_pc);
#endif
//          DebugDumpTrace(4, 1);
}

uchar
TimerInt()
{
	if (io.timer_start) {
		io.timer_counter--;
		if (io.timer_counter > 128) {
			io.timer_counter = io.timer_reload;

			if (!(io.irq_mask & TIRQ)) {
				io.irq_status |= TIRQ;
				return INT_TIMER;
			}

		}
	}
	return INT_NONE;
}


/*****************************************************************************

		Function: CartLoad

		Description: load a card
		Parameters: char* name (the filename to load)
		Return: -1 on error else 0
		set rom_file_name or builtin_system

*****************************************************************************/
int
CartLoad(char *name)
{
	MESSAGE_INFO("Opening %s...\n", name);
	strcpy(rom_file_name, name);

	if (cart_name != name) {
		// Avoids warning when copying passing cart_name as parameter
		#warning find where this weird call is done
		strcpy(cart_name, name);
	}

	if (ROM != NULL) {
		// ROM was already loaded, likely from memory / zip file
		return 0;
	}

	// Load PCE, we always load a PCE even with a cd (syscard)
	FILE *fp = fopen(rom_file_name, "rb");
	// find file size
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);

	// ajust var if header present
	fseek(fp, fsize & 0x1fff, SEEK_SET);
	fsize &= ~0x1fff;

	// read ROM
	ROM = (uchar *)rg_alloc(fsize, MEM_SLOW);
	ROM_size = fsize / 0x2000;
	fread(ROM, 1, fsize, fp);

	fclose(fp);

	return 0;
}


int
ResetPCE()
{
	int i;

	memset(SPRAM, 0, 64 * 8);

	TimerCount = TimerPeriod;
	hard_reset_io();
	scanline = 0;
	io.vdc_status = 0;
	io.vdc_inc = 1;
	io.minline = 0;
	io.maxline = 255;
	io.irq_mask = 0;
	io.psg_volume = 0;
	io.psg_ch = 0;

	zp_base = RAM;
	sp_base = RAM + 0x100;

	/* TEST */
	io.screen_w = 255;
	/* TEST normally 256 */

	/* TEST */
	// io.screen_h = 214;
	/* TEST */

	/* TEST */
	//  io.screen_h = 240;
	/* TEST */

	io.screen_h = 224;

	{
		uint32 x, y = (WIDTH - io.screen_w) / 2 - 512 * WIDTH;
		for (x = 0; x < 1024; x++) {
			// spr_init_pos[x] = y;
			y += WIDTH;
		}
		// pos = WIDTH*(HEIGHT-FC_H)/2+(WIDTH-FC_W)/2+WIDTH*y+x;
	}

	for (i = 0; i < 6; i++) {
		io.PSG[i][4] = 0x80;
	}

#if !defined(TEST_ROM_RELOCATED)
	mmr[7] = 0x00;
	bank_set(7, 0x00);

	mmr[6] = 0x05;
	bank_set(6, 0x05);

	mmr[5] = 0x04;
	bank_set(5, 0x04);

	mmr[4] = 0x03;
	bank_set(4, 0x03);

	mmr[3] = 0x02;
	bank_set(3, 0x02);

	mmr[2] = 0x01;
	bank_set(2, 0x01);
#else
	mmr[7] = 0x68;
	bank_set(7, 0x68);

	mmr[6] = 0x05;
	bank_set(6, 0x05 + 0x68);

	mmr[5] = 0x04;
	bank_set(5, 0x04 + 0x68);

	mmr[4] = 0x03;
	bank_set(4, 0x03 + 0x68);

	mmr[3] = 0x02;
	bank_set(3, 0x02 + 0x68);

	mmr[2] = 0x01;
	bank_set(2, 0x01 + 0x68);
#endif							/* TEST_ROM_RELOCATED */

	mmr[1] = 0xF8;
	bank_set(1, 0xF8);

	mmr[0] = 0xFF;
	bank_set(0, 0xFF);

	reg_a = reg_x = reg_y = 0x00;
	reg_p = FL_TIQ;

	reg_s = 0xFF;

	reg_pc = Op6502(VEC_RESET) + 256 * Op6502(VEC_RESET + 1);

	//CycleNew = 0;
	//CycleOld = 0;

	if (debug_on_beginning) {
		Bp_list[GIVE_HAND_BP].position = reg_pc;
		Bp_list[GIVE_HAND_BP].original_op = Op6502(reg_pc);
		Bp_list[GIVE_HAND_BP].flag = ENABLED;
		Wr6502(reg_pc, 0xB + 0x10 * GIVE_HAND_BP);
	}

	return 0;
}


int
InitPCE(char *name)
{
	int i = 0, ROMmask;
	char *tmp_dummy;
	char local_us_encoded_card = 0;

	option.want_fullscreen_aspect = 0;
	option.want_supergraphx_emulation = 1;

	if (CartLoad(name))
		return 1;

	if (!(tmp_dummy = (char *) (strrchr(cart_name, '/'))))
		tmp_dummy = &cart_name[0];
	else
		tmp_dummy++;

	memset(short_cart_name, 0, 80);
	while ((tmp_dummy[i]) && (tmp_dummy[i] != '.')) {
		short_cart_name[i] = tmp_dummy[i];
		i++;
	}

	if (strlen(short_cart_name))
		if (short_cart_name[strlen(short_cart_name) - 1] != '.') {
			short_cart_name[strlen(short_cart_name) + 1] = 0;
			short_cart_name[strlen(short_cart_name)] = '.';
		}

	// Set the base frequency
	BaseClock = 7800000;

	// Set the interruption period
	IPeriod = BaseClock / (scanlines_per_frame * 60);

	hard_init();

	/* TEST */
	io.screen_h = 224;
	/* TEST */
	io.screen_w = 256;

	uint32 CRC = CRC_buffer(ROM, ROM_size * 0x2000);

	/* I'm doing it only here 'coz cartload set
	   true_file_name       */

	NO_ROM = 0xFFFF;

	int index;
	for (index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (CRC == kKnownRoms[index].CRC)
			NO_ROM = index;
	}

	if (NO_ROM == 0xFFFF)
		printf("ROM not in database: CRC=%lx\n", CRC);

	memset(WRAM, 0, 0x2000);
	WRAM[0] = 0x48;				/* 'H' */
	WRAM[1] = 0x55;				/* 'U' */
	WRAM[2] = 0x42;				/* 'B' */
	WRAM[3] = 0x4D;				/* 'M' */
	WRAM[5] = 0xA0;				/* WRAM[4-5] = 0xA000, end of free mem ? */
	WRAM[6] = 0x10;				/* WRAM[6-7] = 0x8010, beginning of free mem ? */
	WRAM[7] = 0x80;

	local_us_encoded_card = US_encoded_card;

	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & US_ENCODED))
		local_us_encoded_card = 1;

	if (ROM[0x1FFF] < 0xE0) {
		Log("This rom is probably US encrypted, decrypting...\n");
		local_us_encoded_card = 1;
	}

	if (local_us_encoded_card) {
		uint32 x;
		uchar inverted_nibble[16] = { 0, 8, 4, 12,
			2, 10, 6, 14,
			1, 9, 5, 13,
			3, 11, 7, 15
		};

		for (x = 0; x < ROM_size * 0x2000; x++) {
			uchar temp;

			temp = ROM[x] & 15;

			ROM[x] &= ~0x0F;
			ROM[x] |= inverted_nibble[ROM[x] >> 4];

			ROM[x] &= ~0xF0;
			ROM[x] |= inverted_nibble[temp] << 4;

		}
	}

	// For example with Devil Crush 512Ko
	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & TWO_PART_ROM))
		ROM_size = 0x30;

	ROMmask = 1;
	while (ROMmask < ROM_size)
		ROMmask <<= 1;
	ROMmask--;

#ifndef FINAL_RELEASE
	fprintf(stderr, "ROMmask=%02X, ROM_size=%02X\n", ROMmask, ROM_size);
#endif

	for (i = 0; i < 0xFF; i++) {
		ROMMapR[i] = TRAPRAM;
		ROMMapW[i] = TRAPRAM;
	}

#if ! defined(TEST_ROM_RELOCATED)
	for (i = 0; i < 0x80; i++) {
		if (ROM_size == 0x30) {
			switch (i & 0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				ROMMapR[i] = ROM + (i & ROMmask) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				ROMMapR[i] = ROM + ((i - 0x20) & ROMmask) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				ROMMapR[i] = ROM + ((i - 0x10) & ROMmask) * 0x2000;
				break;
			case 0x40:
				ROMMapR[i] = ROM + ((i - 0x20) & ROMmask) * 0x2000;
				break;
			}
		} else {
			ROMMapR[i] = ROM + (i & ROMmask) * 0x2000;
		}
	}
#else
	for (i = 0x68; i < 0x88; i++) {
		if (ROM_size == 0x30) {
			switch (i & 0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				ROMMapR[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
				ROMMapW[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				ROMMapR[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				ROMMapW[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				ROMMapR[i] =
					ROM + (((i - 0x68) - 0x10) & ROMmask) * 0x2000;
				ROMMapW[i] =
					ROM + (((i - 0x68) - 0x10) & ROMmask) * 0x2000;
				break;
			case 0x40:
				ROMMapR[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				ROMMapW[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				break;
			}
		} else {
			ROMMapR[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
			ROMMapW[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
		}
	}
#endif

	if (NO_ROM != 0xFFFF) {
		MESSAGE_INFO("Rom Name: %s\n",
			(kKnownRoms[NO_ROM].Name) ? kKnownRoms[NO_ROM].Name : "Unknown");
		MESSAGE_INFO("Publisher: %s\n",
			(kKnownRoms[NO_ROM].Publisher) ? kKnownRoms[NO_ROM].Publisher : "Unknown");
	} else {
		MESSAGE_ERROR("Unknown ROM\n");
	}

	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & POPULOUS)) {
		populus = 1;

		MESSAGE_INFO("Special Rom: Populous detected!\n");
		if (!(PopRAM = (uchar*)malloc(PopRAMsize)))
			perror("Populous: Not enough memory!");

		ROMMapW[0x40] = PopRAM;
		ROMMapW[0x41] = PopRAM + 0x2000;
		ROMMapW[0x42] = PopRAM + 0x4000;
		ROMMapW[0x43] = PopRAM + 0x6000;

		ROMMapR[0x40] = PopRAM;
		ROMMapR[0x41] = PopRAM + 0x2000;
		ROMMapR[0x42] = PopRAM + 0x4000;
		ROMMapR[0x43] = PopRAM + 0x6000;

	} else {
		populus = 0;
		PopRAM = NULL;
	}

	ROMMapR[0xF7] = WRAM;
	ROMMapW[0xF7] = WRAM;

	ROMMapR[0xF8] = RAM;
	ROMMapW[0xF8] = RAM;

	if (option.want_supergraphx_emulation) {
		ROMMapW[0xF9] = RAM + 0x2000;
		ROMMapW[0xFA] = RAM + 0x4000;
		ROMMapW[0xFB] = RAM + 0x6000;

		ROMMapR[0xF9] = RAM + 0x2000;
		ROMMapR[0xFA] = RAM + 0x4000;
		ROMMapR[0xFB] = RAM + 0x6000;
	}

	/*
	   #warning REMOVE ME
	   // ROMMapR[0xFC] = RAM + 0x6000;
	   ROMMapW[0xFC] = NULL;
	 */

	ROMMapR[0xFF] = IOAREA;
	ROMMapW[0xFF] = IOAREA;

	{
		// char backupmem[PCE_PATH_MAX];
		// snprintf(backupmem, PCE_PATH_MAX, "%s/backupmem.bin", config_basepath);

		// FILE *fp;
		// fp = fopen(backupmem, "rb");

		// if (fp == NULL)
		// 	fprintf(stderr, "Can't open %s\n", backupmem);
		// else {
		// 	fread(WRAM, 0x2000, 1, fp);
		// 	fclose(fp);
		// }

	}

	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & CD_SYSTEM)) {
		uint16 offset = 0;
		uchar new_val = 0;

		switch(kKnownRoms[NO_ROM].CRC) {
			case 0X3F9F95A4:
				// CD-ROM SYSTEM VER. 1.00
				offset = 56254;
				new_val = 17;
				break;
			case 0X52520BC6:
			case 0X283B74E0:
				// CD-ROM SYSTEM VER. 2.00
				// CD-ROM SYSTEM VER. 2.10
				offset = 51356;
				new_val = 128;
				break;
			case 0XDD35451D:
			case 0XE6F16616:
				// CD ROM 2 SYSTEM 3.0
				// SUPER CD-ROM2 SYSTEM VER. 3.00
				// SUPER CD-ROM2 SYSTEM VER. 3.00
				offset = 51401;
				new_val = 128;
				break;
		}

		if (offset > 0)
			ROMMapW[0xE1][offset & 0x1fff] = new_val;
	}

	return 0;
}


int
RunPCE(void)
{
	if (!ResetPCE())
		exe_go();
	return 1;
}

void
TrashPCE()
{
	FILE *fp;
	char *tmp_buf = (char *) alloca(256);

	// char backupmem[PCE_PATH_MAX];
	// snprintf(backupmem, PCE_PATH_MAX, "%s/backupmem.bin", config_basepath);

	// Save the backup ram into file
	// if (!(fp = fopen(backupmem, "wb"))) {
	// 	memset(WRAM, 0, 0x2000);
	// 	MESSAGE_ERROR("Can't open %s for saving RAM\n", backupmem);
	// } else {
	// 	fwrite(WRAM, 0x2000, 1, fp);
	// 	fclose(fp);
	// 	MESSAGE_INFO("%s used for saving RAM\n", backupmem);
	// }

	// Set volume to zero
	io.psg_volume = 0;

	if (IOAREA)
		free(IOAREA);

	if (ROM)
		free(ROM);

	if (PopRAM)
		free(PopRAM);

	hard_term();

	return;
}


#ifdef CHRONO
unsigned nb_used[256], time_used[256];
#endif

#ifndef FINAL_RELEASE
extern int mseq(unsigned *);
extern void mseq_end();
extern void WriteBuffer_end();
extern void write_psg_end();
extern void WriteBuffer(char *, int, unsigned);
#endif


FILE *out_snd;


#ifdef MY_VDC_VARS

// DRAM_ATTR
// WORD_ALIGNED_ATTR

#define VAR_PREFIX DRAM_ATTR WORD_ALIGNED_ATTR

VAR_PREFIX pair IO_VDC_00_MAWR ;
VAR_PREFIX pair IO_VDC_01_MARR ;
VAR_PREFIX pair IO_VDC_02_VWR  ;
VAR_PREFIX pair IO_VDC_03_vdc3 ;
VAR_PREFIX pair IO_VDC_04_vdc4 ;
VAR_PREFIX pair IO_VDC_05_CR   ;
VAR_PREFIX pair IO_VDC_06_RCR  ;
VAR_PREFIX pair IO_VDC_07_BXR  ;
VAR_PREFIX pair IO_VDC_08_BYR  ;
VAR_PREFIX pair IO_VDC_09_MWR  ;
VAR_PREFIX pair IO_VDC_0A_HSR  ;
VAR_PREFIX pair IO_VDC_0B_HDR  ;
VAR_PREFIX pair IO_VDC_0C_VPR  ;
VAR_PREFIX pair IO_VDC_0D_VDW  ;
VAR_PREFIX pair IO_VDC_0E_VCR  ;
VAR_PREFIX pair IO_VDC_0F_DCR  ;
VAR_PREFIX pair IO_VDC_10_SOUR ;
VAR_PREFIX pair IO_VDC_11_DISTR;
VAR_PREFIX pair IO_VDC_12_LENR ;
VAR_PREFIX pair IO_VDC_13_SATB ;
VAR_PREFIX pair IO_VDC_14      ;
VAR_PREFIX pair *IO_VDC_active_ref;
#endif
