#if 0
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <rg_system.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "libs/dr_mp3.h"
#include "music.h"
#include "gui.h"

static rg_scandir_t *music_files = NULL;
static size_t music_files_count = 0;
static QueueHandle_t playback_queue;

static void music_player(void *arg);
static void tab_refresh(tab_t *tab);

static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    rg_scandir_t *file = (rg_scandir_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        tab_refresh(tab);
    }
    else if (event == TAB_REFRESH)
    {
        tab_refresh(tab);
    }
    else if (event == TAB_ENTER || event == TAB_SCROLL)
    {
        gui_set_status(tab, NULL, "");
        gui_set_preview(tab, NULL);
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_IDLE)
    {
        //
    }
    else if (event == TAB_ACTION)
    {
        if (file)
        {
            char filepath[RG_PATH_MAX];
            snprintf(filepath, RG_PATH_MAX, "%s/%s", RG_BASE_PATH_MUSIC, file->name);
            xQueueSend(playback_queue, &filepath, portMAX_DELAY);
        }
    }
    else if (event == TAB_BACK)
    {
        // This is now reserved for subfolder navigation (go back)
    }
}

static void tab_refresh(tab_t *tab)
{
    RG_ASSERT(tab, "Bad param");

    memset(&tab->status, 0, sizeof(tab->status));

    if (music_files)
        free(music_files);

    music_files = rg_storage_scandir(RG_BASE_PATH_MUSIC, NULL, 0);
    music_files_count = 0;

    for (rg_scandir_t *file = music_files; file && file->is_valid; file++)
    {
        music_files_count++;
    }

    if (music_files_count == 0)
    {
        gui_resize_list(tab, 6);
        sprintf(tab->listbox.items[0].text, "Welcome to Retro-Go!");
        sprintf(tab->listbox.items[1].text, " ");
        sprintf(tab->listbox.items[2].text, "You have no mp3 files");
        sprintf(tab->listbox.items[3].text, " ");
        sprintf(tab->listbox.items[4].text, "You can hide this tab in the menu");
        tab->listbox.cursor = 3;
    }
    else
    {
        gui_resize_list(tab, music_files_count);
        for (int i = 0; i < music_files_count; i++)
        {
            listbox_item_t *listitem = &tab->listbox.items[i];
            rg_scandir_t *file = &music_files[i];
            snprintf(listitem->text, 128, "%s", file->name);
            listitem->arg = file;
            listitem->id = i;
        }
        gui_sort_list(tab);
    }
}

static void music_player(void *arg)
{
    rg_audio_frame_t *buffer = malloc(180 * sizeof(rg_audio_frame_t));
    drmp3 *mp3 = malloc(sizeof(drmp3));
    char filepath[RG_PATH_MAX];

    while (1)
    {
        xQueueReceive(playback_queue, &filepath, portMAX_DELAY);

        RG_LOGI("Playing file '%s'", filepath);

        if (drmp3_init_file(mp3, filepath, NULL))
        {
            rg_audio_set_sample_rate(mp3->sampleRate);

            while (!mp3->atEnd && !uxQueueMessagesWaiting(playback_queue))
            {
                drmp3_uint64 len = drmp3_read_pcm_frames_s16(mp3, 180, (drmp3_int16 *)buffer);
                rg_audio_submit(buffer, len);
            }

            drmp3_uninit(mp3);
        }
    }

    free(buffer);
    free(mp3);
}

void music_init(void)
{
    gui_add_tab("music", "Music Player", NULL, event_handler);

    playback_queue = xQueueCreate(1, RG_PATH_MAX);
    rg_task_create("music_player", &music_player, NULL, 32000, RG_TASK_PRIORITY, -1);
}

#else
void music_init(void)
{
}
#endif
