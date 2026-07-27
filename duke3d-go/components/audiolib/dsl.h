#ifndef AUDIOLIB__DSL_H
#define AUDIOLIB__DSL_H

#define MONO_8BIT    0
#define STEREO      1
#define SIXTEEN_BIT 2
#define STEREO_16BIT ( STEREO | SIXTEEN_BIT )

enum DSL_ERRORS
   {
   DSL_Warning = -2,
   DSL_Error = -1,
   DSL_Ok = 0,
   DSL_SDLInitFailure,
   DSL_MixerActive,
   DSL_MixerInitFailure
   };

char *DSL_ErrorString( int ErrorNumber );
int   DSL_Init( void );
void  DSL_StopPlayback( void );
unsigned DSL_GetPlaybackRate( void );
int   DSL_BeginBufferedPlayback( char *BufferStart,
      int BufferSize, int NumDivisions, unsigned SampleRate,
      int MixMode, void ( *CallBackFunc )( void ) );
void  DSL_Shutdown( void );

#endif
