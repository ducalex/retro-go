//
//	ID Engine
//	ID_US_1.c - User Manager - General routines
//	v1.1d1
//	By Jason Blochowiak
//	Hacked up for Catacomb 3D
//

//
//	This module handles dealing with user input & feedback
//
//	Depends on: Input Mgr, View Mgr, some variables from the Sound, Caching,
//		and Refresh Mgrs, Memory Mgr for background save/restore
//
//	Globals:
//		ingame - Flag set by game indicating if a game is in progress
//		loadedgame - Flag set if a game was loaded
//		PrintX, PrintY - Where the User Mgr will print (global coords)
//		WindowX,WindowY,WindowW,WindowH - The dimensions of the current
//			window
//

#include "wl_def.h"

#if _MSC_VER == 1200            // Visual C++ 6
	#define vsnprintf _vsnprintf
#endif

//	Global variables
		word		PrintX,PrintY;
		word		WindowX,WindowY,WindowW,WindowH;

//	Internal variables
#define	ConfigVersion	1

static	boolean		US_Started;

		void		(*USL_MeasureString)(const char *,word *,word *) = VW_MeasurePropString;
		void		(*USL_DrawString)(const char *) = VWB_DrawPropString;

		SaveGame	Games[MaxSaveGames];
		HighScore	Scores[MaxScores] =
					{
						{"id software-'92",10000,1,0},
						{"Adrian Carmack",10000,1,0},
						{"John Carmack",10000,1,0},
						{"Kevin Cloud",10000,1,0},
						{"Tom Hall",10000,1,0},
						{"John Romero",10000,1,0},
						{"Jay Wilbur",10000,1,0},
					};

int rndindex = 0;

static byte rndtable[] = {
      0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	 74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	 95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	 52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	 25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	 94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	 80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	 24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	 28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	 71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	 17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120, 163, 236, 249 };

//	Internal routines

//	Public routines

///////////////////////////////////////////////////////////////////////////
//
//	US_Startup() - Starts the User Mgr
//
///////////////////////////////////////////////////////////////////////////
void US_Startup()
{
	if (US_Started)
		return;

	US_InitRndT(true);		// Initialize the random number generator

	US_Started = true;
}


///////////////////////////////////////////////////////////////////////////
//
//	US_Shutdown() - Shuts down the User Mgr
//
///////////////////////////////////////////////////////////////////////////
void
US_Shutdown(void)
{
	if (!US_Started)
		return;

	US_Started = false;
}

//	Window/Printing routines

///////////////////////////////////////////////////////////////////////////
//
//	US_SetPrintRoutines() - Sets the routines used to measure and print
//		from within the User Mgr. Primarily provided to allow switching
//		between masked and non-masked fonts
//
///////////////////////////////////////////////////////////////////////////
void
US_SetPrintRoutines(void (*measure)(const char *,word *,word *),
    void (*print)(const char *))
{
	USL_MeasureString = measure;
	USL_DrawString = print;
}

///////////////////////////////////////////////////////////////////////////
//
//	US_Print() - Prints a string in the current window. Newlines are
//		supported.
//
///////////////////////////////////////////////////////////////////////////
void
US_Print(const char *sorg)
{
	char c;
	char *sstart = strdup(sorg);
	char *s = sstart;
	char *se;
	word w,h;

	while (*s)
	{
		se = s;
		while ((c = *se)!=0 && (c != '\n'))
			se++;
		*se = '\0';

		USL_MeasureString(s,&w,&h);
		px = PrintX;
		py = PrintY;
		USL_DrawString(s);

		s = se;
		if (c)
		{
			*se = c;
			s++;

			PrintX = WindowX;
			PrintY += h;
		}
		else
			PrintX += w;
	}
	free(sstart);
}

///////////////////////////////////////////////////////////////////////////
//
//	US_PrintUnsigned() - Prints an unsigned long
//
///////////////////////////////////////////////////////////////////////////
void
US_PrintUnsigned(longword n)
{
	char	buffer[32];
	sprintf(buffer, "%lu", (unsigned long)n);

	US_Print(buffer);
}

