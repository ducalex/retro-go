/*
 * A reimplementation of Jim Dose's FX_MAN routines, using  SDL_mixer 1.2.
 *   Whee. FX_MAN is also known as the "Apogee Sound System", or "ASS" for
 *   short. How strangely appropriate that seems.
 *
 * Written by Ryan C. Gordon. (icculus@clutteredmind.org)
 */

#include "rt_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#if (defined __WATCOMC__)
#include "dukesnd_watcom.h"
#endif

#if (!defined __WATCOMC__)
#define cdecl
#endif

#include "SDL.h"
#include "SDL_mixer.h"
#include "rt_cfg.h"      // ROTT music hack
#include "rt_util.h"     // ROTT music hack
#include "fx_man.h"
#include "music.h"

#define __FX_TRUE  (1 == 1)
#define __FX_FALSE (!__FX_TRUE)

#define DUKESND_DEBUG       "DUKESND_DEBUG"

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

typedef struct __DUKECHANINFO
{
    int in_use;                 // 1 or 0.
    int priority;               // priority, defined by application.
    Uint32 birthday;            // ticks when channel was grabbed.
    unsigned long callbackval;  // callback value from application.
} duke_channel_info;


static char warningMessage[80];
static char errorMessage[80];
static int fx_initialized = 0;
static int numChannels = MIX_CHANNELS;
static void (*callback)(unsigned long);
static int reverseStereo = 0;
static int reverbDelay = 256;
static int reverbLevel = 0;
static int fastReverb = 0;
static FILE *debug_file = NULL;
static int initialized_debugging = 0;
static int maxReverbDelay = 256;
static int mixerIsStereo = 1;
static duke_channel_info *chaninfo = NULL;

#define HandleOffset       1

/* these come from the real ASS */
#define MV_MaxPanPosition  31
#define MV_NumPanPositions ( MV_MaxPanPosition + 1 )
#define MV_MaxVolume       63

#define MIX_VOLUME( volume ) \
   ( ( max( 0, min( ( volume ), 255 ) ) * ( MV_MaxVolume + 1 ) ) >> 8 )
   
typedef struct
{
    unsigned char left;
    unsigned char right;
} Pan;
    
static Pan MV_PanTable[ MV_NumPanPositions ][ MV_MaxVolume + 1 ];

static void MV_CalcPanTable
   (
   void
   )

   {
   int   level;
   int   angle;
   int   distance;
   int   HalfAngle;
   int   ramp;

   HalfAngle = ( MV_NumPanPositions / 2 );

   for( distance = 0; distance <= MV_MaxVolume; distance++ )
      {
      level = ( 255 * ( MV_MaxVolume - distance ) ) / MV_MaxVolume;
      for( angle = 0; angle <= HalfAngle / 2; angle++ )
         {
         ramp = level - ( ( level * angle ) /
            ( MV_NumPanPositions / 4 ) );

         MV_PanTable[ angle ][ distance ].left = ramp;
         MV_PanTable[ HalfAngle - angle ][ distance ].left = ramp;
         MV_PanTable[ HalfAngle + angle ][ distance ].left = level;
         MV_PanTable[ MV_MaxPanPosition - angle ][ distance ].left = level;

         MV_PanTable[ angle ][ distance ].right = level;
         MV_PanTable[ HalfAngle - angle ][ distance ].right = level;
         MV_PanTable[ HalfAngle + angle ][ distance ].right = ramp;
         MV_PanTable[ MV_MaxPanPosition - angle ][ distance ].right = ramp;
         }
      }
   }
/* end ASS copy-pastage */

#ifdef __WATCOMC__
#pragma aux (__cdecl) channelDoneCallback;
#endif

// This function is called whenever an SDL_mixer channel completes playback.
//  We use this for state management and calling the application's callback.
static void channelDoneCallback(int channel)
{
    Mix_FreeChunk(Mix_GetChunk(channel));
    if (callback)
    {
        callback(chaninfo[channel].callbackval);
        chaninfo[channel].in_use = 0;
    } // if
} // channelDoneCallback


// This gets called all over the place for information and debugging messages.
//  If the user set the DUKESND_DEBUG environment variable, the messages
//  go to the file that is specified in that variable. Otherwise, they
//  are ignored for the expense of the function call. If DUKESND_DEBUG is
//  set to "-" (without the quotes), then the output goes to stdout.
static void snddebug(const char *fmt, ...) __attribute__((format(printf,1,2)));
static void snddebug(const char *fmt, ...)
{
    va_list ap;

    if (debug_file)
    {
        fprintf(debug_file, "DUKESND: ");
        va_start(ap, fmt);
        vfprintf(debug_file, fmt, ap);
        va_end(ap);
        fprintf(debug_file, "\n");
        fflush(debug_file);
    } // if
} // snddebug


