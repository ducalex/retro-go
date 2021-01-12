//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#if 0 // We handle that in retro-go's main.cpp
//
// This is the main program entry point
//

#ifdef TARGET_OD
#include <SDL/SDL.h>
#else
#ifndef TARGET_PSP
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#endif
#endif

#include <zlib.h>

#include "../opendingux/unzip.h"

#ifndef __GP32__
#include "StdAfx.h"
#endif

#include "main.h"


//#include "msounds.h"
#include "memory.h"
//#include "mainemu.h"
#include "tlcs900h.h"
#include "input.h"

#include "graphics.h"
#include "tlcs900h.h"
#ifndef __GP32__
//#include "timer.h"
#endif

#include "neopopsound.h"

#ifdef DRZ80
#include "DrZ80_support.h"
#else
#if defined(CZ80)
#include "cz80_support.h"
#else
#include "z80.h"
#endif
#endif

#ifdef __GP32__
#include "fileio.h"
#include "gfx.h"
#elif !defined(TARGET_PSP)
SDL_Surface* screen = NULL;						// Main program screen
SDL_Surface* actualScreen = NULL;						// Main program screen
#endif

BOOL		m_bIsActive;
int		exitNow=0;
EMUINFO		m_emuInfo;
SYSTEMINFO	m_sysInfo[NR_OF_SYSTEMS];

FILE	*errorLog = NULL;
FILE	*outputRam = NULL;

#define numberof(a)		(sizeof(a)/sizeof(*(a)))

void mainemuinit()
{
	// initialize cpu memory
	mem_init();
#ifndef __GP32__
	graphics_init(NULL);
#else
	graphics_init();
#endif

    // initialize the TLCS-900H cpu
    tlcs_init();

#if defined(DRZ80) || defined(CZ80)
	Z80_Init();
    Z80_Reset();
#else
    z80Init();
#endif

    // if neogeo pocket color rom, act if we are a neogeo pocket color
    tlcsMemWriteB(0x6F91,tlcsMemReadB(0x00200023));
    // pretend we're running in English mode
    tlcsMemWriteB(0x00006F87,0x01);
    // kludges & fixes
    switch (tlcsMemReadW(0x00200020))
    {
        case 0x0059:	// Sonic
        case 0x0061:	// Metal SLug 2nd
            *get_address(0x0020001F) = 0xFF;
            break;
    }
    ngpSoundOff();
    //Flavor sound_start();
}

#if !defined(__GP32__) && !defined(TARGET_PSP)  && !defined(TARGET_OD)

void AfxMessageBox(char *a, int b, int c)
{
	dbg_print(a);
}
void AfxMessageBox(char *a, int b)
{
	dbg_print(a);
}
void AfxMessageBox(char *a)
{
	dbg_print(a);
}

#endif

void	SetActive(BOOL bActive)
{
	m_bIsActive = bActive;
}

void	SetEmu(int machine)
{
	m_emuInfo.machine = machine;
	m_emuInfo.drv = &m_sysInfo[machine];
}

bool initRom()
{
	char	*licenseInfo = (char *) " BY SNK CORPORATION";
	char	*ggLicenseInfo = (char *) "TMR SEGA";
	BOOL	romFound = TRUE;
	int		i, m;

	//dbg_print("in openNgp(%s)\n", lpszPathName);

	// first stop the current emulation
	//dbg_print("openNgp: SetEmu(NONE)\n");
	SetEmu(NGPC);
	//dbg_print("openNgp: SetActive(FALSE)\n");
	SetActive(FALSE);

	// check NEOGEO POCKET
	// check license info
	for (i=0;i<19;i++)
	{
		if (mainrom[0x000009 + i] != licenseInfo[i])
			romFound = FALSE;
	}
	if (romFound)
	{
		//dbg_print("openNgp: romFound == TRUE\n");
		i = mainrom[0x000023];
		if (i == 0x10 || i == 0x00)
		{
			// initiazlie emulation
			if (i == 0x10) {
				m = NGPC;
			} else {
				// fix for missing Mono/Color setting in Cool Coom Jam SAMPLE rom
				if (mainrom[0x000020] == 0x34 && mainrom[0x000021] == 0x12)
					m = NGPC;
				else m = NGP;
			}

			//dbg_print("openNgp: SetEmu(%d)\n", m);
			SetEmu(m);

			//dbg_print("openNgp: Calling mainemuinit(%s)\n", lpszPathName);
			mainemuinit();
			// start running the emulation loop
			//dbg_print("openNgp: SetActive(TRUE)\n");
			SetActive(TRUE);

			// acknowledge opening of the document went fine
			//dbg_print("openNgp: returning success\n");
			return TRUE;
		}

		fprintf(stderr, "Not a valid or unsupported rom file.\n");
		return FALSE;
	}

	fprintf(stderr, "Not a valid or unsupported rom file. romFound==FALSE\n");
	return FALSE;
}

