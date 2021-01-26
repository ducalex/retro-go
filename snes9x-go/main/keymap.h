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
		{"Joypad1 A", GAMEPAD_KEY_A, 0, 0},
		{"Joypad1 B", GAMEPAD_KEY_B, 0, 0},
		{"Joypad1 X", GAMEPAD_KEY_START, 0, 0},
		{"Joypad1 Y", GAMEPAD_KEY_SELECT, 0, 0},
		{"Joypad1 L", GAMEPAD_KEY_B, 1, 0},
		{"Joypad1 R", GAMEPAD_KEY_A, 1, 0},
		{"Joypad1 Start", GAMEPAD_KEY_START, 1, 0},
		{"Joypad1 Select", GAMEPAD_KEY_SELECT, 1, 0},
		{"Joypad1 Up", GAMEPAD_KEY_UP, 0, 0},
		{"Joypad1 Down", GAMEPAD_KEY_DOWN, 0, 0},
		{"Joypad1 Left", GAMEPAD_KEY_LEFT, 0, 0},
		{"Joypad1 Right", GAMEPAD_KEY_RIGHT, 0, 0},
	}},
	{"Type B", 12, {
		{"Joypad1 A", GAMEPAD_KEY_START, 0, 0},
		{"Joypad1 B", GAMEPAD_KEY_A, 0, 0},
		{"Joypad1 X", GAMEPAD_KEY_SELECT, 0, 0},
		{"Joypad1 Y", GAMEPAD_KEY_B, 0, 0},
		{"Joypad1 L", GAMEPAD_KEY_B, 1, 0},
		{"Joypad1 R", GAMEPAD_KEY_A, 1, 0},
		{"Joypad1 Start", GAMEPAD_KEY_START, 1, 0},
		{"Joypad1 Select", GAMEPAD_KEY_SELECT, 1, 0},
		{"Joypad1 Up", GAMEPAD_KEY_UP, 0, 0},
		{"Joypad1 Down", GAMEPAD_KEY_DOWN, 0, 0},
		{"Joypad1 Left", GAMEPAD_KEY_LEFT, 0, 0},
		{"Joypad1 Right", GAMEPAD_KEY_RIGHT, 0, 0},
	}},
	{"Type C", 8, {
		{"Joypad1 A", GAMEPAD_KEY_A, 0, 0},
		{"Joypad1 B", GAMEPAD_KEY_B, 0, 0},
		{"Joypad1 Start", GAMEPAD_KEY_START, 0, 0},
		{"Joypad1 Select", GAMEPAD_KEY_SELECT, 0, 0},
		{"Joypad1 Up", GAMEPAD_KEY_UP, 0, 0},
		{"Joypad1 Down", GAMEPAD_KEY_DOWN, 0, 0},
		{"Joypad1 Left", GAMEPAD_KEY_LEFT, 0, 0},
		{"Joypad1 Right", GAMEPAD_KEY_RIGHT, 0, 0},
	}},
};

static const size_t KEYMAPS_COUNT = (sizeof(KEYMAPS) / sizeof(keymap_t));

static const char *KEYNAMES[] = {
	"UP", "RIGHT", "DOWN", "LEFT", "SELECT", "START", "A", "B", "MENU", "VOL", "ANY"
};
