//
//  sdl_midi.c
//  Duke3D
//
//  Created by fabien sanglard on 12-12-15.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#include <stdio.h>
#include "../audiolib/music.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "build.h"

/*
 Because the music is stored in a GRP file that is never fully loaded in RAM
 (the full version of Duke Nukem 3D is a 43MB GRP) we need to extract the music
 from it and store it in RAM.
*/
#define KILOBYTE (1024*1024)
uint8_t musicDataBuffer[100 * KILOBYTE];

char  *MUSIC_ErrorString(int ErrorNumber)
{
	return "";
}

int MUSIC_Init(int SoundCard, int Address)
{
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
	    printf("Mix_OpenAudio: %s\n", Mix_GetError());
    }
    
    return MUSIC_Ok;
}

int MUSIC_Shutdown(void)
{
	MUSIC_StopSong();
    return(MUSIC_Ok);
}

void MUSIC_SetMaxFMMidiChannel(int channel)
{
}

void MUSIC_SetVolume(int volume)
{
    Mix_VolumeMusic((int)(volume / 2));
}

void MUSIC_SetMidiChannelVolume(int channel, int volume)
{
}

void MUSIC_ResetMidiChannelVolumes(void)
{
}

int MUSIC_GetVolume(void)
{
	return 0;
}

void MUSIC_SetLoopFlag(int loopflag)
{
}

int MUSIC_SongPlaying(void)
{
	return Mix_PlayingMusic();
}

void MUSIC_Continue(void)
{
    if (Mix_PausedMusic())
        Mix_ResumeMusic();
}

void MUSIC_Pause(void)
{
    Mix_PauseMusic();
}

int MUSIC_StopSong(void)
{
	if ( (Mix_PlayingMusic()) || (Mix_PausedMusic()) )
        Mix_HaltMusic();
    
    return(MUSIC_Ok);
}



int MUSIC_PlaySong(char  *songFilename, int loopflag)
{
    int32_t fd =  0;
    int fileSize;
    SDL_RWops *rw;
    Mix_Music* sdlMusic;
    
    fd = kopen4load(songFilename,0);
    
	if(fd == -1){ 
        printf("The music '%s' cannot be found in the GRP or the filesystem.\n",songFilename);
        return 0;
    }
    
    
    
    fileSize = kfilelength( fd );
    if(fileSize >= sizeof(musicDataBuffer))
    {
        printf("The music '%s' was found but is too big (%dKB)to fit in the buffer (%luKB).\n",songFilename,fileSize/1024,sizeof(musicDataBuffer)/1024);
        kclose(fd);
        return 0;
    }
    
    kread( fd, musicDataBuffer, fileSize);
    kclose( fd );
    
    //Ok, the file is in memory
    rw = SDL_RWFromMem((void *) musicDataBuffer, fileSize); 
    
    sdlMusic = Mix_LoadMUS_RW(rw);
    
    Mix_PlayMusic(sdlMusic, (loopflag == MUSIC_PlayOnce) ? 0 : -1);
    
    return 1;
}


void MUSIC_SetContext(int context)
{
}

int MUSIC_GetContext(void)
{
	return 0;
}

void MUSIC_SetSongTick(uint32_t PositionInTicks)
{
}

void MUSIC_SetSongTime(uint32_t milliseconds)
{
}

void MUSIC_SetSongPosition(int measure, int beat, int tick)
{
}

void MUSIC_GetSongPosition(songposition *pos)
{
}

void MUSIC_GetSongLength(songposition *pos)
{
}

int MUSIC_FadeVolume(int tovolume, int milliseconds)
{
	return(MUSIC_Ok);
}

int MUSIC_FadeActive(void)
{
	return 0;
}

void MUSIC_StopFade(void)

{
}

void MUSIC_RerouteMidiChannel(int channel, int cdecl function( int event, int c1, int c2 ))
{
}

void MUSIC_RegisterTimbreBank(uint8_t  *timbres)
{
}

// This is the method called from the Game Module.
void PlayMusic(char  *fileName){
    MUSIC_PlaySong(fileName,1);
}
