// WL_TEXT.C

#include "wl_def.h"

/*
=============================================================================

TEXT FORMATTING COMMANDS
------------------------
^C<hex digit>           Change text color
^E[enter]               End of layout (all pages)
^G<y>,<x>,<pic>[enter]  Draw a graphic and push margins
^P[enter]               start new page, must be the first chars in a layout
^L<x>,<y>[ENTER]        Locate to a specific spot, x in pixels, y in lines

=============================================================================
*/

/*
=============================================================================

                                                 LOCAL CONSTANTS

=============================================================================
*/

#ifndef SPEAR

#define BACKCOLOR       0x11


#define WORDLIMIT       80
#define FONTHEIGHT      10
#define TOPMARGIN       16
#define BOTTOMMARGIN    32
#define LEFTMARGIN      16
#define RIGHTMARGIN     16
#define PICMARGIN       8
#define TEXTROWS        ((200-TOPMARGIN-BOTTOMMARGIN)/FONTHEIGHT)
#define SPACEWIDTH      7
#define SCREENPIXWIDTH  320
#define SCREENMID       (SCREENPIXWIDTH/2)

/*
=============================================================================

                                LOCAL VARIABLES

=============================================================================
*/

static int pagenum;
static int numpages;

static unsigned leftmargin[TEXTROWS];
static unsigned rightmargin[TEXTROWS];
static char*    text;
static unsigned rowon;

static int     picx;
static int     picy;
static int     picnum;
static int     picdelay;
static boolean layoutdone;

//===========================================================================

#ifndef JAPAN
/*
=====================
=
= RipToEOL
=
=====================
*/

void RipToEOL (void)
{
    while (*text++ != '\n')         // scan to end of line
        ;
}


/*
=====================
=
= ParseNumber
=
=====================
*/

int ParseNumber (void)
{
    char  ch;
    char  num[80];
    char *numptr;

    //
    // scan until a number is found
    //
    ch = *text;
    while (ch < '0' || ch >'9')
        ch = *++text;

    //
    // copy the number out
    //
    numptr = num;
    do
    {
        *numptr++ = ch;
        ch = *++text;
    } while (ch >= '0' && ch <= '9');
    *numptr = 0;

    return atoi (num);
}



/*
=====================
=
= ParsePicCommand
=
= Call with text pointing just after a ^P
= Upon exit text points to the start of next line
=
=====================
*/

void ParsePicCommand (void)
{
    picy=ParseNumber();
    picx=ParseNumber();
    picnum=ParseNumber();
    RipToEOL ();
}


void ParseTimedCommand (void)
{
    picy=ParseNumber();
    picx=ParseNumber();
    picnum=ParseNumber();
    picdelay=ParseNumber();
    RipToEOL ();
}


/*
=====================
=
= TimedPicCommand
=
= Call with text pointing just after a ^P
= Upon exit text points to the start of next line
=
=====================
*/

void TimedPicCommand (void)
{
    ParseTimedCommand ();

    //
    // update the screen, and wait for time delay
    //
    VW_UpdateScreen ();

    //
    // wait for time
    //
    Delay(picdelay);

    //
    // draw pic
    //
    VWB_DrawPic (picx&~7,picy,picnum);
}


/*
=====================
=
= HandleCommand
=
=====================
*/

