/*
 * This file is part of doom-ng-odroid-go.
 * Copyright (c) 2019 ducalex.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rg_system.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <doomtype.h>
#include <doomstat.h>
#include <doomdef.h>
#include <d_main.h>
#include <g_game.h>
#include <i_system.h>
#include <i_video.h>
#include <i_sound.h>
#include <i_main.h>
#include <m_argv.h>
#include <m_fixed.h>
#include <m_misc.h>
#include <r_draw.h>
#include <r_fps.h>
#include <s_sound.h>
#include <st_stuff.h>
#include <mmus2mid.h>
#include <midifile.h>
#include <oplplayer.h>

#include "prboom.h"

#define SAMPLERATE 11025
#define SAMPLECOUNT (SAMPLERATE / TICRATE + 1)
#define NUM_MIX_CHANNELS 8

static rg_video_update_t update;
static rg_app_t *app;

// Expected variables by doom
int ms_to_next_tick = 0;
int snd_card = 1, mus_card = 1;
int snd_samplerate = SAMPLERATE;
int realtic_clock_rate = 100;
int (*I_GetTime)(void) = I_GetTime_RealTime;

static struct {
    uint8_t *data;   // Sample
    uint8_t *endptr; // End of data
    int start;       // Time/gametic that the channel started playing
    int sfxid;       // SFX id of the playing sound effect.
} channels[NUM_MIX_CHANNELS];
static short mixbuffer[SAMPLECOUNT * 2];
static int sfx_lengths[NUMSFX];
static const music_player_t *music_player = &opl_synth_player;
static bool musicPlaying = false;

static int key_yes = 'y';
static int key_no = 'n';

static const struct {int mask; int *key;} keymap[] = {
    {RG_KEY_UP, &key_up},
    {RG_KEY_DOWN, &key_down},
    {RG_KEY_LEFT, &key_left},
    {RG_KEY_RIGHT, &key_right},
    {RG_KEY_A, &key_yes},
    {RG_KEY_A, &key_fire},
    {RG_KEY_A, &key_menu_enter},
    {RG_KEY_B, &key_no},
    {RG_KEY_B, &key_speed},
    {RG_KEY_B, &key_strafe},
    {RG_KEY_B, &key_menu_backspace},
    {RG_KEY_MENU, &key_escape},
    // {RG_KEY_OPTION, &key_map},
    {RG_KEY_OPTION, &key_escape},
    {RG_KEY_START, &key_use},
    {RG_KEY_SELECT, &key_weapontoggle},
};


void I_StartFrame(void)
{
    //
}

void I_UpdateNoBlit(void)
{
    //
}

void I_FinishUpdate(void)
{
    update.buffer = screens[0].data;
    update.synchronous = true;
    rg_display_queue_update(&update, NULL);
}

bool I_StartDisplay(void)
{
    return true;
}

void I_EndDisplay(void)
{
    //
}

void I_SetPalette(int pal)
{
    int pplump = W_GetNumForName("PLAYPAL");
    const byte *palette = W_CacheLumpNum(pplump) + (pal * 3 * 256);
    for (int i = 0; i < 255; i++)
    {
        unsigned v = ((palette[0] >> 3) << 11) + ((palette[1] >> 2) << 5) + (palette[2] >> 3);
        update.palette[i] = (v << 8) | (v >> 8);
        palette += 3;
    }
    W_UnlockLumpNum(pplump);
}

void I_SetRes(void)
{
    // set first three to standard values
    for (int i = 0; i < 3; i++)
    {
        screens[i].width = SCREENWIDTH;
        screens[i].height = SCREENHEIGHT;
        screens[i].byte_pitch = SCREENPITCH;
        screens[i].short_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE16);
        screens[i].int_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE32);
    }

    // statusbar
    screens[4].width = SCREENWIDTH;
    screens[4].height = (ST_SCALED_HEIGHT + 1);
    screens[4].byte_pitch = SCREENPITCH;
    screens[4].short_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE16);
    screens[4].int_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE32);

    rg_display_set_source_format(SCREENWIDTH, SCREENHEIGHT, 0, 0, SCREENPITCH, RG_PIXEL_PAL565_BE);

    RG_LOGI("Using resolution %dx%d\n", SCREENWIDTH, SCREENHEIGHT);
}

void I_InitGraphics(void)
{
    V_InitMode(VID_MODE8);
    V_DestroyUnusedTrueColorPalettes();
    V_FreeScreens();
    I_SetRes();
    V_AllocScreens();
    R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);
}

void I_CalculateRes(unsigned int width, unsigned int height)
{
    //
}

int I_GetTime_RealTime(void)
{
    return ((esp_timer_get_time() * TICRATE) / 1000000);
}

int I_GetTimeFrac(void)
{
    unsigned long now;
    fixed_t frac;

    now = esp_timer_get_time() / 1000;

    if (tic_vars.step == 0)
        return FRACUNIT;
    else
    {
        frac = (fixed_t)((now - tic_vars.start) * FRACUNIT / tic_vars.step);
        if (frac < 0)
            frac = 0;
        if (frac > FRACUNIT)
            frac = FRACUNIT;
        return frac;
    }
}

void I_GetTime_SaveMS(void)
{
    if (!movement_smooth)
        return;

    tic_vars.start = esp_timer_get_time() / 1000;
    tic_vars.next = (unsigned int)((tic_vars.start * tic_vars.msec + 1.0f) / tic_vars.msec);
    tic_vars.step = tic_vars.next - tic_vars.start;
}

unsigned long I_GetRandomTimeSeed(void)
{
    return 4; //per https://xkcd.com/221/
}

void I_uSleep(unsigned long usecs)
{
    usleep(usecs);
}

const char *I_DoomExeDir(void)
{
    return RG_BASE_PATH_ROMS "/doom";
}

char *I_FindFile(const char *fname, const char *ext)
{
    char filepath[PATH_MAX + 1];

    // Absolute path
    if (fname[0] == '/' && access(fname, R_OK) != -1)
    {
        RG_LOGI("Found %s... ", fname);
        return strdup(fname);
    }

    // Relative path
    snprintf(filepath, PATH_MAX, "%s/%s", I_DoomExeDir(), fname);
    if (access(filepath, R_OK) != -1)
    {
        RG_LOGI("Found: %s\n", filepath);
        return strdup(filepath);
    }

    RG_LOGI("Not found: %s.\n");
    return NULL;
}

void I_UpdateSoundParams(int handle, int volume, int seperation, int pitch)
{
}

void I_SetChannels()
{
}

int I_StartSound(int sfxid, int channel, int vol, int sep, int pitch, int priority)
{
    int oldest = gametic;
    int slot = 0;

    // These sound are played only once at a time. Stop any running ones.
    if (sfxid == sfx_sawup || sfxid == sfx_sawidl || sfxid == sfx_sawful
        || sfxid == sfx_sawhit || sfxid == sfx_stnmov || sfxid == sfx_pistol)
    {
        for (int i = 0; i < NUM_MIX_CHANNELS; i++)
        {
            if (channels[i].sfxid == sfxid)
                channels[i].data = NULL;
        }
    }

    // Find available channel or steal the oldest
    for (int i = 0; i < NUM_MIX_CHANNELS; i++)
    {
        if (channels[i].data == NULL)
        {
            slot = i;
            break;
        }
        else if (channels[i].start < oldest)
        {
            slot = i;
            oldest = channels[i].start;
        }
    }

    // Use empty channel if available, otherwise reuse the oldest one
    channels[slot].data = S_sfx[sfxid].data;
    channels[slot].endptr = channels[slot].data + sfx_lengths[sfxid];
    channels[slot].sfxid = sfxid;

    return 1;
}

void I_StopSound(int handle)
{
}

bool I_SoundIsPlaying(int handle)
{
    return gametic < handle;
}

bool I_AnySoundStillPlaying(void)
{
    return false;
}

static void soundTask(void *arg)
{
    while (1)
    {
        short *audioBuffer = mixbuffer;
        short stream[2];

        for (int i = 0; i < SAMPLECOUNT; ++i)
        {
            int totalSample = 0;
            int totalSources = 0;
            int sample;

            if (snd_SfxVolume > 0)
            {
                for (int chan = 0; chan < NUM_MIX_CHANNELS; chan++)
                {
                    if (!channels[chan].data)
                        continue;

                    sample = *channels[chan].data++;

                    if (sample != 0)
                    {
                        totalSample += sample;
                        totalSources++;
                    }

                    if (channels[chan].data >= channels[chan].endptr)
                    {
                        channels[chan].data = NULL;
                    }
                }

                totalSample <<= 6;
                totalSample /= (16 - snd_SfxVolume);
            }

            if (musicPlaying && snd_MusicVolume > 0)
            {
                music_player->render(&stream, 1); // It returns 2 (stereo) 16bits values per sample
                sample = (stream[0] + stream[1]) >> 1;
                if (sample > 0)
                {
                    totalSample += sample / (16 - snd_MusicVolume);
                    if (totalSources == 0)
                        totalSources = 1;
                }
            }

            if (totalSources == 0)
            {
                *(audioBuffer++) = 0;
                *(audioBuffer++) = 0;
            }
            else
            {
                *(audioBuffer++) = (short)(totalSample / totalSources);
                *(audioBuffer++) = (short)(totalSample / totalSources);
            }
        }
        rg_audio_submit(mixbuffer, SAMPLECOUNT);
    }
}

// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}

void I_InitSound(void)
{
    RG_LOGI("called\n");

    char sfx_name[32];

    for (int i = 2; i < NUMSFX; i++)
    {
        // Map unknown sounds to pistol
        sprintf(sfx_name, "ds%s", S_sfx[i].name);

        if (!S_sfx[i].link && W_CheckNumForName(sfx_name) == -1)
        {
            S_sfx[i].link = &S_sfx[1];
        }
    }

    for (int i = 1; i < NUMSFX; i++)
    {
        // Previously loaded already (alias)?
        if (S_sfx[i].link)
        {
            S_sfx[i].data = S_sfx[i].link->data;
            sfx_lengths[i] = sfx_lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
            continue;
        }

        sprintf(sfx_name, "ds%s", S_sfx[i].name);
        int sfxlump = W_GetNumForName(sfx_name);
        const void *sfx = W_CacheLumpNum(sfxlump) + 8;
        size_t size = W_LumpLength(sfxlump) - 8;

        // Pads the sound effect out to the mixing buffer size.
        size_t paddedsize = ((size + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;
        void *paddedsfx = Z_Malloc(paddedsize, PU_SOUND, 0);

        memcpy(paddedsfx, sfx, size);
        memset(paddedsfx + size, 0x80, paddedsize - size);

        W_UnlockLumpNum(sfxlump);
        Z_FreeTags(PU_CACHE, PU_CACHE);

        sfx_lengths[i] = paddedsize;
        S_sfx[i].data = paddedsfx;
    }
    RG_LOGI("pre-cached all sound data!\n");

    default_numChannels = NUM_MIX_CHANNELS;
    snd_samplerate = SAMPLERATE;
    snd_MusicVolume = 15;
    snd_SfxVolume = 15;

    music_player->init(snd_samplerate);
    music_player->setvolume(snd_MusicVolume);

    xTaskCreatePinnedToCore(&soundTask, "soundTask", 1024, NULL, 5, NULL, 1);
}

void I_ShutdownSound(void)
{
    RG_LOGI("called\n");
    music_player->shutdown();
}

void I_PlaySong(int handle, int looping)
{
    RG_LOGI("%d %d\n", handle, looping);
    music_player->play((void *)handle, looping);
    musicPlaying = true;
}

void I_PauseSong(int handle)
{
    RG_LOGI("handle: %d.\n", handle);
    music_player->pause();
    musicPlaying = false;
}

void I_ResumeSong(int handle)
{
    RG_LOGI("handle: %d.\n", handle);
    music_player->resume();
    musicPlaying = true;
}

void I_StopSong(int handle)
{
    RG_LOGI("handle: %d.\n", handle);
    music_player->stop();
    musicPlaying = false;
}

void I_UnRegisterSong(int handle)
{
    RG_LOGI("handle: %d\n", handle);
    music_player->unregistersong((void *)handle);
}

int I_RegisterSong(const void *data, size_t len)
{
    const uint16_t *header = data;
    MIDI mididata;
    uint8_t *mid;
    int handle = 0;
    int midlen;

    RG_LOGI("Length: %d, Start: %d, Channels: %d, SecChannels: %d, Instruments: %d.\n",
            header[2], header[3], header[4], header[5], header[6]);

    if (mmus2mid(data, &mididata, 64, 1) != 0)
    {
        return 0;
    }

    if (MIDIToMidi(&mididata, &mid, &midlen) == 0)
    {
        free_mididata(&mididata);
        handle = (intptr_t)music_player->registersong(mid, midlen);
        free(mid);
    }

    return handle;
}

void I_SetMusicVolume(int volume)
{
    //music_player->setvolume(volume);
}

void I_StartTic(void)
{
    static uint64_t last_time = 0;
    static uint32_t prev_joystick = 0x0000;
    uint32_t joystick = rg_input_read_gamepad();
    uint32_t changed = prev_joystick ^ joystick;
    event_t event = {0};

    // if (joystick & RG_KEY_MENU)
    // {
    //     rg_gui_game_menu();
    // }
    // else
    if (joystick & RG_KEY_OPTION)
    {
        rg_gui_game_settings_menu();
    }
    else if (changed)
    {
        for (int i = 0; i < RG_COUNT(keymap); i++)
        {
            if (changed & keymap[i].mask)
            {
                event.type = (joystick & keymap[i].mask) ? ev_keydown : ev_keyup;
                event.data1 = *keymap[i].key;
                D_PostEvent(&event);
            }
        }
    }

    rg_system_tick(get_elapsed_time_since(last_time));
    last_time = get_elapsed_time();
    prev_joystick = joystick;
}

void I_Init(void)
{
    I_InitSound();
    R_InitInterpolation();
}

static bool screenshot_handler(const char *filename, int width, int height)
{
	return rg_display_save_frame(filename, &update, width, height);
}

static bool save_state_handler(const char *filename)
{
    return false;
}

static bool load_state_handler(const char *filename)
{
    return false;
}

static bool reset_handler(bool hard)
{
    return false;
}

static void settings_handler(void)
{
    return;
}

static void doomEngineTask(void *pvParameters)
{
    const char *romtype = "-iwad";
    FILE *fp;

    vTaskDelay(1);

    const rg_emu_proc_t handlers = {
        // .loadState = &load_state_handler,
        // .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        // .settings = &settings_handler,
    };

    app = rg_system_init(SAMPLERATE, &handlers);
    app->refreshRate = TICRATE;

    // TO DO: We should probably make prboom detect what we're passing instead
    // and choose which default IWAD to use, if any.
    if ((fp = fopen(app->romPath, "rb")))
    {
        if (fgetc(fp) == 'P')
            romtype = "-file";
        fclose(fp);
    }

    myargv = (const char *[]){"doom", "-save", RG_BASE_PATH_SAVES "/doom", romtype, app->romPath};
    myargc = 5;

    Z_Init();
    D_DoomMain();
}

void app_main()
{
    // Switch immediately to increase our stack size
    // xTaskCreate(&doomEngineTask, "doomEngine", 14000, NULL, uxTaskPriorityGet(NULL), NULL);
    xTaskCreatePinnedToCore(&doomEngineTask, "doomEngine", 12000, NULL, uxTaskPriorityGet(NULL), NULL, 0);
    vTaskDelete(NULL);
}
