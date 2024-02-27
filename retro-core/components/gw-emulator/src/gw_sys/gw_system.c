/*

This program implements the main functions to use the LCD Game emulator.
It implements also sound buzzer and keyboard functions.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define GW_NO_SM510_DEFINES

/* Emulated system */
#include "sm510.h"
#include "sm500.h"
#include "gw_romloader.h"
#include "gw_system.h"
#include "gw_graphic.h"

static void (*device_reset)();
static void (*device_start)();
static void (*device_run)();
static void (*device_blit)(unsigned short *active_framebuffer);

static unsigned char previous_dpad;
static bool gw_keyboard_multikey[8];

/* map functions and custom configuration */
bool gw_system_config()
{

	/* Supported System */
	/* SM500 SM5A(kb1013vk12) SM510 SM511 SM512 */

	/* init graphics parameters */
	gw_gfx_init();

	/* parse keyboard mapping to detect DPAD multi-key mode */
	// determine if there is a multi-key configuration (number of keys > 4)
	// for some game joypad is used with combination
	//  LEFT_UP, LEFT_DOWN, RIGHT_UP and RIGH_DOWN
	// in this case, 2 bits are returned by the HAL as a dual key pressed
	// here, we try to detect this configuration.

	// Parse all keys configuration
	// S1..S8
	// set a flag if it's single key mode or mulyi-key mode

	for (int i = 0; i < 8; i++)
	{

		/* By default it's not multikey */
		gw_keyboard_multikey[i] = false;

		un32 score = 0;

		/* 4 combination to find */
		un8 K1 = gw_keyboard[i] & 0xFF;
		un8 K2 = (gw_keyboard[i] >> 8) & 0xFF;
		un8 K3 = (gw_keyboard[i] >> 16) & 0xFF;
		un8 K4 = (gw_keyboard[i] >> 24) & 0xFF;

		un8 C1 = GW_BUTTON_LEFT + GW_BUTTON_UP;
		un8 C2 = GW_BUTTON_LEFT + GW_BUTTON_DOWN;
		un8 C3 = GW_BUTTON_RIGHT + GW_BUTTON_UP;
		un8 C4 = GW_BUTTON_RIGHT + GW_BUTTON_DOWN;

		if (K1 == C1)
			score++;
		else if (K2 == C1)
			score++;
		else if (K3 == C1)
			score++;
		else if (K4 == C1)
			score++;

		if (K1 == C2)
			score++;
		else if (K2 == C2)
			score++;
		else if (K3 == C2)
			score++;
		else if (K4 == C2)
			score++;

		if (K1 == C3)
			score++;
		else if (K2 == C3)
			score++;
		else if (K3 == C3)
			score++;
		else if (K4 == C3)
			score++;

		if (K1 == C4)
			score++;
		else if (K2 == C4)
			score++;
		else if (K3 == C4)
			score++;
		else if (K4 == C4)
			score++;

		/* 4 combinations are found ? */
		if (score == 4)
			gw_keyboard_multikey[i] = true;
	}

	/* init dpad to default position */
	previous_dpad = 0;

	/* depending on the CPU, set functions pointers */

	// SM500
	if (strncmp(gw_head.cpu_name, ROM_CPU_SM500, 5) == 0)
	{
		device_start = sm500_device_start;
		device_reset = sm500_device_reset;
		device_run = sm500_execute_run;
		device_blit = gw_gfx_sm500_rendering;
		return true;
	}

	// SM5A
	if (strncmp(gw_head.cpu_name, ROM_CPU_SM5A, 5) == 0)
	{
		device_start = sm5a_device_start;
		device_reset = sm5a_device_reset;
		device_run = sm5a_execute_run;
		device_blit = gw_gfx_sm500_rendering;
		return true;
	}

	// SM510
	if (strncmp(gw_head.cpu_name, ROM_CPU_SM510, 5) == 0)
	{
		device_start = sm510_device_start;
		device_reset = sm510_device_reset;
		device_run = sm510_execute_run;
		device_blit = gw_gfx_sm510_rendering;
		return true;
	}

	// SM511
	if (strncmp(gw_head.cpu_name, ROM_CPU_SM511, 5) == 0)
	{
		device_start = sm510_device_start;
		device_reset = sm511_device_reset;
		device_run = sm511_execute_run;
		device_blit = gw_gfx_sm510_rendering;
		return (sm511_init_melody(gw_melody));
	}

	// SM512
	if (strncmp(gw_head.cpu_name, ROM_CPU_SM512, 5) == 0)
	{
		device_start = sm510_device_start;
		device_reset = sm511_device_reset;
		device_run = sm511_execute_run;
		device_blit = gw_gfx_sm510_rendering;
		return (sm511_init_melody(gw_melody));
	}

	return false;
}

