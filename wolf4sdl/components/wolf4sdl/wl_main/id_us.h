//
//	ID Engine
//	ID_US.h - Header file for the User Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__ID_US__
#define	__ID_US__

#ifdef	__DEBUG__
#define	__DEBUG_UserMgr__
#endif

//#define	HELPTEXTLINKED

#define	MaxX	320
#define	MaxY	200

#define	MaxHelpLines	500

#define	MaxHighName	57
#define	MaxScores	7
typedef	struct
{
    char	name[MaxHighName + 1];
    int32_t	score;
    word	completed,episode;
} HighScore;

#define	MaxGameName		32
#define	MaxSaveGames	6
typedef	struct
{
    char	signature[4];
    word	*oldtest;
    boolean	present;
    char	name[MaxGameName + 1];
} SaveGame;

#define	MaxString	128	// Maximum input string size

typedef	struct
{
    int	x,y,
        w,h,
        px,py;
} WindowRec;	// Record used to save & restore screen windows

extern	boolean		ingame,		// Set by game code if a game is in progress
					loadedgame;	// Set if the current game was loaded
extern	word		PrintX,PrintY;	// Current printing location in the window
extern	word		WindowX,WindowY,// Current location of window
					WindowW,WindowH;// Current size of window

extern	void		(*USL_MeasureString)(const char *,word *,word *);
extern void			(*USL_DrawString)(const char *);

extern	boolean		(*USL_SaveGame)(int),(*USL_LoadGame)(int);
extern	void		(*USL_ResetGame)(void);
extern	SaveGame	Games[MaxSaveGames];
extern	HighScore	Scores[];

#define	US_HomeWindow()	{PrintX = WindowX; PrintY = WindowY;}

void            US_Startup(void);
void            US_Shutdown(void);
void			US_TextScreen(void),
				US_UpdateTextScreen(void),
				US_FinishTextScreen(void);
void			US_DrawWindow(word x,word y,word w,word h);
void			US_CenterWindow(word,word);
void			US_SaveWindow(WindowRec *win),
				US_RestoreWindow(WindowRec *win);
void 			US_ClearWindow(void);
void			US_SetPrintRoutines(void (*measure)(const char *,word *,word *),
									void (*print)(const char *));
void			US_PrintCentered(const char *s),
				US_CPrint(const char *s),
				US_CPrintLine(const char *s),
				US_Print(const char *s);
void			US_Printf(const char *formatStr, ...);
void			US_CPrintf(const char *formatStr, ...);

void			US_PrintUnsigned(longword n);
void			US_PrintSigned(int32_t n);
void			US_StartCursor(void),
				US_ShutCursor(void);
void			US_CheckHighScore(int32_t score,word other);
void			US_DisplayHighScores(int which);
extern	boolean	US_UpdateCursor(void);
boolean         US_LineInput(int x,int y,char *buf,const char *def,boolean escok,
                             int maxchars,int maxwidth);

void	        USL_PrintInCenter(const char *s,Rect r);
char 	        *USL_GiveSaveName(word game);

void            US_InitRndT(int randomize);
int             US_RndT();

#endif