///////////////////////////////////////////////////////////////////////////
//
//	US_PrintSigned() - Prints a signed long
//
///////////////////////////////////////////////////////////////////////////
void
US_PrintSigned(int32_t n)
{
	char	buffer[32];

	US_Print(ltoa(n,buffer,10));
}

///////////////////////////////////////////////////////////////////////////
//
//	USL_PrintInCenter() - Prints a string in the center of the given rect
//
///////////////////////////////////////////////////////////////////////////
void
USL_PrintInCenter(const char *s,Rect r)
{
	word	w,h,
			rw,rh;

	USL_MeasureString(s,&w,&h);
	rw = r.lr.x - r.ul.x;
	rh = r.lr.y - r.ul.y;

	px = r.ul.x + ((rw - w) / 2);
	py = r.ul.y + ((rh - h) / 2);
	USL_DrawString(s);
}

///////////////////////////////////////////////////////////////////////////
//
//	US_PrintCentered() - Prints a string centered in the current window.
//
///////////////////////////////////////////////////////////////////////////
void
US_PrintCentered(const char *s)
{
	Rect	r;

	r.ul.x = WindowX;
	r.ul.y = WindowY;
	r.lr.x = r.ul.x + WindowW;
	r.lr.y = r.ul.y + WindowH;

	USL_PrintInCenter(s,r);
}

///////////////////////////////////////////////////////////////////////////
//
//	US_CPrintLine() - Prints a string centered on the current line and
//		advances to the next line. Newlines are not supported.
//
///////////////////////////////////////////////////////////////////////////
void
US_CPrintLine(const char *s)
{
	word	w,h;

	USL_MeasureString(s,&w,&h);

	if (w > WindowW)
		Quit("US_CPrintLine() - String exceeds width");
	px = WindowX + ((WindowW - w) / 2);
	py = PrintY;
	USL_DrawString(s);
	PrintY += h;
}

///////////////////////////////////////////////////////////////////////////
//
//  US_CPrint() - Prints a string centered in the current window.
//      Newlines are supported.
//
///////////////////////////////////////////////////////////////////////////
void
US_CPrint(const char *sorg)
{
	char	c;
	char *sstart = strdup(sorg);
	char *s = sstart;
	char *se;

	while (*s)
	{
		se = s;
		while ((c = *se)!=0 && (c != '\n'))
			se++;
		*se = '\0';

		US_CPrintLine(s);

		s = se;
		if (c)
		{
			*se = c;
			s++;
		}
	}
	free(sstart);
}

///////////////////////////////////////////////////////////////////////////
//
//  US_Printf() - Prints a formatted string in the current window.
//      Newlines are supported.
//
///////////////////////////////////////////////////////////////////////////

void US_Printf(const char *formatStr, ...)
{
    char strbuf[256];
    va_list vlist;
    va_start(vlist, formatStr);
    int len = vsnprintf(strbuf, sizeof(strbuf), formatStr, vlist);
    va_end(vlist);
    if(len <= -1 || len >= sizeof(strbuf))
        strbuf[sizeof(strbuf) - 1] = 0;
    US_Print(strbuf);
}

///////////////////////////////////////////////////////////////////////////
//
//  US_CPrintf() - Prints a formatted string centered in the current window.
//      Newlines are supported.
//
///////////////////////////////////////////////////////////////////////////

void US_CPrintf(const char *formatStr, ...)
{
    char strbuf[256];
    va_list vlist;
    va_start(vlist, formatStr);
    int len = vsnprintf(strbuf, sizeof(strbuf), formatStr, vlist);
    va_end(vlist);
    if(len <= -1 || len >= sizeof(strbuf))
        strbuf[sizeof(strbuf) - 1] = 0;
    US_CPrint(strbuf);
}

