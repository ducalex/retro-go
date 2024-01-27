/* This file is part of Snes9x. See LICENSE file. */

#ifndef USE_BLARGG_APU

#include "snes9x.h"
#include "spc700.h"
#include "apu.h"
#include "apumem.h"
#include "soundux.h"
#include "cpuexec.h"

extern const int32_t NoiseFreq[32];

bool S9xInitAPU()
{
   IAPU.RAM = (uint8_t*) malloc(0x10000);

   if (!IAPU.RAM)
   {
      S9xDeinitAPU();
      return false;
   }

   return true;
}

void S9xDeinitAPU()
{
   if (IAPU.RAM)
   {
      free(IAPU.RAM);
      IAPU.RAM = NULL;
   }
}

void S9xResetAPU()
{
   int32_t i, j;
   Settings.APUEnabled = true;
   memset(IAPU.RAM, 0, 0x100);
   memset(IAPU.RAM + 0x20, 0xFF, 0x20);
   memset(IAPU.RAM + 0x60, 0xFF, 0x20);
   memset(IAPU.RAM + 0xA0, 0xFF, 0x20);
   memset(IAPU.RAM + 0xE0, 0xFF, 0x20);

   for (i = 1; i < 256; i++)
      memcpy(IAPU.RAM + (i << 8), IAPU.RAM, 0x100);

   memset(APU.OutPorts, 0, sizeof(APU.OutPorts));
   IAPU.DirectPage = IAPU.RAM;
   /* memmove converted: Different mallocs [Neb]
    * DS2 DMA notes: The APU ROM is not 32-byte aligned [Neb] */
   memcpy(&IAPU.RAM [0xffc0], APUROM, sizeof(APUROM));
   /* memmove converted: Different mallocs [Neb]
    * DS2 DMA notes: The APU ROM is not 32-byte aligned [Neb] */
   memcpy(APU.ExtraRAM, APUROM, sizeof(APUROM));
   IAPU.PC = IAPU.RAM + IAPU.RAM [0xfffe] + (IAPU.RAM [0xffff] << 8);
   APU.Cycles = 0;
   IAPU.Registers.YA.W = 0;
   IAPU.Registers.X = 0;
   IAPU.Registers.S = 0xef;
   IAPU.Registers.P = 0x02;
   S9xAPUUnpackStatus();
   IAPU.Registers.PC = 0;
   IAPU.APUExecuting = Settings.APUEnabled;
   IAPU.WaitAddress1 = NULL;
   IAPU.WaitAddress2 = NULL;
   IAPU.WaitCounter = 1;
   APU.ShowROM = true;
   IAPU.RAM [0xf1] = 0x80;

   for (i = 0; i < 3; i++)
   {
      APU.TimerEnabled [i] = false;
      APU.TimerTarget [i] = 0;
      APU.Timer [i] = 0;
   }
   for (j = 0; j < 0x80; j++)
      APU.DSP [j] = 0;

   IAPU.TwoCycles = IAPU.OneCycle * 2;

   for (i = 0; i < 256; i++)
      S9xAPUCycles [i] = S9xAPUCycleLengths [i] * IAPU.OneCycle;

   APU.DSP [APU_ENDX] = 0;
   APU.DSP [APU_KOFF] = 0;
   APU.DSP [APU_KON] = 0;
   APU.DSP[APU_FLG] = APU_SOFT_RESET | APU_MUTE;
   APU.KeyedChannels = 0;

   S9xResetSound(true);
   S9xSetEchoEnable(0);
}

