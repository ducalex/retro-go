#include "odroid_ui_choosefile.h"
#include "freertos/FreeRTOS.h"
#include "odroid_system.h"
#include "odroid_sdcard.h"
#include "odroid_settings.h"
#include "odroid_display.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

int comparator(const void *p, const void *q) 
{ 
    // Get the values at given addresses 
    char *l = (char*)(*(uint32_t*)p); 
    char *r = (char*)(*(uint32_t*)q);
    return strcmp(l,r); 
} 


char *odroid_ui_choose_file(const char *path, const char *ext) {
    char* selected_file = odroid_settings_RomFilePath_get();
    if (selected_file) {
        if (strlen(selected_file) <strlen(ext)+1 ||
            strcasecmp(ext, &selected_file[strlen(selected_file)-strlen(ext)])!=0 ) {
           printf("odroid_ui_choose_file: Ignoring last file: '%s'\n", selected_file);
           free(selected_file);
           selected_file = NULL;
        } else {
            odroid_gamepad_state joystick;
            odroid_input_gamepad_read(&joystick);
            bool choose = joystick.values[ODROID_INPUT_SELECT];

            if (odroid_settings_ForceInternalGameSelect_get())
            {
                odroid_settings_ForceInternalGameSelect_set(0);
                choose=true;
            }
            
            if (!choose) {
               return selected_file;
            }
        }
    }
    
    DIR* dir = opendir(path);
    if (!dir)
    {
        printf("odroid_ui_choose_file: failed '%s'. ERR: %d\n", path, errno);
        return NULL;    
    }
    int count = 0;
    int count_chars = 0;
    struct dirent* in_file;
    uint32_t *entries_refs = NULL;
    char *entries_buffer = NULL;
    while (!entries_refs)
    {
    rewinddir(dir);
    if (count>0) {
       entries_refs = (uint32_t*)heap_caps_malloc(count*4, MALLOC_CAP_SPIRAM);
       entries_buffer = (char*)heap_caps_malloc(count_chars, MALLOC_CAP_SPIRAM);
       count = 0;
       count_chars = 0;
    }
    while ((in_file = readdir(dir))) 
    {
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))    
            continue;
        if (strlen(in_file->d_name) < strlen(ext)+1)
            continue;
        if (in_file->d_name[0] == '.' && in_file->d_name[1] == '_')    
            continue;
        if (strcasecmp(ext, &in_file->d_name[strlen(in_file->d_name)-strlen(ext)]) != 0)
            continue;
        
        if (entries_refs) {
           entries_refs[count] = ((uint32_t)entries_buffer) + count_chars;
           strcpy(&entries_buffer[count_chars],in_file->d_name);
           //printf("DIR entry: %p; %d; Offset: %d; File: %s\n", (char*)entries_refs[count], count, count_chars, in_file->d_name);
        }
        count++;
        count_chars+=strlen(in_file->d_name)+1;
    }
      if (count == 0) {
        break;
      }
    }
    closedir(dir);
    
    if (!entries_refs) {
       printf("Could not find any files\n");
       return NULL;
    }
    qsort((void*)entries_refs, count, sizeof(entries_refs[0]), comparator); 
    odroid_ui_init();
    odroid_ui_clean_draw_buffer();
    odroid_gamepad_state lastJoysticState;
    odroid_input_gamepad_read(&lastJoysticState);
    odroid_display_lock();
    
    int selected_last = -1;
    int selected = 0;
    int last_key = -1;
    bool run = true;
    /*
    for (int i = 0;i < count; i++) {
        char *file = (char*)entries_refs[i];
        printf("%s\n", file);
    }
    */
    
    if (selected_file) {
       for (int i = 0;i < count; i++) {
          char *file = (char*)entries_refs[i];
          if (strlen(selected_file) < strlen(file)) continue;
          char *file2 = &selected_file[strlen(selected_file)-strlen(file)];
          if (strcmp(file, file2) == 0) {
              selected = i;
              printf("Last selected: %d: '%s'\n", selected, file);
              break;
          }
       }
       free(selected_file);
       selected_file = NULL;
    }
    
    uint8_t repeat = 0;
    
    while (run)
    {
        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);
        
        if (last_key >= 0) {
            if (!joystick.values[last_key]) {
                last_key = -1;
                repeat = 0;
            } else if (repeat++>6) {
             repeat = 6;
             last_key = -1;
         }
        } else {
            if (joystick.values[ODROID_INPUT_B]) {
                last_key = ODROID_INPUT_B;
                    //entry_rc = ODROID_UI_FUNC_TOGGLE_RC_MENU_CLOSE;
            } else if (joystick.values[ODROID_INPUT_VOLUME]) {
                last_key = ODROID_INPUT_VOLUME;
                   // entry_rc = ODROID_UI_FUNC_TOGGLE_RC_MENU_CLOSE;
            } else if (joystick.values[ODROID_INPUT_UP]) {
                    last_key = ODROID_INPUT_UP;
                    selected--;
                    if (selected<0) selected = count - 1;
            } else if (joystick.values[ODROID_INPUT_DOWN]) {
                    last_key = ODROID_INPUT_DOWN;
                    selected++;
                    if (selected>=count) selected = 0;
            } else if (joystick.values[ODROID_INPUT_A]) {
                    last_key = ODROID_INPUT_A;
                    //entry_rc = window.entries[selected]->func_toggle(window.entries[selected], &joystick);
                    run = false;
            } else if (joystick.values[ODROID_INPUT_LEFT]) {
                last_key = ODROID_INPUT_LEFT;
                char st = ((char*)entries_refs[selected])[0];
                while (selected>0)
                {
                   selected--;
                   if (st != ((char*)entries_refs[selected])[0]) break;
                }
                //selected-=10;
                //if (selected<0) selected = 0;
            } else if (joystick.values[ODROID_INPUT_RIGHT]) {
                last_key = ODROID_INPUT_RIGHT;
                char st = ((char*)entries_refs[selected])[0];
                while (selected<count-1)
                {
                   selected++;
                   if (st != ((char*)entries_refs[selected])[0]) break;
                }
                //selected+=10;
                //if (selected>=count) selected = count - 1;
            }
        }
        
        if (selected_last != selected) {
            /*
            int x = 0;
            int y = 0;
            char *text = (char*)entries_refs[selected];
            odroid_ui_draw_chars(x, y, 32, text, color_default, color_bg_default);
            printf("Selected: %d; %s\n", selected, text);
            */
            int x = 0;
            for (int i = 0;i < 30; i++) {
                int y = i * 8;
                int entry = selected + i - 15;
                char *text;
                if (entry>=0 && entry < count)
                {
                    text = (char*)entries_refs[entry];
                } else
                {
                    text = " ";
                }
                odroid_ui_draw_chars(x, y, 39, text, entry==selected?color_selected:color_default, color_bg_default);
            }
        }
        usleep(20*1000UL);
        selected_last = selected;
    }
    odroid_ui_wait_for_key(last_key, false);
    odroid_display_unlock();
    ili9341_clear(C_BLACK);
    //char *file = &entries_buffer[entries_refs[selected]];
    char *file = (char*)entries_refs[selected];
    char *rc = (char*)malloc(strlen(path) + 1+ strlen(file)+1);
    strcpy(rc, path);
    strcat(rc, "/");
    strcat(rc, file);
    
    if (entries_refs) heap_caps_free(entries_refs);
    if (entries_buffer) heap_caps_free(entries_buffer);
    odroid_settings_RomFilePath_set(rc);
    return rc;
}