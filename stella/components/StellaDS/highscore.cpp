// =====================================================================================================
// Stella DS/DSi Pheonix Edition - Improved Version by Dave Bernazzani (wavemotion)
//
// Copyright (c) 2020-2026 by Dave Bernazzani
//
// Copying and distribution of this emulator, it's source code and associated 
// readme files, with or without modification, are permitted in any medium without 
// royalty provided this copyright notice is used and wavemotion-dave (Phoenix-Edition),
// Alekmaul (original port) are thanked profusely along with the entire Stella Team.
//
// The StellaDS emulator is offered as-is, without any warranty.
// =====================================================================================================
#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <dirent.h>
#include <unistd.h>
#include "StellaDS.h"
#include "highscore.h"
#include "Console.hxx"
#include "Cart.hxx"
#include "bgHighScore.h"
#include "bgBottom.h"
#include "printf.h"

#define MAX_HS_GAMES    1000
#define HS_VERSION      0x0001

#define HS_OPT_SORTMASK  0x0003
#define HS_OPT_SORTLOW   0x0001
#define HS_OPT_SORTTIME  0x0002
#define HS_OPT_SORTASCII 0x0003

#define HS_FILE     "/data/StellaDS.hi"

u32 hs_file_offset = 0;     // So we know where to write-back the highscore entry
u8  header_dirty_flag = 0;  // So we know if we should write the header (initials changed)

#pragma pack(1)

struct score_t 
{
    char    initials[4];
    char    score[7];
    char    reserved[5];
    uint16  year;
    uint8   month;
    uint8   day;
};

struct highscore_t
{
    char    md5sum[33];
    char    notes[21];
    uint16  options;
    struct score_t scores[10];
} highscore;

struct highscore_header_t
{
    uint16 version;
    char   last_initials[4];
} highscore_header;


extern int bg0, bg0b,bg1b;


