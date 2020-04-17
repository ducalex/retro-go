#include "osd_keyboard.h"
#include "hard_pce.h"
#include "sound.h"

#if 0

#if defined(NETPLAY_DEBUG)
#include <sys/timeb.h>
#endif

#ifndef SDLK_NUMLOCK
// SDL 2.0>
#define SDLK_NUMLOCK SDLK_NUMLOCKCLEAR
#endif


Uint16 read_input (Uint16 port);

// info about the input config

Uint8 *key;
int16 joy[J_MAX];

input_config config[16] = {
	{  // Config 0
		{
			{ 0, 0, 0, 0, 0, 0,
				{ SDL_SCANCODE_UP,
				  SDL_SCANCODE_DOWN,
				  SDL_SCANCODE_LEFT,
				  SDL_SCANCODE_RIGHT,
				  SDL_SCANCODE_Z,
				  SDL_SCANCODE_X,
				  SDL_SCANCODE_TAB,
				  SDL_SCANCODE_RETURN,
				  SDL_SCANCODE_Q,
				  SDL_SCANCODE_W,
				-1, -1, -1, -1, -1, -1, -1, -1 } },
			{ 0, 0, 0, 0, 0, 0, { 0 } },
			{ 0, 0, 0, 0, 0, 0, { 0 } },
			{ 0, 0, 0, 0, 0, 0, { 0 } },
			{ 0, 0, 0, 0, 0, 0, { 0 } }
		}
	}
};

#if defined(ENABLE_NETPLAY)

//! socket set to check for activity on server socket (on client side)
static SDLNet_SocketSet client_socket_set;

//! client side socket for sending and receiving packets with the server
static UDPsocket client_socket;

//! incoming packet storage
static UDPpacket *incoming_packet;

//! outgoing packet storage
static UDPpacket *outgoing_packet;

//! address of the server to connect to
static IPaddress server_address;

//! Send identification packet(s) until we're sure the server is aware of our needs
static void send_identification (uchar *);

//! Send packet for telling the server what is the status of the current input reading
static void send_local_status (uchar *);

//! Wait for digest information from server using the lan protocol
static void wait_lan_digest_status (uchar *);

//! Wait for digest information from server using the internet protocol
static void wait_internet_digest_status (uchar *);


#endif // ENABLE_NETPLAY

uchar current_config;

// the number of the current config
char tmp_buf[100];

extern int UPeriod;


/* For joypad */
SDL_Joystick *joypad[5];


/* for keyboard */
uint16
read_input(uint16 port)
{
	static char autoI_delay = 0, autoII_delay = 0;
	uint16 tmp;

	key = SDL_GetKeyboardState(NULL);

	for (tmp = J_PAD_START; tmp < J_MAX; tmp++)
		joy[tmp] = 0;

	tmp = 0;

	if (config[current_config].individual_config[port].joydev) {
		SDL_Joystick * current_joypad
			= joypad[config[current_config].individual_config[port].joydev - 1];

		joy[J_PXAXIS]
			= SDL_JoystickGetAxis (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PXAXIS]);
		joy[J_PYAXIS]
			= SDL_JoystickGetAxis (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PYAXIS]);
		joy[J_PRUN]
			= SDL_JoystickGetButton (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PRUN]);
		joy[J_PSELECT]
			= SDL_JoystickGetButton (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PSELECT]);
		joy[J_PI]
			= SDL_JoystickGetButton (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PI]);
		joy[J_PII]
			= SDL_JoystickGetButton (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PII]);
		joy[J_PAUTOI]
			= SDL_JoystickGetButton (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PAUTOI]);
		joy[J_PAUTOII]
			= SDL_JoystickGetButton (current_joypad,
				config[current_config].individual_config[port].
				joy_mapping[J_PAUTOII]);
	}

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_UP]] || (joy[J_PYAXIS] < -16384))
		tmp |= JOY_UP;
	else if (key[config[current_config].individual_config[port]
		.joy_mapping[J_DOWN]] || (joy[J_PYAXIS] > 16383))
		tmp |= JOY_DOWN;

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_LEFT]] || (joy[J_PXAXIS] < -16384))
		tmp |= JOY_LEFT;
	else if (key[config[current_config].individual_config[port]
		.joy_mapping[J_RIGHT]] || (joy[J_PXAXIS] > 16383))
		tmp |= JOY_RIGHT;

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_II]] || joy[J_PII]) {
		if (!config[current_config].individual_config[port].autoII)
			tmp |= JOY_B;
		else {
			config[current_config].individual_config[port].firedII
				= !config[current_config].individual_config[port].firedII;
			if (!config[current_config].individual_config[port].firedII)
				tmp |= JOY_B;
		}
	}

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_I]] || joy[J_PI]) {
		if (!config[current_config].individual_config[port].autoI)
			tmp |= JOY_A;
		else {
			config[current_config].individual_config[port].firedI
				= !config[current_config].individual_config[port].firedI;
			if (!config[current_config].individual_config[port].firedI)
				tmp |= JOY_A;
		}
	}

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_SELECT]] || joy[J_PSELECT])
		tmp |= JOY_SELECT;

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_RUN]] || joy[J_PRUN])
		tmp |= JOY_RUN;

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_AUTOI]] || joy[J_PAUTOI]) {
		if (!autoI_delay) {
			config[current_config].individual_config[port].autoI
				= !config[current_config].individual_config[port].autoI;
			autoI_delay = 20;
		}
	}

	if (key[config[current_config].individual_config[port]
		.joy_mapping[J_AUTOII]] || joy[J_PAUTOII]) {
		if (!autoII_delay) {
			config[current_config].individual_config[port].autoII
				= !config[current_config].individual_config[port].autoII;
			autoII_delay = 20;
		}
	}

	if (autoI_delay)
		autoI_delay--;

	if (autoII_delay)
		autoII_delay--;

	return tmp;
}