void HandleCommand (void)
{
    int     i,margin,top,bottom;
    int     picwidth,picheight,picmid;

    switch (toupper(*++text))
    {
        case 'B':
            picy=ParseNumber();
            picx=ParseNumber();
            picwidth=ParseNumber();
            picheight=ParseNumber();
            VWB_Bar(picx,picy,picwidth,picheight,BACKCOLOR);
            RipToEOL();
            break;
        case ';':               // comment
            RipToEOL();
            break;
        case 'P':               // ^P is start of next page, ^E is end of file
        case 'E':
            layoutdone = true;
            text--;             // back up to the '^'
            break;

        case 'C':               // ^c<hex digit> changes text color
            i = toupper(*++text);
            if (i>='0' && i<='9')
                fontcolor = i-'0';
            else if (i>='A' && i<='F')
                fontcolor = i-'A'+10;

            fontcolor *= 16;
            i = toupper(*++text);
            if (i>='0' && i<='9')
                fontcolor += i-'0';
            else if (i>='A' && i<='F')
                fontcolor += i-'A'+10;
            text++;
            break;

        case '>':
            px = 160;
            text++;
            break;

        case 'L':
            py=ParseNumber();
            rowon = (py-TOPMARGIN)/FONTHEIGHT;
            py = TOPMARGIN+rowon*FONTHEIGHT;
            px=ParseNumber();
            while (*text++ != '\n')         // scan to end of line
                ;
            break;

        case 'T':               // ^Tyyy,xxx,ppp,ttt waits ttt tics, then draws pic
            TimedPicCommand ();
            break;

        case 'G':               // ^Gyyy,xxx,ppp draws graphic
            ParsePicCommand ();
            VWB_DrawPic (picx&~7,picy,picnum);
            picwidth = pictable[picnum-STARTPICS].width;
            picheight = pictable[picnum-STARTPICS].height;
            //
            // adjust margins
            //
            picmid = picx + picwidth/2;
            if (picmid > SCREENMID)
                margin = picx-PICMARGIN;                        // new right margin
            else
                margin = picx+picwidth+PICMARGIN;       // new left margin

            top = (picy-TOPMARGIN)/FONTHEIGHT;
            if (top<0)
                top = 0;
            bottom = (picy+picheight-TOPMARGIN)/FONTHEIGHT;
            if (bottom>=TEXTROWS)
                bottom = TEXTROWS-1;

            for (i=top;i<=bottom;i++)
                if (picmid > SCREENMID)
                    rightmargin[i] = margin;
                else
                    leftmargin[i] = margin;

            //
            // adjust this line if needed
            //
            if (px < (int) leftmargin[rowon])
                px = leftmargin[rowon];
            break;
    }
}


/*
=====================
=
= NewLine
=
=====================
*/

void NewLine (void)
{
    char    ch;

    if (++rowon == TEXTROWS)
    {
        //
        // overflowed the page, so skip until next page break
        //
        layoutdone = true;
        do
        {
            if (*text == '^')
            {
                ch = toupper(*(text+1));
                if (ch == 'E' || ch == 'P')
                {
                    layoutdone = true;
                    return;
                }
            }
            text++;
        } while (1);
    }
    px = leftmargin[rowon];
    py+= FONTHEIGHT;
}



/*
=====================
=
= HandleCtrls
=
=====================
*/

void HandleCtrls (void)
{
    char    ch;

    ch = *text++;                   // get the character and advance

    if (ch == '\n')
    {
        NewLine ();
        return;
    }
}


/*
=====================
=
= HandleWord
=
=====================
*/

void HandleWord (void)
{
    char    wword[WORDLIMIT];
    int     wordindex;
    word    wwidth,wheight,newpos;


    //
    // copy the next word into [word]
    //
    wword[0] = *text++;
    wordindex = 1;
    while (*text>32)
    {
        wword[wordindex] = *text++;
        if (++wordindex == WORDLIMIT)
            Quit ("PageLayout: Word limit exceeded");
    }
    wword[wordindex] = 0;            // stick a null at end for C

    //
    // see if it fits on this line
    //
    VW_MeasurePropString (wword,&wwidth,&wheight);

    while (px+wwidth > (int) rightmargin[rowon])
    {
        NewLine ();
        if (layoutdone)
            return;         // overflowed page
    }

    //
    // print it
    //
    newpos = px+wwidth;
    VWB_DrawPropString (wword);
    px = newpos;

    //
    // suck up any extra spaces
    //
    while (*text == ' ')
    {
        px += SPACEWIDTH;
        text++;
    }
}

/*
=====================
=
= PageLayout
=
= Clears the screen, draws the pics on the page, and word wraps the text.
= Returns a pointer to the terminating command
=
=====================
*/

