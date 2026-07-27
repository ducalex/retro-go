// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//  System interface for music.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rg_system.h>

#include "opl.h"
#include "midifile.h"
#include "oplplayer.h"

#define lprintf(x, ...) RG_LOGI(__VA_ARGS__)

#define MIDI_CHANNELS_PER_TRACK 16
#define MIDI_META_END_OF_TRACK 0x2f
#define MIDI_CONTROLLER_MAIN_VOLUME 0x07

typedef uint8_t boolean;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define PACKEDATTR __attribute__((packed))

#define GENMIDI_FLAG_FIXED      0x0001         /* fixed pitch */
#define GENMIDI_FLAG_2VOICE     0x0004         /* double voice (OPL3) */

typedef struct
{
    byte tremolo;
    byte attack;
    byte sustain;
    byte waveform;
    byte scale;
    byte level;
} PACKEDATTR genmidi_op_t;

typedef struct
{
    genmidi_op_t modulator;
    byte feedback;
    genmidi_op_t carrier;
    byte unused;
    short base_note_offset;
} PACKEDATTR genmidi_voice_t;

typedef struct
{
    unsigned short flags;
    byte fine_tuning;
    byte fixed_note;

    genmidi_voice_t voices[2];
} PACKEDATTR genmidi_instr_t;

typedef struct
{
    unsigned char SAVEK[ 2 ];
    unsigned char Level[ 2 ];
    unsigned char Env1[ 2 ];
    unsigned char Env2[ 2 ];
    unsigned char Wave[ 2 ];
    unsigned char Feedback;
    signed   char Transpose;
} TIMBRE;

extern TIMBRE ADLIB_TimbreBank[ 256 ];

static void LoadOperatorData(int operator, const genmidi_op_t *data, boolean max_level);

typedef struct
{
    const genmidi_instr_t *instrument;
    int volume;
    int bend;
} opl_channel_data_t;

typedef struct
{
    opl_channel_data_t channels[MIDI_CHANNELS_PER_TRACK];
    midi_track_iter_t *iter;
    uint32_t delay;
    boolean active;
} opl_track_data_t;

typedef struct opl_voice_s opl_voice_t;

struct opl_voice_s
{
    int index;
    int op1, op2;
    const genmidi_instr_t *current_instr;
    unsigned int current_instr_voice;
    opl_channel_data_t *channel;
    unsigned int key;
    unsigned int note;
    unsigned int freq;
    unsigned int note_volume;
    unsigned int reg_volume;
    opl_voice_t *next;
};

static const int voice_operators[2][OPL_NUM_VOICES] = {
    { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12 },
    { 0x03, 0x04, 0x05, 0x0b, 0x0c, 0x0d, 0x13, 0x14, 0x15 }
};