// FIXME: Consolidate this code.
// Same as snddebug(), but a different tag is put on each line.
static void musdebug(const char *fmt, ...) __attribute__((format(printf,1,2)));
static void musdebug(const char *fmt, ...)
{
    va_list ap;

    if (debug_file)
    {
        fprintf(debug_file, "DUKEMUS: ");
        va_start(ap, fmt);
        vfprintf(debug_file, fmt, ap);
        va_end(ap);
        fprintf(debug_file, "\n");
        fflush(debug_file);
    } // if
} // snddebug


static void init_debugging(void)
{
    const char *envr;

    if (initialized_debugging)
        return;

    envr = getenv(DUKESND_DEBUG);
    if (envr != NULL)
    {
        if (strcmp(envr, "-") == 0)
            debug_file = stdout;
        else
            debug_file = fopen(envr, "w");

        if (debug_file == NULL)
            fprintf(stderr, "DUKESND: -WARNING- Could not open debug file!\n");
        else
            setbuf(debug_file, NULL);
    } // if

    initialized_debugging = 1;
} // init_debugging


// find an available SDL_mixer channel, and reserve it.
//  This would be a race condition, but hey, it's for a DOS game.  :)
static int grabMixerChannel(int priority)
{
    int replaceable = -1;
    int i;

    for (i = 0; i < numChannels; i++)
    {
        if (chaninfo[i].in_use == 0)
        {
            chaninfo[i].in_use = 1;
            chaninfo[i].priority = priority;
            chaninfo[i].birthday = SDL_GetTicks();
            return(i);
        } // if

        // !!! FIXME: Should this just be lower priority, or equal too?
        if (chaninfo[i].priority <= priority)
        {
            if ((replaceable == -1) ||
                (chaninfo[i].birthday < chaninfo[replaceable].birthday))
            {
                replaceable = i;
            } // if
        } // if
    } // for

    // if you land here, all mixer channels are playing...
    if (replaceable != -1)  // nothing expendable right now.
    {
        chaninfo[replaceable].in_use = 1;
        chaninfo[replaceable].priority = priority;
        chaninfo[replaceable].birthday = SDL_GetTicks();
    } // if

    return(replaceable);
} // grabMixerChannel


// !!! FIXME: Is this correct behaviour?
char *FX_ErrorString( int ErrorNumber )
{
    switch (ErrorNumber)
    {
        case FX_Warning:
            return(warningMessage);

        case FX_Error:
            return(errorMessage);

        case FX_Ok:
            return("OK; no error.");

        case FX_ASSVersion:
            return("Incorrect sound library version.");

        case FX_BlasterError:
            return("SoundBlaster Error.");

        case FX_SoundCardError:
            return("General sound card error.");

        case FX_InvalidCard:
            return("Invalid sound card.");

        case FX_MultiVocError:
            return("Multiple voc error.");

        case FX_DPMI_Error:
            return("DPMI error.");

        default:
            return("Unknown error.");
    } // switch

    assert(0);    // shouldn't hit this point.
    return(NULL);
} // FX_ErrorString


static void setWarningMessage(const char *msg)
{
    strncpy(warningMessage, msg, sizeof (warningMessage));
    // strncpy() doesn't add the null char if there isn't room...
    warningMessage[sizeof (warningMessage) - 1] = '\0';
    snddebug("Warning message set to [%s].", warningMessage);
} // setErrorMessage


static void setErrorMessage(const char *msg)
{
    strncpy(errorMessage, msg, sizeof (errorMessage));
    // strncpy() doesn't add the null char if there isn't room...
    errorMessage[sizeof (errorMessage) - 1] = '\0';
    snddebug("Error message set to [%s].", errorMessage);
} // setErrorMessage

int FX_SetupCard( int SoundCard, fx_device *device )
{
    init_debugging();

    snddebug("FX_SetupCard looking at card id #%d...", SoundCard);

    if (device == NULL)  // sanity check.
    {
        setErrorMessage("fx_device is NULL in FX_SetupCard!");
        return(FX_Error);
    } // if

        // Since the actual hardware is abstracted out on modern operating
        //  systems, we just pretend that the system's got a SoundScape.
        //  I always liked that card, even though Ensoniq screwed me on OS/2
        //  drivers back in the day.  :)
    if (SoundCard != SoundScape)
    {
        setErrorMessage("Card not found.");
        snddebug("We pretend to be an Ensoniq SoundScape only.");
        return(FX_Error);
    } // if

    device->MaxVoices = 8;
    device->MaxSampleBits = 16;       // SDL_mixer downsamples if needed.
    device->MaxChannels = 2;          // SDL_mixer converts to mono if needed.

    return(FX_Ok);
} // FX_SetupCard


