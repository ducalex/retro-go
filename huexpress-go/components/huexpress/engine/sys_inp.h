#ifndef _INCLUDE_SYS_INP_H
#define _INCLUDE_SYS_INP_H

#include "cleantypes.h"
#if defined(ENABLE_NETPLAY)
#include "SDL_net.h"
#endif

/*
 * Input section
 *
 * This one function part implements the input functionality
 * It's called every vsync I think, i.e. almost 60 times a sec
 */

/*!
 * Updates the Joypad variables, save/load game, make screenshots, display
 * gui or launch fileselector if asked
 * \return 1 if we must quit the emulation
 * \return 0 else
 */
int osd_keyboard(void);

/*!
 * Initialize the input services
 * \return 1 if the initialization failed
 * \return 0 on success
 */
int osd_init_input(void);

/*!
 * Shutdown input services
 */
void osd_shutdown_input(void);

#if defined(ENABLE_NETPLAY)
/*!
 * Initialize the netplay support
 * \return 1 if the initialization failed
 * \return 0 on success
 */
int osd_init_netplay(void);

/*!
 * Shutdown netplay support
 */
void osd_shutdown_netplay(void);
#endif

/*!
 * Number of the input configuration
 */
extern uchar current_config;

/*
 * joymap
 *
 *
 */
typedef enum {
	J_UP = 0,
	J_DOWN,
	J_LEFT,
	J_RIGHT,
	J_I,
	J_II,
	J_SELECT,
	J_RUN,
	J_AUTOI,
	J_AUTOII,
	J_PI,
	J_PII,
	J_PSELECT,
	J_PRUN,
	J_PAUTOI,
	J_PAUTOII,
	J_PXAXIS,
	J_PYAXIS,
	J_MAX,

	J_PAD_START = J_PI
} joymap;

extern const char *joymap_reverse[J_MAX];

/* 
 * input_config
 *
 * Definition of the type representating an input configuration
 */

/*
 * local_input means the input data can be read locally and need not been relayed
 * server_input means the data 
 */

typedef enum { LOCAL_PROTOCOL, LAN_PROTOCOL,
		INTERNET_PROTOCOL } netplay_type;

typedef struct {

	//! Mouse device, 0 for none
	uchar mousedev;

	//! Joypad device, 0 for none
	uchar joydev;

	//! whether autofire is set
	uchar autoI;
	uchar autoII;

	//! whether autofire triggered event
	uchar firedI;
	uchar firedII;

	//! mapping for joypad and keyboard
	//! J_UP to J_PAD_START (excluded) are for use with the keyboard, others are
	//! for the joypad
	uint16 joy_mapping[J_MAX];

} individual_input_config;

typedef struct {
	individual_input_config individual_config[5];
} input_config;

/*
 * config
 *
 * The array of input configuration settings
 */
extern input_config config[16];

/* defines for input type field */

#define NONE      0

#define KEYBOARD1 1
#define KEYBOARD2 2
#define KEYBOARD3 3
#define KEYBOARD4 4
#define KEYBOARD5 5

#define JOYPAD1  11
#define JOYPAD2  12
#define JOYPAD3  13
#define JOYPAD4  14

#define MOUSE1   20
#define MOUSE2   21

#define SYNAPLINK 255

	/*
	 * The associated prototypes for common input functions
	 */

/* for nothing */
uint16 noinput();

/* for keyboard/gamepad input */
uint16 input1();
uint16 input2();
uint16 input3();
uint16 input4();
uint16 input5();

/* for joypad */
uint16 joypad1();
uint16 joypad2();
uint16 joypad3();
uint16 joypad4();

/* for mouse */
uint16 mouse1();
uint16 mouse2();

/* for synaptic link */
uint16 synaplink();

#endif
