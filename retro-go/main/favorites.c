#include <odroid_system.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui.h"

int garbage;

static void event_handler(gui_event_t event, tab_t *tab)
{
    if (event == TAB_INIT)
    {
        tab->listbox.length = 5;
        tab->listbox.cursor = 1;
        tab->listbox.items = calloc(tab->listbox.length, sizeof(listbox_item_t));
        sprintf(tab->listbox.items[1].text, "Favorites!");
        usleep(500 * 1000);
        sprintf(tab->status, "Loaded!");
    }
    else
    {
        odroid_overlay_alert("TEST");
        gui_redraw();
    }
}

void favorites_init()
{
    gui_add_tab("favorites", (void*)&garbage, NULL, NULL, event_handler);
}