void gw_system_reset() { device_reset(); }
void gw_system_start() { device_start(); }
void gw_system_blit(unsigned short *active_framebuffer) { device_blit(active_framebuffer); }
bool gw_system_romload() { return gw_romloader(); }

/******** Audio functions *******************/
static unsigned char mspeaker_data = 0;
static int gw_audio_buffer_idx = 0;

/* Audio buffer */
unsigned char gw_audio_buffer[GW_AUDIO_BUFFER_LENGTH * 2];
bool gw_audio_buffer_copied;

void gw_system_sound_init()
{
	/* Init Sound */
	/* clear shared audio buffer with emulator */
	memset(gw_audio_buffer, 0, sizeof(gw_audio_buffer));
	gw_audio_buffer_copied = false;

	gw_audio_buffer_idx = 0;
	mspeaker_data = 0;
}

static void gw_system_sound_melody(unsigned char data)
{
	if (gw_audio_buffer_copied)
	{
		gw_audio_buffer_copied = false;

		gw_audio_buffer_idx = gw_audio_buffer_idx - GW_AUDIO_BUFFER_LENGTH;

		if (gw_audio_buffer_idx < 0)
			gw_audio_buffer_idx = 0;

		// check if some samples have to be copied from the previous loop cycles
		if (gw_audio_buffer_idx != 0)
		{
			for (int i = 0; i < gw_audio_buffer_idx; i++)
				gw_audio_buffer[i] = gw_audio_buffer[i + GW_AUDIO_BUFFER_LENGTH];
		}
	}

	// SM511 R pin is melody output
	if (gw_melody != 0)

		mspeaker_data = data;

	// Piezo buzzer
	else
	{

		switch (gw_head.flags & FLAG_SOUND_MASK)
		{
			// R1 to piezo
		case FLAG_SOUND_R1_PIEZO:
			mspeaker_data = data & 1;
			break;

			// R2 to piezo
		case FLAG_SOUND_R2_PIEZO:
			mspeaker_data = data >> 1 & 1;
			break;

			// R1&R2 to piezo
		case FLAG_SOUND_R1R2_PIEZO:
			mspeaker_data = data & 3;
			break;

			// R1(+S1) to piezo
		case FLAG_SOUND_R1S1_PIEZO:
			mspeaker_data = (sm510.m_s_out & ~1) | (data & 1);
			break;

			// S1(+R1) to piezo, other to input mux
		case FLAG_SOUND_S1R1_PIEZO:
			mspeaker_data = (sm510.m_s_out & ~2) | (data << 1 & 2);
			break;

			// R1 to piezo
		default:
			mspeaker_data = data & 1;
		}
	}

	if (gw_audio_buffer_idx >= GW_AUDIO_BUFFER_LENGTH * 2)
		return;

	gw_audio_buffer[gw_audio_buffer_idx] = mspeaker_data;

	gw_audio_buffer_idx++;
}

void gw_writeR(unsigned char data) { gw_system_sound_melody(data); };

/******** Keys functions **********************/
/*
 S[8]xK[4],B and BA keys input function
 8 buttons, state known using gw_get_buttons()
 gw_get_buttons() : returns
 left         | (up << 1)   | (right << 2) | (down << 3) |
 (a << 4)     | (b << 5)    | (time << 6)  | (game << 7) |
 (pause << 8) | (power << 9)

 We use only 8bits LSB, pause and power are ignored

*/

// default value is 1 (Pull-up)
unsigned char gw_readB()
{

	unsigned int keys_pressed = gw_get_buttons() & 0xff;

	if (keys_pressed == 0)
		return 1;

	if (gw_keyboard[9] & keys_pressed)
		return 0;

	return 1;
}

// default value is 1 (Pull-up)
unsigned char gw_readBA()
{

	unsigned int keys_pressed = gw_get_buttons() & 0xff;

	if (keys_pressed == 0)
		return 1;

	if (gw_keyboard[8] & keys_pressed)
		return 0;

	return 1;
}