void S9xSetAPUDSP(uint8_t byte)
{
   uint8_t reg = IAPU.RAM [0xf2];
   static uint8_t KeyOn;
   static uint8_t KeyOnPrev;
   int32_t i;

   switch (reg)
   {
   case APU_FLG:
      if (byte & APU_SOFT_RESET)
      {
         APU.DSP [reg] = APU_MUTE | APU_ECHO_DISABLED | (byte & 0x1f);
         APU.DSP [APU_ENDX] = 0;
         APU.DSP [APU_KOFF] = 0;
         APU.DSP [APU_KON] = 0;
         S9xSetEchoWriteEnable(false);

         /* Kill sound */
         S9xResetSound(false);
      }
      else
      {
         S9xSetEchoWriteEnable(!(byte & APU_ECHO_DISABLED));
         so.mute_sound = !!(byte & APU_MUTE);
         SoundData.noise_hertz = NoiseFreq [byte & 0x1f];
         for (i = 0; i < 8; i++)
            if (SoundData.channels [i].type == SOUND_NOISE)
               S9xSetSoundFrequency(i, SoundData.noise_hertz);
      }
      break;
   case APU_NON:
      if (byte != APU.DSP [APU_NON])
      {
         int32_t c;
         uint8_t mask = 1;
         for (c = 0; c < 8; c++, mask <<= 1)
         {
            int32_t type;

            if (byte & mask)
               type = SOUND_NOISE;
            else
               type = SOUND_SAMPLE;

            S9xSetSoundType(c, type);
         }
      }
      break;
   case APU_MVOL_LEFT:
      if (byte != APU.DSP [APU_MVOL_LEFT])
         S9xSetMasterVolume((int8_t) byte, (int8_t) APU.DSP [APU_MVOL_RIGHT]);
      break;
   case APU_MVOL_RIGHT:
      if (byte != APU.DSP [APU_MVOL_RIGHT])
         S9xSetMasterVolume((int8_t) APU.DSP [APU_MVOL_LEFT], (int8_t) byte);
      break;
   case APU_EVOL_LEFT:
      if (byte != APU.DSP [APU_EVOL_LEFT])
         S9xSetEchoVolume((int8_t) byte, (int8_t) APU.DSP [APU_EVOL_RIGHT]);
      break;
   case APU_EVOL_RIGHT:
      if (byte != APU.DSP [APU_EVOL_RIGHT])
         S9xSetEchoVolume((int8_t) APU.DSP [APU_EVOL_LEFT], (int8_t) byte);
      break;
   case APU_ENDX:
      byte = 0;
      break;
   case APU_KOFF:
   {
      int32_t c;
      uint8_t mask = 1;
      for (c = 0; c < 8; c++, mask <<= 1)
      {
         if ((byte & mask) != 0)
         {
            if (APU.KeyedChannels & mask)
            {
               KeyOnPrev &= ~mask;
               APU.KeyedChannels &= ~mask;
               APU.DSP [APU_KON] &= ~mask;
               S9xSetSoundKeyOff(c);
            }
         }
         else if ((KeyOnPrev & mask) != 0)
         {
            KeyOnPrev &= ~mask;
            APU.KeyedChannels |= mask;
            APU.DSP [APU_KOFF] &= ~mask;
            APU.DSP [APU_ENDX] &= ~mask;
            S9xPlaySample(c);
         }
      }

      APU.DSP [APU_KOFF] = byte;
      return;
   }
   case APU_KON:
      if (byte)
      {
         int32_t c;
         uint8_t mask = 1;
         for (c = 0; c < 8; c++, mask <<= 1)
         {
            if ((byte & mask) != 0)
            {
               /* Pac-In-Time requires that channels can be key-on
                * regardeless of their current state. */
               if ((APU.DSP [APU_KOFF] & mask) == 0)
               {
                  KeyOnPrev &= ~mask;
                  APU.KeyedChannels |= mask;
                  APU.DSP [APU_ENDX] &= ~mask;
                  S9xPlaySample(c);
               }
               else
                  KeyOn |= mask;
            }
         }
      }
      return;
   case APU_VOL_LEFT + 0x00:
   case APU_VOL_LEFT + 0x10:
   case APU_VOL_LEFT + 0x20:
   case APU_VOL_LEFT + 0x30:
   case APU_VOL_LEFT + 0x40:
   case APU_VOL_LEFT + 0x50:
   case APU_VOL_LEFT + 0x60:
   case APU_VOL_LEFT + 0x70:
      S9xSetSoundVolume(reg >> 4, (int8_t) byte, (int8_t) APU.DSP [reg + 1]);
      break;
   case APU_VOL_RIGHT + 0x00:
   case APU_VOL_RIGHT + 0x10:
   case APU_VOL_RIGHT + 0x20:
   case APU_VOL_RIGHT + 0x30:
   case APU_VOL_RIGHT + 0x40:
   case APU_VOL_RIGHT + 0x50:
   case APU_VOL_RIGHT + 0x60:
   case APU_VOL_RIGHT + 0x70:
      S9xSetSoundVolume(reg >> 4, (int8_t) APU.DSP [reg - 1], (int8_t) byte);
      break;
   case APU_P_LOW + 0x00:
   case APU_P_LOW + 0x10:
   case APU_P_LOW + 0x20:
   case APU_P_LOW + 0x30:
   case APU_P_LOW + 0x40:
   case APU_P_LOW + 0x50:
   case APU_P_LOW + 0x60:
   case APU_P_LOW + 0x70:
      S9xSetSoundHertz(reg >> 4, (((int16_t) byte + ((int16_t) APU.DSP [reg + 1] << 8)) & FREQUENCY_MASK) * 8);
      break;
   case APU_P_HIGH + 0x00:
   case APU_P_HIGH + 0x10:
   case APU_P_HIGH + 0x20:
   case APU_P_HIGH + 0x30:
   case APU_P_HIGH + 0x40:
   case APU_P_HIGH + 0x50:
   case APU_P_HIGH + 0x60:
   case APU_P_HIGH + 0x70:
      S9xSetSoundHertz(reg >> 4, ((((int16_t) byte << 8) + (int16_t) APU.DSP [reg - 1]) & FREQUENCY_MASK) * 8);
      break;
   case APU_ADSR1 + 0x00:
   case APU_ADSR1 + 0x10:
   case APU_ADSR1 + 0x20:
   case APU_ADSR1 + 0x30:
   case APU_ADSR1 + 0x40:
   case APU_ADSR1 + 0x50:
   case APU_ADSR1 + 0x60:
   case APU_ADSR1 + 0x70:
      if(byte != APU.DSP [reg])
         S9xFixEnvelope(reg >> 4, APU.DSP [reg + 2], byte, APU.DSP [reg + 1]);
      break;
   case APU_ADSR2 + 0x00:
   case APU_ADSR2 + 0x10:
   case APU_ADSR2 + 0x20:
   case APU_ADSR2 + 0x30:
   case APU_ADSR2 + 0x40:
   case APU_ADSR2 + 0x50:
   case APU_ADSR2 + 0x60:
   case APU_ADSR2 + 0x70:
      if(byte != APU.DSP [reg])
         S9xFixEnvelope(reg >> 4, APU.DSP [reg + 1], APU.DSP [reg - 1], byte);
      break;
   case APU_GAIN + 0x00:
   case APU_GAIN + 0x10:
   case APU_GAIN + 0x20:
   case APU_GAIN + 0x30:
   case APU_GAIN + 0x40:
   case APU_GAIN + 0x50:
   case APU_GAIN + 0x60:
   case APU_GAIN + 0x70:
      if(byte != APU.DSP [reg])
         S9xFixEnvelope(reg >> 4, byte, APU.DSP [reg - 2], APU.DSP [reg - 1]);
      break;
   case APU_PMON:
      if(byte != APU.DSP [APU_PMON])
         S9xSetFrequencyModulationEnable(byte);
      break;
   case APU_EON:
      if(byte != APU.DSP [APU_EON])
         S9xSetEchoEnable(byte);
      break;
   case APU_EFB:
      S9xSetEchoFeedback((int8_t) byte);
      break;
   case APU_EDL:
      S9xSetEchoDelay(byte & 0xf);
      break;
   case APU_C0:
   case APU_C1:
   case APU_C2:
   case APU_C3:
   case APU_C4:
   case APU_C5:
   case APU_C6:
   case APU_C7:
      S9xSetFilterCoefficient(reg >> 4, (int8_t) byte);
      break;
   default:
      break;
   }

   KeyOnPrev |= KeyOn;
   KeyOn = 0;

   if (reg < 0x80)
      APU.DSP [reg] = byte;
}

