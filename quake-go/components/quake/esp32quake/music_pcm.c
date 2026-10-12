#include "music_pcm.h"
#include <limits.h>
#include <string.h>

bool music_track_path(char *out, size_t capacity, const char *rom_root,
                      const char *selected_pak, unsigned track)
{
    if (!capacity || !out) return false;
    out[0] = 0;
    if (!rom_root || !*rom_root || track < 2 || track > 255) return false;
    size_t root_length = strlen(rom_root);
    while (root_length && rom_root[root_length - 1] == '/') root_length--;
    if (root_length > INT_MAX) return false;
    int length;
    if (selected_pak && *selected_pak)
    {
        // The launcher-selected directory is authoritative, including mods,
        // but music must stay under the platform's ROM root, never config.
        if (strncmp(selected_pak, rom_root, root_length) ||
            selected_pak[root_length] != '/') return false;
        const char *slash = strrchr(selected_pak, '/');
        if (!slash || !slash[1] || (size_t)(slash - selected_pak) > INT_MAX) return false;
        length = snprintf(out, capacity, "%.*s/music/track%02u.wav",
                          (int)(slash - selected_pak), selected_pak, track);
    }
    else
        length = snprintf(out, capacity, "%.*s/quake/id1/music/track%02u.wav",
                          (int)root_length, rom_root, track);
    if (length < 0 || (size_t)length >= capacity) {
        out[0] = 0;
        return false;
    }
    return true;
}

static uint16_t le16(const unsigned char *p)
{
    return (uint16_t)(p[0] | (uint16_t)p[1] << 8);
}

static uint32_t le32(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

void music_wav_close(music_wav_t *wav)
{
    if (wav->file) fclose(wav->file);
    memset(wav, 0, sizeof(*wav));
}

bool music_wav_open(music_wav_t *wav, const char *path)
{
    unsigned char header[16];
    bool format_found = false, data_found = false;
    music_wav_close(wav);
    wav->file = fopen(path, "rb");
    if (!wav->file) return false;

    if (fseek(wav->file, 0, SEEK_END)) goto invalid;
    long file_bytes = ftell(wav->file);
    if (file_bytes < 12 || fseek(wav->file, 0, SEEK_SET)) goto invalid;
    if (fread(header, 1, 12, wav->file) != 12 ||
        memcmp(header, "RIFF", 4) || memcmp(header + 8, "WAVE", 4)) goto invalid;

    uint64_t riff_end = (uint64_t)le32(header + 4) + 8;
    if (riff_end < 12 || riff_end > (uint64_t)file_bytes || riff_end > LONG_MAX) goto invalid;
    uint64_t offset = 12;
    while (offset + 8 <= riff_end)
    {
        if (fseek(wav->file, (long)offset, SEEK_SET) ||
            fread(header, 1, 8, wav->file) != 8) goto invalid;
        uint32_t bytes = le32(header + 4);
        uint64_t next = offset + 8 + bytes + (bytes & 1u);
        if (next > riff_end) goto invalid;
        if (!memcmp(header, "fmt ", 4))
        {
            if (format_found || bytes < 16 || fread(header, 1, 16, wav->file) != 16) goto invalid;
            if (le16(header) != 1 || le16(header + 2) != 2 ||
                le32(header + 4) != MUSIC_SAMPLE_RATE ||
                le32(header + 8) != MUSIC_SAMPLE_RATE * MUSIC_FRAME_BYTES ||
                le16(header + 12) != MUSIC_FRAME_BYTES || le16(header + 14) != 16) goto invalid;
            format_found = true;
        }
        else if (!memcmp(header, "data", 4))
        {
            if (data_found || !bytes || bytes % MUSIC_FRAME_BYTES) goto invalid;
            wav->data_offset = (long)(offset + 8);
            wav->data_bytes = bytes;
            data_found = true;
        }
        offset = next;
    }
    if (!format_found || !data_found || offset != riff_end ||
        fseek(wav->file, wav->data_offset, SEEK_SET)) goto invalid;
    wav->remaining = wav->data_bytes;
    return true;

invalid:
    music_wav_close(wav);
    return false;
}

size_t music_wav_read(music_wav_t *wav, unsigned char *out, size_t bytes, bool loop)
{
    size_t done = 0;
    bytes -= bytes % MUSIC_FRAME_BYTES;
    if (!wav->file || wav->failed) return 0;
    while (done < bytes)
    {
        if (!wav->remaining)
        {
            if (!loop) break;
            if (fseek(wav->file, wav->data_offset, SEEK_SET)) {
                wav->failed = true;
                break;
            }
            wav->remaining = wav->data_bytes;
        }
        size_t count = bytes - done;
        if (count > wav->remaining) count = wav->remaining;
        size_t got = fread(out + done, 1, count, wav->file);
        done += got - got % MUSIC_FRAME_BYTES;
        wav->remaining -= (uint32_t)got;
        if (got != count) {
            wav->failed = true;
            break;
        }
    }
    return done;
}

void music_ring_init(music_ring_t *ring, unsigned char *storage, size_t bytes)
{
    memset(ring, 0, sizeof(*ring));
    ring->data = storage;
    ring->capacity = bytes - bytes % MUSIC_FRAME_BYTES;
}

size_t music_ring_write(music_ring_t *ring, const unsigned char *data, size_t bytes)
{
    size_t available = ring->capacity - ring->used;
    if (bytes > available) bytes = available;
    bytes -= bytes % MUSIC_FRAME_BYTES;
    if (!bytes || !ring->data) return 0;
    size_t first = ring->capacity - ring->write_pos;
    if (first > bytes) first = bytes;
    memcpy(ring->data + ring->write_pos, data, first);
    memcpy(ring->data, data + first, bytes - first);
    ring->write_pos = (ring->write_pos + bytes) % ring->capacity;
    ring->used += bytes;
    return bytes;
}

bool music_ring_next(music_ring_t *ring, unsigned output_rate, int *left, int *right)
{
    *left = *right = 0;
    if (!ring->data || ring->used < MUSIC_FRAME_BYTES || output_rate < MUSIC_SAMPLE_RATE) return false;
    const unsigned char *frame = ring->data + ring->read_pos;
    *left = (int16_t)le16(frame);
    *right = (int16_t)le16(frame + 2);
    ring->phase += MUSIC_SAMPLE_RATE;
    if (ring->phase >= output_rate)
    {
        ring->phase -= output_rate;
        ring->read_pos = (ring->read_pos + MUSIC_FRAME_BYTES) % ring->capacity;
        ring->used -= MUSIC_FRAME_BYTES;
    }
    return true;
}

int16_t music_mix_sample(int effects, int music, unsigned volume256)
{
    if (volume256 > 256) volume256 = 256;
    int mixed = effects + music * (int)volume256 / 256;
    if (mixed > 32767) mixed = 32767;
    if (mixed < -32768) mixed = -32768;
    return (int16_t)mixed;
}
