#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "music.h"
#include "interrup.h"
#include "oplplayer.h"

static const void *current_song_handle = NULL;
static int music_initialized = 0;
static int music_playing = 0;
static int music_volume = 127;
static unsigned char *music_data_buffer = NULL;

char *MUSIC_ErrorString(int ErrorNumber) { return ""; }

int MUSIC_Init(int SoundCard, int Address) {
    unsigned long flags = DisableInterrupts();
    if (music_initialized) {
        RestoreInterrupts(flags);
        return MUSIC_Ok;
    }

    int res = opl_synth_player.init(11025);
    if (res) {
        music_initialized = 1;
        RestoreInterrupts(flags);
        return MUSIC_Ok;
    }

    RestoreInterrupts(flags);
    return MUSIC_Error;
}

int MUSIC_Shutdown(void) {
    unsigned long flags = DisableInterrupts();

    if (music_initialized) {
        opl_synth_player.shutdown();
        music_initialized = 0;
    }
    if (music_data_buffer) free(music_data_buffer);
    music_data_buffer = NULL;

    current_song_handle = NULL;
    music_playing = 0;
    RestoreInterrupts(flags);
    return MUSIC_Ok;
}

void MUSIC_SetMaxFMMidiChannel(int channel) {}
void MUSIC_SetVolume(int volume) {
    unsigned long flags = DisableInterrupts();
    music_volume = (volume * 127) / 255;
    opl_synth_player.setvolume(music_volume);
    RestoreInterrupts(flags);
}
void MUSIC_SetMidiChannelVolume(int channel, int volume) {}
void MUSIC_ResetMidiChannelVolumes(void) {}
int MUSIC_GetVolume(void) { return (music_volume * 255) / 127; }
void MUSIC_SetLoopFlag(int loopflag) {}
int MUSIC_SongPlaying(void) { return music_playing; }

void MUSIC_Continue(void) {
    unsigned long flags = DisableInterrupts();
    opl_synth_player.resume();
    music_playing = 1;
    RestoreInterrupts(flags);
}

void MUSIC_Pause(void) {
    unsigned long flags = DisableInterrupts();
    opl_synth_player.pause();
    music_playing = 0;
    RestoreInterrupts(flags);
}

int MUSIC_StopSong(void) {
    unsigned long flags = DisableInterrupts();
    opl_synth_player.stop();
    if (current_song_handle) {
        opl_synth_player.unregistersong(current_song_handle);
        current_song_handle = NULL;
    }
    music_playing = 0;
    RestoreInterrupts(flags);
    return MUSIC_Ok;
}

int MUSIC_PlaySong(unsigned char *songData, int loopflag) {
    return MUSIC_Error; // Handled by MUSIC_PlaySongROTT
}

int MUSIC_PlaySongROTT(unsigned char *song, int size, int loopflag) {
    unsigned long flags = DisableInterrupts();

    if (!music_initialized && MUSIC_Init(0, 0) != MUSIC_Ok) {
        RestoreInterrupts(flags);
        return MUSIC_Error;
    }

    MUSIC_StopSong();

    if (!song || size <= 0) {
        RestoreInterrupts(flags);
        return MUSIC_Error;
    }

    if (music_data_buffer) free(music_data_buffer);
    music_data_buffer = malloc(size);

    if (music_data_buffer) {
        memcpy(music_data_buffer, song, size);
        current_song_handle = opl_synth_player.registersong(music_data_buffer, size);
        if (current_song_handle) {
            opl_synth_player.play(current_song_handle, loopflag == MUSIC_LoopSong);
            music_playing = 1;
            RestoreInterrupts(flags);
            return MUSIC_Ok;
        }
    }

    RestoreInterrupts(flags);
    return MUSIC_Error;
}

void MUSIC_SetContext(int context) {}
int MUSIC_GetContext(void) { return 0; }
void MUSIC_SetSongTick(unsigned long PositionInTicks) {}
void MUSIC_SetSongTime(unsigned long milliseconds) {}
void MUSIC_SetSongPosition(int measure, int beat, int tick) {}
void MUSIC_GetSongPosition(songposition *pos) {}
void MUSIC_GetSongLength(songposition *pos) {}
int MUSIC_FadeVolume(int tovolume, int milliseconds) { return MUSIC_Ok; }
int MUSIC_FadeActive(void) { return 0; }
void MUSIC_StopFade(void) {}
void MUSIC_RerouteMidiChannel(int channel, int (*function)(int event, int c1, int c2)) {}
void MUSIC_RegisterTimbreBank(unsigned char *timbres) {}