void initSysInfo()
{
	m_bIsActive = FALSE;

	m_emuInfo.machine = NGPC;
	m_emuInfo.drv = &m_sysInfo[m_emuInfo.machine];
	m_emuInfo.romSize = 0;

/*	strcpy(m_emuInfo.ProgramFolder, "");
	strcpy(m_emuInfo.SavePath, "");
	strcpy(m_emuInfo.DebugPath, "");
	strcpy(m_emuInfo.RomPath, "");
	strcpy(m_emuInfo.OpenFileName, "");
	strcpy(m_emuInfo.SaveFileName, "");
	strcpy(m_emuInfo.OpenFileName, "");
	strcpy(m_emuInfo.DebugPath, "");;
	strcpy(m_emuInfo.RomFileName, "");
	strcpy(m_emuInfo.SaveFileName, "");
	strcpy(m_emuInfo.ScreenPath, "");*/
	strcpy(m_emuInfo.RomFileName, "");
#define NO_SOUND_OUTPUT
#ifdef NO_SOUND_OUTPUT
	m_emuInfo.sample_rate = 0;
#else
	m_emuInfo.sample_rate = 44100;
#endif
	m_emuInfo.stereo = 1;
	//m_emuInfo.fps = 60;//30;//100;  //Flavor, tweak this!

/*	m_sysInfo[NONE].hSize = 160;
	m_sysInfo[NONE].vSize = 152;
	m_sysInfo[NONE].Ticks = 0;
	//m_sysInfo[NONE].sound[0].sound_type = 0;*/

	m_sysInfo[NGP].hSize = 160;
	m_sysInfo[NGP].vSize = 152;
	m_sysInfo[NGP].Ticks = 6*1024*1024;
/*	m_sysInfo[NGP].sound[0].sound_type = SOUND_SN76496;
	m_sysInfo[NGP].sound[0].sound_interface = new_SN76496(1, 3*1024*1024, MIXER(50,MIXER_PAN_CENTER));
	m_sysInfo[NGP].sound[1].sound_type = SOUND_DAC;
	m_sysInfo[NGP].sound[1].sound_interface = new_DAC(2,MIXER(50,MIXER_PAN_LEFT),MIXER(50,MIXER_PAN_RIGHT));
	m_sysInfo[NGP].sound[2].sound_type = 0;
	m_sysInfo[NGP].Back0 = TRUE;
	m_sysInfo[NGP].Back1 = TRUE;
	m_sysInfo[NGP].Sprites = TRUE;*/

	m_sysInfo[NGPC].hSize = 160;
	m_sysInfo[NGPC].vSize = 152;
	m_sysInfo[NGPC].Ticks = 6*1024*1024;
/*	m_sysInfo[NGPC].sound[0].sound_type = SOUND_SN76496;
	m_sysInfo[NGPC].sound[0].sound_interface = new_SN76496(1, 3*1024*1024, 50);
	m_sysInfo[NGPC].sound[1].sound_type = SOUND_DAC;
	m_sysInfo[NGPC].sound[1].sound_interface = new_DAC(2,MIXER(50,MIXER_PAN_LEFT),MIXER(50,MIXER_PAN_RIGHT));
	m_sysInfo[NGPC].sound[2].sound_type = 0;
	m_sysInfo[NGPC].Back0 = TRUE;
	m_sysInfo[NGPC].Back1 = TRUE;
	m_sysInfo[NGPC].Sprites = TRUE;*/
}

char *getFileNameExtension(char *nom_fichier)		{
	char *ptrPoint = nom_fichier;
	while(*nom_fichier)		{
		if (*nom_fichier == '.')
			ptrPoint = nom_fichier;
		nom_fichier++;
	}
	return ptrPoint;
}