void PageLayout (boolean shownumber)
{
    int     i,oldfontcolor;
    char    ch;

    oldfontcolor = fontcolor;

    fontcolor = 0;

    //
    // clear the screen
    //
    VWB_Bar (0,0,320,200,BACKCOLOR);
    VWB_DrawPic (0,0,H_TOPWINDOWPIC);
    VWB_DrawPic (0,8,H_LEFTWINDOWPIC);
    VWB_DrawPic (312,8,H_RIGHTWINDOWPIC);
    VWB_DrawPic (8,176,H_BOTTOMINFOPIC);


    for (i=0; i<TEXTROWS; i++)
    {
        leftmargin[i] = LEFTMARGIN;
        rightmargin[i] = SCREENPIXWIDTH-RIGHTMARGIN;
    }

    px = LEFTMARGIN;
    py = TOPMARGIN;
    rowon = 0;
    layoutdone = false;

    //
    // make sure we are starting layout text (^P first command)
    //
    while (*text <= 32)
        text++;

    if (*text != '^' || toupper(*++text) != 'P')
        Quit ("PageLayout: Text not headed with ^P");

    while (*text++ != '\n')
        ;


    //
    // process text stream
    //
    do
    {
        ch = *text;

        if (ch == '^')
            HandleCommand ();
        else
            if (ch == 9)
            {
                px = (px+8)&0xf8;
                text++;
            }
            else if (ch <= 32)
                HandleCtrls ();
            else
                HandleWord ();

    } while (!layoutdone);

    pagenum++;

    if (shownumber)
    {
#ifdef SPANISH
        sprintf(str, "Hoja %d de %d", pagenum, numpages);
        px = 208;
#else
        sprintf(str, "pg %d of %d", pagenum, numpages);
        px = 213;
#endif
        py = 183;
        fontcolor = 0x4f;                          //12^BACKCOLOR;

        VWB_DrawPropString (str);
    }

    fontcolor = oldfontcolor;
}

//===========================================================================

/*
=====================
=
= BackPage
=
= Scans for a previous ^P
=
=====================
*/

void BackPage (void)
{
    pagenum--;
    do
    {
        text--;
        if (*text == '^' && toupper(*(text+1)) == 'P')
            return;
    } while (1);
}


//===========================================================================


/*
=====================
=
= CacheLayoutGraphics
=
= Scans an entire layout file (until a ^E) marking all graphics used, and
= counting pages, then caches the graphics in
=
=====================
*/
void CacheLayoutGraphics (void)
{
    char    *bombpoint, *textstart;
    char    ch;

    textstart = text;
    bombpoint = text+30000;
    numpages = pagenum = 0;

    do
    {
        if (*text == '^')
        {
            ch = toupper(*++text);
            if (ch == 'P')          // start of a page
                numpages++;
            if (ch == 'E')          // end of file, so load graphics and return
            {
#ifndef SPEAR
                CA_CacheGrChunk(H_TOPWINDOWPIC);
                CA_CacheGrChunk(H_LEFTWINDOWPIC);
                CA_CacheGrChunk(H_RIGHTWINDOWPIC);
                CA_CacheGrChunk(H_BOTTOMINFOPIC);
#endif
                //                              CA_CacheMarks ();
                text = textstart;
                return;
            }
            if (ch == 'G')          // draw graphic command, so mark graphics
            {
                ParsePicCommand ();
                CA_CacheGrChunk (picnum);
            }
            if (ch == 'T')          // timed draw graphic command, so mark graphics
            {
                ParseTimedCommand ();
                CA_CacheGrChunk (picnum);
            }
        }
        else
            text++;

    } while (text<bombpoint);

    Quit ("CacheLayoutGraphics: No ^E to terminate file!");
}
#endif


/*
=====================
=
= ShowArticle
=
=====================
*/