/*
Patch to  improve gameplay of "green House" when it's in rotated view

# A specific custom keyboard is implemented for "Green House" when screen is rotated.
# It permits to play in rotate view with DPAD only.
# According to the character postion, the Button A is emulated over DPAD keys press.
#                 ~ ~   ~ ~
#                 A B C D E
#                 ~   F   ~
#                  ~  G  ~
#                   H I J
#
#In positions :
# A, H : LEFT or UP emulates Button A
# B, D : UP emulates Button A
# E, J : RIGHT or UP emulates Button A
# C,F,G,I :ignore
#
#SM51X series: output to x.y.z, where:
	# x = group a/b/bs/c (0/1/2/3)
	# y = segment 1-16 (0-15)
	# z = common H1-H4 (0-3)

#    CPU RAM display
#	0X50.. 0X5F : c1..c16 (base=80)
#	0X60.. 0X6F : a1..a16 (base=96)
#	0X70.. 0X7F : b1..b16 (base=112)

# Position: segment      : RAM   MASk
#        A  h1b7  1.6.0    118   0x1
#        B  h1b12 1.11.0   123   0x1
#        C  h1a13 0.12.0   108   0x1
#        D  h1b13 1.12.0   124   0x1
#        E  h1a15 0.14.0   110   0x1
#        F  h3b3  1.2.2    114   0x4
#        G  h2b3  1.2.1    114   0x2
#        H  h1a4  0.3.0     99   0x1
#        I  h1b3  1.2.0    114   0x1
#        J  h1a3  0.2.0     98   0x1
*/

static unsigned int patch_gnw_ghouse_keys_rotated_view(unsigned int keys_pressed) {

	// check if it's in rotated view by checking the keys mapping
	// K1 = rom.BTN_DOWN
    // K2 = rom.BTN_RIGHT
    // K3 = rom.BTN_UP
    // K4 = rom.BTN_LEFT
    // rom.BTN_DATA[rom.S2] = K1 | (K2 << 8) | (K3 << 16) | (K4 << 24)

	if ( gw_keyboard[1] != ( (GW_BUTTON_LEFT << 24) + (GW_BUTTON_RIGHT << 8) + (GW_BUTTON_UP << 16)  + GW_BUTTON_DOWN) ) return keys_pressed;

	// Check position CFGI
	if (sm510.gw_ram[108] & 0x1) return keys_pressed;
	if (sm510.gw_ram[114] & 0x4) return keys_pressed;
	if (sm510.gw_ram[114] & 0x2) return keys_pressed;
	if (sm510.gw_ram[114] & 0x1) return keys_pressed;

	// Check position AH
	if ( (sm510.gw_ram[118] & 0x1) | (sm510.gw_ram[99] & 0x1) ) {
		if ( (keys_pressed & (GW_BUTTON_RIGHT+GW_BUTTON_UP) ) )
			keys_pressed = GW_BUTTON_A;
	}
	// Check position BD
	if ( (sm510.gw_ram[123] & 0x1) | (sm510.gw_ram[124] & 0x1) ) {
		if ( (keys_pressed & GW_BUTTON_RIGHT) )
			keys_pressed = GW_BUTTON_A;
	}
	// Check position EJ
	if ( (sm510.gw_ram[110] & 0x1) | (sm510.gw_ram[98] & 0x1) ) {
		if ( (keys_pressed & (GW_BUTTON_RIGHT+GW_BUTTON_DOWN)) )
			keys_pressed = GW_BUTTON_A;
	}

	return keys_pressed;
}

