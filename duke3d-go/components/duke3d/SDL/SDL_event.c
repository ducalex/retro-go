#include "SDL_event.h"
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
int keyMode = 1;

//Mappings from buttons to keys
// static const GPIOKeyMap keymap[2][6]={{
// // Game
// 	{CONFIG_HW_BUTTON_PIN_NUM_BUTTON1, SDL_SCANCODE_LCTRL, SDLK_LCTRL},
// 	{CONFIG_HW_BUTTON_PIN_NUM_SELECT, SDL_SCANCODE_SPACE, SDLK_SPACE},
// 	{CONFIG_HW_BUTTON_PIN_NUM_VOL, SDL_SCANCODE_CAPSLOCK, SDLK_CAPSLOCK},
// 	{CONFIG_HW_BUTTON_PIN_NUM_MENU, SDL_SCANCODE_ESCAPE, SDLK_ESCAPE},
// 	{CONFIG_HW_BUTTON_PIN_NUM_START, SDL_SCANCODE_A, SDLK_a},
// 	{CONFIG_HW_BUTTON_PIN_NUM_BUTTON2, SDL_SCANCODE_LALT, SDLK_LALT},
// },
// // Menu
// {
// 	{CONFIG_HW_BUTTON_PIN_NUM_BUTTON1, SDL_SCANCODE_SPACE, SDLK_SPACE},
// 	{CONFIG_HW_BUTTON_PIN_NUM_BUTTON2, SDL_SCANCODE_ESCAPE, SDLK_ESCAPE},
// 	{CONFIG_HW_BUTTON_PIN_NUM_VOL, SDL_SCANCODE_ESCAPE, SDLK_ESCAPE},
// 	{CONFIG_HW_BUTTON_PIN_NUM_MENU, SDL_SCANCODE_ESCAPE, SDLK_ESCAPE},
// 	{CONFIG_HW_BUTTON_PIN_NUM_START, SDL_SCANCODE_A, SDLK_a},
// 	{CONFIG_HW_BUTTON_PIN_NUM_SELECT, SDL_SCANCODE_LALT, SDLK_LALT},
// }};

typedef struct
{
    uint8_t up;
    uint8_t right;
    uint8_t down;
    uint8_t left;
    uint8_t buttons[6];
} JoystickState;

JoystickState lastState = {0,0,0,0,{0,0,0,0,0,0}};

bool initInput = false;

int readOdroidXY(SDL_Event * event)
{

    //         event->key.keysym.scancode = keymap[keyMode][i].scancode;
    //     event->key.keysym.sym = keymap[keyMode][i].keycode;
    //     event->key.type = state ? SDL_KEYDOWN : SDL_KEYUP;
    //     event->key.state = state ? SDL_PRESSED : SDL_RELEASED;
    // JoystickState state = {0};
    // event->key.keysym.mod = 0;
    // if(checkPin(state.up, &lastState.up, SDL_SCANCODE_UP, SDLK_UP, event))
    //     return 1;
    // if(checkPin(state.down, &lastState.down, SDL_SCANCODE_DOWN, SDLK_DOWN, event))
    //     return 1;
    // if(checkPin(state.left, &lastState.left, SDL_SCANCODE_LEFT, SDLK_LEFT, event))
    //     return 1;
    // if(checkPin(state.right, &lastState.right, SDL_SCANCODE_RIGHT, SDLK_RIGHT, event))
    //     return 1;

    // for(int i = 0; i < 6; i++)
    //     if(checkPinStruct(i, &lastState.buttons[i], event))
    //         return 1;

    return 0;
}

int SDL_PollEvent(SDL_Event * event)
{
    if(!initInput)
        inputInit();

    return readOdroidXY(event);
}

void inputInit()
{
	printf("keyboard: GPIO task created.\n");
    initInput = true;
}