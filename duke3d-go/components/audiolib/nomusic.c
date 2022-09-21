#include "music.h"

char *MUSIC_ErrorString(int ErrorNumber)
{
	return "";
}

int MUSIC_Init(int SoundCard, int Address)
{
	return 0;
}

int MUSIC_Shutdown(void)
{
	return 0;
}

void MUSIC_SetMaxFMMidiChannel(int channel)
{
}

void MUSIC_SetVolume(int volume)
{
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
	return 0;
}

void MUSIC_Continue(void)
{
}

void MUSIC_Pause(void)
{
}

int MUSIC_StopSong(void)
{
	return 0;
}

int MUSIC_PlaySong(char *song, int loopflag)
{
	return 0;
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
	return 0;
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

void MUSIC_RegisterTimbreBank(unsigned char *timbres)
{
}

// This is the method called from the Game Module.
void PlayMusic(char  *fileName){
}