// default value is 0 (Pull-down)
unsigned char gw_readK(unsigned char io_S)
{

	unsigned char io_K = 0;
	static unsigned int dpad_shortcut_diagonal_pressed = 0;
	static unsigned int dpad_shortcut_diagonal_hold = 0;

	unsigned int gw_buttons = gw_get_buttons();

	unsigned int keys_pressed = gw_buttons & 0xff;

	unsigned int key_soft_value = 0;

	const unsigned int KEY_SOFT_TIME = GW_BUTTON_B + GW_BUTTON_TIME;
	const unsigned int KEY_SOFT_ALARM = GW_BUTTON_B + GW_BUTTON_GAME;

	if ((gw_buttons >> 10) & 1)
	{
		keys_pressed = 0;
		key_soft_value = KEY_SOFT_TIME;
	}
	if ((gw_buttons >> 11) & 1)
	{
		keys_pressed = 0;
		key_soft_value = KEY_SOFT_ALARM;
	}

	if (keys_pressed == KEY_SOFT_ALARM) {
		key_soft_value = KEY_SOFT_ALARM;
		keys_pressed = 0;
	}

	if (keys_pressed == KEY_SOFT_TIME) {
		key_soft_value = KEY_SOFT_TIME;
		keys_pressed = 0;
	}


	/* Hack to improve gameplay for "Green House" in rotated view */
	if (memcmp(gw_head.rom_signature, "w_ghouse", 8) == 0 ) keys_pressed = patch_gnw_ghouse_keys_rotated_view(keys_pressed);
	/**********************************************/

	/* monitor A button as diagonal shortcut when pressed */
	if ((keys_pressed & GW_BUTTON_A) != 0) dpad_shortcut_diagonal_pressed = 1;
	if ( (keys_pressed & GW_BUTTON_A) == 0) {
		dpad_shortcut_diagonal_hold = 0;
		dpad_shortcut_diagonal_pressed = 0;
	}

	if ((keys_pressed == 0) & (key_soft_value == 0))
		return 0;

	for (int Sx = 0; Sx < 8; Sx++)
	{

		//case of R/S output is not used to poll buttons (used R2 or S2 configuration)
		if (io_S == 0) io_S = 2;

		if (((io_S >> Sx) & 0x1) != 0)
		{
			// DPAD multi-key with relative change and shortcut
			if (gw_keyboard_multikey[Sx])
			{

				// keep only relevant DPAD keys
				keys_pressed = keys_pressed & (GW_BUTTON_LEFT + GW_BUTTON_RIGHT + GW_BUTTON_UP + GW_BUTTON_DOWN);

				// manage when a single key is pressed but the emulated game expects only diagonal keys combination
				// reuse the previous DPAD value (previous_dpad) and add a complementary key

				if ((dpad_shortcut_diagonal_pressed == 1) & (dpad_shortcut_diagonal_hold == 0))
				{
					dpad_shortcut_diagonal_hold = 1;
					keys_pressed = ~previous_dpad &
								   (GW_BUTTON_LEFT + GW_BUTTON_RIGHT + GW_BUTTON_UP + GW_BUTTON_DOWN);
				}

				if (keys_pressed == GW_BUTTON_LEFT)
					keys_pressed = (previous_dpad & (GW_BUTTON_UP + GW_BUTTON_DOWN)) | GW_BUTTON_LEFT;

				if (keys_pressed == GW_BUTTON_RIGHT)
					keys_pressed = (previous_dpad & (GW_BUTTON_UP + GW_BUTTON_DOWN)) | GW_BUTTON_RIGHT;

				if (keys_pressed == GW_BUTTON_DOWN)
					keys_pressed = (previous_dpad & (GW_BUTTON_LEFT + GW_BUTTON_RIGHT)) | GW_BUTTON_DOWN;

				if (keys_pressed == GW_BUTTON_UP)
					keys_pressed = (previous_dpad & (GW_BUTTON_LEFT + GW_BUTTON_RIGHT)) | GW_BUTTON_UP;

				if ((gw_keyboard[Sx] & GW_MASK_K1) == keys_pressed)
				{
					io_K |= 0x1;
					previous_dpad = keys_pressed;
				}

				// perform  key checking (exclusively, all keys shall match)
				if ((gw_keyboard[Sx] & GW_MASK_K2) == (keys_pressed << 8))
				{
					io_K |= 0x2;
					previous_dpad = keys_pressed;
				}

				if ((gw_keyboard[Sx] & GW_MASK_K3) == (keys_pressed << 16))
				{
					io_K |= 0x4;
					previous_dpad = keys_pressed;
				}

				if ((gw_keyboard[Sx] & GW_MASK_K4) == (keys_pressed << 24))
				{
					io_K |= 0x8;
					previous_dpad = keys_pressed;
				}

				// (Non DPAD case) perform key checking (at least one key shall match)
			}
			else
			{

				// check soft keys
				if ( key_soft_value != 0 )
				{
					if (((gw_keyboard[Sx] & GW_MASK_K1) == (key_soft_value)))
						io_K |= 0x1;
					if (((gw_keyboard[Sx] & GW_MASK_K2) == (key_soft_value << 8)))
						io_K |= 0x2;
					if (((gw_keyboard[Sx] & GW_MASK_K3) == (key_soft_value << 16)))
						io_K |= 0x4;
					if (((gw_keyboard[Sx] & GW_MASK_K4) == (key_soft_value << 24)))
						io_K |= 0x8;
				}

				// check hardware keys excluding soft keys
				else
				{
					if (((gw_keyboard[Sx] & GW_MASK_K1) & (keys_pressed)) != 0)
						if ( ( (gw_keyboard[Sx] & GW_MASK_K1) != (KEY_SOFT_ALARM) )  & ( (gw_keyboard[Sx] & GW_MASK_K1) != (KEY_SOFT_TIME) ) )
							io_K |= 0x1;

					if (((gw_keyboard[Sx] & GW_MASK_K2) & (keys_pressed << 8)) != 0)
						if ( ( (gw_keyboard[Sx] & GW_MASK_K2) != (KEY_SOFT_ALARM << 8) )  & ( (gw_keyboard[Sx] & GW_MASK_K2) != (KEY_SOFT_TIME << 8) ) )

							io_K |= 0x2;
					if (((gw_keyboard[Sx] & GW_MASK_K3) & (keys_pressed << 16)) != 0)
							if ( ( (gw_keyboard[Sx] & GW_MASK_K3) != (KEY_SOFT_ALARM << 16) )  & ( (gw_keyboard[Sx] & GW_MASK_K3) != (KEY_SOFT_TIME << 16) ) )

							io_K |= 0x4;
					if (((gw_keyboard[Sx] & GW_MASK_K4) & (keys_pressed << 24)) != 0)
							if ( ( (gw_keyboard[Sx] & GW_MASK_K4) != (KEY_SOFT_ALARM << 24) )  & ( (gw_keyboard[Sx] & GW_MASK_K4) != (KEY_SOFT_TIME << 24) ) )

							io_K |= 0x8;
				}
			}
		}
		//case of R/S output is not used to poll buttons (used R2 or S2 configuration)
		// same algorithm as previously
/* 		if (io_S == 0)
		{
			//dpad and shortcut
			if (gw_keyboard_multikey[1])
			{

				// keep only relevant DPAD keys
				keys_pressed = keys_pressed & (GW_BUTTON_LEFT + GW_BUTTON_RIGHT + GW_BUTTON_UP + GW_BUTTON_DOWN);

				// manage when a single key is pressed but the emulated game expects only diagonal keys combination
				// reuse the previous DPAD value (previous_dpad) and add a complementary key

				if ( (dpad_shortcut_diagonal_pressed == 1 ) & (dpad_shortcut_diagonal_hold == 0)) {
						dpad_shortcut_diagonal_hold = 1;
						// toogle dpad values to move in diagonal
						keys_pressed = ~previous_dpad & \
						(GW_BUTTON_LEFT + GW_BUTTON_RIGHT + GW_BUTTON_UP + GW_BUTTON_DOWN);

				}

				if (keys_pressed == GW_BUTTON_LEFT)
					keys_pressed = (previous_dpad & (GW_BUTTON_UP + GW_BUTTON_DOWN)) | GW_BUTTON_LEFT;

				if (keys_pressed == GW_BUTTON_RIGHT)
					keys_pressed = (previous_dpad & (GW_BUTTON_UP + GW_BUTTON_DOWN)) | GW_BUTTON_RIGHT;

				if (keys_pressed == GW_BUTTON_DOWN)
					keys_pressed = (previous_dpad & (GW_BUTTON_LEFT + GW_BUTTON_RIGHT)) | GW_BUTTON_DOWN;

				if (keys_pressed == GW_BUTTON_UP)
					keys_pressed = (previous_dpad & (GW_BUTTON_LEFT + GW_BUTTON_RIGHT)) | GW_BUTTON_UP;

				if ((gw_keyboard[1] & GW_MASK_K1) == keys_pressed)
				{
					io_K |= 0x1;
					previous_dpad = keys_pressed;
				}
				if ((gw_keyboard[1] & GW_MASK_K2) == (keys_pressed << 8))
				{
					io_K |= 0x2;
					previous_dpad = keys_pressed;
				}
				if ((gw_keyboard[1] & GW_MASK_K3) == (keys_pressed << 16))
				{
					io_K |= 0x4;
					previous_dpad = keys_pressed;
				}
				if ((gw_keyboard[1] & GW_MASK_K4) == (keys_pressed << 24))
				{
					io_K |= 0x8;
					previous_dpad = keys_pressed;
				}
			}
			else
			{

				// check soft key
				if (key_soft_value)
				{

					if (((gw_keyboard[1] & GW_MASK_K1) == (key_soft_value)))
						io_K |= 0x1;
					if (((gw_keyboard[1] & GW_MASK_K2) == (key_soft_value << 8)))
						io_K |= 0x2;
					if (((gw_keyboard[1] & GW_MASK_K3) == (key_soft_value << 16)))
						io_K |= 0x4;
					if (((gw_keyboard[1] & GW_MASK_K4) == (key_soft_value << 24)))
						io_K |= 0x8;
				}
				else
				{
					if ( (gw_keyboard[1] != KEY_SOFT_TIME) | (gw_keyboard[1] !=KEY_SOFT_ALARM) )	{
							if (((gw_keyboard[1] & GW_MASK_K1) & (keys_pressed)) != 0)
								io_K |= 0x1;
							if (((gw_keyboard[1] & GW_MASK_K2) & (keys_pressed << 8)) != 0)
								io_K |= 0x2;
							if (((gw_keyboard[1] & GW_MASK_K3) & (keys_pressed << 16)) != 0)
								io_K |= 0x4;
							if (((gw_keyboard[1] & GW_MASK_K4) & (keys_pressed << 24)) != 0)
								io_K |= 0x8;
						}
				}
			}
		} */
	}

	return io_K & 0xf;
}

