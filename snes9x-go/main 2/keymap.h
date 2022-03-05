#include <rg_system.h>

typedef struct
{
	char name[16];
	size_t size;
	struct {
		char action[16];
		long key_id;
		bool mod1;
		bool mod2;
	} keys[16];
} keymap_t;

static const keymap_t KEYMAPS[] = {
	{"Type A", 12, {
		{"Joypad1 A", RG_KEY_A, 0, 0},
		{"Joypad1 B", RG_KEY_B, 0, 0},
		{"Joypad1 X", RG_KEY_START, 0, 0},
		{"Joypad1 Y", RG_KEY_SELECT, 0, 0},
		{"Joypad1 L", RG_KEY_B, 1, 0},
		{"Joypad1 R", RG_KEY_A, 1, 0},
		{"Joypad1 Start", RG_KEY_START, 1, 0},
		{"Joypad1 Select", RG_KEY_SELECT, 1, 0},
		{"Joypad1 Up", RG_KEY_UP, 0, 0},
		{"Joypad1 Down", RG_KEY_DOWN, 0, 0},
		{"Joypad1 Left", RG_KEY_LEFT, 0, 0},
		{"Joypad1 Right", RG_KEY_RIGHT, 0, 0},
	}},
	{"Type B", 12, {
		{"Joypad1 A", RG_KEY_START, 0, 0},
		{"Joypad1 B", RG_KEY_A, 0, 0},
		{"Joypad1 X", RG_KEY_SELECT, 0, 0},
		{"Joypad1 Y", RG_KEY_B, 0, 0},
		{"Joypad1 L", RG_KEY_B, 1, 0},
		{"Joypad1 R", RG_KEY_A, 1, 0},
		{"Joypad1 Start", RG_KEY_START, 1, 0},
		{"Joypad1 Select", RG_KEY_SELECT, 1, 0},
		{"Joypad1 Up", RG_KEY_UP, 0, 0},
		{"Joypad1 Down", RG_KEY_DOWN, 0, 0},
		{"Joypad1 Left", RG_KEY_LEFT, 0, 0},
		{"Joypad1 Right", RG_KEY_RIGHT, 0, 0},
	}},
	{"Type C", 8, {
		{"Joypad1 A", RG_KEY_A, 0, 0},
		{"Joypad1 B", RG_KEY_B, 0, 0},
		{"Joypad1 Start", RG_KEY_START, 0, 0},
		{"Joypad1 Select", RG_KEY_SELECT, 0, 0},
		{"Joypad1 Up", RG_KEY_UP, 0, 0},
		{"Joypad1 Down", RG_KEY_DOWN, 0, 0},
		{"Joypad1 Left", RG_KEY_LEFT, 0, 0},
		{"Joypad1 Right", RG_KEY_RIGHT, 0, 0},
	}},
};

static const size_t KEYMAPS_COUNT = (sizeof(KEYMAPS) / sizeof(keymap_t));

static const char *KEYNAMES[] = {
	"UP", "RIGHT", "DOWN", "LEFT", "SELECT", "START", "A", "B", "MENU", "VOL", "ANY"
};
