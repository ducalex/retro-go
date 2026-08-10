#include "SDL_system.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <rg_system.h>

void SDL_Delay(Uint32 ms)
{
    rg_usleep(ms * 1000);
}

int SDL_Init(Uint32 flags)
{
    return 0;
}

void SDL_Quit(void)
{
}

void SDL_InitSD(void)
{
    // Retro-Go handles SD card initialization
}

const SDL_version* SDL_Linked_Version()
{
    static SDL_version vers;
    vers.major = SDL_MAJOR_VERSION;                 
    vers.minor = SDL_MINOR_VERSION;                 
    vers.patch = SDL_PATCHLEVEL;      
    return &vers;
}

char *** allocateTwoDimenArrayOnHeapUsingMalloc(int row, int col)
{
	char ***ptr = (char ***)malloc(row * sizeof(char **) + row * (col * sizeof(char *)));
    // This function looks suspicious in the original code. 
    // Usually it's used for array of strings or similar.
    // Let's fix the pointer arithmetic at least.
	char ** data = (char **)(ptr + row);
	for(int i = 0; i < row; i++)
		ptr[i] = data + i * col;

	return ptr;
}

void SDL_DestroyMutex(SDL_mutex* mutex)
{
}

SDL_mutex* SDL_CreateMutex(void)
{
    return NULL;
}

int SDL_LockMutex(SDL_mutex* mutex)
{
    return 0;
}

int SDL_UnlockMutex(SDL_mutex* mutex)
{
    return 0;
}

// Retro-Go already handles basic file I/O. 
// We bridge these to standard C functions or Retro-Go safe wrappers if needed.
// For now, standard C is fine as long as paths are correct.

int __mkdir(const char *path, mode_t mode)
{
    return mkdir(path, mode);
}

FILE *__fopen( const char *path, const char *mode )
{
	return fopen(path, mode);
}

long __ftell( FILE *f )
{
	return ftell(f);
}

int __feof ( FILE * stream )
{
	return feof ( stream );
}

int __fputc ( int character, FILE * stream )
{
	return fputc ( character, stream );
}

int __fgetc ( FILE * stream )
{
	return fgetc ( stream );
}

size_t __fwrite ( const void * ptr, size_t size, size_t count, FILE * stream )
{
	return fwrite ( ptr, size, count, stream );
}

int __fclose ( FILE * stream )
{
	return fclose ( stream );
}

int __fseek( FILE * stream, long int offset, int origin )
{
	return fseek ( stream, offset, origin );
}

size_t __fread( void *buffer, size_t size, size_t num, FILE *stream )
{
	return fread(buffer, size, num, stream);
}

int __stat(const char *path, struct stat *buf)
{
	return stat ( path, buf );
}

int __open(const char *path, int oflag, ...)
{
	return open(path, oflag);
}

int __close(int fildes)
{
	return close(fildes);
}

ssize_t __read(int fildes, void *buf, size_t nbyte)
{
	return read(fildes, buf, nbyte);
}

ssize_t __write(int fildes, const void *buf, size_t nbyte)
{
	return write(fildes, buf, nbyte);
}

off_t __lseek(int fd, off_t offset, int whence)
{
	return lseek(fd, offset, whence);
}

int __unlink(const char *pathname)
{
	return unlink(pathname);
}