void
sdl_config_joypad_axis(short which, joymap axis, uint16 * bad_axes,
	uint16 num_axes) {

	uchar t;
	int done = 0;

	while (1) {
		if (read (fileno (stdin), &t, 1) == -1)
			break;
	}

	while (!done) {
		time_t stime, ntime;
		int axes;

		if (read (fileno (stdin), &t, 1) == -1) {
			if (errno != EAGAIN)
				break;
		} else
			break;

		SDL_JoystickUpdate();

		for (axes = num_axes - 1; axes >= 0; axes--) {
			if (bad_axes[axes])
				continue;

			if (abs (SDL_JoystickGetAxis (joypad[which], axes)) > 16384) {
				MESSAGE_INFO("Activity reported on axis %d; "
					"Please release axis.\n", axes);
				config[current_config].individual_config[which]
					.joy_mapping[axis] = axes;

				done = 1;

				stime = time (NULL);

				while (abs(SDL_JoystickGetAxis (joypad[which], axes)) > 16384) {
					SDL_JoystickUpdate ();
					SDL_Delay(50);
					ntime = time(NULL);

					if ((ntime - stime) > 3) {
						bad_axes[axes] = 1;
						done = 0;
						MESSAGE_INFO("Activity still reported on axis %d;"
							" marking as invalid.\n",
							axes);
						break;
					}
				}
			}
		}
	}

	while (1) {
		if (read (fileno (stdin), &t, 1) == -1)
			break;
	}
}


void
sdl_config_joypad_button (short which, joymap button, uint16 * bad_buttons,
	uint16 num_buttons)
{
	uchar t;
	int done = 0;

	while (1) {
		if (read (fileno (stdin), &t, 1) == -1)
			break;
	}

	while (!done) {
		time_t stime, ntime;
		int buttons;

		if (read (fileno (stdin), &t, 1) == -1) {
			if (errno != EAGAIN)
			break;
		} else
			break;

		SDL_JoystickUpdate ();

		for (buttons = num_buttons - 1; buttons >= 0; buttons--) {
			if (bad_buttons[buttons])
				continue;

			if (SDL_JoystickGetButton (joypad[which], buttons)) {
				printf
				("    -- Button %d is pressed;  Please release button . . .\n",
					buttons);

				config[current_config].individual_config[which]
					.joy_mapping[button] = buttons;

				done = 1;

				stime = time (NULL);

				while (SDL_JoystickGetButton (joypad[which], buttons)) {
					SDL_JoystickUpdate ();
					SDL_Delay (50);
					ntime = time (NULL);

					if ((ntime - stime) > 3) {
						bad_buttons[buttons] = 1;
						done = 0;

						printf
						("    ** Button %d still claiming to be pressed after 3 seconds, marked as invalid\n\n"
							"    Please try another button now . . .\n",
							buttons);
						break;
					}
				}
			}
		}
	}

	while (1) {
		if (read (fileno (stdin), &t, 1) == -1)
			break;
	}
}


void
sdl_config_joypad (short which)
{
  int sa;
  uint16 *bad_vals, n;

  printf ("    ^ We need to configure this joypad ^\n"
	  "    Press [ENTER] when ready to begin, [ENTER] can also be used to skip configuration steps.\n");
  getchar ();

  sa = fcntl (fileno (stdin), F_GETFL);
  fcntl (fileno (stdin), F_SETFL, O_NONBLOCK);

  n = SDL_JoystickNumAxes (joypad[which]);
  bad_vals = (uint16 *) calloc (sizeof (uint16), n);

  printf ("    - Press axis to use as x-plane (left/right)\n");
  sdl_config_joypad_axis (which, J_PXAXIS, bad_vals, n);

  printf ("    - Press axis to use as y-plane (up/down)\n");
  sdl_config_joypad_axis (which, J_PYAXIS, bad_vals, n);

  free (bad_vals);

  n = SDL_JoystickNumButtons (joypad[which]);
  bad_vals = (uint16 *) calloc (sizeof (uint16), n);

  printf ("    - Press button to use for 'RUN'\n");
  sdl_config_joypad_button (which, J_PRUN, bad_vals, n);

  printf ("    - Press button to use for 'SELECT'\n");
  sdl_config_joypad_button (which, J_PSELECT, bad_vals, n);

  printf
    ("    - Press button to use for 'I' (right-most button on pce pad)\n");
  sdl_config_joypad_button (which, J_PI, bad_vals, n);

  printf ("    - Press button to use for autofire toggle on 'I'\n");
  sdl_config_joypad_button (which, J_PAUTOI, bad_vals, n);

  printf
    ("    - Press button to use for 'II' (left-most button on pce pad)\n");
  sdl_config_joypad_button (which, J_PII, bad_vals, n);

  printf ("    - Press button to use for autofire toggle on 'II'\n");
  sdl_config_joypad_button (which, J_PAUTOII, bad_vals, n);

  free (bad_vals);

  printf ("                    Player %d\n"
	  "      *------------------------------------*\n"
	  "      |    %2d                    %2d   %2d   |\n"
	  "      |    __                  Turbo Turbo |\n"
	  "      |    ||                    __   __   |\n"
	  "      |  |====| %2d              (%2d) (%2d)  |\n"
	  "      |    ||      [%2d]  [%2d]    ~~   ~~   |\n"
	  "      |    ~~    Select  Run     II    I   |\n"
	  "      *------------------------------------*\n\n", which + 1,
	  config[current_config].individual_config[which].joy_mapping[J_PYAXIS],
	  config[current_config].individual_config[which].joy_mapping[J_PAUTOII],
	  config[current_config].individual_config[which].joy_mapping[J_PAUTOI],
	  config[current_config].individual_config[which].joy_mapping[J_PXAXIS],
	  config[current_config].individual_config[which].joy_mapping[J_PII],
	  config[current_config].individual_config[which].joy_mapping[J_PI],
	  config[current_config].individual_config[which].joy_mapping[J_PSELECT],
	  config[current_config].individual_config[which].joy_mapping[J_PRUN]);

  fcntl (fileno (stdin), F_SETFL, sa);
}