static void output_versions(const char *libname, const SDL_version *compiled,
							const SDL_version *linked)
{
    snddebug("This program was compiled against %s %d.%d.%d,\n"
             " and is dynamically linked to %d.%d.%d.\n", libname,
             compiled->major, compiled->minor, compiled->patch,
             linked->major, linked->minor, linked->patch);
}


static void output_version_info(void)
{
    SDL_version compiled;
    const SDL_version *linked;

    snddebug("Library check...");

    SDL_VERSION(&compiled);
    linked = SDL_Linked_Version();
    output_versions("SDL", &compiled, linked);

    MIX_VERSION(&compiled);
    linked = Mix_Linked_Version();
    output_versions("SDL_mixer", &compiled, linked);
} // output_version_info


int FX_Init(int SoundCard, int numvoices, int numchannels,
            int samplebits, unsigned mixrate)
{
    Uint16 audio_format = 0;
    int blocksize;

    init_debugging();

    snddebug("INIT! card=>%d, voices=>%d, chan=>%d, bits=>%d, rate=>%du...",
                SoundCard, numvoices, numchannels, samplebits, mixrate);

    if (fx_initialized)
    {
        setErrorMessage("Sound system is already initialized.");
        return(FX_Error);
    } // if

    if (SoundCard != SoundScape) // We pretend there's a SoundScape installed.
    {
        setErrorMessage("Card not found.");
        snddebug("We pretend to be an Ensoniq SoundScape only.");
        return(FX_Error);
    } // if

        // other sanity checks...
    if ((numvoices < 0) || (numvoices > 8))
    {
        setErrorMessage("Invalid number of voices to mix (must be 0-8).");
        return(FX_Error);
    } // if

    if ((numchannels != MonoFx) && (numchannels != StereoFx))
    {
        setErrorMessage("Invalid number of channels (must be 1 or 2).");
        return(FX_Error);
    } // if

    if ((samplebits != 8) && (samplebits != 16))
    {
        setErrorMessage("Invalid sample size (must be 8 or 16).");
        return(FX_Error);
    } // if

    // build pan tables
    MV_CalcPanTable();
    
    SDL_ClearError();
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        setErrorMessage("SDL_Init(SDL_INIT_AUDIO) failed!");
        snddebug("SDL_GetError() reports: [%s].", SDL_GetError());
        return(FX_Error);
    } // if

    audio_format = (samplebits == 8) ? AUDIO_U8 : AUDIO_S16SYS;
    if (Mix_OpenAudio(mixrate, audio_format, numchannels, 256) < 0)
    {
        setErrorMessage(SDL_GetError());
        return(FX_Error);
    } // if

    output_version_info();

    numChannels = Mix_AllocateChannels(numvoices);
    if (numChannels != numvoices)
    {
        setErrorMessage(SDL_GetError());
        Mix_CloseAudio();
        return(FX_Error);
    } // if

    blocksize = sizeof (duke_channel_info) * numvoices;
    chaninfo = malloc(blocksize);
    if (chaninfo == NULL)  // uhoh.
    {
        setErrorMessage("Out of memory");
        Mix_CloseAudio();
        return(FX_Error);
    } // if
    memset(chaninfo, '\0', blocksize);

    Mix_ChannelFinished(channelDoneCallback);
    maxReverbDelay = (int) (((float) mixrate) * 0.5);

    Mix_QuerySpec(NULL, NULL, &mixerIsStereo);
    mixerIsStereo = (mixerIsStereo == 2);

    fx_initialized = 1;
    return(FX_Ok);
} // FX_Init


int FX_Shutdown( void )
{
    snddebug("shutting down sound subsystem.");

    if (!fx_initialized)
    {
        setErrorMessage("Sound system is not currently initialized.");
        return(FX_Error);
    } // if

    Mix_CloseAudio();
    callback = NULL;
    free(chaninfo);
    chaninfo = NULL;
    reverseStereo = 0;
    reverbLevel = 0;
    fastReverb = 0;
    fx_initialized = 0;
    maxReverbDelay = 256;
    return(FX_Ok);
} // FX_Shutdown


int FX_SetCallBack(void (*func)(unsigned long))
{
    callback = func;
    return(FX_Ok);
} // FX_SetCallBack


void FX_SetVolume(int volume)
{
    snddebug("setting master volume to %f.2 percent.", (volume / 255.0) * 100);
    Mix_Volume(-1, volume >> 1);  // it's 0-128 in SDL_mixer, not 0-255.
} // FX_SetVolume


int FX_GetVolume(void)
{
    return(Mix_Volume(-1, -1) << 1);
} // FX_GetVolume