static const unsigned short frequency_curve[] = {
    0x133, 0x133, 0x134, 0x134, 0x135, 0x136, 0x136, 0x137, 0x137, 0x138, 0x138, 0x139, 0x139, 0x13a, 0x13b, 0x13b,
    0x13c, 0x13c, 0x13d, 0x13d, 0x13e, 0x13f, 0x13f, 0x140, 0x140, 0x141, 0x142, 0x142, 0x143, 0x143, 0x144, 0x144,
    0x145, 0x146, 0x146, 0x147, 0x147, 0x148, 0x149, 0x149, 0x14a, 0x14a, 0x14b, 0x14c, 0x14c, 0x14d, 0x14d, 0x14e,
    0x14f, 0x14f, 0x150, 0x150, 0x151, 0x152, 0x152, 0x153, 0x153, 0x154, 0x155, 0x155, 0x156, 0x157, 0x157, 0x158,
    0x158, 0x159, 0x15a, 0x15a, 0x15b, 0x15b, 0x15c, 0x15d, 0x15d, 0x15e, 0x15f, 0x15f, 0x160, 0x161, 0x161, 0x162,
    0x162, 0x163, 0x164, 0x164, 0x165, 0x166, 0x166, 0x167, 0x168, 0x168, 0x169, 0x16a, 0x16a, 0x16b, 0x16c, 0x16c,
    0x16d, 0x16e, 0x16e, 0x16f, 0x170, 0x170, 0x171, 0x172, 0x172, 0x173, 0x174, 0x174, 0x175, 0x176, 0x176, 0x177,
    0x178, 0x178, 0x179, 0x17a, 0x17a, 0x17b, 0x17c, 0x17c, 0x17d, 0x17e, 0x17e, 0x17f, 0x180, 0x181, 0x181, 0x182,
    0x183, 0x183, 0x184, 0x185, 0x185, 0x186, 0x187, 0x188, 0x188, 0x189, 0x18a, 0x18a, 0x18b, 0x18c, 0x18d, 0x18d,
    0x18e, 0x18f, 0x18f, 0x190, 0x191, 0x192, 0x192, 0x193, 0x194, 0x194, 0x195, 0x196, 0x197, 0x197, 0x198, 0x199,
    0x19a, 0x19a, 0x19b, 0x19c, 0x19d, 0x19d, 0x19e, 0x19f, 0x1a0, 0x1a0, 0x1a1, 0x1a2, 0x1a3, 0x1a3, 0x1a4, 0x1a5,
    0x1a6, 0x1a6, 0x1a7, 0x1a8, 0x1a9, 0x1a9, 0x1aa, 0x1ab, 0x1ac, 0x1ad, 0x1ad, 0x1ae, 0x1af, 0x1b0, 0x1b0, 0x1b1,
    0x1b2, 0x1b3, 0x1b4, 0x1b4, 0x1b5, 0x1b6, 0x1b7, 0x1b8, 0x1b8, 0x1b9, 0x1ba, 0x1bb, 0x1bc, 0x1bc, 0x1bd, 0x1be,
    0x1bf, 0x1c0, 0x1c0, 0x1c1, 0x1c2, 0x1c3, 0x1c4, 0x1c4, 0x1c5, 0x1c6, 0x1c7, 0x1c8, 0x1c9, 0x1c9, 0x1ca, 0x1cb,
    0x1cc, 0x1cd, 0x1ce, 0x1ce, 0x1cf, 0x1d0, 0x1d1, 0x1d2, 0x1d3, 0x1d3, 0x1d4, 0x1d5, 0x1d6, 0x1d7, 0x1d8, 0x1d8,
    0x1d9, 0x1da, 0x1db, 0x1dc, 0x1dd, 0x1de, 0x1de, 0x1df, 0x1e0, 0x1e1, 0x1e2, 0x1e3, 0x1e4, 0x1e5, 0x1e5, 0x1e6,
    0x1e7, 0x1e8, 0x1e9, 0x1ea, 0x1eb, 0x1ec, 0x1ed, 0x1ed, 0x1ee, 0x1ef, 0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5,
    0x1f6, 0x1f6, 0x1f7, 0x1f8, 0x1f9, 0x1fa, 0x1fb, 0x1fc, 0x1fd, 0x1fe, 0x1ff, 0x200, 0x201, 0x201, 0x202, 0x203,
    0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a, 0x20b, 0x20c, 0x20d, 0x20e, 0x20f, 0x210, 0x210, 0x211, 0x212,
    0x213, 0x214, 0x215, 0x216, 0x217, 0x218, 0x219, 0x21a, 0x21b, 0x21c, 0x21d, 0x21e, 0x21f, 0x220, 0x221, 0x222,
    0x223, 0x224, 0x225, 0x226, 0x227, 0x228, 0x229, 0x22a, 0x22b, 0x22c, 0x22d, 0x22e, 0x22f, 0x230, 0x231, 0x232,
    0x233, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x23a, 0x23b, 0x23c, 0x23d, 0x23e, 0x23f, 0x240, 0x241, 0x242,
    0x244, 0x245, 0x246, 0x247, 0x248, 0x249, 0x24a, 0x24b, 0x24c, 0x24d, 0x24e, 0x24f, 0x250, 0x251, 0x252, 0x253,
    0x254, 0x256, 0x257, 0x258, 0x259, 0x25a, 0x25b, 0x25c, 0x25d, 0x25e, 0x25f, 0x260, 0x262, 0x263, 0x264, 0x265,
    0x266, 0x267, 0x268, 0x269, 0x26a, 0x26c, 0x26d, 0x26e, 0x26f, 0x270, 0x271, 0x272, 0x273, 0x275, 0x276, 0x277,
    0x278, 0x279, 0x27a, 0x27b, 0x27d, 0x27e, 0x27f, 0x280, 0x281, 0x282, 0x284, 0x285, 0x286, 0x287, 0x288, 0x289,
    0x28b, 0x28c, 0x28d, 0x28e, 0x28f, 0x290, 0x292, 0x293, 0x294, 0x295, 0x296, 0x298, 0x299, 0x29a, 0x29b, 0x29c,
    0x29e, 0x29f, 0x2a0, 0x2a1, 0x2a2, 0x2a4, 0x2a5, 0x2a6, 0x2a7, 0x2a9, 0x2aa, 0x2ab, 0x2ac, 0x2ae, 0x2af, 0x2b0,
    0x2b1, 0x2b2, 0x2b4, 0x2b5, 0x2b6, 0x2b7, 0x2b9, 0x2ba, 0x2bb, 0x2bd, 0x2be, 0x2bf, 0x2c0, 0x2c2, 0x2c3, 0x2c4,
    0x2c5, 0x2c7, 0x2c8, 0x2c9, 0x2cb, 0x2cc, 0x2cd, 0x2ce, 0x2d0, 0x2d1, 0x2d2, 0x2d4, 0x2d5, 0x2d6, 0x2d8, 0x2d9,
    0x2da, 0x2dc, 0x2dd, 0x2de, 0x2e0, 0x2e1, 0x2e2, 0x2e4, 0x2e5, 0x2e6, 0x2e8, 0x2e9, 0x2ea, 0x2ec, 0x2ed, 0x2ee,
    0x2f0, 0x2f1, 0x2f2, 0x2f4, 0x2f5, 0x2f6, 0x2f8, 0x2f9, 0x2fb, 0x2fc, 0x2fd, 0x2ff, 0x300, 0x302, 0x303, 0x304,
    0x306, 0x307, 0x309, 0x30a, 0x30b, 0x30d, 0x30e, 0x310, 0x311, 0x312, 0x314, 0x315, 0x317, 0x318, 0x31a, 0x31b,
    0x31c, 0x31e, 0x31f, 0x321, 0x322, 0x324, 0x325, 0x327, 0x328, 0x329, 0x32b, 0x32c, 0x32e, 0x32f, 0x331, 0x332,
    0x334, 0x335, 0x337, 0x338, 0x33a, 0x33b, 0x33d, 0x33e, 0x340, 0x341, 0x343, 0x344, 0x346, 0x347, 0x349, 0x34a,
    0x34c, 0x34d, 0x34f, 0x350, 0x352, 0x353, 0x355, 0x357, 0x358, 0x35a, 0x35b, 0x35d, 0x35e, 0x360, 0x361, 0x363,
    0x365, 0x366, 0x368, 0x369, 0x36b, 0x36c, 0x36e, 0x370, 0x371, 0x373, 0x374, 0x376, 0x378, 0x379, 0x37b, 0x37c,
    0x37e, 0x380, 0x381, 0x383, 0x384, 0x386, 0x388, 0x389, 0x38b, 0x38d, 0x38e, 0x390, 0x392, 0x393, 0x395, 0x397,
    0x398, 0x39a, 0x39c, 0x39d, 0x39f, 0x3a1, 0x3a2, 0x3a4, 0x3a6, 0x3a7, 0x3a9, 0x3ab, 0x3ac, 0x3ae, 0x3b0, 0x3b1,
    0x3b3, 0x3b5, 0x3b7, 0x3b8, 0x3ba, 0x3bc, 0x3bd, 0x3bf, 0x3c1, 0x3c3, 0x3c4, 0x3c6, 0x3c8, 0x3ca, 0x3cb, 0x3cd,
    0x3cf, 0x3d1, 0x3d2, 0x3d4, 0x3d6, 0x3d8, 0x3da, 0x3db, 0x3dd, 0x3df, 0x3e1, 0x3e3, 0x3e4, 0x3e6, 0x3e8, 0x3ea,
    0x3ec, 0x3ed, 0x3ef, 0x3f1, 0x3f3, 0x3f5, 0x3f6, 0x3f8, 0x3fa, 0x3fc, 0x3fe, 0x36c,
};