void S9xFixEnvelope(int32_t channel, uint8_t gain, uint8_t adsr1, uint8_t adsr2)
{
   if (adsr1 & 0x80) /* ADSR mode */
   {
      /* XXX: can DSP be switched to ADSR mode directly from GAIN/INCREASE/
       * DECREASE mode? And if so, what stage of the sequence does it start
       * at? */
      if(S9xSetSoundMode(channel, MODE_ADSR))
         S9xSetSoundADSR(channel, adsr1 & 0xf, (adsr1 >> 4) & 7, adsr2 & 0x1f, (adsr2 >> 5) & 7, 8);
   } /* Gain mode */
   else if (!(gain & 0x80))
   {
      if (S9xSetSoundMode(channel, MODE_GAIN))
      {
         S9xSetEnvelopeRate(channel, 0, 0, gain & 0x7f, 0);
         S9xSetEnvelopeHeight(channel, gain & 0x7f);
      }
   }
   else if (gain & 0x40)
   {
      /* Increase mode */
      if(S9xSetSoundMode(channel, (gain & 0x20) ? MODE_INCREASE_BENT_LINE : MODE_INCREASE_LINEAR))
         S9xSetEnvelopeRate(channel, gain, 1, 127, (3 << 28) | gain);
   }
   else if (gain & 0x20)
   {
      if(S9xSetSoundMode(channel, MODE_DECREASE_EXPONENTIAL))
         S9xSetEnvelopeRate(channel, gain, -1, 0, (4 << 28) | gain);
   }
   else
   {
      if (S9xSetSoundMode(channel, MODE_DECREASE_LINEAR))
         S9xSetEnvelopeRate(channel, gain, -1, 0, (3 << 28) | gain);
   }
}