void FX_SetReverseStereo(int setting)
{
    snddebug("Reverse stereo set to %s.\n", setting ? "ON" : "OFF");
    Mix_SetReverseStereo(MIX_CHANNEL_POST, setting);
    reverseStereo = setting;
} // FX_SetReverseStereo


int FX_GetReverseStereo(void)
{
    return(reverseStereo);
} // FX_GetReverseStereo


void FX_SetReverb(int reverb)
{
    reverbLevel = reverb;
    fastReverb = 0;

#if 1
    // !!! FIXME
    if (reverbLevel)
        setWarningMessage("reverb filter is not yet implemented!");
#endif
} // FX_SetReverb


void FX_SetFastReverb(int reverb)
{
    reverbLevel = reverb;
    fastReverb = 1;

#if 1
    // !!! FIXME
    if (reverbLevel)
        setWarningMessage("fast reverb filter is not yet implemented!");
#endif
} // FX_SetFastReverb


int FX_GetMaxReverbDelay(void)
{
    return(maxReverbDelay);
} // FX_GetMaxReverbDelay


int FX_GetReverbDelay(void)
{
    return(reverbDelay);
} // FX_GetReverbDelay


void FX_SetReverbDelay(int delay)
{
        // !!! FIXME: Should I be clamping these values?
    if (delay < 256)
        delay = 256;

    if (delay > maxReverbDelay)
        delay = maxReverbDelay;

    reverbDelay = delay;

#if 1
    // !!! FIXME
    setWarningMessage("reverb delay is not yet implemented!");
#endif
} // FX_SetReverbDelay


int FX_VoiceAvailable(int priority)
{
    int chan = grabMixerChannel(priority);
    int rc = (chan != -1);

    if (rc)
        chaninfo[chan].in_use = 0;

    return(rc);
} // FX_VoiceAvailable


static int doSetPan(int handle, int vol, int left,
                    int right, int checkIfPlaying)
{
    int retval = FX_Warning;

    if ((handle < 0) || (handle >= numChannels))
        setWarningMessage("Invalid handle in doSetPan().");
    else if ((checkIfPlaying) && (!Mix_Playing(handle)))
        setWarningMessage("voice is no longer playing in doSetPan().");
    else
    {
        if (mixerIsStereo)
        {
            if ((left < 0) || (left > 255) ||
                (right < 0) || (right > 255))
            {
                setErrorMessage("Invalid argument to FX_SetPan().");
                retval = FX_Error;
            } // if

            else
            {
                Mix_SetPanning(handle, left, right);
            } // else
        } // if
        else
        {
            if ((vol < 0) || (vol > 255))
            {
                setErrorMessage("Invalid argument to FX_SetPan().");
                retval = FX_Error;
            } // if
            else
            {
                // volume must be from 0-128, so the ">> 1" converts.
                Mix_Volume(handle, vol >> 1);
            } // else
        } // else

        retval = FX_Ok;
    } // else

    return(retval);
} // doSetPan


int FX_SetPan(int handle, int vol, int left, int right)
{
    return(doSetPan(handle - HandleOffset, vol, left, right, 1));
} // FX_SetPan


int FX_SetPitch(int handle, int pitchoffset)
{
    snddebug("FX_SetPitch() ... NOT IMPLEMENTED YET!");
    return(FX_Ok);
} // FX_SetPitch


int FX_SetFrequency(int handle, int frequency)
{
    snddebug("FX_SetFrequency() ... NOT IMPLEMENTED YET!");
    return(FX_Ok);
} // FX_SetFrequency



// If this returns FX_Ok, then chunk and chan will be filled with the
//  the block of audio data in the format desired by the audio device
//  and the SDL_mixer channel it will play on, respectively.
//  If the value is not FX_Ok, then the warning or error message is set,
//  and you should bail.
// size added by SBF for ROTT
static int setupVocPlayback(char *ptr, int size, int priority, unsigned long callbackval,
                            int *chan, Mix_Chunk **chunk)
{
    SDL_RWops *rw;

    *chan = grabMixerChannel(priority);
    if (*chan == -1)
    {
        setErrorMessage("No available channels");
        return(FX_Error);
    } // if

    if (size == -1) {
        // !!! FIXME: This could be a problem...SDL/SDL_mixer wants a RWops, which
        // !!! FIXME:  is an i/o abstraction. Since we already have the VOC data
        // !!! FIXME:  in memory, we fake it with a memory-based RWops. None of
        // !!! FIXME:  this is a problem, except the RWops wants to know how big
        // !!! FIXME:  its memory block is (so it can do things like seek on an
        // !!! FIXME:  offset from the end of the block), and since we don't have
        // !!! FIXME:  this information, we have to give it SOMETHING. My VOC
        // !!! FIXME:  decoder never does seeks from EOF, nor checks for
        // !!! FIXME:  end-of-file, so we should be fine. However, we've got a
        // !!! FIXME:  limit of 10 megs for one file. I hope that'll cover it. :)

        rw = SDL_RWFromMem((void *) ptr, (10 * 1024) * 1024);  /* yikes. */
    } else {
        // A valid file size! Excellent.
        rw = SDL_RWFromMem((void *) ptr, size);
    }
    
    *chunk = Mix_LoadWAV_RW(rw, 1);
    if (*chunk == NULL)
    {
        setErrorMessage("Couldn't decode voice sample.");
        chaninfo[*chan].in_use = 0;
        return(FX_Error);
    } // if

    chaninfo[*chan].callbackval = callbackval;
    return(FX_Ok);
} // setupVocPlayback

