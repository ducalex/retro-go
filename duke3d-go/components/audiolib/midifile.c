#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midifile.h"

struct midi_file_s {
    uint16_t format;
    uint16_t num_tracks;
    uint16_t division;
    byte **tracks;
    uint32_t *track_lens;
};

struct midi_track_iter_s {
    const midi_file_t *file;
    uint32_t track_num;
    uint32_t pos;
    byte running_status;
};

static uint32_t read_varlen(const byte *data, uint32_t *pos, uint32_t len) {
    uint32_t val = 0;
    byte b;
    do {
        if (*pos >= len) break;
        b = data[(*pos)++];
        val = (val << 7) | (b & 0x7f);
    } while (b & 0x80);
    return val;
}

static uint32_t read32(const byte *data, uint32_t *pos) {
    uint32_t val = (data[*pos] << 24) | (data[*pos+1] << 16) | (data[*pos+2] << 8) | data[*pos+3];
    *pos += 4;
    return val;
}

static uint16_t read16(const byte *data, uint32_t *pos) {
    uint16_t val = (data[*pos] << 8) | (data[*pos+1]);
    *pos += 2;
    return val;
}

midi_file_t *MIDI_LoadFile(midimem_t *mf) {
    uint32_t pos = 0;
    if (mf->len < 14) return NULL;
    if (memcmp(mf->data, "MThd", 4) != 0) return NULL;
    pos += 4;
    uint32_t header_len = read32(mf->data, &pos);
    midi_file_t *file = calloc(1, sizeof(midi_file_t));
    file->format = read16(mf->data, &pos);
    file->num_tracks = read16(mf->data, &pos);
    file->division = read16(mf->data, &pos);
    pos = 4 + 4 + header_len;
    file->tracks = calloc(file->num_tracks, sizeof(byte*));
    file->track_lens = calloc(file->num_tracks, sizeof(uint32_t));
    for (int i = 0; i < file->num_tracks; i++) {
        if (pos + 8 > mf->len) break;
        if (memcmp(mf->data + pos, "MTrk", 4) != 0) break;
        pos += 4;
        file->track_lens[i] = read32(mf->data, &pos);
        file->tracks[i] = (byte*)mf->data + pos;
        pos += file->track_lens[i];
    }
    return file;
}

void MIDI_FreeFile(midi_file_t *file) {
    if (file) {
        free(file->tracks);
        free(file->track_lens);
        free(file);
    }
}

unsigned int MIDI_GetFileTimeDivision(const midi_file_t *file) { return file->division; }
unsigned int MIDI_NumTracks(const midi_file_t *file) { return file->num_tracks; }

midi_track_iter_t *MIDI_IterateTrack(const midi_file_t *file, unsigned int track_num) {
    if (track_num >= file->num_tracks) return NULL;
    midi_track_iter_t *iter = calloc(1, sizeof(midi_track_iter_t));
    iter->file = file;
    iter->track_num = track_num;
    return iter;
}

void MIDI_FreeIterator(midi_track_iter_t *iter) { free(iter); }

unsigned int MIDI_GetDeltaTime(midi_track_iter_t *iter) {
    uint32_t len = iter->file->track_lens[iter->track_num];
    const byte *data = iter->file->tracks[iter->track_num];
    uint32_t pos = iter->pos;
    if (pos >= len) return 0;
    return read_varlen(data, &pos, len);
}

int MIDI_GetNextEvent(midi_track_iter_t *iter, midi_event_t **event_out) {
    static midi_event_t ev;
    uint32_t len = iter->file->track_lens[iter->track_num];
    const byte *data = iter->file->tracks[iter->track_num];
    if (iter->pos >= len) return 0;
    ev.delta_time = read_varlen(data, &iter->pos, len);
    if (iter->pos >= len) return 0;
    byte status = data[iter->pos++];
    if (!(status & 0x80)) {
        status = iter->running_status;
        iter->pos--;
    } else {
        iter->running_status = status;
    }
    ev.event_type = status & 0xf0;
    if (status == 0xff) {
        ev.event_type = MIDI_EVENT_META;
        ev.data.meta.type = data[iter->pos++];
        ev.data.meta.length = read_varlen(data, &iter->pos, len);
        ev.data.meta.data = (byte*)data + iter->pos;
        iter->pos += ev.data.meta.length;
    } else if (status >= 0xf0) {
        uint32_t slen = read_varlen(data, &iter->pos, len);
        iter->pos += slen;
        return MIDI_GetNextEvent(iter, event_out);
    } else {
        ev.data.channel.channel = status & 0x0f;
        ev.data.channel.param1 = data[iter->pos++];
        if (ev.event_type != MIDI_EVENT_PROGRAM_CHANGE && ev.event_type != MIDI_EVENT_CHAN_AFTERTOUCH) {
            ev.data.channel.param2 = data[iter->pos++];
        }
    }
    *event_out = &ev;
    return 1;
}

void MIDI_RestartIterator(midi_track_iter_t *iter) {
    iter->pos = 0;
    iter->running_status = 0;
}