void S9xSetAPUControl(uint8_t byte)
{
   if ((byte & 1) && !APU.TimerEnabled [0])
   {
      APU.Timer [0] = 0;
      IAPU.RAM [0xfd] = 0;
      if ((APU.TimerTarget [0] = IAPU.RAM [0xfa]) == 0)
         APU.TimerTarget [0] = 0x100;
   }
   if ((byte & 2) && !APU.TimerEnabled [1])
   {
      APU.Timer [1] = 0;
      IAPU.RAM [0xfe] = 0;
      if ((APU.TimerTarget [1] = IAPU.RAM [0xfb]) == 0)
         APU.TimerTarget [1] = 0x100;
   }
   if ((byte & 4) && !APU.TimerEnabled [2])
   {
      APU.Timer [2] = 0;
      IAPU.RAM [0xff] = 0;
      if ((APU.TimerTarget [2] = IAPU.RAM [0xfc]) == 0)
         APU.TimerTarget [2] = 0x100;
   }
   APU.TimerEnabled [0] = !!(byte & 1);
   APU.TimerEnabled [1] = !!(byte & 2);
   APU.TimerEnabled [2] = !!(byte & 4);

   if (byte & 0x10)
      IAPU.RAM [0xF4] = IAPU.RAM [0xF5] = 0;

   if (byte & 0x20)
      IAPU.RAM [0xF6] = IAPU.RAM [0xF7] = 0;

   if (byte & 0x80)
   {
      if (!APU.ShowROM)
      {
         /* memmove converted: Different mallocs [Neb]
          * DS2 DMA notes: The APU ROM is not 32-byte aligned [Neb] */
         memcpy(&IAPU.RAM [0xffc0], APUROM, sizeof(APUROM));
         APU.ShowROM = true;
      }
   }
   else if (APU.ShowROM)
   {
      APU.ShowROM = false;
      /* memmove converted: Different mallocs [Neb]
       * DS2 DMA notes: The APU ROM is not 32-byte aligned [Neb] */
      memcpy(&IAPU.RAM [0xffc0], APU.ExtraRAM, sizeof(APUROM));
   }
   IAPU.RAM [0xf1] = byte;
}

uint8_t S9xGetAPUDSP()
{
   uint8_t reg = IAPU.RAM [0xf2] & 0x7f;
   uint8_t byte = APU.DSP [reg];

   switch (reg)
   {
   case APU_OUTX + 0x00:
   case APU_OUTX + 0x10:
   case APU_OUTX + 0x20:
   case APU_OUTX + 0x30:
   case APU_OUTX + 0x40:
   case APU_OUTX + 0x50:
   case APU_OUTX + 0x60:
   case APU_OUTX + 0x70:
      if(SoundData.channels [reg >> 4].state == SOUND_SILENT)
         return 0;
      return (SoundData.channels [reg >> 4].sample >> 8) | (SoundData.channels [reg >> 4].sample & 0xff);
   case APU_ENVX + 0x00:
   case APU_ENVX + 0x10:
   case APU_ENVX + 0x20:
   case APU_ENVX + 0x30:
   case APU_ENVX + 0x40:
   case APU_ENVX + 0x50:
   case APU_ENVX + 0x60:
   case APU_ENVX + 0x70:
   {
      int32_t eVal = SoundData.channels [reg >> 4].envx;
      return (eVal > 0x7F) ? 0x7F : (eVal < 0 ? 0 : eVal);
   }
   default:
      break;
   }
   return byte;
}

#endif