static int _FX_SetPosition(int chan, int angle, int distance)
{
    int left;
    int right;
    int mid;
    int volume;
    int status;
 
    if ( distance < 0 ) {
        distance  = -distance;
        angle    += MV_NumPanPositions / 2;
    }
    
    volume = MIX_VOLUME( distance );
    
    // Ensure angle is within 0 - 31
    angle &= MV_MaxPanPosition;
    
    left  = MV_PanTable[ angle ][ volume ].left;
    right = MV_PanTable[ angle ][ volume ].right;
    mid   = max( 0, 255 - distance );

    status = doSetPan( chan, mid, left, right, 0 );
    
    return status;
}

int FX_PlayVOC(char *ptr, int pitchoffset,
               int vol, int left, int right,
               int priority, unsigned long callbackval)
{
    int rc;
    int chan;
    Mix_Chunk *chunk;

    snddebug("Playing voice: mono (%d), left (%d), right (%d), priority (%d).\n",
                vol, left, right, priority);

    rc = setupVocPlayback(ptr, -1, priority, callbackval, &chan, &chunk);
    if (rc != FX_Ok)
        return(rc);

    // !!! FIXME: Need to do something with pitchoffset.

    rc = doSetPan(chan, vol, left, right, 0);
    if (rc != FX_Ok)
    {
        chaninfo[chan].in_use = 0;
        return(rc);
    } // if

    Mix_PlayChannel(chan, chunk, 0);
    return(HandleOffset + chan);
} // FX_PlayVOC


// get the size of a single sample, in bytes.
static int getSampleSize(void)
{
    Uint16 format;
    int channels;

    Mix_QuerySpec(NULL, &format, &channels);
    return( ((format & 0xFF) / 8) * channels );
} // getSampleSize


int FX_PlayLoopedVOC(char *ptr, long loopstart, long loopend,
       int pitchoffset, int vol, int left, int right, int priority,
       unsigned long callbackval)
{
    int rc;
    int chan;
    int samplesize = getSampleSize();
    Uint32 totalsamples;
    Mix_Chunk *chunk;

    snddebug("Playing voice: mono (%d), left (%d), right (%d), priority (%d).\n",
                vol, left, right, priority);
    snddebug("Looping: start (%ld), end (%ld).\n", loopstart, loopend);

    rc = setupVocPlayback(ptr, -1, priority, callbackval, &chan, &chunk);
    if (rc != FX_Ok)
        return(rc);

    // !!! FIXME: Need to do something with pitchoffset.

    totalsamples = chunk->alen / samplesize;

    if ((loopstart >= 0) && ((unsigned int)loopstart < totalsamples))
    {
        if (loopend < 0) loopend = 0;
        if ((unsigned int)loopend > totalsamples) loopend = totalsamples;

        if (loopend < loopstart)
        {
            Mix_FreeChunk(chunk);
            chaninfo[chan].in_use = 0;
            setErrorMessage("Loop end is before loop start.");
            return(FX_Error);
        } // if

        chunk->alen = loopend * samplesize;

        if (loopstart > 0)
        {
            loopstart *= samplesize;
            memcpy(chunk->abuf, ((Uint8 *) chunk->abuf) + loopstart,
                    chunk->alen - loopstart);
            chunk->alen -= loopstart;
        } // if
    } // if

    Mix_PlayChannel(chan, chunk, -1);  /* -1 == looping. */
    return(HandleOffset + chan);
} // FX_PlayLoopedVOC

int FX_PlayVOC3D(char *ptr, int pitchoffset, int angle, int distance,
       int priority, unsigned long callbackval)
{
    int rc;
    int chan;
    Mix_Chunk *chunk;

    snddebug("Playing voice at angle (%d), distance (%d), priority (%d).\n",
                angle, distance, priority);

    rc = setupVocPlayback(ptr, -1, priority, callbackval, &chan, &chunk);
    if (rc != FX_Ok)
        return(rc);

    // !!! FIXME: Need to do something with pitchoffset.
    
    _FX_SetPosition(chan, angle, distance);

    Mix_PlayChannel(chan, chunk, 0);
    return(HandleOffset + chan);
} // FX_PlayVOC3D

