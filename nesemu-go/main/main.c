#include "esp_heap_caps.h"
#include "esp_err.h"
#include "string.h"

#include "odroid_system.h"
#include "nofrendo.h"

static char* romPath;
static char* romData;

extern bool forceConsoleReset;


char *osd_getromdata()
{
	printf("Initialized. ROM@%p\n", romData);
	return (char*)romData;
}


void app_main(void)
{
	printf("nesemu (%s-%s).\n", COMPILEDATE, GITREV);

    odroid_system_init(2, 32000, &romPath, &forceConsoleReset);

    // Load ROM
    romData = heap_caps_malloc(1024 * 1024, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if (!romData) abort();

    size_t fileSize = 0;

    if (strcasecmp(romPath + (strlen(romPath) - 4), ".zip") == 0)
    {
        printf("app_main ROM: Reading compressed file: %s\n", romPath);
        fileSize = odroid_sdcard_unzip_file_to_memory(romPath, romData, 1024 * 1024);
    }
    else
    {
        printf("app_main ROM: Reading file: %s\n", romPath);
        fileSize = odroid_sdcard_copy_file_to_memory(romPath, romData, 1024 * 1024);
    }

    printf("app_main ROM: fileSize=%d\n", fileSize);
    if (fileSize == 0)
    {
        odroid_display_show_error(ODROID_SD_ERR_BADFILE);
        odroid_system_halt();
    }

	printf("NoFrendo start!\n");
	char* args[1] = { odroid_sdcard_get_filename(romPath) };
	nofrendo_main(1, args);

	printf("NoFrendo died.\n");
    abort();
}
