/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/****************************************************************************\
*                                                                            *
* CTAWEAPI.H SB AWE32 DOS API header                                         *
*                                                                            *
* (C) Copyright Creative Technology Ltd. 1992-94. All rights reserved        *
* worldwide.                                                                 *
*                                                                            *
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY      *
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE        *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR      *
* PURPOSE.                                                                   *
*                                                                            *
* You have a royalty-free right to use, modify, reproduce and                *
* distribute the Sample Files (and/or any modified version) in               *
* any way you find useful, provided that you agree to                        *
* the Creative's Software Licensing Aggreement and you also agree that       *
* Creative has no warranty obligations or liability for any Sample Files.    *
*                                                                            *
\****************************************************************************/

/****************************************************************************\
*      File name       : CTAWEAPI.H                                          *
*                                                                            *
*      Programmer      : Creative SB AWE32 Team                              *
*                        Creative Technology Ltd, 1994. All rights reserved. *
*                                                                            *
*      Version         : 2.0b                                                *
*                                                                            *
\****************************************************************************/

#ifndef _CTAWEAPI
#define _CTAWEAPI


#define MAXBANKS            64      /* maximum number of banks */
#define MAXNRPN             32      /* maximum number of NRPN */


#if defined(__FLAT__) || defined(__HIGHC__) || defined(DOS386)
#define PACKETSIZE      8192        /* packet size for 32bit libraries */
#else
#define PACKETSIZE      512         /* packet size for real mode libraries */
#endif


#if defined(__FLAT__)
    #define NEAR
    #define FAR
#endif


#if defined(__SC__)
    #pragma pack(1)
    #if defined(DOS386)
        #define NEAR
        #define FAR
    #endif
#endif


#if defined(__WATCOMC__)
    #pragma pack(1)
#endif


#if defined(__HIGHC__)
    #define NEAR
    #define FAR
    #define PASCAL  _DCC((_DEFAULT_CALLING_CONVENTION|_CALLEE_POPS_STACK) & \
                         ~ (_REVERSE_PARMS|_OVERLOADED))
    #pragma Push_align_members(1)
    #pragma Global_aliasing_convention("_%r")
#endif


typedef int                     BOOL;
#define FALSE                   0
#define TRUE                    1

typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;

typedef short int               SHORT;
typedef unsigned int		UINT;
typedef signed long 		LONG;

#ifndef FAR
#define FAR                     __far
#endif

#ifndef HUGE
#define HUGE                    __huge
#endif

#ifndef PASCAL
#define PASCAL                  __pascal
#endif

typedef void FAR*               LPVOID;
typedef BYTE FAR*               LPBYTE;
typedef WORD FAR*               LPWORD;
typedef DWORD FAR*              LPDWORD;

#define LOBYTE(w)               ((BYTE)(w))
#define HIBYTE(w)               ((BYTE)(((UINT)(w) >> 8) & 0xFF))

#define LOWORD(l)               ((WORD)(DWORD)(l))
#define HIWORD(l)               ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))