int loadFromZipByName(unsigned char *buffer, char *archive, char *filename, int *filesize)
{
    char name[_MAX_PATH];
    //unsigned char *buffer;
	int i;
	const char *recognizedExtensions[] = {
		".ngp",
		".npc",
		".ngc"
	};

    int zerror = UNZ_OK;
    unzFile zhandle;
    unz_file_info zinfo;

    zhandle = unzOpen(archive);
    if(!zhandle) return (0);

    /* Seek to first file in archive */
    zerror = unzGoToFirstFile(zhandle);
    if(zerror != UNZ_OK)
    {
        unzClose(zhandle);
        return (0);
    }

	//On scanne tous les fichiers de l'archive et ne prend que ceux qui ont une extension valable, sinon on prend le dernier fichier trouvé...
	while (zerror == UNZ_OK) {
		if (unzGetCurrentFileInfo(zhandle, &zinfo, name, 0xff, NULL, 0, NULL, 0) != UNZ_OK) {
			unzClose(zhandle);
			return 0;
		}

		//Vérifions que c'est la bonne extension
		char *extension = getFileNameExtension(name);

		for (i=0;i<numberof(recognizedExtensions);i++)		{
			if (!strcmp(extension, recognizedExtensions[i]))
				break;
		}
		if (i < numberof(recognizedExtensions))
			break;

		zerror = unzGoToNextFile(zhandle);
	}

    /* Get information about the file */
//    unzGetCurrentFileInfo(zhandle, &zinfo, &name[0], 0xff, NULL, 0, NULL, 0);
    *filesize = zinfo.uncompressed_size;

    /* Error: file size is zero */
    if(*filesize <= 0 || *filesize > (4*1024*1024))
    {
        unzClose(zhandle);
        return (0);
    }

    /* Open current file */
    zerror = unzOpenCurrentFile(zhandle);
    if(zerror != UNZ_OK)
    {
        unzClose(zhandle);
        return (0);
    }

    /* Allocate buffer and read in file */
    //buffer = malloc(*filesize);
    //if(!buffer) return (NULL);
    zerror = unzReadCurrentFile(zhandle, buffer, *filesize);

    /* Internal error: free buffer and close file */
    if(zerror < 0 || zerror != *filesize)
    {
        //free(buffer);
        //buffer = NULL;
        unzCloseCurrentFile(zhandle);
        unzClose(zhandle);
        return (0);
    }

    /* Close current file and archive file */
    unzCloseCurrentFile(zhandle);
    unzClose(zhandle);

    memcpy(filename, name, _MAX_PATH);
    return 1;
}

/*
    Verifies if a file is a ZIP archive or not.
    Returns: 1= ZIP archive, 0= not a ZIP archive
*/
int check_zip(char *filename)
{
    unsigned char buf[2];
    FILE *fd = NULL;
    fd = fopen(filename, "rb");
    if(!fd) return (0);
    fread(buf, 2, 1, fd);
    fclose(fd);
    if(memcmp(buf, "PK", 2) == 0) return (1);
    return (0);
}

int strrchr2(const char *src, int c)
{
  size_t len;

  len=strlen(src);
  while(len>0){
    len--;
    if(*(src+len) == c)
      return len;
  }

  return 0;
}

int handleInputFile(char *romName)
{
	FILE *romFile;
	int iDepth = 0;
	int size;


	initSysInfo();  //initialize it all

	//if it's a ZIP file, we need to handle that here.
	iDepth = strrchr2(romName, '.');
	iDepth++;
	if( ( strcmp( romName + iDepth, "zip" ) == 0 ) || ( strcmp( romName + iDepth, "ZIP" ) == 0 ))
	{
		//get ROM from ZIP
		if(check_zip(romName))
		{
			char name[_MAX_PATH];
			if(!loadFromZipByName(mainrom, romName, name, &size))
			{
				fprintf(stderr, "Load failed from %s\n", romName);
				return 0;
			}
			m_emuInfo.romSize = size;
			strcpy(m_emuInfo.RomFileName, romName);
		}
		else
		{
			fprintf(stderr, "%s not PKZIP file\n", romName);
			return 0;
		}
	}
	else
	{
		//get ROM from binary ROM file
		romFile = fopen(romName, "rb");
		if(!romFile)
		{
			fprintf(stderr, "Couldn't open %s file\n", romName);
			return 0;
		}

		m_emuInfo.romSize = fread(mainrom, 1, 4*1024*1024, romFile);
		strcpy(m_emuInfo.RomFileName, romName);
	}

	if(!initRom())
	{
		fprintf(stderr, "initRom couldn't handle %s file\n", romName);
		return 0;
	}

	setFlashSize(m_emuInfo.romSize);
	return 1;
}


