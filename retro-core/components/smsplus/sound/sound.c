/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
 *
 *  additionnal code by Eke-Eke (SMS Plus GX)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   Sound emulation.
 *
 ******************************************************************************/

#include "shared.h"

snd_t snd;
// static int16 **fm_buffer;
static int16 **psg_buffer;
static int lines_per_frame;
static int samples_per_line;


int sound_init(void)
{
  // FM_Context fmbuf;
  SN76489_Context psgbuf;
  int restore_sound = 0;
  int i;

  snd.fm_which = option.fm;
  snd.fps = (sms.display == DISPLAY_NTSC) ? FPS_NTSC : FPS_PAL;
  snd.fm_clock = (sms.display == DISPLAY_NTSC) ? CLOCK_NTSC : CLOCK_PAL;
  snd.psg_clock = (sms.display == DISPLAY_NTSC) ? CLOCK_NTSC : CLOCK_PAL;
  snd.sample_rate = option.sndrate;
  snd.mixer_callback = NULL;

  /* Save register settings */
  if(snd.enabled)
  {
    restore_sound = 1;

    memcpy(&psgbuf, SN76489_GetContextPtr(0), SN76489_GetContextSize());
#if 0
    FM_GetContext(&fmbuf);
#endif
  }

  /* If we are reinitializing, shut down sound emulation */
  if(snd.enabled)
  {
    sound_shutdown();
  }

  /* Disable sound until initialization is complete */
  snd.enabled = 0;

  /* Check if sample rate is invalid */
  if(snd.sample_rate < 8000 || snd.sample_rate > 48000)
  {
      abort();
  }

  /* Assign stream mixing callback if none provided */
  // if(!snd.mixer_callback)
  //   snd.mixer_callback = sound_mixer_callback;

  /* Calculate number of samples generated per frame */
  snd.sample_count = (snd.sample_rate / snd.fps) + 1;
  snd.buffer_size = snd.sample_count * 2;
  MESSAGE_INFO("sample_count=%d fps=%d (actual=%f)\n", snd.sample_count, snd.fps, (float)snd.sample_rate / snd.fps);

  /* Prepare incremental info */
  snd.done_so_far = 0;
  lines_per_frame = (sms.display == DISPLAY_NTSC) ? 262 : 313;
  samples_per_line = snd.sample_count / lines_per_frame;

  /* Allocate emulated sound streams */
  for(i = 0; i < STREAM_MAX; i++)
  {
    snd.stream[i] = calloc(snd.buffer_size, 1);
    if(!snd.stream[i]) abort();
  }

  /* Allocate sound output streams for the generic mixer */
  if (snd.mixer_callback == sound_mixer_callback)
  {
    snd.output[0] = calloc(snd.buffer_size, 1);
    snd.output[1] = calloc(snd.buffer_size, 1);
    if(!snd.output[0] || !snd.output[1]) abort();
  }

  /* Set up buffer pointers */
  // fm_buffer = (int16 **)&snd.stream[STREAM_FM_MO];
  psg_buffer = (int16 **)&snd.stream[STREAM_PSG_L];

  /* Set up SN76489 emulation */
  SN76489_Init(0, snd.psg_clock, snd.sample_rate);
  SN76489_Config(0, MUTE_ALLON, BOOST_OFF /*BOOST_ON*/, VOL_FULL, (sms.console < CONSOLE_SMS) ? FB_SC3000 : FB_SEGAVDP);

#if 0
  /* Set up YM2413 emulation */
  FM_Init();
#endif

  /* Restore YM2413 register settings */
  if(restore_sound)
  {
    memcpy(SN76489_GetContextPtr(0), &psgbuf, SN76489_GetContextSize());
    //FM_SetContext(&fmbuf);
  }

  /* Inform other functions that we can use sound */
  snd.enabled = 1;

  return 1;
}


