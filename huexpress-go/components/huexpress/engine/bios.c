/***************************************************************************/
/*	                                                                       */
/*	                    hard BIOS support source file                      */
/*	                                                                       */
/* This file contain support for pc engine hudson cd bios without using	   */
/* the standard routines with ports but simulating directly the function.  */
/*	                                                                       */
/* You're welcome to help coding this piece of software, it's really	   */
/* interesting to discover precise use of each function by hacking the	   */
/* cd system and such ^^	                                               */
/***************************************************************************/

#include "h6280.h"
#include "globals.h"
#include "utils.h"
#include "pce.h"

#ifdef MY_EXCLUDE
void handle_bios(void)
{
    printf("handle_bios: not implemented!\n");
    abort();
}

#else

#define get_8bit_addr(addr) Rd6502((uint16)(addr))
#define put_8bit_addr(addr,byte) Wr6502((addr),(byte))

/* CD related functions */

#define CD_BOOT		0x00
#define CD_RESET	0x01
#define CD_BASE		0x02
#define CD_READ		0x03
#define CD_SEEK		0x04
#define CD_EXEC		0x05
#define CD_PLAY		0x06
#define CD_SEARCH	0x07
#define CD_PAUSE	0x08
#define CD_STAT		0x09
#define CD_SUBA		0x0A
#define CD_DINFO	0x0B

#define CD_DINFO_NB_TRACKS 0
#define CD_DINFO_LENGTH 1
#define CD_DINFO_TRACK 2

#define CD_CONTENTS 0x0C
#define CD_SUBRQ	0x0D
#define CD_PCMRD	0x0E
#define CD_FADE		0x0F

/* ADPCM related functions */

#define AD_RESET	0x10
#define AD_TRANS	0x11
#define AD_READ		0x12
#define AD_WRITE	0x13
#define AD_PLAY		0x14
#define AD_CPLAY	0x15
#define AD_STOP		0x16
#define AD_STAT		0x17

/* BACKUP MEM related functions */

#define BM_FORMAT	0x18
#define BM_FREE		0x19
#define BM_READ		0x1A
#define BM_WRITE	0x1B
#define BM_DELETE	0x1C
#define BM_FILES	0x1D

/* Miscelanous functions */

#define EX_GETVER	0x1E
#define EX_SETVEC	0x1F
#define EX_GETFNT	0x20
#define EX_JOYSNS	0x21
#define EX_JOYREP	0x22
#define EX_SCRSIZ	0x23
#define EX_DOTMOD	0x24
#define EX_SCRMOD	0x25
#define EX_IMODE	0x26
#define EX_VMOD		0x27
#define EX_HMOD		0x28
#define EX_VSYNC	0x29
#define EX_RCRON	0x2A
#define EX_RCROFF	0x2B
#define EX_IRQON	0x2C
#define EX_IRQOFF	0x2D
#define EX_BGON		0x2E
#define EX_BGOFF	0x2F
#define EX_SPRON	0x30
#define EX_SPROFF	0x31
#define EX_DSPON	0x32
#define EX_DSPOFF	0x33
#define EX_DMAMOD	0x34
#define EX_SPRDMA	0x35
#define EX_SATCLR	0x36
#define EX_SPRPUT	0x37
#define EX_SETRCR	0x38
#define EX_SETRED	0x39
#define EX_SETWRT	0x3A
#define EX_SETDMA	0x3B
#define EX_BINBCD	0x3C
#define EX_BCDBIN	0x3D
#define EX_RND		0x3E

/* Math related functions */

#define MA_MUL8U	0x3F
#define MA_MUL8S	0x40
#define MA_MUL16U	0x41
#define MA_DIV16U	0x42
#define MA_DIV16S	0x43
#define MA_SQRT		0x44
#define MA_SIN		0x45
#define MA_COS		0x46
#define MA_ATNI		0x47

/* PSG BIOS functions */

#define PSG_BIOS	0x48
#define GRP_BIOS	0x49
#define KEY_BIOS	0x4A
#define PSG_DRIVE	0x4B
#define EX_COLORC	0x4C


#define MA_MUL16S	0x4F
#define MA_CBASIS	0x50

#define _al 0xF8
#define _ah 0xF9
#define _bl 0xFA
#define _bh 0xFB
#define _cl 0xFC
#define _ch 0xFD
#define _dl 0xFE
#define _dh 0xFF