// ROTT Special - SBF
int FX_PlayVOC3D_ROTT(char *ptr, int size, int pitchoffset, int angle, int distance,
       int priority, unsigned long callbackval)
{
    int rc;
    int chan;
    Mix_Chunk *chunk;

    snddebug("Playing voice at angle (%d), distance (%d), priority (%d).\n",
                angle, distance, priority);

    rc = setupVocPlayback(ptr, size, priority, callbackval, &chan, &chunk);
    if (rc != FX_Ok)
        return(rc);

    // !!! FIXME: Need to do something with pitchoffset.

    _FX_SetPosition(chan, angle, distance);

    Mix_PlayChannel(chan, chunk, 0);

    return(HandleOffset + chan);
} // FX_PlayVOC3D_ROTT


    // it's all the same to SDL_mixer.  :)
int FX_PlayWAV( char *ptr, int pitchoffset, int vol, int left, int right,
       int priority, unsigned long callbackval )
{
    return(FX_PlayVOC(ptr, pitchoffset, vol, left, right, priority, callbackval));
} // FX_PlayWAV


int FX_PlayLoopedWAV( char *ptr, long loopstart, long loopend,
       int pitchoffset, int vol, int left, int right, int priority,
       unsigned long callbackval )
{
    return(FX_PlayLoopedVOC(ptr, loopstart, loopend, pitchoffset, vol, left,
                             right, priority, callbackval));
} // FX_PlayLoopedWAV


int FX_PlayWAV3D( char *ptr, int pitchoffset, int angle, int distance,
       int priority, unsigned long callbackval )
{
    return(FX_PlayVOC3D(ptr, pitchoffset, angle, distance, priority, callbackval));
} // FX_PlayWAV3D

// ROTT Special - SBF
int FX_PlayWAV3D_ROTT( char *ptr, int size, int pitchoffset, int angle, int distance,
       int priority, unsigned long callbackval )
{
    return(FX_PlayVOC3D_ROTT(ptr, size, pitchoffset, angle, distance, priority, callbackval));
} // FX_PlayWAV3D_ROTT


int FX_PlayRaw( char *ptr, unsigned long length, unsigned rate,
       int pitchoffset, int vol, int left, int right, int priority,
       unsigned long callbackval )
{
    setErrorMessage("FX_PlayRaw() ... NOT IMPLEMENTED!");
    return(FX_Error);
} // FX_PlayRaw


int FX_PlayLoopedRaw( char *ptr, unsigned long length, char *loopstart,
       char *loopend, unsigned rate, int pitchoffset, int vol, int left,
       int right, int priority, unsigned long callbackval )
{
    setErrorMessage("FX_PlayLoopedRaw() ... NOT IMPLEMENTED!");
    return(FX_Error);
} // FX_PlayLoopedRaw


int FX_Pan3D(int handle, int angle, int distance)
{
    int retval = FX_Warning;

    handle -= HandleOffset;

    if ((handle < 0) || (handle >= numChannels))
        setWarningMessage("Invalid handle in FX_Pan3D().");
    else if (!Mix_Playing(handle))
        setWarningMessage("voice is no longer playing in FX_Pan3D().");
    else
    {
        _FX_SetPosition(handle, angle, distance);
        
        retval = FX_Ok;
    } // else

    return(retval);
} // FX_Pan3D


int FX_SoundActive(int handle)
{
    handle -= HandleOffset;

    if (chaninfo == NULL)
        return(__FX_FALSE);

    if ((handle < 0) || (handle >= numChannels))
    {
        setWarningMessage("Invalid handle in FX_SoundActive().");
        return(__FX_FALSE);
    } // if

    return(chaninfo[handle].in_use != 0);
} // FX_SoundActive


int FX_SoundsPlaying(void)
{
    return(Mix_Playing(-1));
} // FX_SoundsPlaying


int FX_StopSound(int handle)
{
    int retval = FX_Ok;

    snddebug("explicitly halting channel (%d).", handle);
        // !!! FIXME: Should the user callback fire for this?

    handle -= HandleOffset;

    if ((handle < 0) || (handle >= numChannels))
    {
        setWarningMessage("Invalid handle in FX_Pan3D().");
        retval = FX_Warning;
    } // if
    else
    {
        Mix_HaltChannel(handle);
    } // else

    return(retval);
} // FX_StopSound


int FX_StopAllSounds(void)
{
    snddebug("halting all channels.");
        // !!! FIXME: Should the user callback fire for this?
    Mix_HaltGroup(-1);
    return(FX_Ok);
} // FX_StopAllSounds


