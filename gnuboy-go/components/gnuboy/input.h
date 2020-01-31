/*
 * input.h
 *
 * Definitions for input device stuff - buttons, keys, etc.
 */


// #define M_IGNORE 0
// #define M_RELATIVE 1
// #define M_ABSOLUTE 2
//
//
// #define K_SHIFT 0x101
// #define K_CTRL 0x102
// #define K_ALT 0x103
//
// #define K_UP 0x10a
// #define K_DOWN 0x10b
// #define K_RIGHT 0x10c
// #define K_LEFT 0x10d
//
// #define K_ENTER '\r'
// #define K_TAB '\t'
// #define K_SPACE ' '
// #define K_BS 010
// #define K_DEL 127
// #define K_INS 0x121
// #define K_HOME 0x122
// #define K_END 0x123
// #define K_PRIOR 0x124
// #define K_NEXT 0x125
// #define K_ESC 033
// #define K_SYSRQ 0x1fe
// #define K_PAUSE 0x1ff
// #define K_CAPS 0x1f1
// #define K_NUMLOCK 0x1f2
// #define K_SCROLL 0x1f3
//
// #define K_MINUS '-'
// #define K_EQUALS '='
// #define K_TILDE '~'
// #define K_SLASH '/'
// #define K_BSLASH '\\'
// #define K_SEMI ';'
// #define K_QUOTE '\''
//
// #define K_F1 0x131
// #define K_F2 0x132
// #define K_F3 0x133
// #define K_F4 0x134
// #define K_F5 0x135
// #define K_F6 0x136
// #define K_F7 0x137
// #define K_F8 0x138
// #define K_F9 0x139
// #define K_F10 0x13a
// #define K_F11 0x13b
// #define K_F12 0x13c
//
// #define K_NUM0 0x140
// #define K_NUM1 0x141
// #define K_NUM2 0x142
// #define K_NUM3 0x143
// #define K_NUM4 0x144
// #define K_NUM5 0x145
// #define K_NUM6 0x146
// #define K_NUM7 0x147
// #define K_NUM8 0x148
// #define K_NUM9 0x149
// #define K_NUMPLUS 0x14a
// #define K_NUMMINUS 0x14b
// #define K_NUMMUL 0x14c
// #define K_NUMDIV 0x14d
// #define K_NUMDOT 0x14e
// #define K_NUMENTER 0x14f
//
// #define K_MOUSE0 0x1a0
// #define K_MOUSE1 0x1a1
// #define K_MOUSE2 0x1a2
// #define K_MOUSE3 0x1a3
// #define K_MOUSE4 0x1a4
//
// #define K_JOY0 0x1b0
// #define K_JOY1 0x1b1
// #define K_JOY2 0x1b2
// #define K_JOY3 0x1b3
// #define K_JOY4 0x1b4
// #define K_JOY5 0x1b5
// #define K_JOY6 0x1b6
// #define K_JOY7 0x1b7
// #define K_JOY8 0x1b8
// #define K_JOY9 0x1b9
// #define K_JOY10 0x1ba
// #define K_JOY11 0x1bb
// #define K_JOY12 0x1bc
// #define K_JOY13 0x1bd
// #define K_JOY14 0x1be
// #define K_JOY15 0x1bf
//
// #define K_JOYUP 0x1ca
// #define K_JOYDOWN 0x1cb
// #define K_JOYRIGHT 0x1cc
// #define K_JOYLEFT 0x1cd
//
// #define MAX_KEYS 0x200

typedef struct keytable_s
{
	char *name;
	int code;
} keytable_t;

//extern keytable_t keytable[];
//extern char keystates[MAX_KEYS];
//extern int nkeysdown;


//int k_keycode(char *name);
//char *k_keyname(int code);



typedef struct event_s
{
	int type;
	int code;
	int dx, dy;
	int x, y;
} event_t;

#define EV_NONE 0
#define EV_PRESS 1
#define EV_RELEASE 2
#define EV_REPEAT 3
#define EV_MOUSE 4

int ev_postevent(event_t *ev);
int ev_getevent(event_t *ev);