void
sdl_init_joypads (void)
{
	int n;
	int joypad_number = SDL_NumJoysticks();

	MESSAGE_INFO("Found %d joypad%s\n",
		joypad_number, joypad_number > 1 ? "s" : "");

	for (n = 0; n < joypad_number; n++) {
//      if (config[current_config].individual_config[n].joydev == 0)
//	{
//	  joypad[n] = NULL;
//	  continue;
//	}

		if ((joypad[n] = SDL_JoystickOpen(n)) == NULL) {
			MESSAGE_ERROR("SDL could not open system joystick device %d (%s)\n",
				n, SDL_GetError ());
			continue;
		}

		//      printf("joypad[%d] = %p\n", n, joypad[n]);

		MESSAGE_INFO("PCE joypad %d: %s, %d axes, %d buttons\n",
			n + 1, SDL_JoystickName (n), SDL_JoystickNumAxes (joypad[n]),
			SDL_JoystickNumButtons (joypad[n]));

	//      if (option.configure_joypads)
	//	sdl_config_joypad (n);
	}
}


int
osd_keyboard (void)
{
// char tmp_joy;

	SDL_Event event;

#warning "need for pause support"
/*
* while (key[KEY_PAUSE])
* pause ();
*/

	while (SDL_PollEvent (&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_F12:
					case SDLK_ASTERISK:
					{
						uint32 sav_timer = timer_60;
						osd_snd_set_volume(0);
						disass_menu();
						osd_snd_set_volume(gen_vol);
						timer_60 = sav_timer;
						key_delay = 10;
						break;
					}

					case SDLK_F2:
					{
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[0]
								= !io.psg_channel_disabled[0];
						} else {
							uint32 sav_timer = timer_60;
							osd_snd_set_volume (0);
							searchbyte ();
							osd_snd_set_volume(gen_vol);
							timer_60 = sav_timer;
							return 0;
						}
						break;
					}

					case SDLK_F3:
					{
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[1]
								= !io.psg_channel_disabled[1];
						} else {
							uint32 sav_timer = timer_60;
							osd_snd_set_volume (0);
							pokebyte ();
							osd_snd_set_volume (gen_vol);
							timer_60 = sav_timer;
							return 0;
						}
						break;
					}

					// TODO: Put these in GUI
					//case SDLK_F4:
					//{
					//	if (key[SDLK_LSHIFT]) {
					//		io.psg_channel_disabled[2]
					//			= !io.psg_channel_disabled[2];
					//	} else {
					//		uint32 sav_timer = timer_60;
					//		osd_snd_set_volume (0);
					//		freeze_value ();
					//		osd_snd_set_volume (gen_vol);
					//		timer_60 = sav_timer;
					//		return 0;
					//	}
					//	break;
					//}

					//case SDLK_F5:
					//{
					//	if (key[SDLK_LSHIFT])
					//	{
					//		io.psg_channel_disabled[3]
					//			= !io.psg_channel_disabled[3];
					//	} else if (key[SDLK_RSHIFT]) {
					//		if (dump_snd) {
					//			stop_dump_audio ();
					//			dump_snd = 0;
					//		} else {
					//			dump_snd = start_dump_audio ();
					//		}
					//	} else {
					//		if (video_dump_flag || dump_snd) {
					//			stop_dump_video ();
					//			stop_dump_audio ();
					//			video_dump_flag = 0;
					//			dump_snd = 0;
					//		} else {
					//			if (start_dump_video()
					//				&& start_dump_audio()) {
					//				stop_dump_video();
					//				stop_dump_audio();
					//			}
					//		}
					//	}
					//	break;
					//}

					case SDLK_F9:
						SDL_ShowCursor(ToggleFullScreen());
						break;

					case SDLK_ESCAPE:
						return 1;

					case SDLK_F5:
					{
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[5]
								= !io.psg_channel_disabled[5];
						} else {
							uint32 sav_timer = timer_60;
							{
								// AGetVoiceVolume(hVoice, &vol);
								// ASetVoiceVolume(hVoice, 0);
								if (!silent) {
								/*
								voice_stop(PCM_voice);
								set_volume(1, 1);
								voice_set_volume(PCM_voice, 1);
								voice_start(PCM_voice);
								*/
									if (sound_driver == 1)
										osd_snd_set_volume (0);
									}
							}
								if (!savegame()) {
									osd_gfx_set_message("Game state saved\n");
									message_delay = 180;
								}
								if (!silent)
									osd_snd_set_volume (gen_vol);
								timer_60 = sav_timer;
						}
						break;
					}

					case SDLK_F6:
					{
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[4]
								= !io.psg_channel_disabled[4];
						} else {
							uint32 sav_timer = timer_60;
							osd_gfx_savepict();
							osd_gfx_set_message("Screenshot saved");
							message_delay = 180;
							key_delay = 10;
							timer_60 = sav_timer;
						}
						break;
					}

					case SDLK_F10:
					{
						synchro = !synchro;
						osd_gfx_set_message("SYNCHRONISATION TO 60HZ");
						message_delay = 180;
						key_delay = 10;
						break;
					}

					case SDLK_NUMLOCK:
					{
						if (dump_snd) {
							uint32 dummy;
							dummy = filesize (out_snd);
							fseek (out_snd, 4, SEEK_SET);
							fwrite (&dummy, 1, 4, out_snd);
							dummy -= 0x2C;
							fseek (out_snd, 0x28, SEEK_SET);
							fwrite (&dummy, 1, 4, out_snd);
							fclose (out_snd);
							osd_gfx_set_message ("Audio dumping off");
						} else {
							unsigned short tmp = 0;
							strcpy (tmp_buf, "snd0000.wav");
							while ((tmp < 0xFFFF)
								&& ((out_snd = fopen (tmp_buf, "rb"))
								!= NULL)) {
								sprintf (tmp_buf, "snd%04x.wav", ++tmp);
								fclose(out_snd);
							}
							out_snd = fopen (tmp_buf, "wb");
							fwrite ("RIFF\145\330\073\0WAVEfmt ", 16, 1, out_snd);
							putc (0x10, out_snd);	// size
							putc (0x00, out_snd);
							putc (0x00, out_snd);
							putc (0x00, out_snd);
							putc (1, out_snd);	// PCM data
							putc (0, out_snd);
							putc (1, out_snd);	// mono
							putc (0, out_snd);
							putc (host.sound.freq, out_snd);	// frequency
							putc (host.sound.freq >> 8, out_snd);
							putc (host.sound.freq >> 16, out_snd);
							putc (host.sound.freq >> 24, out_snd);
							putc (host.sound.freq, out_snd);	// frequency
							putc (host.sound.freq >> 8, out_snd);
							putc (host.sound.freq >> 16, out_snd);
							putc (host.sound.freq >> 24, out_snd);
							putc (1, out_snd);
							putc (0, out_snd);
							putc (8, out_snd);	// 8 bits
							putc (0, out_snd);
							fwrite ("data\377\377\377\377", 1, 9, out_snd);
							osd_gfx_set_message ("Audio dumping on");
						}
						dump_snd = !dump_snd;
						key_delay = 10;
						message_delay = 180;
						break;
					}

					case SDLK_KP_PLUS:
					{
						if (UPeriod < 15) {
							UPeriod++;
							sprintf (tmp_buf, "Frame Skip: %d", UPeriod);
							osd_gfx_set_message (tmp_buf);
							message_delay = 180;
						};
						key_delay = 10;
						break;
					}

					case SDLK_KP_MINUS:
					{
						if (UPeriod) {
							UPeriod--;
							if (!UPeriod) {
								osd_gfx_set_message("Frame Skip: Disabled");
							} else {
								sprintf (tmp_buf, "Frame Skip: %d", UPeriod);
								osd_gfx_set_message (tmp_buf);
							};
							message_delay = 180;
						};
						key_delay = 10;
					}
						break;

					case SDLK_1:
					{
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[0]
								= !io.psg_channel_disabled[0];
						} else {
							SPONSwitch = !SPONSwitch;
							key_delay = 10;
						}
						break;
					}

					case SDLK_2:
					{
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[1]
								= !io.psg_channel_disabled[1];
						} else {
							BGONSwitch = !BGONSwitch;
							key_delay = 10;
						}
						break;
					}

					case SDLK_F7:
					{
						uint32 sav_timer = timer_60;
						if (!loadgame ()) {
							osd_gfx_set_message("Load game state\n");
							message_delay = 180;
						}
						timer_60 = sav_timer;
						break;
					}

					case SDLK_BACKQUOTE:	/* TILDE */
					{
						char *tmp = (char *) alloca (100);
						sprintf (tmp, "FRAME DELTA = %d",
							frame - HCD_frame_at_beginning_of_track);
						osd_gfx_set_message (tmp);
						message_delay = 240;
						break;
					}

					case SDLK_3:
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[2]
								= !io.psg_channel_disabled[2];
						} else {
							// cd_port_1800 = 0xD0;
							io.cd_port_1800 &= ~0x40;
						}
						break;

					case SDLK_4:
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[3]
								= !io.psg_channel_disabled[3];
						}
						/*
						else
						{
							Wr6502 (0x2066, Rd6502 (0x2066) | 32);
						}
						*/
						break;

					case SDLK_5:
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[4]
								= !io.psg_channel_disabled[4];
						} else {
							io.cd_port_1800 = 0xD8;
						}
						break;

					case SDLK_6:
						if (key[SDLK_LSHIFT]) {
							io.psg_channel_disabled[5]
								= !io.psg_channel_disabled[5];
						}
						/*
						else
						{
							Wr6502 (0x22D6, 1);
						}
						*/
						break;

						/*
						case SDLK_8:
							Wr6502 (0x20D8, 128);
							break;
						*/

					case SDLK_MINUS:
					{
						if (gen_vol > 0)
							gen_vol -= 17;
						else
							gen_vol = 0;

						osd_snd_set_volume(gen_vol);
						drawVolume("Effect Vol. ", gen_vol);
						message_delay = 60;

						break;
					}

					case SDLK_EQUALS:
					{
						if (gen_vol < 255)
							gen_vol += 17;
						else
							gen_vol = 255;

						osd_snd_set_volume(gen_vol);
						drawVolume("Effect Vol. ", gen_vol);
						message_delay = 60;
						break;
					}

					default:
						/* ignore key */
						break;
				}
				break;

			case SDL_QUIT:
				return 1;
				break;
		}
	}

	//! Read events from joypad
	SDL_PumpEvents ();

