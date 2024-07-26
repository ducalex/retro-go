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

#include <sys/dirent.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
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
#include <mus2mid.h>
#include <midifile.h>
#include <oplplayer.h>
#include <rg_system.h>
#ifdef ESP_PLATFORM
#include <esp_heap_caps.h>
#endif

#define AUDIO_SAMPLE_RATE 22050

#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / TICRATE + 1)
#define NUM_MIX_CHANNELS 8

static rg_surface_t *update;
static rg_app_t *app;

static const char *doom_argv[10];

// Expected variables by doom
int snd_card = 1, mus_card = 1;
int snd_samplerate = AUDIO_SAMPLE_RATE;
int current_palette = 0;

typedef struct {
    uint16_t unused1;
    uint16_t samplerate;
    uint16_t length;
    uint16_t unused2;
    byte samples[];
} doom_sfx_t;

typedef struct {
    const doom_sfx_t *sfx;
    size_t pos;
    float factor;
    int starttic;
} channel_t;

static channel_t channels[NUM_MIX_CHANNELS];
static const doom_sfx_t *sfx[NUMSFX];
static rg_audio_sample_t mixbuffer[AUDIO_BUFFER_LENGTH];
static const music_player_t *music_player = &opl_synth_player;
static bool musicPlaying = false;

// TO DO: Detect when menu is open so we can send better keys.

static const struct {int mask; int *key;} keymap[] = {
    {RG_KEY_UP, &key_up},
    {RG_KEY_DOWN, &key_down},
    {RG_KEY_LEFT, &key_left},
    {RG_KEY_RIGHT, &key_right},
    {RG_KEY_A, &key_fire},
    {RG_KEY_A, &key_enter},
    {RG_KEY_B, &key_speed},
    {RG_KEY_B, &key_strafe},
    {RG_KEY_B, &key_backspace},
    {RG_KEY_MENU, &key_escape},
    {RG_KEY_OPTION, &key_map},
    {RG_KEY_START, &key_use},
    {RG_KEY_SELECT, &key_weapontoggle},
};

static const char *SETTING_GAMMA = "Gamma";


static rg_gui_event_t gamma_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int gamma = usegamma;
    int max = 9;

    if (event == RG_DIALOG_PREV)
        gamma = gamma > 0 ? gamma - 1 : max;

    if (event == RG_DIALOG_NEXT)
        gamma = gamma < max ? gamma + 1 : 0;

    if (gamma != usegamma)
    {
        usegamma = gamma;
        rg_settings_set_number(NS_APP, SETTING_GAMMA, gamma);
        I_SetPalette(current_palette);
        return RG_DIALOG_REDRAW;
    }

    sprintf(option->value, "%d/%d", gamma, max);

    return RG_DIALOG_VOID;
}


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
    rg_display_submit(update, 0);
    rg_display_sync(true); // Wait for update->buffer to be released
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
    uint16_t *palette = V_BuildPalette(pal, 16);
    for (int i = 0; i < 256; i++)
        update->palette[i] = palette[i] << 8 | palette[i] >> 8;
    Z_Free(palette);
    current_palette = pal;
}

void I_InitGraphics(void)
{
    // set first three to standard values
    for (int i = 0; i < 3; i++)
    {
        screens[i].width = SCREENWIDTH;
        screens[i].height = SCREENHEIGHT;
        screens[i].byte_pitch = SCREENWIDTH;
    }

    // Main screen uses internal ram for speed
    screens[0].data = update->data;
    screens[0].not_on_heap = true;

    // statusbar
    screens[4].width = SCREENWIDTH;
    screens[4].height = (ST_SCALED_HEIGHT + 1);
    screens[4].byte_pitch = SCREENWIDTH;
}

int I_GetTimeMS(void)
{
    return rg_system_timer() / 1000;
}

int I_GetTime(void)
{
    return I_GetTimeMS() * TICRATE * realtic_clock_rate / 100000;
}

void I_uSleep(unsigned long usecs)
{
    rg_usleep(usecs);
}

void I_SafeExit(int rc)
{
    rg_system_exit();
}

const char *I_DoomExeDir(void)
{
    return RG_BASE_PATH_ROMS "/doom";
}

void I_UpdateSoundParams(int handle, int volume, int seperation, int pitch)
{
}