#ifdef JAPAN
void ShowArticle (int which)
#else
void ShowArticle (char *article)
#endif
{
#ifdef JAPAN
    int snames[10] = {
        H_HELP1PIC,
        H_HELP2PIC,
        H_HELP3PIC,
        H_HELP4PIC,
        H_HELP5PIC,
        H_HELP6PIC,
        H_HELP7PIC,
        H_HELP8PIC,
        H_HELP9PIC,
        H_HELP10PIC};
    int enames[14] = {
        0,0,
#ifndef JAPDEMO
        C_ENDGAME1APIC,
        C_ENDGAME1BPIC,
        C_ENDGAME2APIC,
        C_ENDGAME2BPIC,
        C_ENDGAME3APIC,
        C_ENDGAME3BPIC,
        C_ENDGAME4APIC,
        C_ENDGAME4BPIC,
        C_ENDGAME5APIC,
        C_ENDGAME5BPIC,
        C_ENDGAME6APIC,
        C_ENDGAME6BPIC
#endif
    };
#endif
    unsigned    oldfontnumber;
    boolean     newpage,firstpage;
    ControlInfo ci;

#ifdef JAPAN
    pagenum = 1;
    if (!which)
        numpages = 10;
    else
        numpages = 2;
#else
    text = article;
    oldfontnumber = fontnumber;
    fontnumber = 0;
    CA_CacheGrChunk(STARTFONT);
    VWB_Bar (0,0,320,200,BACKCOLOR);
    CacheLayoutGraphics ();
#endif

    newpage = true;
    firstpage = true;

    do
    {
        if (newpage)
        {
            newpage = false;
#ifdef JAPAN
            if (!which)
                CA_CacheScreen(snames[pagenum - 1]);
            else
                CA_CacheScreen(enames[which*2 + pagenum - 1]);
#else
            PageLayout (true);
#endif
            VW_UpdateScreen ();
            if (firstpage)
            {
                VL_FadeIn(0,255,gamepal,10);
                firstpage = false;
            }
        }
        SDL_Delay(5);

        LastScan = 0;
        ReadAnyControl(&ci);
        Direction dir = ci.dir;
        switch(dir)
        {
            case dir_North:
            case dir_South:
                break;

            default:
                if(ci.button0) dir = dir_South;
                switch(LastScan)
                {
                    case sc_UpArrow:
                    case sc_PgUp:
                    case sc_LeftArrow:
                        dir = dir_North;
                        break;

                    case sc_Enter:
                    case sc_DownArrow:
                    case sc_PgDn:
                    case sc_RightArrow:
                        dir = dir_South;
                        break;
                }
                break;
        }

        switch(dir)
        {
            case dir_North:
            case dir_West:
                if (pagenum>1)
                {
#ifndef JAPAN
                    BackPage ();
                    BackPage ();
#else
                  pagenum--;
#endif
                    newpage = true;
                }
                TicDelay(20);
                break;

            case dir_South:
            case dir_East:
                if (pagenum<numpages)
                {
                    newpage = true;
#ifdef JAPAN
                    pagenum++;
#endif
                }
                TicDelay(20);
                break;
            default:
                break;
        }
    } while (LastScan != sc_Escape && !ci.button1);

    IN_ClearKeysDown ();
    fontnumber = oldfontnumber;
}


//===========================================================================

#ifndef JAPAN
#ifdef ARTSEXTERN
int     endextern = T_ENDART1;
#ifndef SPEAR
int     helpextern = T_HELPART;
#endif
#endif
char helpfilename[13] = "HELPART.",
    endfilename[13] = "ENDART1.";
#endif

/*
=================
=
= HelpScreens
=
=================
*/
#ifndef SPEAR
void HelpScreens (void)
{
    int     artnum;
    char    *text;
#ifndef ARTSEXTERN
    memptr  layout;
#endif


    //      CA_UpLevel ();
    //      MM_SortMem ();
#ifdef JAPAN
    ShowArticle (0);
    VW_FadeOut();
    FreeMusic ();
    CA_DownLevel ();
    MM_SortMem ();
#else

#ifdef ARTSEXTERN
    artnum = helpextern;
    CA_CacheGrChunk (artnum);
    text = (char *)grsegs[artnum];
#else
    CA_LoadFile (helpfilename,&layout);
    text = (char *)layout;
#endif

    ShowArticle (text);

#ifdef ARTSEXTERN
    UNCACHEGRCHUNK(artnum);
#else
    free(layout);
#endif

    VW_FadeOut();

    FreeMusic ();
#endif
}
#endif

//
// END ARTICLES
//
void EndText (void)
{
    int     artnum;
    char    *text;
#ifndef ARTSEXTERN
    memptr  layout;
#endif

    ClearMemory ();

#ifdef JAPAN
    ShowArticle(gamestate.episode + 1);

    VW_FadeOut();

    SETFONTCOLOR(0,15);
    IN_ClearKeysDown();
    if (MousePresent && IN_IsInputGrabbed())
        IN_CenterMouse();  // Clear accumulated mouse movement

    FreeMusic ();
#else



#ifdef ARTSEXTERN
    artnum = endextern+gamestate.episode;
    CA_CacheGrChunk (artnum);
    text = (char *)grsegs[artnum];
#else
    endfilename[6] = '1'+gamestate.episode;
    CA_LoadFile (endfilename,&layout);
    text = (char *)layout;
#endif

    ShowArticle (text);

#ifdef ARTSEXTERN
    UNCACHEGRCHUNK(artnum);
#else
    free(layout);
#endif


    VW_FadeOut();
    SETFONTCOLOR(0,15);
    IN_ClearKeysDown();
    if (MousePresent && IN_IsInputGrabbed())
        IN_CenterMouse();  // Clear accumulated mouse movement

    FreeMusic ();
#endif
}
#endif