static const unsigned int volume_mapping_table[] = {
    0, 1, 3, 5, 6, 8, 10, 11, 13, 14, 16, 17, 19, 20, 22, 23,
    25, 26, 27, 29, 30, 32, 33, 34, 36, 37, 39, 41, 43, 45, 47, 49,
    50, 52, 54, 55, 57, 59, 60, 61, 63, 64, 66, 67, 68, 69, 71, 72,
    73, 74, 75, 76, 77, 79, 80, 81, 82, 83, 84, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 92, 93, 94, 95, 96, 96, 97, 98, 99, 99, 100, 101,
    101, 102, 103, 103, 104, 105, 105, 106, 107, 107, 108, 109, 109, 110, 110, 111,
    112, 112, 113, 113, 114, 114, 115, 115, 116, 117, 117, 118, 118, 119, 119, 120,
    120, 121, 121, 122, 122, 123, 123, 123, 124, 124, 125, 125, 126, 126, 127, 127
};

static boolean music_initialized = false;
static int current_music_volume;
static const genmidi_instr_t *main_instrs;
static const genmidi_instr_t *percussion_instrs;
static opl_voice_t voices[OPL_NUM_VOICES];
static opl_voice_t *voice_free_list;
static opl_voice_t *voice_alloced_list;
static opl_track_data_t *tracks = NULL;
static unsigned int num_tracks = 0;
static boolean song_looping;
static uint64_t us_per_beat = 500000; // 120 BPM default
static uint32_t division = 192;

