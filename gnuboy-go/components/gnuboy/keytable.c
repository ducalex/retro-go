#if 0
/*
 * keytable.c
 *
 * Key names to keycodes mapping.
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "gnuboy.h"
#include "input.h"

/* keytable - Mapping of key names to codes, and back. A single code
   can have more than one name, in which case the first will be used
   when saving config, but any may be used in setting config. */

keytable_t keytable[] =
{
	{ "shift", K_SHIFT },
	{ "ctrl", K_CTRL },
	{ "alt", K_ALT },
	{ "up", K_UP },
	{ "down", K_DOWN },
	{ "right", K_RIGHT },
	{ "left", K_LEFT },
	{ "enter", K_ENTER },
	{ "tab", K_TAB },
	{ "space", K_SPACE },
	{ "bs", K_BS },
	{ "backspace", K_BS },     /* dup */
	{ "del", K_DEL },
	{ "delete", K_DEL },       /* dup */
	{ "ins", K_INS },
	{ "insert", K_INS },       /* dup */
	{ "home", K_HOME },
	{ "end", K_END },
	{ "prior", K_PRIOR },
	{ "next", K_NEXT },
	{ "pgup", K_PRIOR }, /* duplicate for pgup/pgdn fans */
	{ "pgdn", K_NEXT },  /* ditto */
	{ "esc", K_ESC },
	{ "escape", K_ESC },       /* dup */
	{ "pause", K_PAUSE },
	{ "caps", K_CAPS },
	{ "capslock", K_CAPS },    /* dup */
	{ "numlock", K_NUMLOCK },
	{ "scroll", K_SCROLL },

	{ "minus", K_MINUS },
	{ "_", K_MINUS },          /* dup */
	{ "equals", K_EQUALS },
	{ "plus", K_EQUALS },      /* dup */
	{ "+", K_EQUALS },         /* dup */
	{ "tilde", K_TILDE },
	{ "backquote", K_TILDE },  /* dup */
	{ "`", K_TILDE },          /* dup */
	{ "slash", K_SLASH },
	{ "question", K_SLASH },   /* dup */
	{ "?", K_SLASH },          /* dup */
	{ "bslash", K_BSLASH },
	{ "backslash", K_BSLASH }, /* dup */
	{ "pipe", K_BSLASH },      /* dup */
	{ "|", K_BSLASH },         /* dup */
	{ "semi", K_SEMI },
	{ "semicolon", K_SEMI },   /* dup */
	{ "quote", K_QUOTE },

	{ "f1", K_F1 },
	{ "f2", K_F2 },
	{ "f3", K_F3 },
	{ "f4", K_F4 },
	{ "f5", K_F5 },
	{ "f6", K_F6 },
	{ "f7", K_F7 },
	{ "f8", K_F8 },
	{ "f9", K_F9 },
	{ "f10", K_F10 },
	{ "f11", K_F11 },
	{ "f12", K_F12 },

	{ "num0", K_NUM0 },
	{ "num1", K_NUM1 },
	{ "num2", K_NUM2 },
	{ "num3", K_NUM3 },
	{ "num4", K_NUM4 },
	{ "num5", K_NUM5 },
	{ "num6", K_NUM6 },
	{ "num7", K_NUM7 },
	{ "num8", K_NUM8 },
	{ "num9", K_NUM9 },
	{ "numplus", K_NUMPLUS },
	{ "numminus", K_NUMMINUS },
	{ "nummul", K_NUMMUL },
	{ "numdiv", K_NUMDIV },
	{ "numdot", K_NUMDOT },
	{ "numenter", K_NUMENTER },

	/* Note that these are not presently used... */
	{ "mouse0", K_MOUSE0 },
	{ "mouse1", K_MOUSE1 },
	{ "mouse2", K_MOUSE2 },
	{ "mouse3", K_MOUSE3 },
	{ "mouse4", K_MOUSE4 },

	{ "joyleft", K_JOYLEFT },
	{ "joyright", K_JOYRIGHT },
	{ "joyup", K_JOYUP },
	{ "joydown", K_JOYDOWN },

	{ "joy0", K_JOY0 },
	{ "joy1", K_JOY1 },
	{ "joy2", K_JOY2 },
	{ "joy3", K_JOY3 },
	{ "joy4", K_JOY4 },
	{ "joy5", K_JOY5 },
	{ "joy6", K_JOY6 },
	{ "joy7", K_JOY7 },
	{ "joy8", K_JOY8 },
	{ "joy9", K_JOY9 },
	{ "joy10", K_JOY10 },
	{ "joy11", K_JOY11 },
	{ "joy12", K_JOY12 },
	{ "joy13", K_JOY13 },
	{ "joy14", K_JOY14 },
	{ "joy15", K_JOY15 },

	{ NULL, 0 }
};

int k_keycode(char *name)
{
	keytable_t *key;

	for (key = keytable; key->name; key++)
		if (!strcasecmp(key->name, name))
			return key->code;
	if (strlen(name) == 1)
		return tolower((unsigned char)name[0]);
	return 0;
}

char *k_keyname(int code)
{
	keytable_t *key;

	for (key = keytable; key->name; key++)
		if (key->code == code)
			return key->name;
	return NULL;
}

#endif
