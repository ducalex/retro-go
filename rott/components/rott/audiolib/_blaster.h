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
/**********************************************************************
   module: _BLASTER.H

   author: James R. Dose
   date:   February 4, 1994

   Private header for for BLASTER.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef ___BLASTER_H
#define ___BLASTER_H

#define VALID   ( 1 == 1 )
#define INVALID ( !VALID )

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

#define YES ( 1 == 1 )
#define NO  ( !YES )

#define lobyte( num )   ( ( int )*( ( char * )&( num ) ) )
#define hibyte( num )   ( ( int )*( ( ( char * )&( num ) ) + 1 ) )

#define BLASTER_MixerAddressPort  0x04
#define BLASTER_MixerDataPort     0x05
#define BLASTER_ResetPort         0x06
#define BLASTER_ReadPort          0x0A
#define BLASTER_WritePort         0x0C
#define BLASTER_DataAvailablePort 0x0E
#define BLASTER_Ready             0xAA
#define BLASTER_16BitDMAAck       0x0F

#define MIXER_DSP4xxISR_Ack       0x82
#define MIXER_DSP4xxISR_Enable    0x83
#define MIXER_MPU401_INT          0x4
#define MIXER_16BITDMA_INT        0x2
#define MIXER_8BITDMA_INT         0x1
#define MIXER_DisableMPU401Interrupts 0xB
#define MIXER_SBProOutputSetting  0x0E
#define MIXER_SBProStereoFlag     0x02
#define MIXER_SBProVoice          0x04
#define MIXER_SBProMidi           0x26
#define MIXER_SB16VoiceLeft       0x32
#define MIXER_SB16VoiceRight      0x33
#define MIXER_SB16MidiLeft        0x34
#define MIXER_SB16MidiRight       0x35

#define DSP_Version1xx            0x0100
#define DSP_Version2xx            0x0200
#define DSP_Version201            0x0201
#define DSP_Version3xx            0x0300
#define DSP_Version4xx            0x0400
#define DSP_SB16Version           DSP_Version4xx

#define DSP_MaxNormalRate         22000
#define DSP_MaxHighSpeedRate      44000

#define DSP_8BitAutoInitRecord        0x2c
#define DSP_8BitHighSpeedAutoInitRecord 0x98
#define DSP_Old8BitADC                0x24
#define DSP_8BitAutoInitMode          0x1c
#define DSP_8BitHighSpeedAutoInitMode 0x90
#define DSP_SetBlockLength            0x48
#define DSP_Old8BitDAC                0x14
#define DSP_16BitDAC                  0xB6
#define DSP_8BitDAC                   0xC6
#define DSP_8BitADC                   0xCe
#define DSP_SetTimeConstant           0x40
#define DSP_Set_DA_Rate               0x41
#define DSP_Set_AD_Rate               0x42
#define DSP_Halt8bitTransfer          0xd0
#define DSP_Continue8bitTransfer      0xd4
#define DSP_Halt16bitTransfer         0xd5
#define DSP_Continue16bitTransfer     0xd6
#define DSP_SpeakerOn                 0xd1
#define DSP_SpeakerOff                0xd3
#define DSP_GetVersion                0xE1
#define DSP_Reset                     0xFFFF

#define DSP_SignedBit                 0x10
#define DSP_StereoBit                 0x20

#define DSP_UnsignedMonoData      0x00
#define DSP_SignedMonoData        ( DSP_SignedBit )
#define DSP_UnsignedStereoData    ( DSP_StereoBit )
#define DSP_SignedStereoData      ( DSP_SignedBit | DSP_StereoBit )

#define BlasterEnv_Address    'A'
#define BlasterEnv_Interrupt  'I'
#define BlasterEnv_8bitDma    'D'
#define BlasterEnv_16bitDma   'H'
#define BlasterEnv_Type       'T'
#define BlasterEnv_Midi       'P'
#define BlasterEnv_EmuAddress 'E'

#define CalcTimeConstant( rate, samplesize ) \
   ( ( 65536L - ( 256000000L / ( ( samplesize ) * ( rate ) ) ) ) >> 8 )

#define CalcSamplingRate( tc ) \
   ( 256000000L / ( 65536L - ( tc << 8 ) ) )

typedef struct
   {
   int IsSupported;
   int HasMixer;
   int MaxMixMode;
   int MinSamplingRate;
   int MaxSamplingRate;
   } CARD_CAPABILITY;

#endif
