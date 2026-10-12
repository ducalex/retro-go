#include "quake_main.h"
#include <string.h>
#include "esp_log.h"
#include "fatfs_proxy.h"

static const char TAG[] = "quake";

void esp32_quake_main(int argc, char **argv, const char *basedir,
                      const char *pak0Path, const char *pak1Path,
                      uint32_t pakSize, const void *pakMmap);
void Sys_Quit(void);
void Sys_Shutdown(void);
void VID_Redraw(void);
bool VID_SaveScreenshot(const char *filename, int width, int height);

void quake_request_exit(void)
{
    Sys_Quit();
}

void quake_shutdown(void)
{
    Sys_Shutdown();
    fatfs_proxy_deinit();
}

void quake_redraw(void)
{
    VID_Redraw();
}

bool quake_screenshot(const char *filename, int width, int height)
{
    return VID_SaveScreenshot(filename, width, height);
}

void quake_main(const char *basedir, const char *game, const char *pak0Path,
                const char *pak1Path,
                uint32_t pakSize, const void *pakMmap)
{
    const char *argv[6] = { "quake" };
    int argc = 1;

    if (game && !strcmp(game, "rogue")) {
        argv[argc++] = "-rogue";
    } else if (game && !strcmp(game, "hipnotic")) {
        argv[argc++] = "-hipnotic";
    } else if (game && *game) {
        argv[argc++] = "-game";
        argv[argc++] = game;
    }

#if defined(CONFIG_IDF_TARGET_ESP32)
    argv[argc++] = "+map";
    argv[argc++] = "start";
#endif

    fatfs_proxy_init(xTaskGetCurrentTaskHandle());

    ESP_LOGI(TAG, "starting quake...");
    esp32_quake_main(argc, (char**)argv, basedir, pak0Path, pak1Path,
                     pakSize, pakMmap);
    ESP_LOGI(TAG, "exiting quake");

    fatfs_proxy_deinit();
}