int FX_StartDemandFeedPlayback( void ( *function )( char **ptr, unsigned long *length ),
       int rate, int pitchoffset, int vol, int left, int right,
       int priority, unsigned long callbackval )
{
    setErrorMessage("FX_StartDemandFeedPlayback() ... NOT IMPLEMENTED!");
    return(FX_Error);
}


int FX_StartRecording(int MixRate, void (*function)(char *ptr, int length))
{
    setErrorMessage("FX_StartRecording() ... NOT IMPLEMENTED!");
    return(FX_Error);
} // FX_StartRecording


void FX_StopRecord( void )
{
    setErrorMessage("FX_StopRecord() ... NOT IMPLEMENTED!");
} // FX_StopRecord



// The music functions...


char *MUSIC_ErrorString(int ErrorNumber)
{
    switch (ErrorNumber)
    {
        case MUSIC_Warning:
            return(warningMessage);

        case MUSIC_Error:
            return(errorMessage);

        case MUSIC_Ok:
            return("OK; no error.");

        case MUSIC_ASSVersion:
            return("Incorrect sound library version.");

        case MUSIC_SoundCardError:
            return("General sound card error.");

        case MUSIC_InvalidCard:
            return("Invalid sound card.");

        case MUSIC_MidiError:
            return("MIDI error.");

        case MUSIC_MPU401Error:
            return("MPU401 error.");

        case MUSIC_TaskManError:
            return("Task Manager error.");

        case MUSIC_FMNotDetected:
            return("FM not detected error.");

        case MUSIC_DPMI_Error:
            return("DPMI error.");

        default:
            return("Unknown error.");
    } // switch

    assert(0);    // shouldn't hit this point.
    return(NULL);
} // MUSIC_ErrorString


static int music_initialized = 0;
static int music_context = 0;
static int music_loopflag = MUSIC_PlayOnce;
static char *music_songdata = NULL;
static Mix_Music *music_musicchunk = NULL;

int MUSIC_Init(int SoundCard, int Address)
{
    init_debugging();

    musdebug("INIT! card=>%d, address=>%d...", SoundCard, Address);

    if (music_initialized)
    {
        setErrorMessage("Music system is already initialized.");
        return(MUSIC_Error);
    } // if
    
    if (SoundCard != SoundScape) // We pretend there's a SoundScape installed.
    {
        setErrorMessage("Card not found.");
        musdebug("We pretend to be an Ensoniq SoundScape only.");
        return(MUSIC_Error);
    } // if

    music_initialized = 1;
    return(MUSIC_Ok);
} // MUSIC_Init


int MUSIC_Shutdown(void)
{
    musdebug("shutting down sound subsystem.");

    if (!music_initialized)
    {
        setErrorMessage("Music system is not currently initialized.");
        return(MUSIC_Error);
    } // if

    MUSIC_StopSong();
    music_context = 0;
    music_initialized = 0;
    music_loopflag = MUSIC_PlayOnce;

    return(MUSIC_Ok);
} // MUSIC_Shutdown


void MUSIC_SetMaxFMMidiChannel(int channel)
{
    musdebug("STUB ... MUSIC_SetMaxFMMidiChannel(%d).\n", channel);
} // MUSIC_SetMaxFMMidiChannel


void MUSIC_SetVolume(int volume)
{
    Mix_VolumeMusic(volume >> 1);  // convert 0-255 to 0-128.
} // MUSIC_SetVolume


void MUSIC_SetMidiChannelVolume(int channel, int volume)
{
    musdebug("STUB ... MUSIC_SetMidiChannelVolume(%d, %d).\n", channel, volume);
} // MUSIC_SetMidiChannelVolume


void MUSIC_ResetMidiChannelVolumes(void)
{
    musdebug("STUB ... MUSIC_ResetMidiChannelVolumes().\n");
} // MUSIC_ResetMidiChannelVolumes


int MUSIC_GetVolume(void)
{
    return(Mix_VolumeMusic(-1) << 1);  // convert 0-128 to 0-255.
} // MUSIC_GetVolume


void MUSIC_SetLoopFlag(int loopflag)
{
    music_loopflag = loopflag;
} // MUSIC_SetLoopFlag


int MUSIC_SongPlaying(void)
{
    return((Mix_PlayingMusic()) ? __FX_TRUE : __FX_FALSE);
} // MUSIC_SongPlaying


void MUSIC_Continue(void)
{
    if (Mix_PausedMusic())
        Mix_ResumeMusic();
    else if (music_songdata)
        MUSIC_PlaySong(music_songdata, MUSIC_PlayOnce);
} // MUSIC_Continue


void MUSIC_Pause(void)
{
    Mix_PauseMusic();
} // MUSIC_Pause