#if defined(__cplusplus)
extern "C" {
#endif


/* Start of modules */
extern int* __midieng_code(void);
extern int* __hardware_code(void);
extern int* __sbkload_code(void);
extern int* __nrpn_code(void);
extern int __midivar_data;
extern int __nrpnvar_data;
extern int __embed_data;


typedef char SCRATCH[702];
typedef char SOUNDFONT[124];
typedef char GCHANNEL[20];
typedef char MIDICHANNEL[32];
typedef char NRPNCHANNEL[96];

typedef struct {
    SHORT bank_no;          /* Slot number being used */
    SHORT total_banks;      /* Total number of banks */
    LONG FAR* banksizes;    /* Pointer to a list of bank sizes */
    LONG file_size;         /* exact size of the sound font file */
    char FAR* data;         /* Address of buffer of size >= PACKETSIZE */
    char FAR* presets;      /* Allocated memory for preset data */

    LONG total_patch_ram;   /* Total patch ram available */
    SHORT no_sample_packets;/* Number of packets of sound sample to stream */
    LONG sample_seek;       /* Start file location of sound sample */
    LONG preset_seek;       /* Address of preset_seek location */
    LONG preset_read_size;  /* Number of bytes from preset_seek to allocate and read */
    LONG preset_size;       /* Preset actual size */
} SOUND_PACKET;

typedef struct {
    SHORT tag;              /* Must be 0x100 or 0x101 */
    SHORT preset_size;      /* Preset table of this size is required */
    SHORT no_wave_packets;  /* Number of packets of Wave sample to stream. */
    LONG reserved;

    SHORT bank_no;          /* bank number */
    char FAR* data;         /* Address of packet of size PACKETSIZE */
    char FAR* presets;      /* Allocated memory for preset data */
    LONG sample_size;       /* Sample size, i.e. number of samples */
    LONG samples_per_sec;   /* Samples per second */
    SHORT bits_per_sample;  /* Bits per sample, 8 or 16 */
    SHORT no_channels;      /* Number of channels, 1=mono, 2=stereo */
    SHORT looping;          /* Looping? 0=no, 1=yes */
    LONG startloop;         /* if looping, then these are the addresses */
    LONG endloop;
    SHORT release;          /* release time, 0=24ms, 8191=23.78s */
} WAVE_PACKET;

typedef struct {
    LPBYTE SPad1;
    LPBYTE SPad2;
    LPBYTE SPad3;
    LPBYTE SPad4;
    LPBYTE SPad5;
    LPBYTE SPad6;
    LPBYTE SPad7;
} SOUNDPAD;

/* AWE32 variables */
extern WORD         awe32NumG;
extern WORD         awe32BaseAddx;
extern DWORD        awe32DramSize;

/* MIDI variables */
extern SCRATCH      awe32Scratch;
extern SOUNDFONT    awe32SFont[4];
extern GCHANNEL     awe32GChannel[32];
extern MIDICHANNEL  awe32MIDIChannel[16];
extern SOUNDPAD     awe32SoundPad;

/* NRPN variables */
extern NRPNCHANNEL  awe32NRPNChannel[16];

/* SoundFont objects */
extern BYTE awe32SPad1Obj[];
extern BYTE awe32SPad2Obj[];
extern BYTE awe32SPad3Obj[];
extern BYTE awe32SPad4Obj[];
extern BYTE awe32SPad5Obj[];
extern BYTE awe32SPad6Obj[];
extern BYTE awe32SPad7Obj[];

/* AWE register functions */
extern void PASCAL awe32RegW(WORD, WORD);
extern WORD PASCAL awe32RegRW(WORD);
extern void PASCAL awe32RegDW(WORD, DWORD);
extern DWORD PASCAL awe32RegRDW(WORD);

/* MIDI support functions */
extern WORD PASCAL awe32InitMIDI(void);
extern WORD PASCAL awe32NoteOn(WORD, WORD, WORD);
extern WORD PASCAL awe32NoteOff(WORD, WORD, WORD);
extern WORD PASCAL awe32ProgramChange(WORD, WORD);
extern WORD PASCAL awe32Controller(WORD, WORD, WORD);
extern WORD PASCAL awe32PolyKeyPressure(WORD, WORD, WORD);
extern WORD PASCAL awe32ChannelPressure(WORD, WORD);
extern WORD PASCAL awe32PitchBend(WORD, WORD, WORD);
extern WORD PASCAL awe32Sysex(WORD, LPBYTE, WORD);
extern WORD PASCAL __awe32NoteOff(WORD, WORD, WORD, WORD);
extern WORD PASCAL __awe32IsPlaying(WORD, WORD, WORD, WORD);

/* NRPN support functions */
extern WORD PASCAL awe32InitNRPN(void);

/* Hardware support functions */
extern WORD PASCAL awe32Detect(WORD);
extern WORD PASCAL awe32InitHardware(void);
extern WORD PASCAL awe32Terminate(void);

/* SoundFont support functions */
extern WORD PASCAL awe32TotalPatchRam(SOUND_PACKET FAR*);
extern WORD PASCAL awe32DefineBankSizes(SOUND_PACKET FAR*);
extern WORD PASCAL awe32SFontLoadRequest(SOUND_PACKET FAR*);
extern WORD PASCAL awe32StreamSample(SOUND_PACKET FAR*);
extern WORD PASCAL awe32SetPresets(SOUND_PACKET FAR*);
extern WORD PASCAL awe32ReleaseBank(SOUND_PACKET FAR*);
extern WORD PASCAL awe32ReleaseAllBanks(SOUND_PACKET FAR*);
extern WORD PASCAL awe32WPLoadRequest(WAVE_PACKET FAR*);
extern WORD PASCAL awe32WPLoadWave(WAVE_PACKET FAR*);
extern WORD PASCAL awe32WPStreamWave(WAVE_PACKET FAR*);
extern WORD PASCAL awe32WPBuildSFont(WAVE_PACKET FAR*);

/* End of modules */
extern int* __midieng_ecode(void);
extern int* __hardware_ecode(void);
extern int* __sbkload_ecode(void);
extern int* __nrpn_ecode(void);
extern int __midivar_edata;
extern int __nrpnvar_edata;
extern int __embed_edata;


#if defined(__cplusplus)
}
#endif