/* external function use to execute some clock cycles */
int gw_system_run(int clock_cycles)
{
	// check if a key is pressed to wakeup the system
	// set K input lines active state
	sm510.m_k_active = (gw_get_buttons() != 0);

	//1 CPU operation in 2 clock cycles
	if (sm510.m_clk_div == 2)
		sm510.m_icount += (clock_cycles / 2);

	//1 CPU operation in 4 clock cycles
	if (sm510.m_clk_div == 4)
		sm510.m_icount += (clock_cycles / 4);

	device_run();

	// return cycles really executed according to divider
	return sm510.m_icount * sm510.m_clk_div;
}

/* Shutdown gw */
void gw_system_shutdown(void)
{
	/* change audio frequency to default audio clock */
	//gw_set_audio_frequency(48000);
}

static gw_state_t save_state;

/* save state */
bool gw_state_save(void *dest_ptr)
{
	/* add header and signature */
	memcpy(&save_state.magic_word, GW_MAGIC_WORD, 8);
	memcpy(&save_state.rom_signature, &gw_head.rom_signature, 8);

	/* dump all variables from sm510base.c */

	// internal RAM 128x4 bits
	memcpy(&save_state.gw_ram, &sm510.gw_ram, sizeof(sm510.gw_ram));
	memcpy(&save_state.gw_ram_state, &sm510.gw_ram_state, sizeof(sm510.gw_ram_state));

	//program counter, opcode,...
	save_state.m_pc = sm510.m_pc;
	save_state.m_prev_pc = sm510.m_prev_pc;
	save_state.m_op = sm510.m_op;
	save_state.m_prev_op = sm510.m_prev_op;

	save_state.m_param = sm510.m_param;
	save_state.m_stack_levels = sm510.m_stack_levels;
	memcpy(save_state.m_stack, &sm510.m_stack, sizeof(sm510.m_stack));
	save_state.m_icount = sm510.m_icount;

	save_state.m_acc = sm510.m_acc;
	save_state.m_bl = sm510.m_bl;
	save_state.m_bm = sm510.m_bm;
	save_state.m_c = sm510.m_c;
	save_state.m_w = sm510.m_w;
	save_state.m_s_out = sm510.m_s_out;
	save_state.m_r = sm510.m_r;
	save_state.m_r_out = sm510.m_r_out;

	save_state.bool_m_k_active = (un8)sm510.m_k_active;
	save_state.bool_m_halt = (un8)sm510.m_halt;
	save_state.bool_m_sbm = (un8)sm510.m_sbm;
	save_state.bool_m_sbl = (un8)sm510.m_sbl;
	save_state.bool_m_skip = (un8)sm510.m_skip;

	// freerun time counter
	save_state.m_div = sm510.m_div;
	save_state.bool_m_1s = (un8)sm510.m_1s;
	save_state.m_clk_div = sm510.m_clk_div;

	// melody controller
	save_state.m_r_mask_option = sm510.m_r_mask_option;
	save_state.m_melody_rd = sm510.m_melody_rd;
	save_state.m_melody_step_count = sm510.m_melody_step_count;
	save_state.m_melody_duty_count = sm510.m_melody_duty_count;
	save_state.m_melody_duty_index = sm510.m_melody_duty_index;
	save_state.m_melody_address = sm510.m_melody_address;

	// lcd driver
	save_state.m_l = sm510.m_l;
	save_state.m_x = sm510.m_x;
	save_state.m_y = sm510.m_y;
	save_state.m_bp = sm510.m_bp;
	save_state.bool_m_bc = (un8)sm510.m_bc;

	// SM500 internals
	save_state.m_o_pins = sm510.m_o_pins;
	memcpy(&save_state.m_ox, &sm510.m_ox, sizeof(sm510.m_ox));
	memcpy(&save_state.m_o, &sm510.m_o, sizeof(sm510.m_o));
	memcpy(&save_state.m_ox_state, &sm510.m_ox_state, sizeof(sm510.m_ox_state));
	memcpy(&save_state.m_o_state, &sm510.m_o_state, sizeof(sm510.m_o_state));

	save_state.m_cn = sm510.m_cn;
	save_state.m_mx = sm510.m_mx;

	save_state.m_cb = sm510.m_cb;
	save_state.m_s = sm510.m_s;
	save_state.bool_m_rsub = (un8)sm510.m_rsub;

	/* save state */
	memcpy(dest_ptr, &save_state, sizeof(save_state));
	return true;
}

