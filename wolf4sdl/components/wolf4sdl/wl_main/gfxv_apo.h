//////////////////////////////////////
//
// Graphics .H file for Apogee v1.4
// IGRAB-ed on Sun May 03 01:19:32 1992
//
//////////////////////////////////////

typedef enum {
    // Lump Start
    H_BJPIC=3,
    H_CASTLEPIC,                 // 4
    H_KEYBOARDPIC,               // 5
    H_JOYPIC,                    // 6
    H_HEALPIC,                   // 7
    H_TREASUREPIC,               // 8
    H_GUNPIC,                    // 9
    H_KEYPIC,                    // 10
    H_BLAZEPIC,                  // 11
    H_WEAPON1234PIC,             // 12
    H_WOLFLOGOPIC,               // 13
    H_VISAPIC,                   // 14
    H_MCPIC,                     // 15
    H_IDLOGOPIC,                 // 16
    H_TOPWINDOWPIC,              // 17
    H_LEFTWINDOWPIC,             // 18
    H_RIGHTWINDOWPIC,            // 19
    H_BOTTOMINFOPIC,             // 20
#if !defined(APOGEE_1_0) && !defined(APOGEE_1_1) && !defined(APOGEE_1_2)
    H_SPEARADPIC,                // 21
#endif
    // Lump Start
    C_OPTIONSPIC,                // 22
    C_CURSOR1PIC,                // 23
    C_CURSOR2PIC,                // 24
    C_NOTSELECTEDPIC,            // 25
    C_SELECTEDPIC,               // 26
    C_FXTITLEPIC,                // 27
    C_DIGITITLEPIC,              // 28
    C_MUSICTITLEPIC,             // 29
    C_MOUSELBACKPIC,             // 30
    C_BABYMODEPIC,               // 31
    C_EASYPIC,                   // 32
    C_NORMALPIC,                 // 33
    C_HARDPIC,                   // 34
    C_LOADSAVEDISKPIC,           // 35
    C_DISKLOADING1PIC,           // 36
    C_DISKLOADING2PIC,           // 37
    C_CONTROLPIC,                // 38
    C_CUSTOMIZEPIC,              // 39
    C_LOADGAMEPIC,               // 40
    C_SAVEGAMEPIC,               // 41
    C_EPISODE1PIC,               // 42
    C_EPISODE2PIC,               // 43
    C_EPISODE3PIC,               // 44
    C_EPISODE4PIC,               // 45
    C_EPISODE5PIC,               // 46
    C_EPISODE6PIC,               // 47
    C_CODEPIC,                   // 48
#ifndef APOGEE_1_0
    C_TIMECODEPIC,               // 49
    C_LEVELPIC,                  // 50
    C_NAMEPIC,                   // 51
    C_SCOREPIC,                  // 52
#if !defined(APOGEE_1_1) && !defined(APOGEE_1_2)
    C_JOY1PIC,                   // 53
    C_JOY2PIC,                   // 54
#endif
#else
    C_TIMECODEPIC=C_CODEPIC,     // 47
#endif
    // Lump Start
    L_GUYPIC,                    // 55
    L_COLONPIC,                  // 56
    L_NUM0PIC,                   // 57
    L_NUM1PIC,                   // 58
    L_NUM2PIC,                   // 59
    L_NUM3PIC,                   // 60
    L_NUM4PIC,                   // 61
    L_NUM5PIC,                   // 62
    L_NUM6PIC,                   // 63
    L_NUM7PIC,                   // 64
    L_NUM8PIC,                   // 65
    L_NUM9PIC,                   // 66
    L_PERCENTPIC,                // 67
    L_APIC,                      // 68
    L_BPIC,                      // 69
    L_CPIC,                      // 70
    L_DPIC,                      // 71
    L_EPIC,                      // 72
    L_FPIC,                      // 73
    L_GPIC,                      // 74
    L_HPIC,                      // 75
    L_IPIC,                      // 76
    L_JPIC,                      // 77
    L_KPIC,                      // 78
    L_LPIC,                      // 79
    L_MPIC,                      // 80
    L_NPIC,                      // 81
    L_OPIC,                      // 82
    L_PPIC,                      // 83
    L_QPIC,                      // 84
    L_RPIC,                      // 85
    L_SPIC,                      // 86
    L_TPIC,                      // 87
    L_UPIC,                      // 88
    L_VPIC,                      // 89
    L_WPIC,                      // 90
    L_XPIC,                      // 91
    L_YPIC,                      // 92
    L_ZPIC,                      // 93
    L_EXPOINTPIC,                // 94
#ifndef APOGEE_1_0
    L_APOSTROPHEPIC,             // 95
#endif
    L_GUY2PIC,                   // 96
    L_BJWINSPIC,                 // 97
    STATUSBARPIC,                // 98
    TITLEPIC,                    // 99
    PG13PIC,                     // 100
    CREDITSPIC,                  // 101
    HIGHSCORESPIC,               // 102
    // Lump Start
    KNIFEPIC,                    // 103
    GUNPIC,                      // 104
    MACHINEGUNPIC,               // 105
    GATLINGGUNPIC,               // 106
    NOKEYPIC,                    // 107
    GOLDKEYPIC,                  // 108
    SILVERKEYPIC,                // 109
    N_BLANKPIC,                  // 110
    N_0PIC,                      // 111
    N_1PIC,                      // 112
    N_2PIC,                      // 113
    N_3PIC,                      // 114
    N_4PIC,                      // 115
    N_5PIC,                      // 116
    N_6PIC,                      // 117
    N_7PIC,                      // 118
    N_8PIC,                      // 119
    N_9PIC,                      // 120
    FACE1APIC,                   // 121
    FACE1BPIC,                   // 122
    FACE1CPIC,                   // 123
    FACE2APIC,                   // 124
    FACE2BPIC,                   // 125
    FACE2CPIC,                   // 126
    FACE3APIC,                   // 127
    FACE3BPIC,                   // 128
    FACE3CPIC,                   // 129
    FACE4APIC,                   // 130
    FACE4BPIC,                   // 131
    FACE4CPIC,                   // 132
    FACE5APIC,                   // 133
    FACE5BPIC,                   // 134
    FACE5CPIC,                   // 135
    FACE6APIC,                   // 136
    FACE6BPIC,                   // 137
    FACE6CPIC,                   // 138
    FACE7APIC,                   // 139
    FACE7BPIC,                   // 140
    FACE7CPIC,                   // 141
    FACE8APIC,                   // 142
    GOTGATLINGPIC,               // 143
    MUTANTBJPIC,                 // 144
    PAUSEDPIC,                   // 145
    GETPSYCHEDPIC,               // 146

    TILE8,                       // 147

    ORDERSCREEN,                 // 148
    ERRORSCREEN,                 // 149
    T_HELPART,                   // 150
#ifdef APOGEE_1_0
    T_ENDART1,                   // 143
#endif
    T_DEMO0,                     // 151
    T_DEMO1,                     // 152
    T_DEMO2,                     // 153
    T_DEMO3,                     // 154
#ifndef APOGEE_1_0
    T_ENDART1,                   // 155
    T_ENDART2,                   // 156
    T_ENDART3,                   // 157
    T_ENDART4,                   // 158
    T_ENDART5,                   // 159
    T_ENDART6,                   // 160
#endif

    ENUMEND
} graphicnums;