int MUSIC_StopSong(void)
{
    if (!fx_initialized)
    {
        setErrorMessage("Need FX system initialized, too. Sorry.");
        return(MUSIC_Error);
    } // if

    if ( (Mix_PlayingMusic()) || (Mix_PausedMusic()) )
        Mix_HaltMusic();

    if (music_musicchunk)
        Mix_FreeMusic(music_musicchunk);

    music_songdata = NULL;
    music_musicchunk = NULL;
    return(MUSIC_Ok);
} // MUSIC_StopSong


int MUSIC_PlaySong(unsigned char *song, int loopflag)
{
    //SDL_RWops *rw;

    MUSIC_StopSong();

    music_songdata = song;

    // !!! FIXME: This could be a problem...SDL/SDL_mixer wants a RWops, which
    // !!! FIXME:  is an i/o abstraction. Since we already have the MIDI data
    // !!! FIXME:  in memory, we fake it with a memory-based RWops. None of
    // !!! FIXME:  this is a problem, except the RWops wants to know how big
    // !!! FIXME:  its memory block is (so it can do things like seek on an
    // !!! FIXME:  offset from the end of the block), and since we don't have
    // !!! FIXME:  this information, we have to give it SOMETHING.

    /* !!! ARGH! There's no LoadMUS_RW  ?!
    rw = SDL_RWFromMem((void *) song, (10 * 1024) * 1024);  // yikes.
    music_musicchunk = Mix_LoadMUS_RW(rw);
    Mix_PlayMusic(music_musicchunk, (loopflag == MUSIC_PlayOnce) ? 0 : -1);
    */

    return(MUSIC_Ok);
} // MUSIC_PlaySong

// ROTT Special - SBF
int MUSIC_PlaySongROTT(unsigned char *song, int size, int loopflag)
{
    char filename[MAX_PATH];
    int handle;
    
    MUSIC_StopSong();

    // save the file somewhere, so SDL_mixer can load it
    GetPathFromEnvironment(filename, ApogeePath, "tmpsong.mid");
    handle = SafeOpenWrite(filename);
    
    SafeWrite(handle, song, size);
    close(handle);
    
    music_songdata = song;

    // finally, we can load it with SDL_mixer
    music_musicchunk = Mix_LoadMUS(filename);
    if (music_musicchunk == NULL) {
        return MUSIC_Error;
    }
    
    Mix_PlayMusic(music_musicchunk, (loopflag == MUSIC_PlayOnce) ? 0 : -1);

    return(MUSIC_Ok);
} // MUSIC_PlaySongROTT

void MUSIC_SetContext(int context)
{
    musdebug("STUB ... MUSIC_SetContext().\n");
    music_context = context;
} // MUSIC_SetContext


int MUSIC_GetContext(void)
{
    return(music_context);
} // MUSIC_GetContext


void MUSIC_SetSongTick(unsigned long PositionInTicks)
{
    musdebug("STUB ... MUSIC_SetSongTick().\n");
} // MUSIC_SetSongTick


void MUSIC_SetSongTime(unsigned long milliseconds)
{
    musdebug("STUB ... MUSIC_SetSongTime().\n");
}// MUSIC_SetSongTime


void MUSIC_SetSongPosition(int measure, int beat, int tick)
{
    musdebug("STUB ... MUSIC_SetSongPosition().\n");
} // MUSIC_SetSongPosition


void MUSIC_GetSongPosition(songposition *pos)
{
    musdebug("STUB ... MUSIC_GetSongPosition().\n");
} // MUSIC_GetSongPosition


void MUSIC_GetSongLength(songposition *pos)
{
    musdebug("STUB ... MUSIC_GetSongLength().\n");
} // MUSIC_GetSongLength


int MUSIC_FadeVolume(int tovolume, int milliseconds)
{
    Mix_FadeOutMusic(milliseconds);
    return(MUSIC_Ok);
} // MUSIC_FadeVolume


int MUSIC_FadeActive(void)
{
    return((Mix_FadingMusic() == MIX_FADING_OUT) ? __FX_TRUE : __FX_FALSE);
} // MUSIC_FadeActive


void MUSIC_StopFade(void)
{
    musdebug("STUB ... MUSIC_StopFade().\n");
} // MUSIC_StopFade


void MUSIC_RerouteMidiChannel(int channel, int cdecl function( int event, int c1, int c2 ))
{
    musdebug("STUB ... MUSIC_RerouteMidiChannel().\n");
} // MUSIC_RerouteMidiChannel


void MUSIC_RegisterTimbreBank(unsigned char *timbres)
{
    musdebug("STUB ... MUSIC_RegisterTimbreBank().\n");
} // MUSIC_RegisterTimbreBank


// end of fx_man.c ...
