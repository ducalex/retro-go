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

#define ROTT

#ifdef DUKE3D
#include "duke3d.h"
#include "buildengine/cache1d.h"
#endif

#if PLATFORM_DOS
// Use the original Apogee Sound System libs instead.  --ryan.
#error you probably should not compile this.
#endif

#if (defined __WATCOMC__)
// This is probably out of date.  --ryan.
#include "dukesnd_watcom.h"
#endif

#if (!defined __WATCOMC__)
#define cdecl
#endif

#include "SDL.h"
#include "SDL_mixer.h"
#ifdef ROTT
#include "rt_cfg.h"      // ROTT music hack
#include "rt_util.h"     // ROTT music hack
#endif
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

int MUSIC_ErrorCode = MUSIC_Ok;

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
static int mixerIsStereo = 1;

// This gets called all over the place for information and debugging messages.
//  If the user set the DUKESND_DEBUG environment variable, the messages
//  go to the file that is specified in that variable. Otherwise, they
//  are ignored for the expense of the function call. If DUKESND_DEBUG is
//  set to "-" (without the quotes), then the output goes to stdout.
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
} // musdebug

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

static void setWarningMessage(const char *msg)
{
    strncpy(warningMessage, msg, sizeof (warningMessage));
    // strncpy() doesn't add the null char if there isn't room...
    warningMessage[sizeof (warningMessage) - 1] = '\0';
    musdebug("Warning message set to [%s].", warningMessage);
} // setErrorMessage


static void setErrorMessage(const char *msg)
{
    strncpy(errorMessage, msg, sizeof (errorMessage));
    // strncpy() doesn't add the null char if there isn't room...
    errorMessage[sizeof (errorMessage) - 1] = '\0';
    musdebug("Error message set to [%s].", errorMessage);
} // setErrorMessage

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
    //if (!fx_initialized)
    if (!Mix_QuerySpec(NULL, NULL, NULL))
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

musdebug("Need to use PlaySongROTT.  :(");

    return(MUSIC_Ok);
} // MUSIC_PlaySong


extern char ApogeePath[256];

#ifdef DUKE3D
// Duke3D-specific.  --ryan.
void PlayMusic(char *_filename)
{
    //char filename[MAX_PATH];
    //strcpy(filename, _filename);
    //FixFilePath(filename);

    char filename[MAX_PATH];
    long handle;
    long size;
    void *song;
    long rc;

    MUSIC_StopSong();

    // Read from a groupfile, write it to disk so SDL_mixer can read it.
    //   Lame.  --ryan.
    handle = kopen4load(_filename, 0);
    if (handle == -1)
        return;

    size = kfilelength(handle);
    if (size == -1)
    {
        kclose(handle);
        return;
    } // if

    song = malloc(size);
    if (song == NULL)
    {
        kclose(handle);
        return;
    } // if

    rc = kread(handle, song, size);
    kclose(handle);
    if (rc != size)
    {
        free(song);
        return;
    } // if

    // save the file somewhere, so SDL_mixer can load it
    GetPathFromEnvironment(filename, MAX_PATH, "tmpsong.mid");
    handle = SafeOpenWrite(filename, filetype_binary);
    
    SafeWrite(handle, song, size);
    close(handle);
    free(song);
    
    //music_songdata = song;

    music_musicchunk = Mix_LoadMUS(filename);
    if (music_musicchunk != NULL)
    {
        // !!! FIXME: I set the music to loop. Hope that's okay. --ryan.
        Mix_PlayMusic(music_musicchunk, -1);
    } // if
}
#endif

#ifdef ROTT
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
#endif


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


void MUSIC_RerouteMidiChannel(int channel, int cdecl (*function)( int event, int c1, int c2 ))
{
    musdebug("STUB ... MUSIC_RerouteMidiChannel().\n");
} // MUSIC_RerouteMidiChannel


void MUSIC_RegisterTimbreBank(unsigned char *timbres)
{
    musdebug("STUB ... MUSIC_RegisterTimbreBank().\n");
} // MUSIC_RegisterTimbreBank


// end of fx_man.c ...