void sound_shutdown(void)
{
  int i;

  if(!snd.enabled)
    return;

  /* Free emulated sound streams */
  for(i = 0; i < STREAM_MAX; i++)
  {
    if(snd.stream[i])
    {
      free(snd.stream[i]);
      snd.stream[i] = NULL;
    }
  }

  /* Free sound output buffers */
  for(i = 0; i < 2; i++)
  {
    if(snd.output[i])
    {
      free(snd.output[i]);
      snd.output[i] = NULL;
    }
  }

  /* Shut down SN76489 emulation */
  SN76489_Shutdown();

#if 0
  /* Shut down YM2413 emulation */
  FM_Shutdown();
#endif
}


void sound_reset(void)
{
  if(!snd.enabled)
    return;

  /* Reset SN76489 emulator */
  SN76489_Reset(0);

#if 0
  /* Reset YM2413 emulator */
  FM_Reset();
 #endif
}


void sound_update(int line)
{
  int16 *psg[2];
  // int16 *fm[2];

  if(!snd.enabled)
    return;

  /* Finish buffers at end of frame */
  if(line == lines_per_frame - 1)
  {
    psg[0] = psg_buffer[0] + snd.done_so_far;
    psg[1] = psg_buffer[1] + snd.done_so_far;
    // fm[0]  = fm_buffer[0] + snd.done_so_far;
    // fm[1]  = fm_buffer[1] + snd.done_so_far;

    /* Generate SN76489 sample data */
    SN76489_Update(0, psg, snd.sample_count - snd.done_so_far);

#if 0
    /* Generate YM2413 sample data */
    FM_Update(fm, snd.sample_count - snd.done_so_far);
#endif

    /* Mix streams into output buffer */
    if (snd.mixer_callback)
      snd.mixer_callback(snd.stream, snd.output, snd.sample_count);

    /* Reset */
    snd.done_so_far = 0;
  }
  else
  {
    /* Do a tiny bit */
    psg[0] = psg_buffer[0] + snd.done_so_far;
    psg[1] = psg_buffer[1] + snd.done_so_far;
    // fm[0]  = fm_buffer[0] + snd.done_so_far;
    // fm[1]  = fm_buffer[1] + snd.done_so_far;

    /* Generate SN76489 sample data */
    SN76489_Update(0, psg, samples_per_line);

#if 0
    /* Generate YM2413 sample data */
    FM_Update(fm, samples_per_line);
#endif

    /* Sum total */
    snd.done_so_far += samples_per_line;
  }
}

/* Generic FM+PSG stereo mixer callback */
void sound_mixer_callback(int16 **stream, int16 **output, int length)
{
  int i;
  for(i = 0; i < length; i++)
  {
    //int16 temp = (fm_buffer[0][i] + fm_buffer[1][i]) / 2;
    //int16 temp = psg_buffer[1][i];
    output[0][i] = psg_buffer[0][i] * 2.75f;
    output[1][i] = psg_buffer[1][i] * 2.75f;
  }
}


/*--------------------------------------------------------------------------*/
/* Sound chip access handlers                                               */
/*--------------------------------------------------------------------------*/

void psg_stereo_w(int data)
{
  if(!snd.enabled) return;
  SN76489_GGStereoWrite(0, data);
}

void stream_update(int which, int position)
{
}


void psg_write(int data)
{
  if(!snd.enabled) return;
  SN76489_Write(0, data);
}

/*--------------------------------------------------------------------------*/
/* Mark III FM Unit / Master System (J) built-in FM handlers                */
/*--------------------------------------------------------------------------*/

int fmunit_detect_r(void)
{
  return sms.fm_detect;
}

void fmunit_detect_w(int data)
{
  if(!snd.enabled || !sms.use_fm) return;
  sms.fm_detect = data;
}

void fmunit_write(int offset, int data)
{
  if(!snd.enabled || !sms.use_fm) return;
  MESSAGE_ERROR("FM Not supported write %02x at %04x\n", data, offset);
  // FM_Write(offset, data);
}