#define _ax 0xF8
#define _bx 0xFA
#define _cx 0xFC
#define _dx 0xFE


int testadpcm = 0;

#if ENABLE_TRACING_BIOS
const char *
cdbios_functions(int index)
{
	switch (index) {
	case CD_BOOT:
		return "CD_BOOT";
	case CD_RESET:
		return "CD_RESET";
	case CD_BASE:
		return "CD_BASE";
	case CD_READ:
		return "CD_READ";
	case CD_SEEK:
		return "CD_SEEK";
	case CD_EXEC:
		return "CD_EXEC";
	case CD_PLAY:
		return "CD_PLAY";
	case CD_SEARCH:
		return "CD_SEARCH";
	case CD_PAUSE:
		return "CD_PAUSE";
	case CD_STAT:
		return "CD_STAT";
	case CD_SUBA:
		return "CD_SUBA";
	case CD_DINFO:
		return "CD_DINFO";
	case CD_CONTENTS:
		return "CD_CONTENTS";
	case CD_SUBRQ:
		return "CD_SUBRQ";
	case CD_PCMRD:
		return "CD_PCMRD";
	case CD_FADE:
		return "CD_FADE";

	case AD_RESET:
		return "AD_RESET";
	case AD_TRANS:
		return "AD_TRANS";
	case AD_READ:
		return "AD_READ";
	case AD_WRITE:
		return "AD_WRITE";
	case AD_PLAY:
		return "AD_PLAY";
	case AD_CPLAY:
		return "AD_CPLAY";
	case AD_STOP:
		return "AD_STOP";
	case AD_STAT:
		return "AD_STAT";

	case BM_FORMAT:
		return "BM_FORMAT";
	case BM_FREE:
		return "BM_FREE";
	case BM_READ:
		return "BM_READ";
	case BM_WRITE:
		return "BM_WRITE";
	case BM_DELETE:
		return "BM_DELETE";
	case BM_FILES:
		return "BM_FILES";

	case EX_GETVER:
		return "EX_GETVER";
	case EX_SETVEC:
		return "EX_SETVEC";
	case EX_GETFNT:
		return "EX_GETFNT";
	case EX_JOYSNS:
		return "EX_JOYSNS";
	case EX_JOYREP:
		return "EX_JOYREP";

	case EX_SCRSIZ:
		return "EX_SCRSIZ";
	case EX_DOTMOD:
		return "EX_DOTMOD";
	case EX_SCRMOD:
		return "EX_SCRMOD";
	case EX_IMODE:
		return "EX_IMODE";
	case EX_VMOD:
		return "EX_VMOD";
	case EX_HMOD:
		return "EX_HMOD";
	case EX_VSYNC:
		return "EX_VSYNC";
	case EX_RCRON:
		return "EX_RCRON";
	case EX_RCROFF:
		return "EX_RCROFF";
	case EX_IRQON:
		return "EX_IRQON";
	case EX_IRQOFF:
		return "EX_IRQOFF";
	case EX_BGON:
		return "EX_BGON";
	case EX_BGOFF:
		return "EX_BGOFF";
	case EX_SPRON:
		return "EX_SPRON";
	case EX_SPROFF:
		return "EX_SPROFF";
	case EX_DSPON:
		return "EX_DSPON";
	case EX_DSPOFF:
		return "EX_DSPOFF";
	case EX_DMAMOD:
		return "EX_DMAMOD";
	case EX_SPRDMA:
		return "EX_SPRDMA";
	case EX_SATCLR:
		return "EX_SATCLR";
	case EX_SPRPUT:
		return "EX_SPRPUT";
	case EX_SETRCR:
		return "EX_SETRCR";
	case EX_SETRED:
		return "EX_SETRED";
	case EX_SETWRT:
		return "EX_SETWRT";
	case EX_SETDMA:
		return "EX_SETDMA";
	case EX_BINBCD:
		return "EX_BINBCD";
	case EX_BCDBIN:
		return "EX_BCDBIN";
	case EX_RND:
		return "EX_RND";

	case MA_MUL8U:
		return "MA_MUL8U";
	case MA_MUL8S:
		return "MA_MUL8S";
	case MA_MUL16U:
		return "MA_MUL16U";
	case MA_DIV16S:
		return "MA_DIV16S";
	case MA_DIV16U:
		return "MA_DIV16U";
	case MA_SQRT:
		return "MA_SQRT";
	case MA_SIN:
		return "MA_SIN";
	case MA_COS:
		return "MA_COS";
	case MA_ATNI:
		return "MA_ATNI";

	case PSG_BIOS:
		return "PSG_BIOS";

	case GRP_BIOS:
		return "GRP_BIOS";
	case KEY_BIOS:
		return "KEY_BIOS";
	case PSG_DRIVE:
		return "PSG_DRIVE";
	case EX_COLORC:
		return "EX_COLORC";

	default:
		break;
	}

	return "?UNKNOWN?";
}
#endif