//
// Data LUMPs
//
#define README_LUMP_START       H_BJPIC
#define README_LUMP_END         H_BOTTOMINFOPIC

#define CONTROLS_LUMP_START     C_OPTIONSPIC
#define CONTROLS_LUMP_END       (L_GUYPIC - 1)

#define LEVELEND_LUMP_START     L_GUYPIC
#define LEVELEND_LUMP_END       L_BJWINSPIC

#define LATCHPICS_LUMP_START    KNIFEPIC
#define LATCHPICS_LUMP_END      GETPSYCHEDPIC


//
// Amount of each data item
//
#define NUMCHUNKS    ENUMEND
#define NUMFONT      2
#define NUMFONTM     0
#define NUMPICS      (GETPSYCHEDPIC - NUMFONT)
#define NUMPICM      0
#define NUMSPRITES   0
#define NUMTILE8     72
#define NUMTILE8M    0
#define NUMTILE16    0
#define NUMTILE16M   0
#define NUMTILE32    0
#define NUMTILE32M   0
#define NUMEXTERNS   13
//
// File offsets for data items
//
#define STRUCTPIC    0

#define STARTFONT    1
#define STARTFONTM   3
#define STARTPICS    3
#define STARTPICM    TILE8
#define STARTSPRITES TILE8
#define STARTTILE8   TILE8
#define STARTTILE8M  ORDERSCREEN
#define STARTTILE16  ORDERSCREEN
#define STARTTILE16M ORDERSCREEN
#define STARTTILE32  ORDERSCREEN
#define STARTTILE32M ORDERSCREEN
#define STARTEXTERNS ORDERSCREEN

//
// Thank you for using IGRAB!
//