// Fixed-point timing variables
static int32_t render_timer = 0;
static int32_t render_tempo = 0; // Ticks per second

static genmidi_instr_t converted_instrs[256];

static boolean LoadInstrumentTable(void)
{
    for (int i = 0; i < 256; i++) {
        genmidi_instr_t *out = &converted_instrs[i];
        TIMBRE *in = &ADLIB_TimbreBank[i];
        memset(out, 0, sizeof(genmidi_instr_t));
        out->voices[0].modulator.tremolo = in->SAVEK[0];
        out->voices[0].modulator.attack = in->Env1[0];
        out->voices[0].modulator.sustain = in->Env2[0];
        out->voices[0].modulator.waveform = in->Wave[0];
        out->voices[0].modulator.level = in->Level[0];
        out->voices[0].carrier.tremolo = in->SAVEK[1];
        out->voices[0].carrier.attack = in->Env1[1];
        out->voices[0].carrier.sustain = in->Env2[1];
        out->voices[0].carrier.waveform = in->Wave[1];
        out->voices[0].carrier.level = in->Level[1];
        out->voices[0].feedback = in->Feedback;
    }
    main_instrs = &converted_instrs[0];
    percussion_instrs = &converted_instrs[128]; 
    return true;
}

static opl_voice_t *GetFreeVoice(void)
{
    if (voice_free_list == NULL) return NULL;
    opl_voice_t *result = voice_free_list;
    voice_free_list = voice_free_list->next;
    result->next = voice_alloced_list;
    voice_alloced_list = result;
    return result;
}

static void RemoveVoiceFromAllocedList(opl_voice_t *voice)
{
    opl_voice_t **rover = &voice_alloced_list;
    while (*rover != NULL) {
        if (*rover == voice) {
            *rover = voice->next;
            voice->next = NULL;
            break;
        }
        rover = &(*rover)->next;
    }
}

static void VoiceKeyOff(opl_voice_t *voice)
{
    OPL_WriteRegister(OPL_REGS_FREQ_2 + voice->index, voice->freq >> 8);
}

static void ReleaseVoice(opl_voice_t *voice)
{
    voice->channel = NULL;
    voice->note = 0;
    RemoveVoiceFromAllocedList(voice);
    opl_voice_t **rover = &voice_free_list;
    while (*rover != NULL) rover = &(*rover)->next;
    *rover = voice;
    voice->next = NULL;
}

static void LoadOperatorData(int operator, const genmidi_op_t *data, boolean max_level)
{
    int level = (data->scale & 0xc0) | (data->level & 0x3f);
    if (max_level) level |= 0x3f;
    OPL_WriteRegister(OPL_REGS_LEVEL + operator, level);
    OPL_WriteRegister(OPL_REGS_TREMOLO + operator, data->tremolo);
    OPL_WriteRegister(OPL_REGS_ATTACK + operator, data->attack);
    OPL_WriteRegister(OPL_REGS_SUSTAIN + operator, data->sustain);
    OPL_WriteRegister(OPL_REGS_WAVEFORM + operator, data->waveform);
}

