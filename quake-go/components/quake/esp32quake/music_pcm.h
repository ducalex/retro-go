#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define MUSIC_SAMPLE_RATE 22050u
#define MUSIC_FRAME_BYTES 4u

/* ROM root comes from RG_BASE_PATH_ROMS; selected_pak is the launcher path. */
bool music_track_path(char *out, size_t capacity, const char *rom_root,
                      const char *selected_pak, unsigned track);

typedef struct {
    FILE *file;
    long data_offset;
    uint32_t data_bytes;
    uint32_t remaining;
    bool failed;
} music_wav_t;

bool music_wav_open(music_wav_t *wav, const char *path);
size_t music_wav_read(music_wav_t *wav, unsigned char *out, size_t bytes, bool loop);
void music_wav_close(music_wav_t *wav);

/* Caller serializes access. Storage contains interleaved little-endian stereo. */
typedef struct {
    unsigned char *data;
    size_t capacity, read_pos, write_pos, used;
    unsigned phase;
} music_ring_t;

void music_ring_init(music_ring_t *ring, unsigned char *storage, size_t bytes);
size_t music_ring_write(music_ring_t *ring, const unsigned char *data, size_t bytes);
bool music_ring_next(music_ring_t *ring, unsigned output_rate, int *left, int *right);
int16_t music_mix_sample(int effects, int music, unsigned volume256);