/* load state */
bool gw_state_load(void *src_ptr)
{

	memcpy(&save_state, src_ptr, sizeof(save_state));

	/* Check header and signature */
	if (memcmp(&save_state.magic_word, GW_MAGIC_WORD, 8) != 0)
		return false;
	if (memcmp(&save_state.rom_signature, &gw_head.rom_signature, 8) != 0)
		return false; // 16

	/* set all variables from sm510base.c */

	// internal RAM 128x4 bits
	memcpy(&sm510.gw_ram, &save_state.gw_ram, sizeof(sm510.gw_ram));
	memcpy(&sm510.gw_ram_state, &save_state.gw_ram_state, sizeof(sm510.gw_ram_state));

	//program counter, opcode
	sm510.m_pc = save_state.m_pc;
	sm510.m_prev_pc = save_state.m_prev_pc;
	sm510.m_op = save_state.m_op;
	sm510.m_prev_op = save_state.m_prev_op;

	sm510.m_param = save_state.m_param;
	sm510.m_stack_levels = save_state.m_stack_levels;
	memcpy(sm510.m_stack, &save_state.m_stack, sizeof(sm510.m_stack));
	sm510.m_icount = save_state.m_icount;

	sm510.m_acc = save_state.m_acc;
	sm510.m_bl = save_state.m_bl;
	sm510.m_bm = save_state.m_bm;
	sm510.m_c = save_state.m_c;
	sm510.m_w = save_state.m_w;
	sm510.m_s_out = save_state.m_s_out;
	sm510.m_r = save_state.m_r;
	sm510.m_r_out = save_state.m_r_out;

	sm510.m_k_active = (bool)save_state.bool_m_k_active;
	sm510.m_halt = (bool)save_state.bool_m_halt;
	sm510.m_sbm = (bool)save_state.bool_m_sbm;
	sm510.m_sbl = (bool)save_state.bool_m_sbl;
	sm510.m_skip = (bool)save_state.bool_m_skip;

	// melody controller
	sm510.m_r_mask_option = save_state.m_r_mask_option;
	sm510.m_melody_rd = save_state.m_melody_rd;
	sm510.m_melody_step_count = save_state.m_melody_step_count;
	sm510.m_melody_duty_count = save_state.m_melody_duty_count;
	sm510.m_melody_duty_index = save_state.m_melody_duty_index;
	sm510.m_melody_address = save_state.m_melody_address;

	// freerun time counter
	sm510.m_clk_div = save_state.m_clk_div;
	sm510.m_div = save_state.m_div;
	sm510.m_1s = (bool)save_state.bool_m_1s;
	sm510.m_melody_duty_index = save_state.m_melody_duty_index;
	sm510.m_melody_address = save_state.m_melody_address;

	// lcd driver
	sm510.m_l = save_state.m_l;
	sm510.m_x = save_state.m_x;
	sm510.m_y = save_state.m_y;
	sm510.m_bp = save_state.m_bp;
	sm510.m_bc = (bool)save_state.bool_m_bc;

	// SM500 internals
	sm510.m_o_pins = save_state.m_o_pins;
	memcpy(&sm510.m_ox, &save_state.m_ox, sizeof(sm510.m_ox));
	memcpy(&sm510.m_o, &save_state.m_o, sizeof(sm510.m_o));
	memcpy(&sm510.m_ox_state, &save_state.m_ox_state, sizeof(sm510.m_ox_state));
	memcpy(&sm510.m_o_state, &save_state.m_o_state, sizeof(sm510.m_o_state));

	sm510.m_cn = save_state.m_cn;
	sm510.m_mx = save_state.m_mx;

	sm510.m_cb = save_state.m_cb;
	sm510.m_s = save_state.m_s;
	sm510.m_rsub = (bool)save_state.bool_m_rsub;

	return true;
}