int I_StartSound(int sfxid, int channel, int vol, int sep, int pitch, int priority)
{
    int oldest = gametic;
    int slot = 0;

    // Unknown sound
    if (!sfx[sfxid])
        return -1;

    // These sound are played only once at a time. Stop any running ones.
    if (sfxid == sfx_sawup || sfxid == sfx_sawidl || sfxid == sfx_sawful
        || sfxid == sfx_sawhit || sfxid == sfx_stnmov || sfxid == sfx_pistol)
    {
        for (int i = 0; i < NUM_MIX_CHANNELS; i++)
        {
            if (channels[i].sfx == sfx[sfxid])
                channels[i].sfx = NULL;
        }
    }

    // Find available channel or steal the oldest
    for (int i = 0; i < NUM_MIX_CHANNELS; i++)
    {
        if (channels[i].sfx == NULL)
        {
            slot = i;
            break;
        }
        else if (channels[i].starttic < oldest)
        {
            slot = i;
            oldest = channels[i].starttic;
        }
    }

    channel_t *chan = &channels[slot];
    chan->sfx = sfx[sfxid];
    chan->factor = (float)chan->sfx->samplerate / snd_samplerate;
    chan->pos = 0;

    return slot;
}

void I_StopSound(int handle)
{
    if (handle < NUM_MIX_CHANNELS)
        channels[handle].sfx = NULL;
}

bool I_SoundIsPlaying(int handle)
{
    // return (handle < NUM_MIX_CHANNELS && channels[handle].sfx);
    return false;
}

bool I_AnySoundStillPlaying(void)
{
    for (int i = 0; i < NUM_MIX_CHANNELS; i++)
        if (channels[i].sfx)
            return true;
    return false;
}

static void soundTask(void *arg)
{
    while (1)
    {
        bool haveMusic = snd_MusicVolume > 0 && musicPlaying;
        bool haveSFX = snd_SfxVolume > 0 && I_AnySoundStillPlaying();

        if (haveMusic)
        {
            music_player->render(mixbuffer, AUDIO_BUFFER_LENGTH);
        }

        if (haveSFX)
        {
            int16_t *audioBuffer = (int16_t *)mixbuffer;
            int16_t *audioBufferEnd = audioBuffer + AUDIO_BUFFER_LENGTH * 2;
            while (audioBuffer < audioBufferEnd)
            {
                int totalSample = 0;
                int totalSources = 0;
                int sample;

                for (int i = 0; i < NUM_MIX_CHANNELS; i++)
                {
                    channel_t *chan = &channels[i];
                    if (!chan->sfx)
                        continue;

                    size_t pos = (size_t)(chan->pos++ * chan->factor);

                    if (pos >= chan->sfx->length)
                    {
                        chan->sfx = NULL;
                    }
                    else if ((sample = chan->sfx->samples[pos]))
                    {
                        totalSample += sample - 127;
                        totalSources++;
                    }
                }

                totalSample <<= 7;
                totalSample /= (16 - snd_SfxVolume);

                if (haveMusic)
                {
                    totalSample += *audioBuffer;
                    totalSources += (totalSources == 0);
                }

                if (totalSources > 0)
                    totalSample /= totalSources;

                if (totalSample > 32767)
                    totalSample = 32767;
                else if (totalSample < -32768)
                    totalSample = -32768;

                *audioBuffer++ = totalSample;
                *audioBuffer++ = totalSample;
            }
        }

        if (!haveMusic && !haveSFX)
        {
            memset(mixbuffer, 0, sizeof(mixbuffer));
        }

        rg_audio_submit(mixbuffer, AUDIO_BUFFER_LENGTH);
    }
}

void I_InitSound(void)
{
    for (int i = 1; i < NUMSFX; i++)
    {
        if (S_sfx[i].lumpnum != -1)
            sfx[i] = W_CacheLumpNum(S_sfx[i].lumpnum);
    }

    music_player->init(snd_samplerate);
    music_player->setvolume(snd_MusicVolume);

    rg_task_create("doom_sound", &soundTask, NULL, 2048, RG_TASK_PRIORITY_2, 1);
}

void I_ShutdownSound(void)
{
    music_player->shutdown();
}

void I_PlaySong(int handle, int looping)
{
    music_player->play((void *)handle, looping);
    musicPlaying = true;
}

void I_PauseSong(int handle)
{
    music_player->pause();
    musicPlaying = false;
}

void I_ResumeSong(int handle)
{
    music_player->resume();
    musicPlaying = true;
}

void I_StopSong(int handle)
{
    music_player->stop();
    musicPlaying = false;
}

void I_UnRegisterSong(int handle)
{
    music_player->unregistersong((void *)handle);
}

int I_RegisterSong(const void *data, size_t len)
{
    uint8_t *mid = NULL;
    size_t midlen;
    int handle = 0;

    if (mus2mid(data, len, &mid, &midlen, 64) == 0)
        handle = (int)music_player->registersong(mid, midlen);
    else
        handle = (int)music_player->registersong(data, len);

    free(mid);

    return handle;
}

void I_SetMusicVolume(int volume)
{
    music_player->setvolume(volume);
}