#if defined(ENABLE_NETPLAY)

  if (option.want_netplay)
    {

      uchar local_input[5];
      int input_index;

      for (input_index = 0; input_index < 5; input_index++)
	{
	  local_input[input_index] = read_input (input_index);
	}

      if (option.want_netplay == LAN_PROTOCOL)
	{
	  wait_lan_digest_status (local_input);
	}
      else
	{
	  /* option.want_netplay == INTERNET_PROTOCOL */
	  wait_internet_digest_status (local_input);
	}
      return 0;
    }

#endif // ENABLE_NETPLAY

  io.JOY[0] = read_input (0);
  io.JOY[1] = read_input (1);
  io.JOY[2] = read_input (2);
  io.JOY[3] = read_input (3);
  io.JOY[4] = read_input (4);

  return 0;
}


int
osd_init_input(void)
{
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
		MESSAGE_ERROR("Unable to init SDL Input layer!\n");
		return 1;
	}

	sdl_init_joypads ();

	return 0;
}


void
osd_shutdown_input ()
{
}

#if defined(ENABLE_NETPLAY)

//! initialise netplay, returns 0 if ok
int
osd_init_netplay()
{
	if (option.want_netplay)
    {
      if (init_network () != 0)
	{
	  return -1;
	}
      Log ("Netplay subsystem initialized\n");

      /* TODO: Add configurable port number */
      /* TODO: Add support for ip address directly */
      if (SDLNet_ResolveHost
	  (&server_address, option.server_hostname, DEFAULT_SERVER_PORT) != 0)
	{
	  Log ("Couldn't resolve server address (%s)\n",
	       option.server_hostname);
	  return -1;
	}

      /* Allocate client socket */
      client_socket = SDLNet_UDP_Open (0);

      if (client_socket == NULL)
	{
	  Log ("Couldn't open client socket\n");
	  return -1;
	}

      /* Allocate incoming packet storage */
      incoming_packet = SDLNet_AllocPacket (SERVER_PACKET_SIZE);

      if (incoming_packet == NULL)
	{
	  Log
	    ("Couldn't allocate incoming packet storage (not enough memory ?)\n");
	  SDLNet_UDP_Close (client_socket);
	  return -1;
	}

      /* Allocate outgoing packet storage */
      outgoing_packet = SDLNet_AllocPacket (CLIENT_PACKET_SIZE);

      if (outgoing_packet == NULL)
	{
	  Log
	    ("Couldn't allocate outgoing packet storage (not enough memory ?)\n");
	  SDLNet_UDP_Close (client_socket);
	  SDLNet_FreePacket (incoming_packet);
	  return -1;
	}

      /* Allocate a single socket set */
      client_socket_set = SDLNet_AllocSocketSet (1);

      if (client_socket_set == NULL)
	{
	  Log
	    ("Couldn't allocate socket set storage (not enough memory ?)\n");
	  SDLNet_UDP_Close (client_socket);
	  SDLNet_FreePacket (incoming_packet);
	  SDLNet_FreePacket (outgoing_packet);
	  return -1;
	}

      /* Add the client socket to the set used to passively wait */
      if (SDLNet_UDP_AddSocket (client_socket_set, client_socket) != 1)
	{
	  Log ("Error when adding socket to socket set.\n");
	  SDLNet_UDP_Close (client_socket);
	  SDLNet_FreePacket (incoming_packet);
	  SDLNet_FreePacket (outgoing_packet);
	  SDLNet_FreeSocketSet (client_socket_set);
	  return -1;
	}

      send_identification (option.local_input_mapping);

    }

	return 0;

}

