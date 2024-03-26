#include <rg_system.h>

typedef struct
{
	char name[16];
	struct {
		uint16_t snes9x_mask;
		uint16_t local_mask;
		uint16_t mod_mask;
	} keys[16];
} keymap_t;

static const keymap_t KEYMAPS[] = {
	{"Type A", {
		{SNES_A_MASK, RG_KEY_A, 0},
		{SNES_B_MASK, RG_KEY_B, 0},
		{SNES_X_MASK, RG_KEY_START, 0},
		{SNES_Y_MASK, RG_KEY_SELECT, 0},
		{SNES_TL_MASK, RG_KEY_B, RG_KEY_MENU},
		{SNES_TR_MASK, RG_KEY_A, RG_KEY_MENU},
		{SNES_START_MASK, RG_KEY_START, RG_KEY_MENU},
		{SNES_SELECT_MASK, RG_KEY_SELECT, RG_KEY_MENU},
		{SNES_UP_MASK, RG_KEY_UP, 0},
		{SNES_DOWN_MASK, RG_KEY_DOWN, 0},
		{SNES_LEFT_MASK, RG_KEY_LEFT, 0},
		{SNES_RIGHT_MASK, RG_KEY_RIGHT, 0},
	}},
	{"Type B", {
		{SNES_A_MASK, RG_KEY_START, 0},
		{SNES_B_MASK, RG_KEY_A, 0},
		{SNES_X_MASK, RG_KEY_SELECT, 0},
		{SNES_Y_MASK, RG_KEY_B, 0},
		{SNES_TL_MASK, RG_KEY_B, RG_KEY_MENU},
		{SNES_TR_MASK, RG_KEY_A, RG_KEY_MENU},
		{SNES_START_MASK, RG_KEY_START, RG_KEY_MENU},
		{SNES_SELECT_MASK, RG_KEY_SELECT, RG_KEY_MENU},
		{SNES_UP_MASK, RG_KEY_UP, 0},
		{SNES_DOWN_MASK, RG_KEY_DOWN, 0},
		{SNES_LEFT_MASK, RG_KEY_LEFT, 0},
		{SNES_RIGHT_MASK, RG_KEY_RIGHT, 0},
	}},
	{"Type C", {
		{SNES_A_MASK, RG_KEY_A, 0},
		{SNES_B_MASK, RG_KEY_B, 0},
		{SNES_X_MASK, 0, 0},
		{SNES_Y_MASK, 0, 0},
		{SNES_TL_MASK, 0, 0},
		{SNES_TR_MASK, 0, 0},
		{SNES_START_MASK, RG_KEY_START, 0},
		{SNES_SELECT_MASK, RG_KEY_SELECT, 0},
		{SNES_UP_MASK, RG_KEY_UP, 0},
		{SNES_DOWN_MASK, RG_KEY_DOWN, 0},
		{SNES_LEFT_MASK, RG_KEY_LEFT, 0},
		{SNES_RIGHT_MASK, RG_KEY_RIGHT, 0},
	}},
};

static const size_t KEYMAPS_COUNT = (sizeof(KEYMAPS) / sizeof(keymap_t));

static const char *SNES_BUTTONS[] = {
	"None", "None", "None", "None", "R", "L", "X", "A", "Right", "Left", "Down", "Up", "Start", "Select", "Y", "B"
};
