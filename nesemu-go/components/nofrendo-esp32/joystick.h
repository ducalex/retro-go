// #pragma once
//
// #include <stdint.h>
//
// #define JOY_X ADC1_CHANNEL_6
// #define JOY_Y ADC1_CHANNEL_7
// #define JOY_SELECT GPIO_NUM_27
// #define JOY_START GPIO_NUM_39
// #define JOY_A GPIO_NUM_32
// #define JOY_B GPIO_NUM_33
// // #define BTN_MENU GPIO_NUM_0
// // #define BTN_VOLUME GPIO_NUM_13
// #define BTN_MENU GPIO_NUM_13
// #define BTN_VOLUME GPIO_NUM_0
//
//
// typedef struct
// {
//     uint8_t Up;
//     uint8_t Right;
//     uint8_t Down;
//     uint8_t Left;
//     uint8_t Select;
//     uint8_t Start;
//     uint8_t A;
//     uint8_t B;
//     uint8_t Menu;
//     uint8_t Volume;
// } JoystickState;
//
// void JoystickInit();
// JoystickState JoystickRead();