int
handle_bios()
{
#if ENABLE_TRACING_BIOS
	static int last_op = -1, last_ax = -1, last_bx = -1, last_cx =
		-1, last_dx = -1;
	int this_op = imm_operand_(reg_pc + 1), this_ax =
		get_16bit_zp_(_ax), this_bx = get_16bit_zp_(_bx), this_cx =
		get_16bit_zp_(_cx), this_dx = get_16bit_zp_(_dx);

	/*
	 * Skip over polling functions to avoid the spam
	 */
	if ((this_op != CD_PCMRD) && (this_op != CD_SUBA)
		&& (this_op != EX_JOYSNS) && (this_op != AD_STAT)) {
		if ((last_op != this_op) || (last_ax != this_ax)
			|| (last_bx != this_bx)
			|| (last_cx != this_cx)
			|| (last_dx != this_dx)) {
			TRACE("%s: ax=%d ah=%d al=%d bx=%d bh=%d bl=%d\ncx=%d "
				  "ch=%d cl=%d dx=%d dh=%d dl=%d\n",
				  cdbios_functions(imm_operand_(reg_pc + 1)),
				  get_16bit_zp_(_ax), get_8bit_zp_(_ah), get_8bit_zp_(_al),
				  get_16bit_zp_(_bx), get_8bit_zp_(_bh), get_8bit_zp_(_bl),
				  get_16bit_zp_(_cx), get_8bit_zp_(_ch), get_8bit_zp_(_cl),
				  get_16bit_zp_(_dx), get_8bit_zp_(_dh), get_8bit_zp_(_dl));
			last_op = this_op;
			last_ax = this_ax;
			last_bx = this_bx;
			last_cx = this_cx;
			last_dx = this_dx;
		}
	}
#endif

	switch (imm_operand_((uint16) (reg_pc + 1))) {
	case CD_RESET:
		switch (CD_emulation) {
		case 1:				/* true CD */
			if (osd_cd_init(ISO_filename) != 0) {
				MESSAGE_ERROR("CDRom2: Init error!\n");
				exit(4);
			}
			break;
		case 2:				/* ISO */
		case 3:				/* ISQ */
		case 4:				/* BIN */
			fill_cd_info();
			break;
			/* HCD : nothing to be done */
		}

		put_8bit_addr(0x222D, 1);
		// This byte is set to 1 if a disc if present

		rts();
		break;

	case CD_READ:
		{
			uchar mode = get_8bit_zp_(_dh);
			uint32 nb_to_read = get_8bit_zp_(_al);
			uint16 offset = get_16bit_zp_(_bx);

			pce_cd_sectoraddy = (get_8bit_zp_(_cl) << 16)
				+ (get_8bit_zp_(_ch) << 8)
				+ (get_8bit_zp_(_dl));

			pce_cd_sectoraddy += (get_8bit_addr(0x2274 + 3
												*
												get_8bit_addr(0x2273)) <<
								  16)
				+ (get_8bit_addr(0x2275 + 3 * get_8bit_addr(0x2273)) << 8)
				+ (get_8bit_addr(0x2276 + 3 * get_8bit_addr(0x2273)));

			switch (mode) {
			case 0:			// local, size in byte
				nb_to_read = get_16bit_zp_(_ax);
				while (nb_to_read >= 2048) {
					int index;

					pce_cd_read_sector();
					for (index = 0; index < 2048; index++)
						put_8bit_addr(offset++, cd_read_buffer[index]);
					nb_to_read -= 2048;
				}

				if (nb_to_read) {
					uint32 index;

					pce_cd_read_sector();
					for (index = 0; index < nb_to_read; index++)
						put_8bit_addr(offset++, cd_read_buffer[index]);
				}

				reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
						 | flnz_list[reg_a = 0]);

				cd_sectorcnt = 0;
				cd_read_buffer = NULL;
				pce_cd_read_datacnt = 0;

				rts();
				break;


			case 1:			// local, size in sector
				while (nb_to_read) {
					int index;

					pce_cd_read_sector();
					for (index = 0; index < 2048; index++)
						put_8bit_addr(offset++, cd_read_buffer[index]);
					nb_to_read--;
				}
				reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
						 | flnz_list[reg_a = 0]);

				// TEST
				io.cd_port_1800 |= 0xD0;
				// TEST

				cd_sectorcnt = 0;
				cd_read_buffer = NULL;
				pce_cd_read_datacnt = 0;

				rts();
				break;

			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				{
					uchar nb_bank_to_fill_completely = nb_to_read >> 2;
					uchar remaining_block_to_write = nb_to_read & 3;
					uchar bank_where_to_write = get_8bit_zp_(_bl);
					uint16 offset_in_bank = 0;

					while (nb_bank_to_fill_completely--) {
						pce_cd_read_sector();
						memcpy(ROMMapW[bank_where_to_write],
							   cd_read_buffer, 2048);

						pce_cd_read_sector();
						memcpy(ROMMapW[bank_where_to_write] + 2048,
							   cd_read_buffer, 2048);

						pce_cd_read_sector();
						memcpy(ROMMapW[bank_where_to_write] + 2048 * 2,
							   cd_read_buffer, 2048);

						pce_cd_read_sector();
						memcpy(ROMMapW[bank_where_to_write] + 2048 * 3,
							   cd_read_buffer, 2048);

						bank_where_to_write++;
					}

					offset_in_bank = 0;
					while (remaining_block_to_write--) {
						pce_cd_read_sector();
#if ENABLE_TRACING_BIOS
						TRACE
							("BIOS: Writing quarter to ROMMap[0x%x] + 0x%x\n",
							 bank_where_to_write, offset_in_bank);
#endif
						memcpy(ROMMapW[bank_where_to_write] +
							   offset_in_bank, cd_read_buffer, 2048);
						offset_in_bank += 2048;
					}
				}

				cd_sectorcnt = 0;
				cd_read_buffer = NULL;
				pce_cd_read_datacnt = 0;

				reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
						 | flnz_list[reg_a = 0]);
				rts();
				break;

			case 0xFE:
				IO_write(0, 0);
				IO_write(2, (uchar) (offset & 0xFF));
				IO_write(3, (uchar) (offset >> 8));
				IO_write(0, 2);
				{
					uint32 nb_sector;
					nb_to_read = get_16bit_zp_(_ax);
					nb_sector = (nb_to_read >> 11)
						+ ((nb_to_read & 2047) ? 1 : 0);

					while (nb_sector) {
						int x, index = MIN(2048, (int) nb_to_read);

						pce_cd_read_sector();

						// memcpy(&VRAM[offset],cd_read_buffer,index);
						for (x = 0; x < index; x += 2) {
							IO_write(2, cd_read_buffer[x]);
							IO_write(3, cd_read_buffer[x + 1]);
						}

						nb_to_read -= index;
						nb_sector--;
					}

					cd_sectorcnt = 0;
					cd_read_buffer = NULL;
					pce_cd_read_datacnt = 0;

					reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
							 | flnz_list[reg_a = 0]);
					rts();
				}
				break;

			case 0xFF:
				if (!nb_to_read)
					reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
							 | flnz_list[reg_a = 0x22]);
				else {
					IO_write(0, 0);
					IO_write(2, (uchar) (offset & 0xFF));
					IO_write(3, (uchar) (offset >> 8));
					IO_write(0, 2);

					while (nb_to_read) {
						int index;

						pce_cd_read_sector();

						for (index = 0; index < 2048; index += 2) {
							IO_write(2, cd_read_buffer[index]);
							IO_write(3, cd_read_buffer[index + 1]);
						}

						nb_to_read--;
					}

					cd_sectorcnt = 0;
					cd_read_buffer = NULL;
					pce_cd_read_datacnt = 0;

					reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
							 | flnz_list[reg_a = 0]);
				}
				rts();
				break;

			default:
				/* the reading mode isn't supported and we simulate the
				 * behaviour of the 2 first byte opcode and keep going
				 * thus hooking further calls to this function
				 */
				put_8bit_addr(0x2273, 0);
				reg_pc += 2;