void
osd_shutdown_netplay()
{

  if (option.want_netplay)
    {
      SDLNet_UDP_Close (client_socket);
      SDLNet_FreePacket (incoming_packet);
      SDLNet_FreePacket (outgoing_packet);
      SDLNet_FreeSocketSet (client_socket_set);
    }

}


#if defined(NETPLAY_DEBUG)

/*!
 * Print a string adding an accurate timestamp
 * \param format the printf style format string
 * \param ... the extra parameters for the printf
 */
void
netplay_debug_printf (char *format, ...)
{
  FILE *log_file;
  char buf[256];
  char time_buffer[256];
  struct timeb time_structure;

  va_list ap;
  va_start (ap, format);
  vsprintf (buf, format, ap);
  va_end (ap);

  ftime (&time_structure);

  ctime_r (&time_structure.time, time_buffer);

  if (strlen (time_buffer) > 0) {
      /* If this buffer is not empty, remove the last character (which is normally a newline) */
      time_buffer[strlen (time_buffer) - 1] = 0;
  }

  fprintf (stderr, "%s:%3u ", time_buffer, time_structure.millitm);
  fprintf (stderr, buf);
  fprintf (stderr, "\n");

  fflush (stderr);

  return;
}

#endif

/*!
 * Read an incoming packet and stores it for future computations
 * \return non null if we could read a packet
 * \return 0 if we didn't read a packet (we then set the len to 0 to prevent matching)
 */
int
read_incoming_client_packet ()
{
  switch (SDLNet_UDP_Recv (client_socket, incoming_packet))
    {
    case 0:
      incoming_packet->len = 0;
      return 0;
    case 1:
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf ("Stored a packet from server.");
#endif
      return 1;
    default:
      Log ("Internal warning: impossible case (%s:%s)\n", __FILE__, __LINE__);
      incoming_packet->len = 0;
      return 0;
    }
  return 0;
}

/*!
 * Check validity of the identification acknowledge packet stored in 'incoming_packet' and extract
 * the number of allocated slots if the packet is valid
 * \return 0 in case of invalid packet reception
 * \return else, number of allocated slots
 */
int
count_allocated_slots ()
{

  int allocated_slots;

  if (incoming_packet->len != PACKET_IDENTIFICATION_ACKNOWLEDGE_LENGTH)
    {
      /* Discarding packet of invalid length */
#if defined(NETPLAY_DEBUG)
      Log
	("Invalid packet received when in identification acknowledge phase (received length: %d, expected length: %d)\n",
	 incoming_packet->len, PACKET_IDENTIFICATION_ACKNOWLEDGE_LENGTH);
#endif
      return 0;
    }

  if (incoming_packet->data[PACKET_IDENTIFIER] !=
      PACKET_IDENTIFICATION_ACKNOWLEDGE_ID)
    {
      /* Discarding packet with invalid identifier */
#if defined(NETPLAY_DEBUG)
      Log
	("Invalid packet received when in identification acknowledge phase (received ID: 0x%02x, expected ID: 0x%02x)\n",
	 incoming_packet->data[PACKET_IDENTIFIER],
	 PACKET_IDENTIFICATION_ACKNOWLEDGE_ID);
#endif
      return 0;
    }

  /* TODO : Add checksum verification */

  allocated_slots =
    incoming_packet->
    data[PACKET_IDENTIFICATION_ACKNOWLEDGE_NUMBER_ALLOCATED_SLOTS];

#if defined(NETPLAY_DEBUG)
  Log ("Received identification acknowledge with %d allocated slot(s)\n",
       allocated_slots);
#endif

  if ((allocated_slots < 1) || (allocated_slots > 5))
    {
      Log ("Invalid number of allocated slots : %d\n", allocated_slots);
      return 0;
    }

  /* We're finished with the acknowledge packet, let's clear it */
  bzero (incoming_packet->data, SERVER_PACKET_SIZE);

  return allocated_slots;

}

/*!
 * Wait for identification acknowledge packet or timeout
 * \return 0 in case of time out or invalid packet reception
 * \return non null in case of acknowledge packet reception with the number of allocated slots
 */
int
wait_identification_acknowledge_packet ()
{

  int number_ready_socket;

#if defined(NETPLAY_DEBUG)
  Log ("Waiting for acknowledge packet\n");
#endif

  /* Wait for activity on the socket */
  number_ready_socket =
    SDLNet_CheckSockets (client_socket_set, CLIENT_SOCKET_TIMEOUT);

#if defined(NETPLAY_DEBUG)
  Log ("Finished waiting, number of socket ready = %d\n",
       number_ready_socket);
#endif

  switch (number_ready_socket)
    {
    case -1:
      Log ("Unknown error when waiting for activity on socket\n");
      return 0;
    case 0:
      /* No activity */
      break;
    case 1:

#if defined(NETPLAY_DEBUG)
      Log ("Grabbed a packet\n");
#endif

      /* A single socket is ready */
      if (read_incoming_client_packet ())
	{
	  return count_allocated_slots ();
	}
      return 0;
      break;
    }

  return 0;
}

