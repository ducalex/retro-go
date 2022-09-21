#include <rg_system.h>

#include "game.h"


void dukeTask(void *pvParameters)
{
    char *argv[]={"duke3d", "/nm", NULL};
    main(2, argv);
}


void app_main(void)
{
    rg_app_t *app = rg_system_init(11025, NULL, NULL);
    rg_display_set_source_format(320, 200, 0, 0, 320, RG_PIXEL_PAL565_BE);
	rg_task_create("dukeTask", &dukeTask, NULL, 16000, 5, 0);
}