#if ENABLE_TRACING_BIOS
				TRACE("BIOS: CDRom2: Reading mode not supported:"
					  " %d\n_AX=0x%04x\n_BX=0x%04x\n_CX=0x%04x\n_DX=0x%04x\n",
					  mode, get_16bit_zp_(_ax), get_16bit_zp_(_bx),
					  get_16bit_zp_(_cx), get_16bit_zp_(_dx));
#endif
			}
		}
		break;

	case CD_PAUSE:
		switch (CD_emulation) {
		case 1:
			osd_cd_pause();
			break;
		case 2:
		case 3:
		case 4:
			break;
		case 5:
			HCD_pause_playing();
			break;
		}
		reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
		rts();
		break;

	case CD_STAT:
		{
			/* TODO : makes this function work for cd and hcd at least
			 * gives info on the status of playing music
			 */
			int retval;

			osd_cd_status(&retval);

			reg_p =
				((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
		}
		rts();
		break;

	case CD_SUBA:
		/* TODO : check the real functionality of this function */
		/* seems to fill a whole buffer of 10 bytes but */
		/* meaning of this array is mostly unknown */
		{
			uint16 offset = get_16bit_zp_(_bx);
			// static uchar result = 3;

			// result = 3 - result;

			// Wr6502(offset, result);
			// TEST, for golden axe (3) and solid force (0)
			osd_cd_subchannel_info(offset);
		}

		reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
		rts();
		break;

	case CD_PCMRD:
		// do almost nothing
		// fake the audio player, maybe not other piece of code
		reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
				 | flnz_list[reg_a = get_8bit_zp_(0x41)]);
		rts();
		break;

	case CD_SEARCH:
		/* unsure how this operates
		 * needed for playing audio discs with a system card
		 *
		 * _al contains the track we're "searching" for.
		 * If I'm not mistaken _bh is a flag with that 7th bit (128) set
		 * for SEEK_SET type behaviour, while if the 2nd bit (2) is set
		 * we play the track after searching for it.
		 */
		{
			//  if (get_8bit_zp_(_bh) & 0x02) {
			//      osd_cd_play_audio_track(bcdbin[get_8bit_zp_(_al)]);
			//  } else
			/* uint16 bufaddr = get_16bit_zp_(_bx); */
			int min, sec, fra, con;

			osd_cd_stop_audio();

			osd_cd_track_info(bcdbin[get_8bit_zp_(_al)],
							  &min, &sec, &fra, &con);

			/*
			   put_8bit_addr(bufaddr, min);
			   put_8bit_addr(bufaddr + 1, sec);
			   put_8bit_addr(bufaddr + 2, fra);
			   put_8bit_addr(bufaddr + 3, con);
			 */

			if (get_8bit_zp_(_bh) & 0x02)
				osd_cd_play_audio_track(bcdbin[get_8bit_zp_(_al)]);
			// else
			// osd_cd_stop_audio();
		}
		reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
				 | flnz_list[reg_a = 0]);
		rts();
		break;

	case AD_RESET:
		// do nothing
		// don't return any value
		// reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
		rts();
		break;

	case AD_TRANS:
		{
			uint32 nb_to_read = get_8bit_zp_(_al);
			uint16 ADPCM_offset = get_16bit_zp_(_bx);

			pce_cd_sectoraddy = (get_8bit_zp_(_cl) << 16)
				+ (get_8bit_zp_(_ch) << 8)
				+ (get_8bit_zp_(_dl));

			pce_cd_sectoraddy
				+=
				(get_8bit_addr(0x2274 + 3 * get_8bit_addr(0x2273)) << 16)
				+ (get_8bit_addr(0x2275 + 3 * get_8bit_addr(0x2273)) << 8)
				+ (get_8bit_addr(0x2276 + 3 * get_8bit_addr(0x2273)));

			if (!get_8bit_zp_(_dh))
				io.adpcm_dmaptr = ADPCM_offset;
			else
				ADPCM_offset = io.adpcm_dmaptr;

			while (nb_to_read) {
				pce_cd_read_sector();
				memcpy(PCM + ADPCM_offset, cd_read_buffer, 2048);
				ADPCM_offset += 2048;
				nb_to_read--;
			}

			io.adpcm_dmaptr = ADPCM_offset;

			reg_p =
				((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
		}
		rts();
		break;

	case AD_READ:
		{
			uint16 ADPCM_buffer = get_16bit_zp_(_cx);
			uchar type = get_8bit_zp_(_dh);
			uint16 address = get_16bit_zp_(_bx);
			uint16 size = get_16bit_zp_(_ax);

			switch (type) {
			case 0:			// memory write
				io.adpcm_rptr = ADPCM_buffer;
				while (size) {
					put_8bit_addr(address++, PCM[io.adpcm_rptr++]);
					size--;
				}
				break;
			case 0xFF:			// VRAM write
				io.adpcm_rptr = ADPCM_buffer;

				IO_write(0, 0);
				IO_write(2, (uchar) (address & 0xFF));
				IO_write(3, (uchar) (address >> 8));

				IO_write(0, 2);

				while (size) {
					IO_write(2, PCM[io.adpcm_rptr++]);
					size--;

					if (size) {
						IO_write(3, PCM[io.adpcm_rptr++]);
						size--;
					}
				}
				break;
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				{
					uchar bank_to_fill = get_8bit_zp_(_bl);
					uint32 i;

					while (size >= 2048) {
						for (i = 0; i < 2048; i++)
							ROMMapW[bank_to_fill][i] =
								PCM[io.adpcm_rptr++];

						bank_to_fill++;

						size -= 2048;
					}

					for (i = 0; i < size; i++)
						ROMMapW[bank_to_fill][i] = PCM[io.adpcm_rptr++];

				}
				break;
			default:
				TRACE("BIOS: Type reading not supported in AD_READ : %x\n",
					  type);
				exit(-2);
			}
			reg_p =
				((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
			rts();
		}
		break;

	case AD_PLAY:
		io.adpcm_pptr = get_16bit_zp_(_bx) << 1;

		io.adpcm_psize = get_16bit_zp_(_ax) << 1;

		io.adpcm_rate = (uchar) (32 / (16 - (get_8bit_zp_(_dh) & 15)));

		new_adpcm_play = 1;

		reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
		rts();
		break;

	case AD_STOP:
		AdpcmFilledBuf = new_adpcm_play = 0;
		rts();
		break;

	case AD_STAT:
		{

			if (AdpcmFilledBuf > (io.adpcm_psize / 2))
				reg_x = 4;
			else if (AdpcmFilledBuf == 0)
				reg_x = 1;
			else
				reg_x = 0;

			reg_p =
				((reg_p & (~(FL_N | FL_T | FL_Z))) |
				 flnz_list[reg_a = (uchar) (reg_x == 1 ? 0 : 1)]);
		}
		rts();
		break;

	case CD_DINFO:
		switch (get_8bit_zp_(_al)) {
		case CD_DINFO_TRACK:
			{
				uint16 buf_offset = get_16bit_zp_(_bx);
				// usually 0x2256 in system 3.0
				// _ah contain the number of the track

				switch (CD_emulation) {
				case 2:
				case 3:
				case 4:
				case 5:
					put_8bit_addr((uint16) buf_offset,
								  CD_track[bcdbin[get_8bit_zp_(_ah)]].
								  beg_min);
					put_8bit_addr((uint16) (buf_offset + 1),
								  CD_track[bcdbin[get_8bit_zp_(_ah)]].
								  beg_sec);
					put_8bit_addr((uint16) (buf_offset + 2),
								  CD_track[bcdbin[get_8bit_zp_(_ah)]].
								  beg_fra);
					put_8bit_addr((uint16) (buf_offset + 3),
								  CD_track[bcdbin[get_8bit_zp_(_ah)]].type);
					break;
				case 1:
					{
						int Min, Sec, Fra, Ctrl;

						osd_cd_track_info(bcdbin[get_8bit_zp_(_ah)],
										  &Min, &Sec, &Fra, &Ctrl);

						put_8bit_addr((uint16) (buf_offset), binbcd[Min]);
						put_8bit_addr((uint16) (buf_offset + 1),
									  binbcd[Sec]);
						put_8bit_addr((uint16) (buf_offset + 2),
									  binbcd[Fra]);
						put_8bit_addr((uint16) (buf_offset + 3),
									  (uchar) Ctrl);
					}
					break;
				}
				reg_p =
					((reg_p & (~(FL_N | FL_T | FL_Z))) |
					 flnz_list[reg_a = 0]);
			}
			rts();
			break;

		case CD_DINFO_NB_TRACKS:
			{
#if ENABLE_TRACING_BIOS
				TRACE("BIOS: Request for number of CDRom2 tracks\n");
#endif
				uint16 buf_offset = get_16bit_zp_(_bx);

				switch (CD_emulation) {
				case 2:
				case 3:
				case 4:
					put_8bit_addr((uint16) (buf_offset), binbcd[01]);
					// Number of first track    (BCD)
					put_8bit_addr((uint16) (buf_offset + 1),
								  binbcd[nb_max_track]);
					// Number of last track (BCD)
					break;
				case 1:
					{
						int first_track, last_track;

						osd_cd_nb_tracks(&first_track, &last_track);

						put_8bit_addr((uint16) (buf_offset),
									  binbcd[first_track]);
						put_8bit_addr((uint16) (buf_offset + 1),
									  binbcd[last_track]);
					}
					break;
				case 5:
					put_8bit_addr((uint16) (buf_offset),
								  binbcd[HCD_first_track]);
					put_8bit_addr((uint16) (buf_offset + 1),
								  binbcd[HCD_last_track]);
					break;
				}
			}
			reg_p =
				((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
			rts();
			break;

		case CD_DINFO_LENGTH:
			{
				uint16 buf_offset = get_16bit_zp_(_bx);
				int min, sec, frame;

				osd_cd_length(&min, &sec, &frame);
#if ENABLE_TRACING_BIOS
				TRACE("BIOS: CD length - Min: %d, Sec: %d, Frame: %d\n",
					  min, sec, frame);
#endif

				put_8bit_addr((uint16) (buf_offset), binbcd[min]);
				put_8bit_addr((uint16) (buf_offset + 1), binbcd[sec]);
				put_8bit_addr((uint16) (buf_offset + 2), binbcd[frame]);

				reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z)))
						 | flnz_list[reg_a = 0]);
			}
			rts();
			break;

		default:
			TRACE("BIOS: Sub function 0X%02X from CD_DINFO not handled\n",
				  get_8bit_zp_(_al));
			reg_p =
				((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 1]);
			rts();
			break;
		}
		break;

	case CD_PLAY:
		if (get_8bit_zp_(_bh) == 0x80) {
			int status;

			// playing a whole track

			switch (CD_emulation) {
			case 1:
				osd_cd_status(&status);

				if (status == CDROM_AUDIO_PAUSED)
					osd_cd_resume();
				else if (status == CDROM_AUDIO_PLAY)
					osd_cd_stop_audio();

				osd_cd_play_audio_track(bcdbin[get_8bit_zp_(_al)]);
				break;

			case 2:
			case 3:
			case 4:
				// ignoring cd playing
				break;
			case 5:
				HCD_play_track(bcdbin[get_8bit_zp_(_al)],
							   (SBYTE) (get_8bit_zp_(_dh) & 1));
				break;
			}
		} else if (get_8bit_zp_(_bh) == 192) {	/* resume from pause if paused */
			int status;

			osd_cd_status(&status);

			if (status == CDROM_AUDIO_PAUSED)
				osd_cd_resume();
			else
				osd_cd_play_audio_track(bcdbin[get_8bit_zp_(_al)]);

		} else {
			int status;

			int min1 = bcdbin[get_8bit_zp_(_al)];
			int sec1 = bcdbin[get_8bit_zp_(_ah)];
			int fra1 = bcdbin[get_8bit_zp_(_bl)];

			int min2 = bcdbin[get_8bit_zp_(_cl)];
			int sec2 = bcdbin[get_8bit_zp_(_ch)];
			int fra2 = bcdbin[get_8bit_zp_(_dl)];

			switch (CD_emulation) {
			case 1:
				osd_cd_status(&status);
				if ((status == CDROM_AUDIO_PLAY)
					|| (status == CDROM_AUDIO_PAUSED))
					osd_cd_stop_audio();
				osd_cd_play_audio_range((uchar) min1, (uchar) sec1,
										(uchar) fra1, (uchar) min2,
										(uchar) sec2, (uchar) fra2);
				break;
			case 2:
			case 3:
			case 4:
				// ignoring cd playing
				break;
			case 5:
				//                  HCD_play_sectors(begin_sect, sect_len, get_8bit_zp_(_dh) & 1);
				break;
			}
		}
		reg_p = ((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
		rts();
		break;

	case EX_JOYSNS:
		{
			uchar dummy[5], index;

			for (index = 0; index < 5; index++) {
				dummy[index] = get_8bit_addr(0x2228 + index);
				put_8bit_addr((uint16) (0x2232 + index), dummy[index]);
				put_8bit_addr((uint16) (0x2228 + index), io.JOY[index]);
				put_8bit_addr((uint16) (0x222D + index),
							  (io.JOY[index] ^ dummy[index]) & io.
							  JOY[index]);
			}
		}
		/* TODO : check if A <- 0 is needed here */
		rts();
		break;

	case BM_FREE:
		{
			Sint16 free_mem;

			free_mem = (Sint16) (WRAM[4] + (WRAM[5] << 8));
			free_mem -= WRAM[6] + (WRAM[7] << 8);
			free_mem -= 0x12;	/* maybe the header */

			if (free_mem < 0)
				free_mem = 0;

			put_8bit_zp_(_cl, (uchar) (free_mem & 0xFF));
			put_8bit_zp_(_ch, (uchar) (free_mem >> 8));

			reg_p =
				((reg_p & (~(FL_N | FL_T | FL_Z))) | flnz_list[reg_a = 0]);
			rts();
			break;
		}

#if ENABLE_TRACING_BIOS
	case MA_MUL8U:
		{
			uint16 res;

			res = get_8bit_zp_(_ax) * get_8bit_zp_(_bx);

			put_8bit_zp_(_cl, res & 0xFF);
			put_8bit_zp_(_ch, res >> 8);

			rts();
			break;
		}

	case MA_MUL8S:
		{
			Sint16 res;

			res = get_8bit_zp_(_ax) * get_8bit_zp_(_bx);

			put_8bit_zp_(_cl, res & 0xFF);
			put_8bit_zp_(_ch, res >> 8);

			rts();
			break;
		}

	case MA_MUL16U:
		{
			uint32 res;

			res = get_16bit_zp_(_ax) * get_16bit_zp_(_bx);

			put_8bit_zp_(_cl, res & 0xFF);
			put_8bit_zp_(_ch, (res >> 8) & 0xFF);
			put_8bit_zp_(_dl, (res >> 16) & 0xFF);
			put_8bit_zp_(_dh, (res >> 24) & 0xFF);

			rts();
			break;
		}

	case MA_DIV16U:
		{
			uint16 res, rem;

			res = get_16bit_zp_(_ax) / get_16bit_zp_(_bx);
			rem = get_16bit_zp_(_ax) % get_16bit_zp_(_bx);

			put_8bit_zp_(_cl, res & 0xFF);
			put_8bit_zp_(_ch, res >> 8);
			put_8bit_zp_(_dl, rem & 0xFF);
			put_8bit_zp_(_dh, res >> 8);

			rts();
			break;
		}

	case MA_DIV16S:
		{
			Sint16 res, rem;

			res = get_16bit_zp_(_ax) / get_16bit_zp_(_bx);
			rem = get_16bit_zp_(_ax) % get_16bit_zp_(_bx);

			put_8bit_zp_(_cl, res & 0xFF);
			put_8bit_zp_(_ch, res >> 8);
			put_8bit_zp_(_dl, rem & 0xFF);
			put_8bit_zp_(_dh, rem >> 8);

			rts();
			break;
		}

	case MA_MUL16S:
		{
			int32 res;

			res = get_16bit_zp_(_ax) * get_16bit_zp_(_bx);

			put_8bit_zp_(_cl, res & 0xFF);
			put_8bit_zp_(_ch, (res >> 8) & 0xFF);
			put_8bit_zp_(_dl, (res >> 16) & 0xFF);
			put_8bit_zp_(_dh, (res >> 24) & 0xFF);

			rts();
			break;
		}
#endif

	default:
		/* unhandled function, restoring initial behaviour */
		put_8bit_addr((uint16) (reg_pc),
					  CDBIOS_replace[imm_operand_((uint16) (reg_pc + 1))]
					  [0]);
		put_8bit_addr((uint16) (reg_pc + 1),
					  CDBIOS_replace[imm_operand_((uint16) (reg_pc + 1))]
					  [1]);
	}
	return 0;
}

#endif