void I_StartTic(void)
{
    static int64_t last_time = 0;
    static int32_t prev_joystick = 0x0000;
    static int32_t rg_menu_delay = 0;
    uint32_t joystick = rg_input_read_gamepad();
    uint32_t changed = prev_joystick ^ joystick;
    event_t event = {0};

    // Long press on menu will open retro-go's menu if needed, instead of DOOM's.
    // This is still needed to quit (DOOM 2) and for the debug menu. We'll unify that mess soon...
    if (joystick & (RG_KEY_MENU|RG_KEY_OPTION))
    {
        if (joystick & RG_KEY_OPTION)
        {
            Z_FreeTags(PU_CACHE, PU_CACHE); // At this point the heap is usually full. Let's reclaim some!
            rg_gui_options_menu();
            changed = 0;
        }
        else if (rg_menu_delay++ == TICRATE / 2)
        {
            Z_FreeTags(PU_CACHE, PU_CACHE); // At this point the heap is usually full. Let's reclaim some!
            rg_gui_game_menu();
        }
        realtic_clock_rate = app->speed * 100;
        R_InitInterpolation();
    }
    else
    {
        rg_menu_delay = 0;
    }

    if (changed)
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

    rg_system_tick(rg_system_timer() - last_time);
    last_time = rg_system_timer();
    prev_joystick = joystick;
}

void I_Init(void)
{
    snd_channels = NUM_MIX_CHANNELS;
    snd_samplerate = AUDIO_SAMPLE_RATE;
    snd_MusicVolume = 15;
    snd_SfxVolume = 15;
    usegamma = rg_settings_get_number(NS_APP, SETTING_GAMMA, 0);
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    Z_FreeTags(PU_CACHE, PU_CACHE); // At this point the heap is usually full. Let's reclaim some!
	return rg_surface_save_image_file(update, filename, width, height);
}

static bool save_state_handler(const char *filename)
{
    rg_gui_alert("Not implemented", "Please use the in-game menu");
    return false;
}

static bool load_state_handler(const char *filename)
{
    rg_gui_alert("Not implemented", "Please use the in-game menu");
    return false;
}

static bool reset_handler(bool hard)
{
    return false;
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_SHUTDOWN)
    {
        // DOOM fully fills the internal heap and this causes some shutdown
        // steps to fail so we try to free everything!
        Z_FreeTags(0, PU_MAX);
        rg_audio_set_mute(true);
    }
    else if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(update, 0);
    }
}

bool is_iwad(const char *path)
{
    char header[16] = {0};
    void *data = &header;
    size_t data_len = 16;
    if (rg_extension_match(path, "zip"))
        rg_storage_unzip_file(path, NULL, &data, &data_len, RG_FILE_USER_BUFFER);
    else
        rg_storage_read_file(path, &data, &data_len, RG_FILE_USER_BUFFER);
    return header[0] == 'I' && header[1] == 'W';
}

void app_main()
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Gamma Boost", "-", RG_DIALOG_FLAG_NORMAL, &gamma_update_cb},
        RG_DIALOG_END
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);
    app->tickRate = TICRATE;

    const rg_display_t *display = rg_display_get_info();
    SCREENWIDTH = RG_MIN(display->screen.width, MAX_SCREENWIDTH);
    SCREENHEIGHT = RG_MIN(display->screen.height, MAX_SCREENHEIGHT);

    update = rg_surface_create(SCREENWIDTH, SCREENHEIGHT, RG_PIXEL_PAL565_BE, MEM_FAST);

    const char *iwad = NULL;
    const char *pwad = NULL;

    if (is_iwad(app->romPath))
        iwad = app->romPath;
    else
        pwad = app->romPath;

    if (!iwad)
    {
        iwad = rg_gui_file_picker("Select IWAD file", I_DoomExeDir(), is_iwad, false) ?: "";
        rg_gui_draw_hourglass(); // Redraw hourglass to indicate loading...
    }

    myargv = doom_argv;
    myargc = pwad ? 7 : 5;
    doom_argv[0] = "doom";
    doom_argv[1] = "-save";
    doom_argv[2] = RG_BASE_PATH_SAVES "/doom";
    doom_argv[3] = "-iwad";
    doom_argv[4] = iwad;
    doom_argv[5] = "-file";
    doom_argv[6] = pwad;
    doom_argv[myargc] = 0;

#ifdef ESP_PLATFORM
    // Some things might be nice to place in internal RAM, but I do not have time to find such
    // structures. So for now, prefer external RAM for most things except the framebuffer which
    // is allocated above.
    heap_caps_malloc_extmem_enable(0);
#endif

    Z_Init();
    D_DoomMain();
}
