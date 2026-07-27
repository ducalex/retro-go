#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t byte;

typedef struct midi_file_s midi_file_t;
typedef struct midi_track_iter_s midi_track_iter_t;

typedef struct
{
    const byte *data;
    size_t len;
    size_t pos;
} midimem_t;

typedef enum
{
    MIDI_EVENT_NOTE_OFF        = 0x80,
    MIDI_EVENT_NOTE_ON         = 0x90,
    MIDI_EVENT_AFTERTOUCH      = 0xa0,
    MIDI_EVENT_CONTROLLER      = 0xb0,
    MIDI_EVENT_PROGRAM_CHANGE  = 0xc0,
    MIDI_EVENT_CHAN_AFTERTOUCH = 0xd0,
    MIDI_EVENT_PITCH_BEND      = 0xe0,
    MIDI_EVENT_SYSEX           = 0xf0,
    MIDI_EVENT_SYSEX_SPLIT     = 0xf7,
    MIDI_EVENT_META            = 0xff,
} midi_event_type_t;

typedef enum
{
    MIDI_META_SEQUENCE_NUMBER       = 0x0,
    MIDI_META_TEXT                  = 0x1,
    MIDI_META_COPYRIGHT             = 0x2,
    MIDI_META_TRACK_NAME            = 0x3,
    MIDI_META_INSTR_NAME            = 0x4,
    MIDI_META_LYRICS                = 0x5,
    MIDI_META_MARKER                = 0x6,
    MIDI_META_CUE_POINT             = 0x7,
    MIDI_META_CHANNEL_PREFIX        = 0x20,
    MIDI_META_END_OF_TRACK          = 0x2f,
    MIDI_META_SET_TEMPO             = 0x51,
    MIDI_META_SMPTE_OFFSET          = 0x54,
    MIDI_META_TIME_SIGNATURE        = 0x58,
    MIDI_META_KEY_SIGNATURE         = 0x59,
    MIDI_META_SEQUENCER_SPECIFIC    = 0x7f,
} midi_meta_event_type_t;

typedef struct
{
    unsigned int type;
    unsigned int length;
    byte *data;
} midi_meta_event_data_t;

typedef struct
{
    unsigned int length;
    byte *data;
} midi_sysex_event_data_t;

typedef struct
{
    unsigned int channel;
    unsigned int param1;
    unsigned int param2;
} midi_channel_event_data_t;

typedef struct
{
    unsigned int delta_time;
    midi_event_type_t event_type;
    union
    {
        midi_channel_event_data_t channel;
        midi_meta_event_data_t meta;
        midi_sysex_event_data_t sysex;
    } data;
} midi_event_t;

midi_file_t *MIDI_LoadFile(midimem_t *mf);
void MIDI_FreeFile(midi_file_t *file);
unsigned int MIDI_GetFileTimeDivision(const midi_file_t *file);
unsigned int MIDI_NumTracks(const midi_file_t *file);
midi_track_iter_t *MIDI_IterateTrack(const midi_file_t *file, unsigned int track_num);
void MIDI_FreeIterator(midi_track_iter_t *iter);
unsigned int MIDI_GetDeltaTime(midi_track_iter_t *iter);
int MIDI_GetNextEvent(midi_track_iter_t *iter, midi_event_t **event);
void MIDI_RestartIterator(midi_track_iter_t *iter);

#endif