gw_time_t gw_system_get_time()
{
	gw_time_t time = {0};

	/* Rom hour is not set ? */
	if ((gw_head.time_hour_address_msb == 0) && (gw_head.time_hour_address_lsb == 0)) {

		// return time.hours > 24 to indicate the feature is not support
		time.hours = 127;
		return time;
	}

	// AM/PM is not supported
	if (gw_head.time_hour_msb_pm == 0) {

		// return time.hours > 24 to indicate the feature is not support
		time.hours = 127;
		return time;
	}

	unsigned int hour_msb;
	unsigned int hour_lsb;
	unsigned int pm_flag;


	hour_msb = sm510.gw_ram[gw_head.time_hour_address_msb];
	hour_lsb = sm510.gw_ram[gw_head.time_hour_address_lsb];
	pm_flag  = gw_head.time_hour_msb_pm;

	/* Minutes */
	time.minutes = (sm510.gw_ram[gw_head.time_min_address_msb] * 10) + \
	sm510.gw_ram[gw_head.time_min_address_lsb];

	/* Seconds */
	time.seconds = (sm510.gw_ram[gw_head.time_sec_address_msb] * 10) + \
	sm510.gw_ram[gw_head.time_sec_address_lsb];

	//PM
	if (hour_msb & pm_flag) {
		hour_msb  = hour_msb & ~pm_flag;

		// PM12
		if ( (hour_msb == 1) && (hour_lsb == 2) ) time.hours = 12;

		// PM1 <-> PM11
		else time.hours = (10 * hour_msb) + 12 + hour_lsb;

	//AM
	} else {
		// AM12
		if ( (hour_msb == 1) && (hour_lsb == 2) ) time.hours = 0;

		// AM1 <-> AM11
		else time.hours = (10 * hour_msb) + hour_lsb;
	}

	return time;

}