static void SetVoiceInstrument(opl_voice_t *voice, const genmidi_instr_t *instr, unsigned int instr_voice)
{
    if (voice->current_instr == instr && voice->current_instr_voice == instr_voice) return;
    voice->current_instr = instr;
    voice->current_instr_voice = instr_voice;
    const genmidi_voice_t *data = &instr->voices[instr_voice];
    boolean modulating = (data->feedback & 0x01) == 0;
    LoadOperatorData(voice->op2, &data->carrier, true);
    LoadOperatorData(voice->op1, &data->modulator, !modulating);
    OPL_WriteRegister(OPL_REGS_FEEDBACK + voice->index, data->feedback | 0x30);
    voice->reg_volume = 999;
}

static void SetVoiceVolume(opl_voice_t *voice, unsigned int volume)
{
    voice->note_volume = volume;
    const genmidi_voice_t *opl_voice = &voice->current_instr->voices[voice->current_instr_voice];
    unsigned int full_volume = (volume_mapping_table[voice->note_volume]
                   * volume_mapping_table[voice->channel->volume]
                   * volume_mapping_table[current_music_volume]) / (127 * 127);
    unsigned int op_volume = 0x3f - opl_voice->carrier.level;
    unsigned int reg_volume = (op_volume * full_volume) / 128;
    reg_volume = (0x3f - reg_volume) | opl_voice->carrier.scale;
    if (reg_volume != voice->reg_volume) {
        voice->reg_volume = reg_volume;
        OPL_WriteRegister(OPL_REGS_LEVEL + voice->op2, reg_volume);
        if ((opl_voice->feedback & 0x01) != 0) OPL_WriteRegister(OPL_REGS_LEVEL + voice->op1, reg_volume);
    }
}

static void KeyOffEvent(opl_track_data_t *track, midi_event_t *event)
{
    opl_channel_data_t *channel = &track->channels[event->data.channel.channel];
    unsigned int key = event->data.channel.param1;
    for (int i=0; i<OPL_NUM_VOICES; ++i) {
        if (voices[i].channel == channel && voices[i].key == key) {
            VoiceKeyOff(&voices[i]);
            ReleaseVoice(&voices[i]);
        }
    }
}

static unsigned int FrequencyForVoice(opl_voice_t *voice)
{
    const genmidi_voice_t *gm_voice = &voice->current_instr->voices[voice->current_instr_voice];
    unsigned int note = voice->note;
    if ((voice->current_instr->flags & GENMIDI_FLAG_FIXED) == 0) note += (signed short)gm_voice->base_note_offset;
    if (note > 0x7f) note = voice->note;
    unsigned int freq_index = 64 + 32 * note + voice->channel->bend;
    if (voice->current_instr_voice != 0) freq_index += (voice->current_instr->fine_tuning / 2) - 64;
    if (freq_index < 284) return frequency_curve[freq_index];
    unsigned int sub_index = (freq_index - 284) % (12 * 32);
    unsigned int octave = (freq_index - 284) / (12 * 32);
    if (octave >= 7) octave = (sub_index < 5) ? 7 : 6;
    return frequency_curve[sub_index + 284] | (octave << 10);
}

static void UpdateVoiceFrequency(opl_voice_t *voice)
{
    unsigned int freq = FrequencyForVoice(voice);
    if (voice->freq != freq) {
        OPL_WriteRegister(OPL_REGS_FREQ_1 + voice->index, freq & 0xff);
        OPL_WriteRegister(OPL_REGS_FREQ_2 + voice->index, (freq >> 8) | 0x20);
        voice->freq = freq;
    }
}

static void VoiceKeyOn(opl_channel_data_t *channel, const genmidi_instr_t *instrument, unsigned int instrument_voice, unsigned int key, unsigned int volume)
{
    opl_voice_t *voice = GetFreeVoice();
    if (voice == NULL) {
        if (instrument_voice == 0) {
            voice = voice_alloced_list; // Replace oldest
            if (voice) {
                VoiceKeyOff(voice);
                ReleaseVoice(voice);
                voice = GetFreeVoice();
            }
        }
    }
    if (voice) {
        voice->channel = channel;
        voice->key = key;
        voice->note = ((instrument->flags & GENMIDI_FLAG_FIXED) != 0) ? instrument->fixed_note : key;
        SetVoiceInstrument(voice, instrument, instrument_voice);
        SetVoiceVolume(voice, volume);
        voice->freq = 0;
        UpdateVoiceFrequency(voice);
    }
}

