/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/***************************************************************************
*	NAME:  GF1.H
**	COPYRIGHT:
**	"Copyright (c) 1991,1992, by FORTE
**
**       "This software is furnished under a license and may be used,
**       copied, or disclosed only in accordance with the terms of such
**       license and with the inclusion of the above copyright notice.
**       This software or any other copies thereof may not be provided or
**       otherwise made available to any other person. No title to and
**       ownership of the software is hereby transfered."
****************************************************************************
*  CREATION DATE: 07/01/92
*--------------------------------------------------------------------------*
*     VERSION	DATE	   NAME		DESCRIPTION
*>	1.0	07/01/92		Original
***************************************************************************/

#ifndef	_GF1_H		/* allow header to be processed only once */
#define _GF1_H

/* error codes */
#define OK			0
#define NO_MORE_VOICES		-1
#define	BASE_NOT_FOUND		1
#define BAD_IRQ			2
#define BAD_DMA			3
#define OS_LOADED		4
#define NOT_LOADED		5
#define NO_MEMORY		6
#define DMA_BUSY		7
#define NO_MORE_HANDLERS	8
#define DMA_HUNG		9
#define CARD_NOT_FOUND		10
#define CARD_BEING_USED		11
#define NO_MORE_INTERRUPTS	12
#define BAD_TIMER		13
#define BAD_PATCH		14
#define OLD_PATCH		15
#define DOS_ERROR		16
#define FILE_NOT_FOUND		17

/* bits */
#define	BIT0	0x01
#define	BIT1	0x02
#define	BIT2	0x04
#define	BIT3	0x08
#define	BIT4	0x10
#define	BIT5	0x20
#define	BIT6	0x40
#define	BIT7	0x80

/* bounds for volume enveloping functions */
#define MIN_OFFSET      5U
#define MAX_OFFSET      251U

/* bounds for voice allocation */
#define MIN_VOICES	14
#define MAX_VOICES	32

/* DMA control bits */
#define DMA_ENABLE		BIT0
#define DMA_READ		BIT1
#define DMA_WIDTH_16		BIT2 /* width of DMA channel */
#define DMA_RATE_DIV_1		BIT3
#define DMA_RATE_DIV_2		BIT4
#define DMA_IRQ_ENABLE		BIT5
#define DMA_IRQ_PRESENT		BIT6
#define DMA_DATA_16		BIT6 /* width of data */
#define DMA_INVERT_MSB		BIT7

/* SAMPLE control bits */
#define DMA_STEREO		2

/* DMA flags */
#define GF1_RECORD	0 /* use dma control or sample control */
#define GF1_DMA		1

/* MIDI control register */
#define MIDI_RESET	(BIT0|BIT1)
#define MIDI_TD_INT	BIT5
#define MIDI_RD_INT	BIT7

/* MIDI_STATUS_REGISTER */
#define MIDI_RD		BIT0
#define MIDI_TD		BIT1
#define MIDI_ERR_FRAMING BIT4
#define MIDI_ERR_OVERRUN BIT5

/* digital playback flags */
#define TYPE_8BIT	BIT0	/* 1 use 8 bit data */
				/* 0 use 16 bit data */
#define TYPE_PRELOAD	BIT1	/* preload data */
#define TYPE_INVERT_MSB BIT2	/* invert most significant bit during dma */
#define TYPE_STEREO	BIT3	/* 1 for stereo data */

/* sound effects and digital music types */
#define SND_LOOP_MASK		(BIT0|BIT1)
#define SND_LOOP_NONE		0
#define SND_LOOP		1
#define SND_LOOP_BIDIR		2
#define SND_8BIT		(BIT2)
#define SND_BACKWARD		(BIT3)

#define SOUND_PLAYING		2
#define SOUND_ACTIVE		1

/* patch macros */
#define HEADER_SIZE	           12
#define ID_SIZE		           10
#define DESC_SIZE 	           60
#define RESERVED_SIZE	           40
#define PATCH_HEADER_RESERVED_SIZE 36
#define LAYER_RESERVED_SIZE	   40
#define PATCH_DATA_RESERVED_SIZE   36
#define GF1_HEADER_TEXT            "GF1PATCH110"
#define INST_NAME_SIZE		   16
#define ENVELOPES		   6
#define MAX_LAYERS		   4