///////////////////////////////////////////////////////////////////////////
//
//	US_ClearWindow() - Clears the current window to white and homes the
//		cursor
//
///////////////////////////////////////////////////////////////////////////
void
US_ClearWindow(void)
{
	VWB_Bar(WindowX,WindowY,WindowW,WindowH,WHITE);
	PrintX = WindowX;
	PrintY = WindowY;
}

///////////////////////////////////////////////////////////////////////////
//
//	US_DrawWindow() - Draws a frame and sets the current window parms
//
///////////////////////////////////////////////////////////////////////////
void
US_DrawWindow(word x,word y,word w,word h)
{
	word	i,
			sx,sy,sw,sh;

	WindowX = x * 8;
	WindowY = y * 8;
	WindowW = w * 8;
	WindowH = h * 8;

	PrintX = WindowX;
	PrintY = WindowY;

	sx = (x - 1) * 8;
	sy = (y - 1) * 8;
	sw = (w + 1) * 8;
	sh = (h + 1) * 8;

	US_ClearWindow();

	VWB_DrawTile8(sx,sy,0),VWB_DrawTile8(sx,sy + sh,5);
	for (i = sx + 8;i <= sx + sw - 8;i += 8)
		VWB_DrawTile8(i,sy,1),VWB_DrawTile8(i,sy + sh,6);
	VWB_DrawTile8(i,sy,2),VWB_DrawTile8(i,sy + sh,7);

	for (i = sy + 8;i <= sy + sh - 8;i += 8)
		VWB_DrawTile8(sx,i,3),VWB_DrawTile8(sx + sw,i,4);
}

///////////////////////////////////////////////////////////////////////////
//
//	US_CenterWindow() - Generates a window of a given width & height in the
//		middle of the screen
//
///////////////////////////////////////////////////////////////////////////
void
US_CenterWindow(word w,word h)
{
	US_DrawWindow(((MaxX / 8) - w) / 2,((MaxY / 8) - h) / 2,w,h);
}

///////////////////////////////////////////////////////////////////////////
//
//	US_SaveWindow() - Saves the current window parms into a record for
//		later restoration
//
///////////////////////////////////////////////////////////////////////////
void
US_SaveWindow(WindowRec *win)
{
	win->x = WindowX;
	win->y = WindowY;
	win->w = WindowW;
	win->h = WindowH;

	win->px = PrintX;
	win->py = PrintY;
}

///////////////////////////////////////////////////////////////////////////
//
//	US_RestoreWindow() - Sets the current window parms to those held in the
//		record
//
///////////////////////////////////////////////////////////////////////////
void
US_RestoreWindow(WindowRec *win)
{
	WindowX = win->x;
	WindowY = win->y;
	WindowW = win->w;
	WindowH = win->h;

	PrintX = win->px;
	PrintY = win->py;
}

//	Input routines

///////////////////////////////////////////////////////////////////////////
//
//	USL_XORICursor() - XORs the I-bar text cursor. Used by US_LineInput()
//
///////////////////////////////////////////////////////////////////////////
static void
USL_XORICursor(int x,int y,const char *s,word cursor)
{
	static	boolean	status;		// VGA doesn't XOR...
	char	buf[MaxString];
	int		temp;
	word	w,h;

	strcpy(buf,s);
	buf[cursor] = '\0';
	USL_MeasureString(buf,&w,&h);

	px = x + w - 1;
	py = y;
	if (status^=1)
		USL_DrawString("\x80");
	else
	{
		temp = fontcolor;
		fontcolor = backcolor;
		USL_DrawString("\x80");
		fontcolor = temp;
	}
}