static void KeyOnEvent(opl_track_data_t *track, midi_event_t *event)
{
    if (event->data.channel.param2 == 0) {
        KeyOffEvent (track, event);
        return;
    }
    opl_channel_data_t *channel = &track->channels[event->data.channel.channel];
    unsigned int key = event->data.channel.param1;
    unsigned int volume = event->data.channel.param2;
    const genmidi_instr_t *instrument = (event->data.channel.channel == 9) ? &percussion_instrs[key - 35] : channel->instrument;
    if (event->data.channel.channel == 9 && (key < 35 || key > 81)) return;
    VoiceKeyOn(channel, instrument, 0, key, volume);
    if ((instrument->flags & GENMIDI_FLAG_2VOICE) != 0) VoiceKeyOn(channel, instrument, 1, key, volume);
}

static double opl_tempo_multiplier = 1.0;

static void UpdateRenderTempo(void)
{
    if (us_per_beat > 0)
    {
        double base_tempo = (double)(1000000ULL * division) / us_per_beat;
        render_tempo = (int32_t)(base_tempo * opl_tempo_multiplier);
    }
    else
        render_tempo = 0;
}

void I_OPL_SetTempoMultiplier(double multiplier)
{
    opl_tempo_multiplier = multiplier;
    UpdateRenderTempo();
}

static void ProcessEvent(opl_track_data_t *track, midi_event_t *event)
{
    int channel = event->data.channel.channel;
    switch (event->event_type) {
        case MIDI_EVENT_NOTE_OFF: KeyOffEvent(track, event); break;
        case MIDI_EVENT_NOTE_ON: KeyOnEvent(track, event); break;
        case MIDI_EVENT_PROGRAM_CHANGE: track->channels[channel].instrument = &main_instrs[event->data.channel.param1]; break;
        case MIDI_EVENT_CONTROLLER: if (event->data.channel.param1 == MIDI_CONTROLLER_MAIN_VOLUME) {
            track->channels[channel].volume = event->data.channel.param2;
            for (int i=0; i<OPL_NUM_VOICES; ++i) if (voices[i].channel == &track->channels[channel]) SetVoiceVolume(&voices[i], voices[i].note_volume);
        } break;
        case MIDI_EVENT_PITCH_BEND: track->channels[channel].bend = event->data.channel.param2 - 64;
            for (int i=0; i<OPL_NUM_VOICES; ++i) if (voices[i].channel == &track->channels[channel]) UpdateVoiceFrequency(&voices[i]);
            break;
        case MIDI_EVENT_META:
            if (event->data.meta.type == MIDI_META_SET_TEMPO && event->data.meta.length == 3) {
                us_per_beat = (event->data.meta.data[0] << 16) | (event->data.meta.data[1] << 8) | event->data.meta.data[2];
                UpdateRenderTempo();
            }
            break;
        default: break;
    }
}

static void MIDI_ServiceRoutine(void)
{
    if (!tracks) return;
    for (int i = 0; i < (int)num_tracks; i++) {
        if (!tracks[i].active) continue;
        
        while (tracks[i].active && tracks[i].delay == 0) {
            midi_event_t *event;
            if (!MIDI_GetNextEvent(tracks[i].iter, &event)) {
                tracks[i].active = false;
                break;
            }
            ProcessEvent(&tracks[i], event);
            if (event->event_type == MIDI_EVENT_META && event->data.meta.type == MIDI_META_END_OF_TRACK) {
                tracks[i].active = false;
                break;
            }
            tracks[i].delay = MIDI_GetDeltaTime(tracks[i].iter);
        }
        
        if (tracks[i].delay > 0) tracks[i].delay--;
    }
}

static void RestartSong(void)
{
    if (!tracks) return;
    us_per_beat = 500000;
    UpdateRenderTempo();
    render_timer = 0;
    for (int i = 0; i < (int)num_tracks; i++) {
        if (tracks[i].iter) {
            MIDI_RestartIterator(tracks[i].iter);
            tracks[i].active = true;
            tracks[i].delay = MIDI_GetDeltaTime(tracks[i].iter);
        }
    }
}