/* patch modes */
#define PATCH_16		BIT0
#define PATCH_UNSIGNED		BIT1
#define PATCH_LOOPEN		BIT2
#define PATCH_BIDIR		BIT3
#define PATCH_BACKWARD  	BIT4
#define PATCH_SUSTAIN   	BIT5
#define PATCH_NO_SRELEASE	BIT6
#define PATCH_FAST_REL		BIT7

/* flags for patch loading */
#define PATCH_LOAD_8_BIT BIT0

/* digital playback callback reasons & return values */
#define	DIG_DONE	   0
#define DIG_MORE_DATA	   1
#define DIG_BUFFER_DONE    2
#define DIG_PAUSE          3

/* log table used for vibrato and pitch bend.  log table made public for
** developers use */
#define LOG_TAB_SIZE 12
extern long gf1_log_table[LOG_TAB_SIZE];

#if defined(__BORLANDC__)
#undef RFAR
#define RFAR far
#elif defined(_MSC_VER) && (_MSC_VER <= 600)
#define RFAR far
#elif defined(_MSC_VER) && (_MSC_VER > 600)
#define RFAR __far
#else
#undef RFAR
#define RFAR
#endif

/* structure definitions */
struct	load_os	
{
	unsigned short 	voices;
	unsigned short	forced_base_port;
	unsigned char	forced_gf1_irq;
	unsigned char	forced_midi_irq;
	unsigned char	forced_channel_in;
	unsigned char	forced_channel_out;
};

struct	patchheader
{
	char		header[ HEADER_SIZE ];	
	char		gravis_id[ ID_SIZE ];	/* Id = "ID#000002" */
	char		description[ DESC_SIZE ];
	unsigned char	instruments;
	char		voices;
	char		channels;
	unsigned short	wave_forms;
	unsigned short	master_volume;
	unsigned long	data_size;
	char		reserved[ PATCH_HEADER_RESERVED_SIZE ];
};

struct	instrumentdata
{
	unsigned short	instrument;
	char		instrument_name[ INST_NAME_SIZE ];
	long		instrument_size;
	char		layers;
	char		reserved[ RESERVED_SIZE ];	
};

struct	layerdata
{
	char		layer_duplicate;
	char		layer;
	long		layer_size;
	char		samples;
	char		reserved[ LAYER_RESERVED_SIZE ];	
};

struct	patchdata
{
	char		wave_name[7];
	unsigned char	fractions;
	long		wave_size;
	long		start_loop;
	long		end_loop;
	unsigned short	sample_rate;
	long		low_frequency;
	long		high_frequency;
	long		root_frequency;
	short		tune;
	unsigned char	balance;
	unsigned char	envelope_rate[ ENVELOPES ];
	unsigned char	envelope_offset[ ENVELOPES ];	
	unsigned char	tremolo_sweep;
	unsigned char	tremolo_rate;
	unsigned char	tremolo_depth;
	unsigned char	vibrato_sweep;
	unsigned char	vibrato_rate;
	unsigned char	vibrato_depth;
	char		modes;
	short		scale_frequency;
	unsigned short	scale_factor;		/* from 0 to 2048 or 0 to 2 */
	char		reserved[ PATCH_DATA_RESERVED_SIZE ];
};

struct wave_struct
{
	unsigned long		start_loop;
	unsigned long		end_loop;
	long		low_frequency;
	long		high_frequency;
	long		root_frequency;
	unsigned long	mem;
	unsigned short	scale_frequency;
	unsigned short	sample_rate;
	unsigned short	scale_factor;
	unsigned short	start_acc_low;
	unsigned short	start_acc_high;
	unsigned short	start_low;
	unsigned short	start_high;
	unsigned short	end_low;
	unsigned short	end_high;
	unsigned short	end_acc_low;
	unsigned short	end_acc_high;
	unsigned short	sample_ratio;
	unsigned long	wave_size;
	unsigned char	fractions;
	unsigned char	balance;
	unsigned char	envelope_rate[ ENVELOPES ];
	unsigned char	envelope_offset[ ENVELOPES ];	
	unsigned char	tremolo_sweep;
	unsigned char	tremolo_rate;
	unsigned char	tremolo_depth;
	unsigned char	vibrato_sweep;
	unsigned char	vibrato_rate;
	unsigned char	vibrato_depth;
	unsigned char	modes;
};

