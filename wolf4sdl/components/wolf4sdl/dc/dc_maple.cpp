//Wolf4SDL\DC
//dc_maple.cpp
//2009 - Cyle Terry

#if defined(_arch_dreamcast)

#include <SDL.h>
#include "dc/maple.h"
#include "dc/maple/controller.h"
#include "dc/maple/vmu.h"

int DC_MousePresent() {
    return maple_first_mouse() != 0;
}

void DC_WaitButtonPress(int button)
{
    int first_controller = 0;
    cont_cond_t controller_condition;

    first_controller = maple_first_controller();
    cont_get_cond(first_controller, &controller_condition);

    while((controller_condition.buttons & button)) {
        SDL_Delay(5);
        cont_get_cond(first_controller, &controller_condition);
    }
}


void DC_WaitButtonRelease(int button)
{
    int first_controller = 0;
    cont_cond_t controller_condition;

    first_controller = maple_first_controller();
    cont_get_cond(first_controller, &controller_condition);

    while(!(controller_condition.buttons & button)) {
        SDL_Delay(5);
        cont_get_cond(first_controller, &controller_condition);
    }
}


int DC_ButtonPress(int button)
{
    int first_controller = 0;
    cont_cond_t controller_condition;

    first_controller = maple_first_controller();
    cont_get_cond(first_controller, &controller_condition);

    if(!(controller_condition.buttons & button))
        return 1;
    else
        return 0;
}

#endif