#if defined(__SC__)
    #pragma pack()
#endif


#if defined(__HIGHC__)
    #pragma Pop_align_members
    #pragma Global_aliasing_convention()
    #pragma Alias(awe32RegW,"AWE32REGW")
    #pragma Alias(awe32RegRW,"AWE32REGRW")
    #pragma Alias(awe32RegDW,"AWE32REGDW")
    #pragma Alias(awe32RegRDW,"AWE32REGRDW")
    #pragma Alias(awe32InitMIDI,"AWE32INITMIDI")
    #pragma Alias(awe32NoteOn,"AWE32NOTEON")
    #pragma Alias(awe32NoteOff,"AWE32NOTEOFF")
    #pragma Alias(awe32ProgramChange,"AWE32PROGRAMCHANGE")
    #pragma Alias(awe32Controller,"AWE32CONTROLLER")
    #pragma Alias(awe32PolyKeyPressure,"AWE32POLYKEYPRESSURE")
    #pragma Alias(awe32ChannelPressure,"AWE32CHANNELPRESSURE")
    #pragma Alias(awe32PitchBend,"AWE32PITCHBEND")
    #pragma Alias(awe32Sysex,"AWE32SYSEX")
    #pragma Alias(__awe32NoteOff,"__AWE32NOTEOFF")
    #pragma Alias(__awe32IsPlaying,"__AWE32ISPLAYING")
    #pragma Alias(awe32InitNRPN,"AWE32INITNRPN")
    #pragma Alias(awe32Detect,"AWE32DETECT")
    #pragma Alias(awe32InitHardware,"AWE32INITHARDWARE")
    #pragma Alias(awe32Terminate,"AWE32TERMINATE")
    #pragma Alias(awe32TotalPatchRam,"AWE32TOTALPATCHRAM")
    #pragma Alias(awe32DefineBankSizes,"AWE32DEFINEBANKSIZES")
    #pragma Alias(awe32SFontLoadRequest,"AWE32SFONTLOADREQUEST")
    #pragma Alias(awe32StreamSample,"AWE32STREAMSAMPLE")
    #pragma Alias(awe32SetPresets,"AWE32SETPRESETS")
    #pragma Alias(awe32ReleaseBank,"AWE32RELEASEBANK")
    #pragma Alias(awe32ReleaseAllBanks,"AWE32RELEASEALLBANKS")
    #pragma Alias(awe32WPLoadRequest,"AWE32WPLOADREQUEST")
    #pragma Alias(awe32WPLoadWave,"AWE32WPLOADWAVE")
    #pragma Alias(awe32WPStreamWave,"AWE32WPSTREAMWAVE")
    #pragma Alias(awe32WPBuildSFont,"AWE32WPBUILDSFONT")
#endif


#if defined(__WATCOMC__)
    #pragma pack()
    #pragma aux awe32NumG "_*"
    #pragma aux awe32BaseAddx "_*"
    #pragma aux awe32DramSize "_*"
    #pragma aux awe32Scratch "_*"
    #pragma aux awe32SFont "_*"
    #pragma aux awe32GChannel "_*"
    #pragma aux awe32MIDIChannel "_*"
    #pragma aux awe32SoundPad "_*"
    #pragma aux awe32NRPNChannel "_*"
    #pragma aux awe32SPad1Obj "_*"
    #pragma aux awe32SPad2Obj "_*"
    #pragma aux awe32SPad3Obj "_*"
    #pragma aux awe32SPad4Obj "_*"
    #pragma aux awe32SPad5Obj "_*"
    #pragma aux awe32SPad6Obj "_*"
    #pragma aux awe32SPad7Obj "_*"
    #pragma aux __midieng_code "_*"
    #pragma aux __midieng_ecode "_*"
    #pragma aux __hardware_code "_*"
    #pragma aux __hardware_ecode "_*"
    #pragma aux __sbkload_code "_*"
    #pragma aux __sbkload_ecode "_*"
    #pragma aux __nrpn_code "_*"
    #pragma aux __nrpn_ecode "_*"
    #pragma aux __midivar_data "_*"
    #pragma aux __midivar_edata "_*"
    #pragma aux __nrpnvar_data "_*"
    #pragma aux __nrpnvar_edata "_*"
    #pragma aux __embed_data "_*"
    #pragma aux __embed_edata "_*"
#endif


#endif      /* _CTAWEAPI */