struct patchinfo {
	struct patchheader header;
	struct instrumentdata idata;
};

struct patch {
	short nlayers;
	struct wave_struct RFAR *layer_waves[MAX_LAYERS];
	short layer_nwaves[MAX_LAYERS];
	unsigned short detune;
};

struct gf1_dma_buff {
	unsigned char RFAR *vptr;
	unsigned long paddr;
};

struct gf1_sound {
	unsigned long mem_pos;
	unsigned long start_loop;
	unsigned long end_loop;
	unsigned char type;
};

/* GLOBAL VARIABLES (flags) */
extern char gf1_linear_volumes;
extern char gf1_dig_use_extra_voice;

/* FUNCTION PROTOTYPES */
/* Initializeation routines */
int	gf1_init_ports(int);
int	gf1_load_os(struct load_os RFAR *os);
int	gf1_unload_os(void);
void	gf1_set_appname(char RFAR *);
void	reset_ultra(int);
int	gf1_asm_init(void);
unsigned char gf1_peek(unsigned long address);
void gf1_poke(unsigned long address, unsigned char data);
void gf1_poke_block(unsigned char RFAR *data, unsigned long address, unsigned long len, unsigned char dma_control);
char gf1_good_dram(unsigned long address);
int GetUltraCfg(struct load_os RFAR *os);
unsigned long gf1_malloc(unsigned long);
void gf1_free(unsigned long);
unsigned long gf1_mem_avail(void);
unsigned long gf1_mem_largest_avail(void);
void gf1_delay(void);
int gf1_allocate_voice(int priority, void (RFAR *steal_notify)(int));
void gf1_free_voice(unsigned int i);
void gf1_adjust_priority(int voice, int priority);
int gf1_dram_xfer(struct gf1_dma_buff RFAR *dptr, unsigned long size, unsigned long dram_address, unsigned char dma_control, unsigned short flags);
void gf1_stop_dma(void);
long convert_to_16bit(long address);
int gf1_wait_dma(void);
int gf1_dma_ready(void);
unsigned long gf1_amount_xferred(void);
int gf1_detect_card(unsigned short port);
char *gf1_error_str(int);
int gf1_play_digital(unsigned short priority, unsigned char RFAR *buffer,
	unsigned long size, unsigned long gf1_addr, unsigned short volume,
	unsigned short pan, unsigned short frequency, unsigned char type,
	struct gf1_dma_buff RFAR *dptr,
	int (RFAR *callback)(int, int, unsigned char RFAR * RFAR *, unsigned long RFAR *));
void gf1_restart_digital(int voice);
void gf1_start_digital(int voice);
void gf1_pause_digital(int voice);
void RFAR gf1_stop_digital(int voice);
void gf1_dig_set_dma_rate(unsigned short rate);
unsigned long gf1_digital_position(int voice);
int gf1_myatoi(void);
int gf1_update_waveform(struct wave_struct RFAR *wave_info);
int gf1_get_patch_info(char RFAR *patch_file, struct patchinfo RFAR *patch);
int gf1_load_patch(char RFAR *patch_file, struct patchinfo RFAR *patchinfo,
	struct patch RFAR *patch,
	struct gf1_dma_buff RFAR *dptr, unsigned short size,
	unsigned char RFAR *wavemem, int flags);