void gw_system_set_time(gw_time_t time)
{
	/* Rom hour is not set ? */
	if ((gw_head.time_hour_address_msb == 0) & (gw_head.time_hour_address_lsb == 0))
		return;

	/* Hours */
	// AM1 <-> AM11
	if ((time.hours > 0) && (time.hours < 12))
	{
		sm510.gw_ram[gw_head.time_hour_address_msb] = time.hours / 10;
		sm510.gw_ram[gw_head.time_hour_address_lsb] = time.hours % 10;
	}

	// AM12
	if (time.hours == 0)
	{
		sm510.gw_ram[gw_head.time_hour_address_msb] = 1;
		sm510.gw_ram[gw_head.time_hour_address_lsb] = 2;
	}

	// PM12
	if (time.hours == 12)
	{
		sm510.gw_ram[gw_head.time_hour_address_msb] = gw_head.time_hour_msb_pm + 1;
		sm510.gw_ram[gw_head.time_hour_address_lsb] = 2;
	}

	// PM1 <-> PM11
	if (time.hours > 12)
	{
		sm510.gw_ram[gw_head.time_hour_address_msb] = gw_head.time_hour_msb_pm + ((time.hours - 12) / 10);
		sm510.gw_ram[gw_head.time_hour_address_lsb] = (time.hours - 12) % 10;
	}

	/* Minutes */
	sm510.gw_ram[gw_head.time_min_address_msb] = time.minutes / 10;
	sm510.gw_ram[gw_head.time_min_address_lsb] = time.minutes % 10;

	/* Seconds */
	sm510.gw_ram[gw_head.time_sec_address_msb] = time.seconds / 10;
	sm510.gw_ram[gw_head.time_sec_address_lsb] = time.seconds % 10;
}