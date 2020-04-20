/*****************************************/
/* ESP32 (Odroid GO) Keyboard Engine     */
/* Released under the GPL license        */
/*                                       */
/* Original Author:                      */
/*		ducalex                          */
/*****************************************/

#include <odroid_input.h>
#include "osd_keyboard.h"
#include "hard_pce.h"


int osd_init_input(void)
{

}

int osd_keyboard(void)
{
    odroid_gamepad_state joystick;
    odroid_input_gamepad_read(&joystick);

	if (joystick.values[ODROID_INPUT_MENU]) {
		odroid_overlay_game_menu();
	}
	else if (joystick.values[ODROID_INPUT_VOLUME]) {
		odroid_overlay_game_settings_menu(NULL);
	}

    uint8_t rc = 0;
    if (joystick.values[ODROID_INPUT_LEFT]) rc |= JOY_LEFT;
    if (joystick.values[ODROID_INPUT_RIGHT]) rc |= JOY_RIGHT;
    if (joystick.values[ODROID_INPUT_UP]) rc |= JOY_UP;
    if (joystick.values[ODROID_INPUT_DOWN]) rc |= JOY_DOWN;

    if (joystick.values[ODROID_INPUT_A]) rc |= JOY_A;
    if (joystick.values[ODROID_INPUT_B]) rc |= JOY_B;
    if (joystick.values[ODROID_INPUT_START]) rc |= JOY_RUN;
    if (joystick.values[ODROID_INPUT_SELECT]) rc |= JOY_SELECT;

    io.JOY[0] = rc;

    return 0;
}