void gf1_unload_patch(struct patch RFAR *patch);
void gf1_detune_patch(struct patch RFAR *patch, unsigned short detune);
unsigned short gf1_calc_fc(unsigned int sample_ratio,long root,long frequency);
void gf1_midi_stop_voice(int voice);
void gf1_midi_wait_voice(int voice);
unsigned short gf1_midi_status_note(int voice);
unsigned short gf1_midi_status_voice(int voice);
void RFAR gf1_midi_stop_note(int note_voice);
void gf1_midi_note_on(struct patch RFAR *patch, int priority, int note, int velocity, int channel);
void gf1_midi_note_off(int note, int channel);
void gf1_midi_silence_patch_notes(struct patch RFAR *patch);
void gf1_midi_patch_removed(struct patch RFAR *patch);
int gf1_enable_timer1(int (RFAR *callback)(void), int resolution);
int gf1_enable_timer2(int (RFAR *callback)(void), int resolution);
void gf1_disable_timer1(void);
void gf1_disable_timer2(void);
void gf1_channel_pitch_bend(int channel, unsigned int bend);
void gf1_midi_synth_volume(unsigned short synth, int master_volume);
void gf1_midi_change_program(struct patch RFAR *patch, int channel);
void gf1_midi_set_vibrato(int channel, int value);
void gf1_midi_change_volume(int channel, unsigned int volume);
void gf1_midi_set_balance(int balance, int channel);
void gf1_midi_channel_sustain(int channel, int sustain);
void gf1_midi_all_notes_off(int channel);
void gf1_midi_pitch_bend(int channel, int lsb, int msb);
void gf1_midi_parameter(int channel, int control, int value);
int gf1_midi_get_channel_notes(int channel, int notes[]);
int gf1_midi_get_channel_volume(int channel);
int gf1_midi_get_master_volume(void);
int gf1_midi_get_volume(int voice);
unsigned short gf1_read(int handle, void RFAR *io_buffer, unsigned short size);
unsigned short gf1_close_file(int handle);
unsigned int gf1_seek(int handle, unsigned long offset, int method);
int gf1_open(char RFAR *name);
#ifdef __FLAT__
int gf1_atoi(char RFAR **str, int base);
#else
int gf1_atoi(void);
#endif
void gf1_leave(void);
short gf1_enter(void);
void gf1_enter1(void);
int gf1_play_next_buffer(int voice, unsigned char RFAR *buff, unsigned long size);
void gf1_dig_set_vol(unsigned short voice, unsigned short vol);
void gf1_dig_set_pan(unsigned short voice, unsigned short pan);
int gf1_set_external_semaphore(void RFAR *addr);
int gf1_clear_external_semaphore(void RFAR *addr);
void gf1_midi_reset(int c);
int gf1_add_midi_recv_handler(int (RFAR *handler)());
int gf1_add_dma_handler(int (*handler)());
int gf1_add_voice_handler(int (*handler)(int));
int gf1_add_volume_handler(int (*handler)(int));
int gf1_add_timer_handler(int timer, int (RFAR *handler)(void));
void gf1_set_record_rate(unsigned long rate);
void gf1_create_patch(struct patch RFAR *patch);
int gf1_add_layer(struct patch RFAR *patch, int layer, char RFAR *wavemem);
void gf1_get_waveform_info(struct patch RFAR *patch, int layer, int waven,
	struct wave_struct RFAR *wave);
void gf1_set_waveform_info(struct patch RFAR *patch, int layer, int waven,
	struct wave_struct RFAR *wave);
void gf1_enable_line_in(void);
void gf1_disable_line_in(void);
void gf1_enable_mic_in(void);
void gf1_disable_mic_in(void);
void gf1_enable_output(void);
void gf1_disable_output(void);
void gf1_sound_volume(unsigned short voice, int volume,
	unsigned long period /* us*10 */);
void gf1_sound_pan(unsigned short voice, unsigned short pan);
void gf1_sound_frequency(unsigned short voice, unsigned long freq);
void RFAR gf1_sound_stop(int voice);
void gf1_sound_mode(int voice, struct gf1_sound RFAR *sound,
	unsigned char type);
int gf1_sound_start(unsigned short priority, struct gf1_sound RFAR *sound,
	short volume, unsigned long period, short pan, unsigned long freq);
int gf1_sound_playing(int voice);
#endif