void highscore_init(void) 
{
    u8 create_defaults = 0;
    FILE *fp;
    
    strcpy(highscore_header.last_initials, "   ");
    
    // --------------------------------------------------------------
    // See if the StellaDS high score file exists... if so, read it!
    // --------------------------------------------------------------
    fp = fopen(HS_FILE, "rb");
    if (fp != NULL)
    {
        fread(&highscore_header, sizeof(highscore_header), 1, fp);
        fclose(fp);
        
        if (highscore_header.version != HS_VERSION) create_defaults = 1;
    }
    else
    {
        create_defaults = 1;
    }
    
    if (create_defaults)  // Doesn't exist yet or is invalid... create defaults and save it...
    {
        DIR* dir = opendir("/data");
        if (dir)
        {
            closedir(dir);  // Directory exists... close it out and move on.
        }
        else
        {
            mkdir("/data", 0777);   // Otherwise create the directory...
        }
        
        highscore_header.version = HS_VERSION;
        strcpy(highscore_header.last_initials, "   ");
        
        strcpy(highscore.md5sum, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
        strcpy(highscore.notes, "                    ");
        highscore.options = 0x0000;
        for (int j=0; j<10; j++)
        {
            strcpy(highscore.scores[j].score, "000000");
            strcpy(highscore.scores[j].initials, "   ");
            strcpy(highscore.scores[j].reserved, "     ");                
            highscore.scores[j].year = 0;
            highscore.scores[j].month = 0;
            highscore.scores[j].day = 0;
        }
        
        // Write the above blank record out to all slots...
        
        FILE *fp = fopen(HS_FILE, "wb");
        
        fwrite(&highscore_header, sizeof(highscore_header), 1, fp);
        
        for (int i=0; i<MAX_HS_GAMES; i++)
        {
            fwrite(&highscore, sizeof(highscore), 1, fp);
        }
        
        fclose(fp);        
    }
}


void highscore_save(void) 
{
    FILE *fp = fopen(HS_FILE, "rb+");   // Open for read/write
    if (header_dirty_flag)
    {
        highscore_header.version = HS_VERSION;
        header_dirty_flag = 0;
        fwrite(&highscore_header, sizeof(highscore_header), 1, fp);
    }
    fseek(fp, hs_file_offset, SEEK_SET);
    fwrite(&highscore, sizeof(highscore), 1, fp);
    fclose(fp);
}

struct score_t score_entry;
char hs_line[33];
char md5[33];

void highscore_showoptions(uint16 options)
{
    if ((options & HS_OPT_SORTMASK) == HS_OPT_SORTLOW)
    {
        dsPrintValue(22,5,0, (char*)"[LOWSC]");
    }
    else if ((options & HS_OPT_SORTMASK) == HS_OPT_SORTTIME)
    {
        dsPrintValue(22,5,0, (char*)"[TIME] ");
    }
    else if ((options & HS_OPT_SORTMASK) == HS_OPT_SORTASCII)
    {
        dsPrintValue(22,5,0, (char*)"[ALPHA]");
    }
    else
    {   
        dsPrintValue(22,5,0, (char*)"       ");    
    }
}

void show_scores(bool bShowLegend)
{
    dsPrintValue(3,5,0, (char*)highscore.notes);
    for (int i=0; i<10; i++)
    {
        if ((highscore.options & HS_OPT_SORTMASK) == HS_OPT_SORTTIME)
        {
            sprintf(hs_line, "%04d-%02d-%02d   %-3s   %c%c:%c%c.%c%c", highscore.scores[i].year, highscore.scores[i].month,highscore.scores[i].day, 
                                                             highscore.scores[i].initials, highscore.scores[i].score[0], highscore.scores[i].score[1],
                                                             highscore.scores[i].score[2], highscore.scores[i].score[3], highscore.scores[i].score[4],
                                                             highscore.scores[i].score[5]);
        }
        else
        {
            sprintf(hs_line, "%04d-%02d-%02d   %-3s   %-6s  ", highscore.scores[i].year, highscore.scores[i].month,highscore.scores[i].day, 
                                                               highscore.scores[i].initials, highscore.scores[i].score);
        }
        dsPrintValue(3,6+i, 0, hs_line);
    }
    
    if (bShowLegend)
    {
        dsPrintValue(3,20,0, (char*)"PRESS X FOR NEW HI SCORE     ");
        dsPrintValue(3,21,0, (char*)"PRESS Y FOR NOTES/OPTIONS    ");
        dsPrintValue(3,22,0, (char*)"PRESS B TO EXIT              ");
        dsPrintValue(3,23,0, (char*)"SCORES AUTO SORT AFTER ENTRY ");
        dsPrintValue(3,17,0, (char*)"                             ");
    }
    highscore_showoptions(highscore.options);
}

char cmp1[21];
char cmp2[21];
void highscore_sort(void)
{
    // Bubblesort!!
    for (int i=0; i<9; i++)
    {
        for (int j=0; j<9; j++)
        {
            if (((highscore.options & HS_OPT_SORTMASK) == HS_OPT_SORTLOW) || ((highscore.options & HS_OPT_SORTMASK) == HS_OPT_SORTTIME))
            {
                if (strcmp(highscore.scores[j+1].score, "000000") == 0)
                     strcpy(cmp1, "999999");
                else 
                    strcpy(cmp1, highscore.scores[j+1].score);
                if (strcmp(highscore.scores[j].score, "000000") == 0)
                     strcpy(cmp2, "999999");
                else 
                    strcpy(cmp2, highscore.scores[j].score);
                if (strcmp(cmp1, cmp2) < 0)
                {
                    // Swap...
                    memcpy(&score_entry, &highscore.scores[j], sizeof(score_entry));
                    memcpy(&highscore.scores[j], &highscore.scores[j+1], sizeof(score_entry));
                    memcpy(&highscore.scores[j+1], &score_entry, sizeof(score_entry));
                }
            }
            else if ((highscore.options & HS_OPT_SORTMASK) == HS_OPT_SORTASCII)
            {
                if (strcmp(highscore.scores[j+1].score, "000000") == 0)
                     strcpy(cmp1, "------");
                else 
                    strcpy(cmp1, highscore.scores[j+1].score);
                if (strcmp(highscore.scores[j].score, "000000") == 0)
                     strcpy(cmp2, "------");
                else 
                    strcpy(cmp2, highscore.scores[j].score);
                
                if (strcmp(cmp1, cmp2) > 0)
                {
                    // Swap...
                    memcpy(&score_entry, &highscore.scores[j], sizeof(score_entry));
                    memcpy(&highscore.scores[j], &highscore.scores[j+1], sizeof(score_entry));
                    memcpy(&highscore.scores[j+1], &score_entry, sizeof(score_entry));
                }
            }
            else
            {
                if (strcmp(highscore.scores[j+1].score, highscore.scores[j].score) > 0)
                {
                    // Swap...
                    memcpy(&score_entry, &highscore.scores[j], sizeof(score_entry));
                    memcpy(&highscore.scores[j], &highscore.scores[j+1], sizeof(score_entry));
                    memcpy(&highscore.scores[j+1], &score_entry, sizeof(score_entry));
                }
            }
        }
    }    
}

void highscore_entry(void)
{
    char bEntryDone = 0;
    char blink=0;
    unsigned short entry_idx=0;
    char dampen=0;
    time_t unixTime = time(NULL);
    struct tm* timeStruct = gmtime((const time_t *)&unixTime);
    
    dsPrintValue(3,20,0, (char*)"UP/DN/LEFT/RIGHT ENTER SCORE");
    dsPrintValue(3,21,0, (char*)"PRESS START TO SAVE SCORE   ");
    dsPrintValue(3,22,0, (char*)"PRESS SELECT TO CANCEL      ");
    dsPrintValue(3,23,0, (char*)"                            ");

    strcpy(score_entry.score, "000000");
    strcpy(score_entry.initials, highscore_header.last_initials);
    score_entry.year  = timeStruct->tm_year +1900;
    score_entry.month = timeStruct->tm_mon+1;
    score_entry.day   = timeStruct->tm_mday;
    while (!bEntryDone)
    {
        swiWaitForVBlank();
        if (keysCurrent() & KEY_SELECT) {bEntryDone=1;}

        if (keysCurrent() & KEY_START) 
        {
            // If last initials changed... force it to write on next save
            if (strcmp(highscore_header.last_initials, score_entry.initials) != 0)
            {
                strcpy(highscore_header.last_initials, score_entry.initials);
                header_dirty_flag = 1;
            }
            memcpy(&highscore.scores[9], &score_entry, sizeof(score_entry));
            strcpy(highscore.md5sum, md5);                    
            highscore_sort();
            highscore_save();
            bEntryDone=1;
        }

        if (dampen == 0)
        {
            if ((keysCurrent() & KEY_RIGHT) || (keysCurrent() & KEY_A))
            {
                if (entry_idx < 8) entry_idx++; 
                blink=25;
                dampen=15;
            }

            if (keysCurrent() & KEY_LEFT)  
            {
                if (entry_idx > 0) entry_idx--; 
                blink=25;
                dampen=15;
            }

            if (keysCurrent() & KEY_UP)
            {
                if (entry_idx < 3) // This is the initials
                {
                    if (score_entry.initials[entry_idx] == ' ')
                        score_entry.initials[entry_idx] = 'A';
                    else if (score_entry.initials[entry_idx] == 'Z')
                        score_entry.initials[entry_idx] = ' ';
                    else score_entry.initials[entry_idx]++;
                }
                else    // This is the score...
                {
                    if ((highscore.options & HS_OPT_SORTMASK) == HS_OPT_SORTASCII)
                    {
                        if (score_entry.score[entry_idx-3] == ' ')
                            score_entry.score[entry_idx-3] = 'A';
                        else if (score_entry.score[entry_idx-3] == 'Z')
                            score_entry.score[entry_idx-3] = '0';
                        else if (score_entry.score[entry_idx-3] == '9')
                            score_entry.score[entry_idx-3] = ' ';
                        else score_entry.score[entry_idx-3]++;
                    }
                    else
                    {
                        score_entry.score[entry_idx-3]++;
                        if (score_entry.score[entry_idx-3] > '9') score_entry.score[entry_idx-3] = '0';
                    }
                }
                blink=0;
                dampen=10;
            }

            if (keysCurrent() & KEY_DOWN)
            {
                if (entry_idx < 3) // // This is the initials
                {
                    if (score_entry.initials[entry_idx] == ' ')
                        score_entry.initials[entry_idx] = 'Z';
                    else if (score_entry.initials[entry_idx] == 'A')
                        score_entry.initials[entry_idx] = ' ';
                    else score_entry.initials[entry_idx]--;
                }
                else   // This is the score...
                {
                    if ((highscore.options & HS_OPT_SORTMASK) == HS_OPT_SORTASCII)
                    {
                        if (score_entry.score[entry_idx-3] == ' ')
                            score_entry.score[entry_idx-3] = '9';
                        else if (score_entry.score[entry_idx-3] == '0')
                            score_entry.score[entry_idx-3] = 'Z';
                        else if (score_entry.score[entry_idx-3] == 'A')
                            score_entry.score[entry_idx-3] = ' ';
                        else score_entry.score[entry_idx-3]--;
                    }
                    else
                    {
                        score_entry.score[entry_idx-3]--;
                        if (score_entry.score[entry_idx-3] < '0') score_entry.score[entry_idx-3] = '9';
                    }
                }
                blink=0;
                dampen=10;
            }
        }
        else
        {
            dampen--;
        }

        sprintf(hs_line, "%04d-%02d-%02d   %-3s   %-6s", score_entry.year, score_entry.month, score_entry.day, score_entry.initials, score_entry.score);
        if ((++blink % 60) > 30)
        {
            if (entry_idx < 3)
                hs_line[13+entry_idx] = '_';
            else
                hs_line[16+entry_idx] = '_';
        }
        dsPrintValue(3,17, 0, (char*)hs_line);
    }
    
    show_scores(true);
}

void highscore_options(char *md5)
{
    uint16 options = 0x0000;
    static char notes[21];
    char bEntryDone = 0;
    char blink=0;
    unsigned short entry_idx=0;
    char dampen=0;
    
    dsPrintValue(3,20,0, (char*)"UP/DN/LEFT/RIGHT ENTER NOTES");
    dsPrintValue(3,21,0, (char*)"X=TOGGLE SORT, L+R=CLR SCORE");
    dsPrintValue(3,22,0, (char*)"PRESS START TO SAVE OPTIONS ");
    dsPrintValue(3,23,0, (char*)"PRESS SELECT TO CANCEL      ");
    dsPrintValue(3,17,0, (char*)"NOTE: ");

    strcpy(notes, highscore.notes);
    options = highscore.options;
    
    while (!bEntryDone)
    {
        swiWaitForVBlank();
        if (keysCurrent() & KEY_SELECT) {bEntryDone=1;}

        if (keysCurrent() & KEY_START) 
        {
            strcpy(highscore.notes, notes);
            highscore.options = options;
            strcpy(highscore.md5sum, md5);
            highscore_sort();
            highscore_save();
            bEntryDone=1;
        }

        if (dampen == 0)
        {
            if ((keysCurrent() & KEY_RIGHT) || (keysCurrent() & KEY_A))
            {
                if (entry_idx < 19) entry_idx++; 
                blink=25;
                dampen=15;
            }

            if (keysCurrent() & KEY_LEFT)  
            {
                if (entry_idx > 0) entry_idx--; 
                blink=25;
                dampen=15;
            }

            if (keysCurrent() & KEY_UP)
            {
                if (notes[entry_idx] == ' ')
                    notes[entry_idx] = 'A';
                else if (notes[entry_idx] == 'Z')
                    notes[entry_idx] = '0';
                else if (notes[entry_idx] == '9')
                    notes[entry_idx] = ' ';
                else notes[entry_idx]++;
                blink=0;
                dampen=10;
            }

            if (keysCurrent() & KEY_DOWN)
            {
                if (notes[entry_idx] == ' ')
                    notes[entry_idx] = '9';
                else if (notes[entry_idx] == '0')
                    notes[entry_idx] = 'Z';
                else if (notes[entry_idx] == 'A')
                    notes[entry_idx] = ' ';
                else notes[entry_idx]--;
                blink=0;
                dampen=10;
            }

            if (keysCurrent() & KEY_X)  
            {
                if ((options & HS_OPT_SORTMASK) == HS_OPT_SORTLOW)
                {
                    options &= (uint16)~HS_OPT_SORTMASK;
                    options |= HS_OPT_SORTTIME;
                }
                else if ((options & HS_OPT_SORTMASK) == HS_OPT_SORTTIME)
                {
                    options &= (uint16)~HS_OPT_SORTMASK;
                    options |= HS_OPT_SORTASCII;
                }
                else if ((options & HS_OPT_SORTMASK) == HS_OPT_SORTASCII)
                {
                    options &= (uint16)~HS_OPT_SORTMASK;
                }
                else
                {
                    options |= (uint16)HS_OPT_SORTLOW;
                }
                highscore_showoptions(options);
                dampen=15;
            }
            
            // Clear the entire game of scores... 
            if ((keysCurrent() & KEY_L) && (keysCurrent() & KEY_R))
            {
                highscore.options = 0x0000;
                strcpy(highscore.notes, "                    ");
                strcpy(notes, "                    ");                
                for (int j=0; j<10; j++)
                {
                    strcpy(highscore.scores[j].score, "000000");
                    strcpy(highscore.scores[j].initials, "   ");
                    strcpy(highscore.scores[j].reserved, "    ");
                    highscore.scores[j].year = 0;
                    highscore.scores[j].month = 0;
                    highscore.scores[j].day = 0;
                }
                show_scores(false);
                highscore_save();                    
            }            
        }
        else
        {
            dampen--;
        }

        sprintf(hs_line, "%-20s", notes);
        if ((++blink % 60) > 30)
        {
            hs_line[entry_idx] = '_';
        }
        dsPrintValue(9,17, 0, (char*)hs_line);
    }
    
    show_scores(true);
}

void highscore_display(void) 
{
    char bDone = 0;

    decompress(bgHighScoreTiles, bgGetGfxPtr(bg0b), LZ77Vram);
    decompress(bgHighScoreMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
    dmaCopy((void *) bgHighScorePal,(u16*) BG_PALETTE_SUB,256*2);
    unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
    dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
    swiWaitForVBlank();
    
    // ---------------------------------------------------------------------------------
    // Get the current CART md5 so we can search for it in our High Score database...
    // ---------------------------------------------------------------------------------
    strcpy(md5, myCartInfo.md5);
    
    FILE *fp = fopen(HS_FILE, "rb");
    
    fread(&highscore_header, sizeof(highscore_header), 1, fp);
    hs_file_offset = sizeof(highscore_header);
    
    for (int i=0; i<MAX_HS_GAMES; i++)
    {
        fread(&highscore, sizeof(highscore), 1, fp);
        if (strcmp(md5, highscore.md5sum) == 0)
        {
            break;  // Found the game CRC - use this slot
        }
        if (strcmp("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", highscore.md5sum) == 0)
        {
            break;  // First blank entry can be used
        }
        
        hs_file_offset += sizeof(highscore); // Tells us where to write this entry back to the high score file
    }
    fclose(fp);    

    show_scores(true);

    while (!bDone)
    {
        if (keysCurrent() & KEY_A) bDone=1;
        if (keysCurrent() & KEY_B) bDone=1;
        if (keysCurrent() & KEY_X) highscore_entry();
        if (keysCurrent() & KEY_Y) highscore_options( md5);
    }    
}


// End of file