void I_OPL_RenderSamples(void *dest, unsigned nsamp)
{
    int16_t *buffer = (int16_t *)dest;
    uint32_t samples_left = nsamp;

    if (!tracks || render_tempo == 0) {
        memset(dest, 0, nsamp * 2 * sizeof(int16_t));
        return;
    }

    while (samples_left > 0) {
        // Run sequencer if it's time for a MIDI tick
        while (render_timer >= (int32_t)opl_sample_rate) {
            boolean all_finished = true;
            for (int t = 0; t < (int)num_tracks; t++) if (tracks[t].active) all_finished = false;
            
            if (all_finished) {
                if (song_looping) RestartSong();
                else {
                    // Fill remaining buffer with silence if song ended
                    memset(buffer, 0, samples_left * 2 * sizeof(int16_t));
                    return;
                }
            }
            
            MIDI_ServiceRoutine();
            render_timer -= (int32_t)opl_sample_rate;
        }

        // How many samples until the next MIDI tick?
        uint32_t samples_to_tick = (opl_sample_rate - render_timer + render_tempo - 1) / render_tempo;
        uint32_t step = (samples_to_tick < samples_left) ? samples_to_tick : samples_left;

        if (step > 0) {
            OPL_Render_Samples(buffer, step);
            buffer += step * 2;
            samples_left -= step;
            render_timer += step * render_tempo;
        }
    }
}

int I_OPL_InitMusic(int samplerate)
{
    if (!OPL_Init(samplerate)) return 0;
    if (!LoadInstrumentTable()) { OPL_Shutdown(); return 0; }
    voice_free_list = NULL; voice_alloced_list = NULL;
    for (int i=0; i<OPL_NUM_VOICES; ++i) {
        voices[i].index = i; voices[i].op1 = voice_operators[0][i]; voices[i].op2 = voice_operators[1][i];
        ReleaseVoice(&voices[i]);
    }
    music_initialized = true;
    return 1;
}

static void I_OPL_StopSong(void);

static void I_OPL_PlaySong(const void *handle, int looping)
{
    if (!music_initialized || handle == NULL) return;
    
    I_OPL_StopSong();
    
    num_tracks = MIDI_NumTracks(handle);
    tracks = calloc(num_tracks, sizeof(opl_track_data_t));
    if (!tracks) return;
    
    song_looping = looping; 
    division = MIDI_GetFileTimeDivision(handle);
    
    for (int i=0; i< (int)num_tracks; ++i) {
        tracks[i].iter = MIDI_IterateTrack(handle, i);
        for (int c=0; c<MIDI_CHANNELS_PER_TRACK; ++c) { 
            tracks[i].channels[c].instrument = &main_instrs[0]; 
            tracks[i].channels[c].volume = 127; 
            tracks[i].channels[c].bend = 0; 
        }
    }
    
    RestartSong();
}

void I_OPL_SetMusicVolume(int volume)
{
    current_music_volume = volume;
    for (int i=0; i<OPL_NUM_VOICES; ++i) if (voices[i].channel != NULL) SetVoiceVolume(&voices[i], voices[i].note_volume);
}

static void I_OPL_StopSong(void)
{
    if (!music_initialized) return;
    for (int i=0; i<OPL_NUM_VOICES; ++i) if (voices[i].channel != NULL) { VoiceKeyOff(&voices[i]); ReleaseVoice(&voices[i]); }
    if (tracks) { 
        for (int i=0; i< (int)num_tracks; ++i) if (tracks[i].iter) MIDI_FreeIterator(tracks[i].iter); 
        free(tracks); 
        tracks = NULL; 
    }
    num_tracks = 0;
}

static const void *I_OPL_RegisterSong(const void *data, unsigned len) {
    midimem_t mf = { (const byte *)data, len, 0 };
    return MIDI_LoadFile(&mf);
}

static void I_OPL_UnRegisterSong(const void *handle) {
    if (handle) MIDI_FreeFile((midi_file_t *)handle);
}

static const char *I_OPL_SynthName(void) { return "OPL3 Synth"; }
static void I_OPL_Pause(void) { OPL_SetPaused(1); }
static void I_OPL_Resume(void) { OPL_SetPaused(0); }
static void I_OPL_Shutdown(void) { I_OPL_StopSong(); OPL_Shutdown(); music_initialized = false; }

const music_player_t opl_synth_player = { 
    I_OPL_SynthName, I_OPL_InitMusic, I_OPL_Shutdown, I_OPL_SetMusicVolume, 
    I_OPL_Pause, I_OPL_Resume, I_OPL_RegisterSong, I_OPL_UnRegisterSong, 
    I_OPL_PlaySong, I_OPL_StopSong, I_OPL_RenderSamples 
};