/*!
 * Send a single packet to warn the server of our need in terms of slots
 */
void
send_identification_packet (uchar * local_input_mapping)
{
  uchar number_requested_slots;
  int input_index;
  int number_destination;

  number_requested_slots = 0;

  /* Set the server address */
  outgoing_packet->address = server_address;

  /* Set the identifier */
  outgoing_packet->data[PACKET_IDENTIFIER] = PACKET_IDENTIFICATION_ID;

  /* Computes the number of requested slots to fit our needs */
  for (input_index = 0; input_index < 5; input_index++)
    {
      if (local_input_mapping[input_index] != 0xFF)
	{
	  number_requested_slots++;
	}
    }

  Log ("Asking for %d slots to server\n", number_requested_slots);

  /* Set the number of requested slots */
  outgoing_packet->data[PACKET_IDENTIFICATION_NUMBER_REQUESTED_SLOTS] =
    number_requested_slots;

  /* Set the mapping to local input devices */
  for (input_index = 0;
       input_index < PACKET_IDENTIFICATION_INPUT_DEVICE_NUMBER; input_index++)
    {
      outgoing_packet->data[PACKET_IDENTIFICATION_INPUT_DEVICE_INDEX +
			    input_index] = local_input_mapping[input_index];
    }

  /* Set checksum value */
  outgoing_packet->data[PACKET_IDENTIFICATION_CHECKSUM] =
    compute_checksum (outgoing_packet->data, PACKET_IDENTIFIER,
		      PACKET_IDENTIFICATION_CHECKSUM);

  /* Set the length of the packet */
  outgoing_packet->len = PACKET_IDENTIFICATION_LENGTH;

  number_destination = SDLNet_UDP_Send (client_socket, -1, outgoing_packet);

  if (number_destination != 1)
    {
      Log ("Couldn't send the status packet to client\n");
    }

}

/*!
 * Ensures identification is done properly
 * \param local_input_mapping mapping between the remote (global) and local input
 */
void
send_identification (uchar * local_input_mapping)
{
  /* TODO : add checking for number of acknowledged slots against requested one */
  do
    {
      send_identification_packet (local_input_mapping);
    }
  while (wait_identification_acknowledge_packet () == 0);
}

/*!
 * Send a packet for explaining what is the current local status
 */
void
send_local_status (uchar * local_input)
{
  int input_index;
  int number_destination;

  /* Set the server address */
  outgoing_packet->address = server_address;

  /* Set the identifier */
  outgoing_packet->data[PACKET_IDENTIFIER] = PACKET_STATUS_ID;

  /* Set the frame number request */
  SDLNet_Write32 (frame, outgoing_packet->data + PACKET_STATUS_FRAME_NUMBER);

  /* Set the local input values */
  for (input_index = 0; input_index < PACKET_STATUS_INPUT_DEVICE_NUMBER;
       input_index++)
    {
      outgoing_packet->data[PACKET_STATUS_INPUT_DEVICE_INDEX + input_index] =
	local_input[input_index];
    }

  /* Set checksum value */
  outgoing_packet->data[PACKET_STATUS_CHECKSUM] =
    compute_checksum (outgoing_packet->data, PACKET_IDENTIFIER,
		      PACKET_STATUS_CHECKSUM);

  /* Set the length of the packet */
  outgoing_packet->len = PACKET_STATUS_LENGTH;

  number_destination = SDLNet_UDP_Send (client_socket, -1, outgoing_packet);

  if (number_destination != 1)
    {
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf ("Couldn't send the status packet to server.");
#endif
      Log ("Couldn't send the status packet to server\n");
    }

}

/*!
 * Grab the digest packet for the current frame. Update the value of io.JOY
 * \return 0 in case of invalid packet
 * \return non null if the packet has been correctly processed
 */
int
handle_digest_packet ()
{

  int allocated_slots;
  int input_index;

  if (incoming_packet->len != PACKET_STATUS_LENGTH)
    {
      /* Discarding packet of invalid length */
#if defined(NETPLAY_DEBUG)
      Log
	("Invalid packet received when in digest phase (received length: %d, expected length: %d)\n",
	 incoming_packet->len, PACKET_STATUS_LENGTH);
#endif
      return 0;
    }

  if (incoming_packet->data[PACKET_IDENTIFIER] != PACKET_DIGEST_ID)
    {
      /* Discarding packet with invalid identifier */
#if defined(NETPLAY_DEBUG)
      Log
	("Invalid packet received when in digest phase (received ID: 0x%02x, expected ID: 0x%02x)\n",
	 incoming_packet->data[PACKET_IDENTIFIER], PACKET_DIGEST_ID);
#endif
      return 0;
    }

  if (frame !=
      SDLNet_Read32 (incoming_packet->data + PACKET_STATUS_FRAME_NUMBER))
    {
#if defined(NETPLAY_DEBUG)
      Log
	("Packet for incorrect frame (received digest for frame : %d, expected : %d)\n",
	 SDLNet_Read32 (incoming_packet->data + PACKET_STATUS_FRAME_NUMBER),
	 frame);
#endif
      return 0;
    }

  /* Verify checksum */
  if (incoming_packet->data[PACKET_STATUS_CHECKSUM] !=
      compute_checksum (incoming_packet->data, PACKET_IDENTIFIER,
			PACKET_STATUS_CHECKSUM))
    {
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("Incorrect checksum in received packet (received checksum : %d, expected : %d)",
	 incoming_packet->data[PACKET_STATUS_CHECKSUM],
	 compute_checksum (incoming_packet->data, PACKET_IDENTIFIER,
			   PACKET_STATUS_CHECKSUM));
      Log
	("Incorrect checksum in received packet (received checksum : %d, expected : %d)\n",
	 incoming_packet->data[PACKET_STATUS_CHECKSUM],
	 compute_checksum (incoming_packet->data, PACKET_IDENTIFIER,
			   PACKET_STATUS_CHECKSUM));
#endif
      return 0;
    }

  for (input_index = 0; input_index < PACKET_STATUS_INPUT_DEVICE_NUMBER;
       input_index++)
    {
      io.JOY[input_index] =
	incoming_packet->data[PACKET_STATUS_INPUT_DEVICE_INDEX + input_index];
    }

  return 1;

}