#if !defined(TARGET_PSP) && !defined(TARGET_OD)
int main(int argc, char *argv[])
{
	char romName[512];

#ifdef TARGET_PSP
    SetupCallbacks();

    scePowerSetClockFrequency(222, 222, 111);
    //scePowerSetClockFrequency(266, 266, 133);
    //scePowerSetClockFrequency(333, 333, 166);
#endif
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK) < 0)
	{
		fprintf(stderr, "SDL_Init failed!\n");
#ifdef TARGET_PSP
		sceKernelExitGame();
#endif
		return -1;
	}
	// Set up quiting so that it automatically runs on exit.
	atexit(SDL_Quit);

	PSP_JoystickOpen();

	actualScreen = SDL_SetVideoMode (SIZEX, SIZEY, 16, SDL_SWSURFACE);
	if (actualScreen == NULL)
	{
		fprintf(stderr, "SDL_SetVideoMode failed!\n");
#ifdef TARGET_PSP
		sceKernelExitGame();
#endif
		return -1;
	}
	else
	{
		dbg_print("screen params: bpp=%d\n", actualScreen->format->BitsPerPixel);
	}
#ifdef ZOOM_SUPPORT
    screen = SDL_CreateRGBSurface (actualScreen->flags,
                                actualScreen->w,
                                actualScreen->h,
                                actualScreen->format->BitsPerPixel,
                                actualScreen->format->Rmask,
                                actualScreen->format->Gmask,
                                actualScreen->format->Bmask,
                                actualScreen->format->Amask);
	if (screen == NULL)
	{
		fprintf(stderr, "SDL_CreateRGBSurface failed!\n");
#ifdef TARGET_PSP
		sceKernelExitGame();
#endif
		return -1;
	}
#else
    screen = actualScreen;
#endif
	SDL_ShowCursor(0);  //disable the cursor

	/* Set up the SDL_TTF */
	TTF_Init();
	atexit(TTF_Quit);
    //printTTF("calling LoadRomFromPSP", 10, TEXT_HEIGHT*1, white, 1, actualScreen, 1);

    sound_system_init();
    while(!exitNow)
    {
#ifdef TARGET_WIN
		strncpy(romName, argv[1], 500);
		dbg_print("Starting emulation on file \"%s\"\n");
#endif
        if(LoadRomFromPSP(romName, actualScreen)<=0)
        {
            break;
        }
        SDL_FillRect(actualScreen, NULL, SDL_MapRGB(screen->format,0,0,0));//fill black
        SDL_Flip(actualScreen);

//printTTF(romName, 20, TEXT_HEIGHT*16, white, 1, actualScreen, 1);

        system_sound_chipreset();	//Resets chips

        handleInputFile(romName);

        InitInput(NULL);

        dbg_print("Running NGPC Emulation\n");

        SDL_PauseAudio(0);  //run audio
#ifdef TARGET_PSP
        switch(options[PSP_MHZ_OPTION])
        {
            case PSP_MHZ_222:
                scePowerSetClockFrequency(222, 222, 111);
                break;
            case PSP_MHZ_266:
                scePowerSetClockFrequency(266, 266, 133);
                break;
            default:
            case PSP_MHZ_333:
                scePowerSetClockFrequency(333, 333, 166);
                break;
        }
#endif
        ngpc_run();
#ifdef TARGET_PSP
        scePowerSetClockFrequency(222, 222, 111);
#endif
        SDL_PauseAudio(1);  //pause audio

        flashShutdown();
    }

    //printTTF("EXITING", 10, TEXT_HEIGHT*10, white, 1);
#ifdef TARGET_PSP
    sceKernelExitGame(); //Exit the program when it is done.
#endif
	return 0;
}
#endif

#endif
