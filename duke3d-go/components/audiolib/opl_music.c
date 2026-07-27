#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "music.h"
#include "oplplayer.h"
#include "midifile.h"
#include "duke3d.h" // For kopen4load, etc.

static const void *current_song_handle = NULL;
static int music_initialized = 0;
static int music_playing = 0;
static int music_volume = 127;
static uint8_t *music_data_buffer = NULL;

char *MUSIC_ErrorString(int ErrorNumber) { return ""; }

int MUSIC_Init(int SoundCard, int Address) {
    if (music_initialized) return MUSIC_Ok;
    if (opl_synth_player.init(11025)) {
        music_initialized = 1;
        return MUSIC_Ok;
    }
    return MUSIC_Error;
}

int MUSIC_Shutdown(void) {
    if (music_initialized) {
        opl_synth_player.shutdown();
        music_initialized = 0;
    }
    if (music_data_buffer) free(music_data_buffer);
    music_data_buffer = NULL;
    return MUSIC_Ok;
}

void MUSIC_SetMaxFMMidiChannel(int channel) {}
void MUSIC_SetVolume(int volume) {
    music_volume = (volume * 127) / 255;
    opl_synth_player.setvolume(music_volume);
}
void MUSIC_SetMidiChannelVolume(int channel, int volume) {}
void MUSIC_ResetMidiChannelVolumes(void) {}
int MUSIC_GetVolume(void) { return (music_volume * 255) / 127; }
void MUSIC_SetLoopFlag(int loopflag) {}
int MUSIC_SongPlaying(void) { return music_playing; }

void MUSIC_Continue(void) {
    opl_synth_player.resume();
    music_playing = 1;
}

void MUSIC_Pause(void) {
    opl_synth_player.pause();
    music_playing = 0;
}

int MUSIC_StopSong(void) {
    opl_synth_player.stop();
    if (current_song_handle) {
        opl_synth_player.unregistersong(current_song_handle);
        current_song_handle = NULL;
    }
    music_playing = 0;
    return MUSIC_Ok;
}

int MUSIC_PlaySong(char *songData, int loopflag) {
    return MUSIC_Error; 
}

void MUSIC_SetContext(int context) {}
int MUSIC_GetContext(void) { return 0; }
void MUSIC_SetSongTick(uint32_t PositionInTicks) {}
void MUSIC_SetSongTime(uint32_t milliseconds) {}
void MUSIC_SetSongPosition(int measure, int beat, int tick) {}
void MUSIC_GetSongPosition(songposition *pos) {}
void MUSIC_GetSongLength(songposition *pos) {}
int MUSIC_FadeVolume(int tovolume, int milliseconds) { return MUSIC_Ok; }
int MUSIC_FadeActive(void) { return 0; }
void MUSIC_StopFade(void) {}
void MUSIC_RerouteMidiChannel(int channel, int cdecl function(int event, int c1, int c2)) {}
void MUSIC_RegisterTimbreBank(unsigned char *timbres) {}

// This is the one actually called by Duke3D Game code in our port
void PlayMusic(char *fileName) {
    if (!music_initialized) MUSIC_Init(0, 0);
    
    MUSIC_StopSong();

    int32_t fd = kopen4load(fileName, 0);
    if (fd == -1) {
        printf("PlayMusic: %s not found\n", fileName);
        return;
    }

    size_t size = kfilelength(fd);
    if (music_data_buffer) free(music_data_buffer);
    music_data_buffer = malloc(size);
    if (music_data_buffer) {
        kread(fd, music_data_buffer, size);
        current_song_handle = opl_synth_player.registersong(music_data_buffer, size);
        if (current_song_handle) {
            opl_synth_player.play(current_song_handle, 1);
            music_playing = 1;
        }
    }
    kclose(fd);
}