char USL_RotateChar(char ch, int dir)
{
    static const char charSet[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ.,-!?0123456789";
    const int numChars = sizeof(charSet) / sizeof(char) - 1;
    int i;
    for(i = 0; i < numChars; i++)
    {
        if(ch == charSet[i]) break;
    }

    if(i == numChars) i = 0;

    i += dir;
    if(i < 0) i = numChars - 1;
    else if(i >= numChars) i = 0;
    return charSet[i];
}

///////////////////////////////////////////////////////////////////////////
//
//	US_LineInput() - Gets a line of user input at (x,y), the string defaults
//		to whatever is pointed at by def. Input is restricted to maxchars
//		chars or maxwidth pixels wide. If the user hits escape (and escok is
//		true), nothing is copied into buf, and false is returned. If the
//		user hits return, the current string is copied into buf, and true is
//		returned
//
///////////////////////////////////////////////////////////////////////////
boolean
US_LineInput(int x,int y,char *buf,const char *def,boolean escok,
				int maxchars,int maxwidth)
{
	boolean		redraw,
				cursorvis,cursormoved,
				done,result, checkkey;
	ScanCode	sc;
	char		c;
	char		s[MaxString],olds[MaxString];
	int         cursor,len;
	word		i,
				w,h,
				temp;
	longword	curtime, lasttime, lastdirtime, lastbuttontime, lastdirmovetime;
	ControlInfo ci;
	Direction   lastdir = dir_None;

	if (def)
		strcpy(s,def);
	else
		*s = '\0';
	*olds = '\0';
	cursor = (int) strlen(s);
	cursormoved = redraw = true;

	cursorvis = done = false;
	lasttime = lastdirtime = lastdirmovetime = GetTimeCount();
	lastbuttontime = lasttime + TickBase / 4;	// 250 ms => first button press accepted after 500 ms
	LastASCII = key_None;
	LastScan = sc_None;

	while (!done)
	{
		ReadAnyControl(&ci);

		if (cursorvis)
			USL_XORICursor(x,y,s,cursor);

		sc = LastScan;
		LastScan = sc_None;
		c = LastASCII;
		LastASCII = key_None;

		checkkey = true;
		curtime = GetTimeCount();

		// After each direction change accept the next change after 250 ms and then everz 125 ms
		if(ci.dir != lastdir || (curtime - lastdirtime > TickBase / 4 && curtime - lastdirmovetime > TickBase / 8))
		{
			if(ci.dir != lastdir)
			{
				lastdir = ci.dir;
				lastdirtime = curtime;
			}
            lastdirmovetime = curtime;

			switch(ci.dir)
			{
				case dir_West:
					if(cursor)
					{
						// Remove trailing whitespace if cursor is at end of string
						if(s[cursor] == ' ' && s[cursor + 1] == 0)
							s[cursor] = 0;
						cursor--;
					}
					cursormoved = true;
					checkkey = false;
					break;
				case dir_East:
					if(cursor >= MaxString - 1) break;

					if(!s[cursor])
					{
						USL_MeasureString(s,&w,&h);
						if(len >= maxchars || (maxwidth && w >= maxwidth)) break;

						s[cursor] = ' ';
						s[cursor + 1] = 0;
					}
					cursor++;
					cursormoved = true;
					checkkey = false;
					break;

				case dir_North:
					if(!s[cursor])
					{
						USL_MeasureString(s,&w,&h);
						if(len >= maxchars || (maxwidth && w >= maxwidth)) break;
						s[cursor + 1] = 0;
					}
					s[cursor] = USL_RotateChar(s[cursor], 1);
					redraw = true;
					checkkey = false;
					break;

				case dir_South:
					if(!s[cursor])
					{
						USL_MeasureString(s,&w,&h);
						if(len >= maxchars || (maxwidth && w >= maxwidth)) break;
						s[cursor + 1] = 0;
					}
					s[cursor] = USL_RotateChar(s[cursor], -1);
					redraw = true;
					checkkey = false;
					break;
				default:
					break;
			}
		}

		if((int)(curtime - lastbuttontime) > TickBase / 4)   // 250 ms
		{
			if(ci.button0)             // acts as return
			{
				strcpy(buf,s);
				done = true;
				result = true;
				checkkey = false;
			}
			if(ci.button1 && escok)    // acts as escape
			{
				done = true;
				result = false;
				checkkey = false;
			}
			if(ci.button2)             // acts as backspace
			{
				lastbuttontime = curtime;
				if(cursor)
				{
					strcpy(s + cursor - 1,s + cursor);
					cursor--;
					redraw = true;
				}
				cursormoved = true;
				checkkey = false;
			}
		}

		if(checkkey)
		{
			switch (sc)
			{
				case sc_LeftArrow:
					if (cursor)
						cursor--;
					c = key_None;
					cursormoved = true;
					break;
				case sc_RightArrow:
					if (s[cursor])
						cursor++;
					c = key_None;
					cursormoved = true;
					break;
				case sc_Home:
					cursor = 0;
					c = key_None;
					cursormoved = true;
					break;
				case sc_End:
					cursor = (int) strlen(s);
					c = key_None;
					cursormoved = true;
					break;

				case sc_Return:
					strcpy(buf,s);
					done = true;
					result = true;
					c = key_None;
					break;
				case sc_Escape:
					if (escok)
					{
						done = true;
						result = false;
					}
					c = key_None;
					break;

				case sc_BackSpace:
					if (cursor)
					{
						strcpy(s + cursor - 1,s + cursor);
						cursor--;
						redraw = true;
					}
					c = key_None;
					cursormoved = true;
					break;
				case sc_Delete:
					if (s[cursor])
					{
						memmove(s + cursor, s + cursor + 1, strlen(s + cursor + 1) + 1);

						redraw = true;
					}
					c = key_None;
					cursormoved = true;
					break;

				case SDLK_KP5: //0x4c:	// Keypad 5 // TODO: hmmm...
				case sc_UpArrow:
				case sc_DownArrow:
				case sc_PgUp:
				case sc_PgDn:
				case sc_Insert:
					c = key_None;
					break;
			}

			if (c)
			{
				len = (int) strlen(s);
				USL_MeasureString(s,&w,&h);

				if(isprint(c) && (len < MaxString - 1) && ((!maxchars) || (len < maxchars))
					&& ((!maxwidth) || (w < maxwidth)))
				{
					for (i = len + 1;i > cursor;i--)
						s[i] = s[i - 1];
					s[cursor++] = c;
					redraw = true;
				}
			}
		}

		if (redraw)
		{
			px = x;
			py = y;
			temp = fontcolor;
			fontcolor = backcolor;
			USL_DrawString(olds);
			fontcolor = (byte) temp;
			strcpy(olds,s);

			px = x;
			py = y;
			USL_DrawString(s);

			redraw = false;
		}

		if (cursormoved)
		{
			cursorvis = false;
			lasttime = curtime - TickBase;

			cursormoved = false;
		}
		if (curtime - lasttime > TickBase / 2)    // 500 ms
		{
			lasttime = curtime;

			cursorvis ^= true;
		}
		else SDL_Delay(5);
		if (cursorvis)
			USL_XORICursor(x,y,s,cursor);

		VW_UpdateScreen();
	}

	if (cursorvis)
		USL_XORICursor(x,y,s,cursor);
	if (!result)
	{
		px = x;
		py = y;
		USL_DrawString(olds);
	}
	VW_UpdateScreen();

	IN_ClearKeysDown();
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
// US_InitRndT - Initializes the pseudo random number generator.
//      If randomize is true, the seed will be initialized depending on the
//      current time
//
///////////////////////////////////////////////////////////////////////////
void US_InitRndT(int randomize)
{
    if(randomize)
        rndindex = (SDL_GetTicks() >> 4) & 0xff;
    else
        rndindex = 0;
}

///////////////////////////////////////////////////////////////////////////
//
// US_RndT - Returns the next 8-bit pseudo random number
//
///////////////////////////////////////////////////////////////////////////
int US_RndT()
{
    rndindex = (rndindex+1)&0xff;
    return rndtable[rndindex];
}