/*!
 * Wait for digest packet or timeout
 * \return 0 in case of timeout or invalid packet
 * \return non null if correct packet was received
 */
int
wait_digest_packet ()
{
  int number_ready_socket;

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf ("Waiting for digest packet.");
#endif

  /* Else, wait for activity on the socket */
  number_ready_socket =
    SDLNet_CheckSockets (client_socket_set, CLIENT_SOCKET_TIMEOUT);

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf ("Finished waiting, number of socket ready = %d.",
			number_ready_socket);
#endif

  switch (number_ready_socket)
    {
    case -1:
      Log ("Unknown error when waiting for activity on socket\n");
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("ERROR: Unknown error while waiting for socket activity.");
#endif
      return 0;
    case 0:
      /* No activity */
      return 0;
    case 1:

#if defined(NETPLAY_DEBUG)
      Log ("Grabbed a packet\n");
      netplay_debug_printf ("There's now a packet waiting to be processed.");
#endif

      /* A single socket is ready */
      if (read_incoming_client_packet ())
	{
	  return handle_digest_packet ();
	}

#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("ERROR: A packet was said to be waiting while there's none.");
#endif

      return 0;
      break;
    }

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf
    ("ERROR: After waiting more than one socket was ready !");
#endif

  return 0;
}

/*!
 * Wait for the digest packet using the lan protocol.
 * Updates io.JOY
 */
void
wait_lan_digest_status (uchar * local_input)
{

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf ("Entering loop for waiting digest for frame %d.",
			frame);
#endif

  do
    {
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("Going to send a local status / digest request packet.");
#endif
      send_local_status (local_input);
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf ("Sent a local status / digest request packet.");
#endif
    }
  while (wait_digest_packet () == 0);
}

/*!
 * Send a packet to server by specifying the frame number. It allows to send a packet with ou without
 * a request in internet mode.
 * \param local_input the local input status
 * \param frame_number the frame number value in the packet
 */
void
send_internet_local_status (uchar * local_input, uint32 frame_number)
{
  int input_index;
  int number_destination;

  /* Set the server address */
  outgoing_packet->address = server_address;

  /* Set the identifier */
  outgoing_packet->data[PACKET_IDENTIFIER] = PACKET_STATUS_ID;

  /* Set the requested frame number */
  SDLNet_Write32 (frame_number,
		  outgoing_packet->data + PACKET_STATUS_FRAME_NUMBER);

  /* Set local input values */
  for (input_index = 0; input_index < PACKET_STATUS_INPUT_DEVICE_NUMBER;
       input_index++)
    {
      outgoing_packet->data[PACKET_STATUS_INPUT_DEVICE_INDEX + input_index] =
	local_input[input_index];
    }

  /* Set checksum value */
  outgoing_packet->data[PACKET_STATUS_CHECKSUM] =
    compute_checksum (outgoing_packet->data, PACKET_IDENTIFIER,
		      PACKET_STATUS_CHECKSUM);

  /* Set the length of the packet */
  outgoing_packet->len = PACKET_STATUS_LENGTH;

  number_destination = SDLNet_UDP_Send (client_socket, -1, outgoing_packet);

  if (number_destination != 1)
    {
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf ("Couldn't send the status packet to client.");
#endif
      Log ("Couldn't send the status packet to client\n");
    }

}

/*!
 * Lookup the digest for the current frame status. It then updates io.JOY if found.
 * \return non null value if io.JOY was updated thanks to the current incoming packet content
 * \return null else
 */
int
search_digest_packet ()
{
  int number_digest;
  uint32 start_frame_number;

  /* Look up the number of digest available in the incoming packet */
  number_digest = incoming_packet->data[PACKET_INTERNET_DIGEST_NUMBER_DIGEST];

#if defined(NETPLAY_DEBUG)
  if (number_digest > PACKET_INTERNET_DIGEST_DIGEST_NUMBER)
    {
      Log ("Abnormal number of digest in the incoming packet (%d)\n",
	   number_digest);
    }

#endif

  start_frame_number =
    SDLNet_Read32 (incoming_packet->data +
		   PACKET_INTERNET_DIGEST_FRAME_NUMBER);

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf
    ("We're looking frame %d and we have frames %d-%d handy\n", frame,
     start_frame_number, start_frame_number + number_digest - 1);
#endif

  if ((frame >= start_frame_number)
      && (frame < start_frame_number + number_digest))
    {
      /* We found the frame info we need */
      memcpy (io.JOY,
	      incoming_packet->data + PACKET_INTERNET_DIGEST_DIGEST_INDEX +
	      DIGEST_SIZE * (frame - start_frame_number), DIGEST_SIZE);

      return 1;
    }

  return 0;
}

/*!
 * Check that the packet is valid, the exploitation of result in it is done in the search_digest_packet procedure. The number of
 * digest value of this packet is nullified on error.
 */
void
handle_internet_digest_packet ()
{

  int number_digest;

#if defined(NETPLAY_DEBUG)
  Log ("Received a packet for handling (length : %d), content follows :\n",
       incoming_packet->len);

  {
    int packet_index;

    for (packet_index = 0; packet_index < incoming_packet->len;
	 packet_index++)
      {
	Log ("%02X ", incoming_packet->data[packet_index]);
      }

    Log ("\n");

  }
#endif

  if ((incoming_packet->len % PACKET_INTERNET_DIGEST_INCREMENT_LENGTH !=
       PACKET_INTERNET_DIGEST_BASE_LENGTH %
       PACKET_INTERNET_DIGEST_INCREMENT_LENGTH)
      && (incoming_packet->len >=
	  PACKET_INTERNET_DIGEST_BASE_LENGTH +
	  PACKET_INTERNET_DIGEST_INCREMENT_LENGTH))
    {
      /* Discarding packet of invalid length */
#if defined(NETPLAY_DEBUG)
      Log
	("Invalid packet received when in internet digest phase (received length: %d)\n",
	 incoming_packet->len);
#endif
      incoming_packet->data[PACKET_INTERNET_DIGEST_NUMBER_DIGEST] = 0;
      return;
    }

  if (incoming_packet->data[PACKET_IDENTIFIER] != PACKET_INTERNET_DIGEST_ID)
    {
      /* Discarding packet with invalid identifier */
#if defined(NETPLAY_DEBUG)
      Log
	("Invalid packet received when in internet digest phase (received ID: 0x%02x, expected ID: 0x%02x)\n",
	 incoming_packet->data[PACKET_IDENTIFIER], PACKET_INTERNET_DIGEST_ID);
#endif
      incoming_packet->data[PACKET_INTERNET_DIGEST_NUMBER_DIGEST] = 0;
      return;
    }

  number_digest = incoming_packet->data[PACKET_INTERNET_DIGEST_NUMBER_DIGEST];

  if (number_digest < 1)
    {
#if defined(NETPLAY_DEBUG)
      Log
	("Invalid packet received when in internet digest phase (received ID: 0x%02x, expected ID: 0x%02x)\n",
	 incoming_packet->data[PACKET_IDENTIFIER], PACKET_INTERNET_DIGEST_ID);
#endif
      incoming_packet->data[PACKET_INTERNET_DIGEST_NUMBER_DIGEST] = 0;
      return;
    }

  /* TODO: maybe add a further check for the number of digest offered */

  /* Verify checksum */
  if (incoming_packet->
      data[PACKET_INTERNET_DIGEST_BASE_LENGTH +
	   PACKET_INTERNET_DIGEST_INCREMENT_LENGTH * number_digest] !=
      compute_checksum (incoming_packet->data, PACKET_IDENTIFIER,
			PACKET_INTERNET_DIGEST_BASE_LENGTH +
			PACKET_INTERNET_DIGEST_INCREMENT_LENGTH *
			number_digest))
    {
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("Incorrect checksum in received packet (received checksum : %d, expected : %d)",
	 incoming_packet->data[PACKET_INTERNET_DIGEST_BASE_LENGTH +
			       PACKET_INTERNET_DIGEST_INCREMENT_LENGTH *
			       number_digest],
	 compute_checksum (incoming_packet->data, PACKET_IDENTIFIER,
			   PACKET_INTERNET_DIGEST_BASE_LENGTH +
			   PACKET_INTERNET_DIGEST_INCREMENT_LENGTH *
			   number_digest));

      Log
	("Incorrect checksum in received packet (received checksum : %d, expected : %d)\n",
	 incoming_packet->data[PACKET_INTERNET_DIGEST_BASE_LENGTH +
			       PACKET_INTERNET_DIGEST_INCREMENT_LENGTH *
			       number_digest],
	 compute_checksum (incoming_packet->data, PACKET_IDENTIFIER,
			   PACKET_INTERNET_DIGEST_BASE_LENGTH +
			   PACKET_INTERNET_DIGEST_INCREMENT_LENGTH *
			   number_digest));

#endif
      incoming_packet->data[PACKET_INTERNET_DIGEST_NUMBER_DIGEST] = 0;
      return;
    }

  /* If we managed to get here, everything looks fine */

}

/*!
 * Removes all pending packets.
 */
void
flush_waiting_packets ()
{
  /* TODO: a better implementation can be achieved with the vector reception stuff */
  while (SDLNet_UDP_Recv (client_socket, incoming_packet) !=
	 0) /* Nothing on purpose */ ;
}

/*!
 * Wait for an incoming packet or an internet protocol timeout
 */
void
wait_internet_digest_packet ()
{
  int number_ready_socket;

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf ("Waiting for digest packet for frame %d.", frame);
#endif

  /* Wait for activity on the socket */
  number_ready_socket =
    SDLNet_CheckSockets (client_socket_set, CLIENT_SOCKET_INTERNET_TIMEOUT);

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf ("Finished waiting, number of socket ready = %d.",
			number_ready_socket);
#endif

  switch (number_ready_socket)
    {
    case -1:
      Log ("Unknown error when waiting for activity on socket\n");
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("ERROR: Unknown error while waiting for socket activity.");
#endif
      return;
    case 0:
      /* No activity */
      return;
    case 1:

#if defined(NETPLAY_DEBUG)
      Log ("Grabbed a packet\n");
      netplay_debug_printf ("There's now a packet waiting to be processed.");
#endif

      /* A single socket is ready */
      if (read_incoming_client_packet ())
	{
	  handle_internet_digest_packet ();
	  /* We're not much interested in old packets, they only raise lag */
	  /* TODO: check how sane I was when I wrote this */
	  /* flush_waiting_packets(); */
	  return;
	}

#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("ERROR: A packet was said to be waiting while there's none.");
#endif

      return;
      break;
    }

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf
    ("ERROR: After waiting more than one socket was ready !");
#endif

  return;

}

/*!
 * Wait for the digest packet using the internet protocol.
 * Updates io.JOY
 */
void
wait_internet_digest_status (uchar * local_input)
{

#if defined(NETPLAY_DEBUG)
  netplay_debug_printf ("Going to send a pure local status packet.");
#endif

  send_internet_local_status (local_input, 0);

  if (search_digest_packet () != 0)
    {
      /* The digest was found and io.JOY updated */
#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("Found the digest info for frame %d in the previously stored packet",
	 frame);
#endif
      return;
    }

  for (;;)
    {
      wait_internet_digest_packet ();

      if (search_digest_packet () != 0)
	{
	  /* The digest was found and io.JOY updated */
#if defined(NETPLAY_DEBUG)
	  netplay_debug_printf
	    ("Found the digest info for frame %d in the freshly stored packet",
	     frame);
#endif
	  return;
	}

#if defined(NETPLAY_DEBUG)
      netplay_debug_printf
	("Going to send a local status / digest request packet for frame %d.",
	 frame);
#endif
      send_internet_local_status (local_input, frame);

    }
}

#endif // ENABLE_NETPLAY

#endif