// =====================================================================================================
// Stella DS/DSi Pheonix Edition - Improved Version by Dave Bernazzani (wavemotion)
//
// Adapted for Retro-Go by Gemini CLI
// =====================================================================================================
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "StellaDS.h"
#include "printf.h"
#include "Console.hxx"
#include "SaveKey.hxx"
#include "TIASound.hxx"
#include "Event.hxx"
#include "StellaEvent.hxx"
#include "EventHandler.hxx"
#include "Cart.hxx"
#include "CartAR.hxx"
#include "CartCTY.hxx"
#include "CartDPCPlus.hxx"
#include "CartCDF.hxx"
#include "Cart3EPlus.hxx"
#include "CartWD.hxx"
#include "stella_config.h"
#include "savestate.h"
#include "M6532.hxx"
#include "M6502.hxx"
#include "TIA.hxx"
#include "System.hxx"
#include "Thumbulator.hxx"

#define SAVE_VERSION 0x0004

// TIA Globals
extern uint32  myBlendBk;
extern uInt32  myPF;
extern uInt32  myColor[4];
extern uInt32  myStartDisplayOffset;
extern uInt32  myStopDisplayOffset;
extern Int32   myVSYNCFinishClock;
extern uInt8*  myCurrentFrameBuffer[2];
extern uInt8*  myFramePointer;
extern Int32   myLastHMOVEClock;
extern uInt32  (*ourPlayfieldTable)[160];
extern Int32   myClockWhenFrameStarted;
extern Int32   myCyclesWhenFrameStarted;
extern Int32   myClockStartDisplay;
extern Int32   myClockStopDisplay;
extern Int32   myClockAtLastUpdate;
extern Int32   myClocksToEndOfScanLine;
extern uInt8*  myCurrentBLMask;
extern uInt8*  myCurrentM0Mask;
extern uInt8*  myCurrentM1Mask;
extern uInt8*  myCurrentP0Mask;
extern uInt8*  myCurrentP1Mask;
extern uInt32* myCurrentPFMask;
extern uInt16* ourCollisionTable;
extern uInt16  myCollision;
extern Int16   myPOSP0;
extern Int16   myPOSP1;
extern Int16   myPOSM0;
extern Int16   myPOSM1;
extern Int16   myPOSBL;
extern uInt32  myPlayfieldPriorityAndScore;
extern uInt8   myCTRLPF;
extern uInt32  myVSYNC;
extern uInt32  myVBLANK;
extern uInt8   myREFP0;
extern uInt8   myREFP1;
extern uInt8   myGRP0;
extern uInt8   myGRP1;
extern uInt8   myDGRP0;
extern uInt8   myDGRP1;
extern uInt8   myENAM0;
extern uInt8   myENAM1;
extern uInt8   myENABL;
extern uInt8   myDENABL;
extern Int8    myHMP0;
extern Int8    myHMP1;
extern Int8    myHMM0;
extern Int8    myHMM1;
extern Int8    myHMBL;
extern uInt8   myVDELP0;
extern uInt8   myVDELP1;
extern uInt8   myVDELBL;
extern uInt8   myRESMP0;
extern uInt8   myRESMP1;
extern uInt8   myHMOVEBlankEnabled;
extern uInt8   myM0CosmicArkMotionEnabled;
extern uInt8   myM0CosmicArkCounter;
extern uInt8   myM1CosmicArkMotionEnabled;
extern uInt8   myM1CosmicArkCounter;
extern uInt32  myCurrentGRP0;
extern uInt32  myCurrentGRP1;
extern uInt8   myNUSIZ0;
extern uInt8   myNUSIZ1;
extern uInt8   (*myPriorityEncoder)[256];
extern uInt8   *ourDisabledMaskTable;
extern uInt8   (*ourMissleMaskTable)[8][4][320];
extern uInt8   (*ourPlayerMaskTable)[2][8][320];
extern Int8    (*ourPlayerPositionResetWhenTable)[160][160];
extern uInt8   *ourPlayerReflectTable;

// M6532 Globals
extern uInt8 myRAM[256];
extern uInt32 myTimer;
extern uInt8 myIntervalShift;
extern Int32 myCyclesWhenTimerSet;
extern uInt8  myInterruptEnabled;
extern uInt8  myInterruptTriggered;
extern uInt8 myOutTimer[4];
extern uInt8 myDDRA;
extern uInt8 myOutA;

// M6502 Globals
extern uInt8 A, X, Y, SP, N, V, B, D, I, notZ, C;
extern uInt16 gPC;
extern uInt8 myDataBusState;
extern uInt16 f8_bankbit;

// Cart Globals
extern uInt16 myCurrentBank;
extern uInt32 myCurrentOffset;
extern uInt32 myCurrentOffset32;
extern uInt8 cartDriver;
extern uInt8 bElevatorAgent;
extern uInt8 bSaveStateXL;
extern uInt8* xl_ram_buffer;
extern uInt8* fast_cart_buffer;
extern uInt8* cart_buffer;

// CartAR Globals
extern uInt8 myWriteEnabled;
extern uInt8 myDataHoldRegister;
extern uInt8 myWritePending;
extern uInt8 bPossibleLoad;
extern uInt8 myNumberOfLoadImages;
extern uInt8 LastConfigurationAR;
extern Int32 myCyclesAtBankswitchInit;
extern uInt8 myPendingBank;
extern uInt8 bWriteOrLoadPossibleAR;

// CartDPCPlus Globals
extern bool myFastFetch;
extern uInt32 myDPCPRandomNumber;
extern Int32 myDPCPCycles;
extern uInt8 myParameterPointer;
extern uInt32 myFractionalCounters[8];
extern uInt32 myFractionalIncrements[8];
extern uInt32 myTops[8];
extern uInt32 myTopsMinusBottoms[8];
extern uInt32 myBottoms[8];
extern uInt32 myCounters[8];
extern uInt32 myMusicCounters[3];
extern uInt32 myMusicFrequencies[3];
extern uInt32 myMusicWaveforms[3];
extern uInt32 myMusicCountersShifted[3];
extern uInt8 myParameter[8];

// CartCDF Globals
extern uInt16 myAmplitudeStream;
extern uInt8  myDataStreamFetch;
extern uInt8  peekvalue;
extern uInt8  myMode;
extern uInt8  myLDXenabled;
extern uInt8  myLDYenabled;
extern uInt16 myFastFetcherOffset;
extern uInt16 myMusicWaveformSize[3];

// CartCTY Globals
extern uInt16 myTunePosition;
extern uInt32 myAudioCycles;
extern uInt32 deltaCyclesX10;
extern uInt8 myOperationType;

// System Globals
extern Int32 gSystemCycles;
extern uInt32 gTotalSystemCycles;
extern uInt32 NumberOfDistinctAccesses;

// TIA Sound Globals
extern uInt8 AUDC[2], AUDF[2], AUDV[2];
extern uInt32 Outvol[2];
extern uInt8 bProcessingSample;
extern uInt16 tia_buf_idx, tia_out_idx;
extern uInt16 *tia_buf;
extern uInt32 Samp_n_max, Samp_n_cnt;
extern uInt8 Bit9[POLY9_SIZE];
extern uInt8 P4[2], P5[2];
extern uInt16 P9[2];
extern uInt32 Div_n_cnt[2], Div_n_max[2];

// Thumbulator Globals
extern uInt32 reg_sys[16];
extern uInt32 cFlag, cStack, cBase, cStart;

#define TYPE_RAW        0
#define TYPE_RAM        1
#define TYPE_CART       2
#define TYPE_FASTCART   3
#define TYPE_XLRAM      4

typedef struct {
    uInt8  peek_type;
    uInt8  poke_type;
    uInt32 peek_offset;
    uInt32 poke_offset;
} Offsets_t;

Offsets_t myPageOffsets[64];
uInt16 myExecutionStatus = 0x0000;

void SaveState(const char *filename)
{
    memset(myPageOffsets, 0x00, sizeof(myPageOffsets));

    for (uInt8 i=0; i<64; i++)
    {
        if ((myPageAccessTable[i].directPeekBase >= &myRAM[0]) && (myPageAccessTable[i].directPeekBase <= &myRAM[255]))
        {
            myPageOffsets[i].peek_type = TYPE_RAM;
            myPageOffsets[i].peek_offset = (myPageAccessTable[i].directPeekBase - myRAM);
        }
        else if (cart_buffer && (myPageAccessTable[i].directPeekBase >= cart_buffer) && (myPageAccessTable[i].directPeekBase < (cart_buffer + MAX_CART_FILE_SIZE)))
        {
            myPageOffsets[i].peek_type = TYPE_CART;
            myPageOffsets[i].peek_offset = (myPageAccessTable[i].directPeekBase - cart_buffer);
        }
        else if (fast_cart_buffer && (myPageAccessTable[i].directPeekBase >= &fast_cart_buffer[0]) && (myPageAccessTable[i].directPeekBase < &fast_cart_buffer[8*1024]))
        {
            myPageOffsets[i].peek_type = TYPE_FASTCART;
            myPageOffsets[i].peek_offset = (myPageAccessTable[i].directPeekBase - fast_cart_buffer);
        }
        else if (xl_ram_buffer && (myPageAccessTable[i].directPeekBase >= xl_ram_buffer) && (myPageAccessTable[i].directPeekBase < (xl_ram_buffer + 32*1024)))
        {
            myPageOffsets[i].peek_type = TYPE_XLRAM;
            myPageOffsets[i].peek_offset = (myPageAccessTable[i].directPeekBase - xl_ram_buffer);
        }
        else if (myPageAccessTable[i].directPeekBase != 0)
        {
            myPageOffsets[i].peek_type = TYPE_RAW;
            myPageOffsets[i].peek_offset = (uInt32)(uintptr_t)myPageAccessTable[i].directPeekBase;
        }

        if ((myPageAccessTable[i].directPokeBase >= &myRAM[0]) && (myPageAccessTable[i].directPokeBase <= &myRAM[255]))
        {
            myPageOffsets[i].poke_type = TYPE_RAM;
            myPageOffsets[i].poke_offset = (myPageAccessTable[i].directPokeBase - myRAM);
        }
        else if (cart_buffer && (myPageAccessTable[i].directPokeBase >= cart_buffer) && (myPageAccessTable[i].directPokeBase < (cart_buffer + MAX_CART_FILE_SIZE)))
        {
            myPageOffsets[i].poke_type = TYPE_CART;
            myPageOffsets[i].poke_offset = (myPageAccessTable[i].directPokeBase - cart_buffer);
        }
        else if (fast_cart_buffer && (myPageAccessTable[i].directPokeBase >= &fast_cart_buffer[0]) && (myPageAccessTable[i].directPokeBase < &fast_cart_buffer[8*1024]))
        {
            myPageOffsets[i].poke_type = TYPE_FASTCART;
            myPageOffsets[i].poke_offset = (myPageAccessTable[i].directPokeBase - fast_cart_buffer);
        }
        else if (xl_ram_buffer && (myPageAccessTable[i].directPokeBase >= xl_ram_buffer) && (myPageAccessTable[i].directPokeBase < (xl_ram_buffer + 32*1024)))
        {
            myPageOffsets[i].poke_type = TYPE_XLRAM;
            myPageOffsets[i].poke_offset = (myPageAccessTable[i].directPokeBase - xl_ram_buffer);
        }
        else if (myPageAccessTable[i].directPokeBase != 0)
        {
            myPageOffsets[i].poke_type = TYPE_RAW;
            myPageOffsets[i].poke_offset = (uInt32)(uintptr_t)myPageAccessTable[i].directPokeBase;
        }
    }

    FILE * fp = fopen(filename, "wb");
    if (!fp) return;

    uInt16 save_ver = SAVE_VERSION;
    fwrite(&save_ver,                   sizeof(save_ver),                   1, fp);
    fwrite(fast_cart_buffer,            8*1024,                             1, fp);
    fwrite(sound_buffer,                SOUND_SIZE,                         1, fp);
    fwrite(&emuState,                   sizeof(emuState),                   1, fp);
    fwrite(&bHaltEmulation,             sizeof(bHaltEmulation),             1, fp);
    fwrite(&bScreenRefresh,             sizeof(bScreenRefresh),             1, fp);
    fwrite(&gAtariFrames,               sizeof(gAtariFrames),               1, fp);
    fwrite(&gTotalAtariFrames,          sizeof(gTotalAtariFrames),          1, fp);
    fwrite(&atari_frames,               sizeof(atari_frames),               1, fp);
    fwrite(&gSaveKeyEEWritten,          sizeof(gSaveKeyEEWritten),          1, fp);
    fwrite(&gSaveKeyIsDirty,            sizeof(gSaveKeyIsDirty),            1, fp);
    fwrite(&mySoundFreq,                sizeof(mySoundFreq),                1, fp);
    uInt16 dummyTimer = 0;
    fwrite(&dummyTimer,                 sizeof(dummyTimer),                 1, fp);
    fwrite(&console_color,              sizeof(console_color),              1, fp);   
    fwrite(&myCartInfo.left_difficulty, sizeof(uInt8),                      1, fp);
    fwrite(&myCartInfo.right_difficulty,sizeof(uInt8),                      1, fp);

    fwrite(myRAM,                       256,                                1, fp);
    fwrite(&myTimer,                    sizeof(myTimer),                    1, fp);
    fwrite(&myIntervalShift,            sizeof(myIntervalShift),            1, fp);
    fwrite(&myCyclesWhenTimerSet,       sizeof(myCyclesWhenTimerSet),       1, fp);
    fwrite(&myInterruptEnabled,         sizeof(myInterruptEnabled),         1, fp);
    fwrite(&myInterruptTriggered,       sizeof(myInterruptTriggered),       1, fp);
    fwrite(&myDDRA,                     sizeof(myDDRA),                     1, fp);
    fwrite(&myDDRA,                     sizeof(myDDRA),                     1, fp);
    fwrite(&myOutA,                     sizeof(myOutA),                     1, fp);
    fwrite(myOutTimer,                  sizeof(myOutTimer),                 1, fp);    

    fwrite(&A,                          sizeof(A),                          1, fp);
    fwrite(&X,                          sizeof(X),                          1, fp);
    fwrite(&Y,                          sizeof(Y),                          1, fp);
    fwrite(&SP,                         sizeof(SP),                         1, fp);
    fwrite(&gPC,                        sizeof(gPC),                        1, fp);
    fwrite(&N,                          sizeof(N),                          1, fp);
    fwrite(&V,                          sizeof(V),                          1, fp);
    fwrite(&B,                          sizeof(B),                          1, fp);
    fwrite(&D,                          sizeof(D),                          1, fp);
    fwrite(&I,                          sizeof(I),                          1, fp);
    fwrite(&C,                          sizeof(C),                          1, fp);
    fwrite(&notZ,                       sizeof(notZ),                       1, fp);
    fwrite(&myExecutionStatus,          sizeof(myExecutionStatus),          1, fp);
    fwrite(&myDataBusState,             sizeof(myDataBusState),             1, fp);
    fwrite(&NumberOfDistinctAccesses,   sizeof(NumberOfDistinctAccesses),   1, fp);

    fwrite(&myCurrentBank,              sizeof(myCurrentBank),              1, fp);
    fwrite(&myCurrentOffset,            sizeof(myCurrentOffset),            1, fp);
    fwrite(&myCurrentOffset32,          sizeof(myCurrentOffset32),          1, fp);
    fwrite(&cartDriver,                 sizeof(cartDriver),                 1, fp);
    fwrite(&f8_bankbit,                 sizeof(f8_bankbit),                 1, fp);
    uInt16 myCurrentBanks[4] = {0,0,0,0}; // Use Cart3EPlus to get these if needed
    fwrite(myCurrentBanks,              sizeof(myCurrentBanks),             1, fp);    

    uInt32 myRandomNumber = 0;
    fwrite(&myRandomNumber,             sizeof(myRandomNumber),             1, fp);
    uInt32 myMusicCycles = 0;
    fwrite(&myMusicCycles,              sizeof(myMusicCycles),              1, fp);
    uInt8 myFlags[16] = {0};
    fwrite(myFlags,                     sizeof(myFlags),                    1, fp);
    uInt8 myMusicMode = 0;
    fwrite(&myMusicMode,                 sizeof(myMusicMode),                1, fp);

    fwrite(&gSystemCycles,              sizeof(gSystemCycles),              1, fp);
    fwrite(&gTotalSystemCycles,         sizeof(gTotalSystemCycles),         1, fp);
    fwrite(&myPageOffsets,              sizeof(myPageOffsets),              1, fp);

    fwrite(ourCollisionTable,           256 * sizeof(uInt16),               1, fp);
    fwrite(myPriorityEncoder,           2 * 256 * sizeof(uInt8),            1, fp);
    fwrite(&myCollision,                sizeof(myCollision),                1, fp);

    fwrite(&myPOSP0,                    sizeof(myPOSP0),                    1, fp);
    fwrite(&myPOSP1,                    sizeof(myPOSP1),                    1, fp);
    fwrite(&myPOSM0,                    sizeof(myPOSM0),                    1, fp);
    fwrite(&myPOSM1,                    sizeof(myPOSM1),                    1, fp);
    fwrite(&myPOSBL,                    sizeof(myPOSBL),                    1, fp);

    fwrite(&myPlayfieldPriorityAndScore,sizeof(myPlayfieldPriorityAndScore),1, fp);
    fwrite(myColor,                     sizeof(myColor),                    1, fp);
    fwrite(&myCTRLPF,                   sizeof(myCTRLPF),                   1, fp);
    fwrite(&myREFP0,                    sizeof(myREFP0),                    1, fp);
    fwrite(&myREFP0,                    sizeof(myREFP0),                    1, fp);
    fwrite(&myREFP1,                    sizeof(myREFP1),                    1, fp);
    fwrite(&myPF,                       sizeof(myPF),                       1, fp);
    fwrite(&myGRP0,                     sizeof(myGRP0),                     1, fp);
    fwrite(&myGRP1,                     sizeof(myGRP1),                     1, fp);
    fwrite(&myDGRP0,                    sizeof(myDGRP0),                    1, fp);
    fwrite(&myDGRP1,                    sizeof(myDGRP1),                    1, fp);
    fwrite(&myENAM0,                    sizeof(myENAM0),                    1, fp);
    fwrite(&myENAM1,                    sizeof(myENAM1),                    1, fp);
    fwrite(&myENABL,                    sizeof(myENABL),                    1, fp);
    fwrite(&myDENABL,                   sizeof(myDENABL),                   1, fp);
    fwrite(&myHMP0,                     sizeof(myHMP0),                     1, fp);
    fwrite(&myHMP1,                     sizeof(myHMP1),                     1, fp);
    fwrite(&myHMM0,                     sizeof(myHMM0),                     1, fp);
    fwrite(&myHMM1,                     sizeof(myHMM1),                     1, fp);
    fwrite(&myHMBL,                     sizeof(myHMBL),                     1, fp);
    fwrite(&myVDELP0,                   sizeof(myVDELP0),                   1, fp);
    fwrite(&myVDELP1,                   sizeof(myVDELP1),                   1, fp);
    fwrite(&myVDELBL,                   sizeof(myVDELBL),                   1, fp);
    fwrite(&myRESMP0,                   sizeof(myRESMP0),                   1, fp);
    fwrite(&myRESMP1,                   sizeof(myRESMP1),                   1, fp);

    fwrite(&myStartDisplayOffset,       sizeof(myStartDisplayOffset),       1, fp);
    fwrite(&myStopDisplayOffset,        sizeof(myStopDisplayOffset),        1, fp);
    fwrite(&myVSYNCFinishClock,         sizeof(myVSYNCFinishClock),         1, fp);

    fwrite(&myEnabledObjects,           sizeof(myEnabledObjects),           1, fp);
    fwrite(&myClockWhenFrameStarted,    sizeof(myClockWhenFrameStarted),    1, fp);
    fwrite(&myCyclesWhenFrameStarted,   sizeof(myCyclesWhenFrameStarted),   1, fp);

    fwrite(&myClockStartDisplay,        sizeof(myClockStartDisplay),        1, fp);
    fwrite(&myClockStopDisplay,         sizeof(myClockStopDisplay),         1, fp);
    fwrite(&myClockAtLastUpdate,        sizeof(myClockAtLastUpdate),        1, fp);
    fwrite(&myClocksToEndOfScanLine,    sizeof(myClocksToEndOfScanLine),    1, fp);

    fwrite(&myVSYNC,                    sizeof(myVSYNC),                    1, fp);
    fwrite(&myVBLANK,                   sizeof(myVBLANK),                   1, fp);
    fwrite(&myLastHMOVEClock,           sizeof(myLastHMOVEClock),           1, fp);
    fwrite(&myHMOVEBlankEnabled,        sizeof(myHMOVEBlankEnabled),        1, fp);

    fwrite(&myM0CosmicArkMotionEnabled, sizeof(myM0CosmicArkMotionEnabled), 1, fp);
    fwrite(&myM0CosmicArkCounter,       sizeof(myM0CosmicArkCounter),       1, fp);

    fwrite(&myCurrentGRP0,              sizeof(myCurrentGRP0),              1, fp);
    fwrite(&myCurrentGRP1,              sizeof(myCurrentGRP1),              1, fp);

    fwrite(&myNUSIZ0,                   sizeof(myNUSIZ0),                   1, fp);
    fwrite(&myNUSIZ1,                   sizeof(myNUSIZ1),                   1, fp);

    fwrite(ourPlayerReflectTable,       256 * sizeof(uInt8),                1, fp);
    fwrite(ourPlayfieldTable,           2 * 160 * sizeof(uInt32),           1, fp);

    fwrite(AUDC,                        2 * sizeof(uInt8),                  1, fp);
    fwrite(AUDF,                        2 * sizeof(uInt8),                  1, fp);
    fwrite(AUDV,                        2 * sizeof(uInt8),                  1, fp);
    fwrite(Outvol,                      2 * sizeof(uInt32),                 1, fp);
    fwrite(&bProcessingSample,          sizeof(bProcessingSample),          1, fp);
    fwrite(&tia_buf_idx,                sizeof(tia_buf_idx),                1, fp);
    fwrite(&tia_out_idx,                sizeof(tia_out_idx),                1, fp);

    fwrite(&Samp_n_max,                 sizeof(Samp_n_max),                 1, fp);
    fwrite(&Samp_n_cnt,                 sizeof(Samp_n_cnt),                 1, fp);
    fwrite(Bit9,                        POLY9_SIZE * sizeof(uInt8),         1, fp);
    fwrite(P4,                          2 * sizeof(uInt8),                  1, fp);
    fwrite(P5,                          2 * sizeof(uInt8),                  1, fp);
    fwrite(P9,                          2 * sizeof(uInt16),                 1, fp);
    fwrite(Div_n_cnt,                   2 * sizeof(uInt32),                 1, fp);
    fwrite(Div_n_max,                   2 * sizeof(uInt32),                 1, fp);

    fwrite(&NumberOfDistinctAccesses,   sizeof(NumberOfDistinctAccesses),  1, fp);
    fwrite(&myWriteEnabled,             sizeof(myWriteEnabled),            1, fp);
    fwrite(&myDataHoldRegister,         sizeof(myDataHoldRegister),        1, fp);
    fwrite(&myWritePending,             sizeof(myWritePending),            1, fp);
    fwrite(&bPossibleLoad,              sizeof(bPossibleLoad),             1, fp);
    fwrite(&myNumberOfLoadImages,       sizeof(myNumberOfLoadImages),      1, fp);

    fwrite(&LastConfigurationAR,        sizeof(LastConfigurationAR),       1, fp);

    fwrite(&myCyclesAtBankswitchInit,   sizeof(myCyclesAtBankswitchInit),  1, fp);
    fwrite(&myPendingBank,              sizeof(myPendingBank),             1, fp);

    static char tmp_buf[SOUND_SIZE * 2];
    memcpy(tmp_buf, tia_buf,            SOUND_SIZE * 2);
    fwrite(tmp_buf,                     SOUND_SIZE * 2,                    1, fp);
    
    fwrite(reg_sys,                     16 * sizeof(uInt32),               1, fp);
    fwrite(&cFlag,                      sizeof(cFlag),                     1, fp);
    fwrite(&cStack,                     sizeof(cStack),                    1, fp);
    fwrite(&cBase,                      sizeof(cBase),                     1, fp);
    fwrite(&cStart,                     sizeof(cStart),                    1, fp);
    
    fwrite(&myFastFetch,                sizeof(myFastFetch),               1, fp);
    fwrite(&myDPCPRandomNumber,         sizeof(myDPCPRandomNumber),        1, fp);
    fwrite(&myDPCPCycles,               sizeof(myDPCPCycles),              1, fp);
    fwrite(&myParameterPointer,         sizeof(myParameterPointer),        1, fp);
    fwrite(myFractionalCounters,        8 * sizeof(uInt32),                1, fp);
    fwrite(myFractionalIncrements,      8 * sizeof(uInt32),                1, fp);
    fwrite(myTops,                      8 * sizeof(uInt32),                1, fp);
    fwrite(myTopsMinusBottoms,          8 * sizeof(uInt32),                1, fp);
    fwrite(myBottoms,                   8 * sizeof(uInt32),                1, fp);
    fwrite(myCounters,                  8 * sizeof(uInt32),                1, fp);
    fwrite(myMusicCounters,             3 * sizeof(uInt32),                1, fp);
    fwrite(myMusicFrequencies,          3 * sizeof(uInt32),                1, fp);
    fwrite(myMusicWaveforms,            3 * sizeof(uInt32),                1, fp);
    fwrite(myMusicCountersShifted,      3 * sizeof(uInt32),                1, fp);
    fwrite(myParameter,                 8 * sizeof(uInt8),                 1, fp);

    fwrite(&myAmplitudeStream,          sizeof(myAmplitudeStream),         1, fp);
    fwrite(&myDataStreamFetch,          sizeof(myDataStreamFetch),         1, fp);    
    fwrite(&peekvalue,                  sizeof(peekvalue),                 1, fp);
    fwrite(&myMode,                     sizeof(myMode),                    1, fp);
    fwrite(&myLDXenabled,               sizeof(myLDXenabled),              1, fp);
    fwrite(&myLDYenabled,               sizeof(myLDYenabled),              1, fp);
    fwrite(&myFastFetcherOffset,        sizeof(myFastFetcherOffset),       1, fp);   
    fwrite(myMusicWaveformSize,         3 * sizeof(uInt16),                1, fp);
    
    fwrite(&bWriteOrLoadPossibleAR,     sizeof(bWriteOrLoadPossibleAR),    1, fp);
    
    if (bSaveStateXL && xl_ram_buffer)
    {
        fwrite(xl_ram_buffer,           32*1024,                           1, fp);   
    }

    fwrite(&myTunePosition,             sizeof(myTunePosition),            1, fp);
    fwrite(&myAudioCycles,              sizeof(myAudioCycles),             1, fp);
    fwrite(&deltaCyclesX10,             sizeof(deltaCyclesX10),            1, fp);
    fwrite(&myOperationType,            sizeof(myOperationType),           1, fp);
    
    static char spare[116];
    memset(spare, 0, 116);
    fwrite(spare,                       116,                               1, fp);
    
    fclose(fp);
}

void LoadState(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return;

    uInt16 save_ver = 0xFFFF;
    fread(&save_ver,                   sizeof(save_ver),                   1, fp);
    if (save_ver == SAVE_VERSION) 
    {
        fread(fast_cart_buffer,            8*1024,                             1, fp);
        fread(sound_buffer,                SOUND_SIZE,                         1, fp);
        fread(&emuState,                   sizeof(emuState),                   1, fp);
        fread(&bHaltEmulation,             sizeof(bHaltEmulation),             1, fp);
        fread(&bScreenRefresh,             sizeof(bScreenRefresh),             1, fp);
        fread(&gAtariFrames,               sizeof(gAtariFrames),               1, fp);
        fread(&gTotalAtariFrames,          sizeof(gTotalAtariFrames),          1, fp);
        fread(&atari_frames,               sizeof(atari_frames),               1, fp);
        fread(&gSaveKeyEEWritten,          sizeof(gSaveKeyEEWritten),          1, fp);
        fread(&gSaveKeyIsDirty,            sizeof(gSaveKeyIsDirty),            1, fp);
        fread(&mySoundFreq,                sizeof(mySoundFreq),                1, fp);
        uInt16 dummyTimer;
        fread(&dummyTimer,                 sizeof(dummyTimer),                 1, fp);
        fread(&console_color,              sizeof(console_color),              1, fp);   
        fread(&myCartInfo.left_difficulty, sizeof(uInt8),                      1, fp);
        fread(&myCartInfo.right_difficulty,sizeof(uInt8),                      1, fp);
        
        fread(myRAM,                       256,                                1, fp);
        fread(&myTimer,                    sizeof(myTimer),                    1, fp);
        fread(&myIntervalShift,            sizeof(myIntervalShift),            1, fp);
        fread(&myCyclesWhenTimerSet,       sizeof(myCyclesWhenTimerSet),       1, fp);
        fread(&myInterruptEnabled,         sizeof(myInterruptEnabled),         1, fp);
        fread(&myInterruptTriggered,       sizeof(myInterruptTriggered),       1, fp);
        fread(&myDDRA,                     sizeof(myDDRA),                     1, fp);
        fread(&myDDRA,                     sizeof(myDDRA),                     1, fp);
        fread(&myOutA,                     sizeof(myOutA),                     1, fp);
        fread(myOutTimer,                  sizeof(myOutTimer),                 1, fp);

        fread(&A,                          sizeof(A),                          1, fp);
        fread(&X,                          sizeof(X),                          1, fp);
        fread(&Y,                          sizeof(Y),                          1, fp);
        fread(&SP,                         sizeof(SP),                         1, fp);
        fread(&gPC,                        sizeof(gPC),                        1, fp);
        fread(&N,                          sizeof(N),                          1, fp);
        fread(&V,                          sizeof(V),                          1, fp);
        fread(&B,                          sizeof(B),                          1, fp);
        fread(&D,                          sizeof(D),                          1, fp);
        fread(&I,                          sizeof(I),                          1, fp);
        fread(&C,                          sizeof(C),                          1, fp);
        fread(&notZ,                       sizeof(notZ),                       1, fp);
        fread(&myExecutionStatus,          sizeof(myExecutionStatus),          1, fp);
        fread(&myDataBusState,             sizeof(myDataBusState),             1, fp);
        fread(&NumberOfDistinctAccesses,   sizeof(NumberOfDistinctAccesses),   1, fp);

        fread(&myCurrentBank,              sizeof(myCurrentBank),              1, fp);
        fread(&myCurrentOffset,            sizeof(myCurrentOffset),            1, fp);
        fread(&myCurrentOffset32,          sizeof(myCurrentOffset32),          1, fp);
        fread(&cartDriver,                 sizeof(cartDriver),                 1, fp);
        fread(&f8_bankbit,                 sizeof(f8_bankbit),                 1, fp);
        uInt16 myCurrentBanks[4];
        fread(myCurrentBanks,              sizeof(myCurrentBanks),             1, fp);

        uInt32 myRandomNumber;
        fread(&myRandomNumber,             sizeof(myRandomNumber),             1, fp);
        uInt32 myMusicCycles;
        fread(&myMusicCycles,              sizeof(myMusicCycles),              1, fp);
        uInt8 myFlags[16];
        fread(myFlags,                     sizeof(myFlags),                    1, fp);
        uInt8 myMusicMode;
        fread(&myMusicMode,                 sizeof(myMusicMode),                1, fp);

        fread(&gSystemCycles,              sizeof(gSystemCycles),              1, fp);
        fread(&gTotalSystemCycles,         sizeof(gTotalSystemCycles),         1, fp);
        fread(&myPageOffsets,              sizeof(myPageOffsets),              1, fp);

        fread(ourCollisionTable,           256 * sizeof(uInt16),               1, fp);
        fread(myPriorityEncoder,           2 * 256 * sizeof(uInt8),            1, fp);
        fread(&myCollision,                sizeof(myCollision),                1, fp);

        fread(&myPOSP0,                    sizeof(myPOSP0),                    1, fp);
        fread(&myPOSP1,                    sizeof(myPOSP1),                    1, fp);
        fread(&myPOSM0,                    sizeof(myPOSM0),                    1, fp);
        fread(&myPOSM1,                    sizeof(myPOSM1),                    1, fp);
        fread(&myPOSBL,                    sizeof(myPOSBL),                    1, fp);

        fread(&myPlayfieldPriorityAndScore,sizeof(myPlayfieldPriorityAndScore),1, fp);
        fread(myColor,                     sizeof(myColor),                    1, fp);
        fread(&myCTRLPF,                   sizeof(myCTRLPF),                   1, fp);
        fread(&myREFP0,                    sizeof(myREFP0),                    1, fp);
        fread(&myREFP0,                    sizeof(myREFP0),                    1, fp);
        fread(&myREFP1,                    sizeof(myREFP1),                    1, fp);
        fread(&myPF,                       sizeof(myPF),                       1, fp);
        fread(&myGRP0,                     sizeof(myGRP0),                     1, fp);
        fread(&myGRP1,                     sizeof(myGRP1),                     1, fp);
        fread(&myDGRP0,                    sizeof(myDGRP0),                    1, fp);
        fread(&myDGRP1,                    sizeof(myDGRP1),                    1, fp);
        fread(&myENAM0,                    sizeof(myENAM0),                    1, fp);
        fread(&myENAM1,                    sizeof(myENAM1),                    1, fp);
        fread(&myENABL,                    sizeof(myENABL),                    1, fp);
        fread(&myDENABL,                   sizeof(myDENABL),                   1, fp);
        fread(&myHMP0,                     sizeof(myHMP0),                     1, fp);
        fread(&myHMP1,                     sizeof(myHMP1),                     1, fp);
        fread(&myHMM0,                     sizeof(myHMM0),                     1, fp);
        fread(&myHMM1,                     sizeof(myHMM1),                     1, fp);
        fread(&myHMBL,                     sizeof(myHMBL),                     1, fp);
        fread(&myVDELP0,                   sizeof(myVDELP0),                   1, fp);
        fread(&myVDELP1,                   sizeof(myVDELP1),                   1, fp);
        fread(&myVDELBL,                   sizeof(myVDELBL),                   1, fp);
        fread(&myRESMP0,                   sizeof(myRESMP0),                   1, fp);
        fread(&myRESMP1,                   sizeof(myRESMP1),                   1, fp);

        fread(&myStartDisplayOffset,       sizeof(myStartDisplayOffset),       1, fp);
        fread(&myStopDisplayOffset,        sizeof(myStopDisplayOffset),        1, fp);
        fread(&myVSYNCFinishClock,         sizeof(myVSYNCFinishClock),         1, fp);

        fread(&myEnabledObjects,           sizeof(myEnabledObjects),           1, fp);
        fread(&myClockWhenFrameStarted,    sizeof(myClockWhenFrameStarted),    1, fp);
        fread(&myCyclesWhenFrameStarted,   sizeof(myCyclesWhenFrameStarted),   1, fp);

        fread(&myClockStartDisplay,        sizeof(myClockStartDisplay),        1, fp);
        fread(&myClockStopDisplay,         sizeof(myClockStopDisplay),         1, fp);
        fread(&myClockAtLastUpdate,        sizeof(myClockAtLastUpdate),        1, fp);
        fread(&myClocksToEndOfScanLine,    sizeof(myClocksToEndOfScanLine),    1, fp);

        fread(&myVSYNC,                    sizeof(myVSYNC),                    1, fp);
        fread(&myVBLANK,                   sizeof(myVBLANK),                   1, fp);
        fread(&myLastHMOVEClock,           sizeof(myLastHMOVEClock),           1, fp);
        fread(&myHMOVEBlankEnabled,        sizeof(myHMOVEBlankEnabled),        1, fp);

        fread(&myM0CosmicArkMotionEnabled, sizeof(myM0CosmicArkMotionEnabled), 1, fp);
        fread(&myM0CosmicArkCounter,       sizeof(myM0CosmicArkCounter),       1, fp);

        fread(&myCurrentGRP0,              sizeof(myCurrentGRP0),              1, fp);
        fread(&myCurrentGRP1,              sizeof(myCurrentGRP1),              1, fp);

        fread(&myNUSIZ0,                   sizeof(myNUSIZ0),                   1, fp);
        fread(&myNUSIZ1,                   sizeof(myNUSIZ1),                   1, fp);

        fread(ourPlayerReflectTable,       256 * sizeof(uInt8),                1, fp);
        fread(ourPlayfieldTable,           2 * 160 * sizeof(uInt32),           1, fp);

        fread(AUDC,                        2 * sizeof(uInt8),                  1, fp);
        fread(AUDF,                        2 * sizeof(uInt8),                  1, fp);
        fread(AUDV,                        2 * sizeof(uInt8),                  1, fp);
        fread(Outvol,                      2 * sizeof(uInt32),                 1, fp);
        fread(&bProcessingSample,          sizeof(bProcessingSample),          1, fp);
        fread(&tia_buf_idx,                sizeof(tia_buf_idx),                1, fp);
        fread(&tia_out_idx,                sizeof(tia_out_idx),                1, fp);

        fread(&Samp_n_max,                 sizeof(Samp_n_max),                 1, fp);
        fread(&Samp_n_cnt,                 sizeof(Samp_n_cnt),                 1, fp);
        fread(Bit9,                        POLY9_SIZE * sizeof(uInt8),         1, fp);
        fread(P4,                          2 * sizeof(uInt8),                  1, fp);
        fread(P5,                          2 * sizeof(uInt8),                  1, fp);
        fread(P9,                          2 * sizeof(uInt16),                 1, fp);
        fread(Div_n_cnt,                   2 * sizeof(uInt32),                 1, fp);
        fread(Div_n_max,                   2 * sizeof(uInt32),                 1, fp);
        fread(&NumberOfDistinctAccesses,   sizeof(NumberOfDistinctAccesses),  1, fp);
        fread(&myWriteEnabled,             sizeof(myWriteEnabled),            1, fp);
        fread(&myDataHoldRegister,         sizeof(myDataHoldRegister),        1, fp);
        fread(&myWritePending,             sizeof(myWritePending),            1, fp);
        fread(&bPossibleLoad,              sizeof(bPossibleLoad),             1, fp);
        fread(&myNumberOfLoadImages,       sizeof(myNumberOfLoadImages),      1, fp);
    
        fread(&LastConfigurationAR,        sizeof(LastConfigurationAR),       1, fp);
        if (LastConfigurationAR != 255) SetConfigurationAR(LastConfigurationAR);
    
        fread(&myCyclesAtBankswitchInit,   sizeof(myCyclesAtBankswitchInit),  1, fp);
        fread(&myPendingBank,              sizeof(myPendingBank),             1, fp);
        
        static char tmp_buf[SOUND_SIZE * 2];
        fread(tmp_buf,                     SOUND_SIZE * 2,                    1, fp);
        memcpy(tia_buf, tmp_buf,           SOUND_SIZE * 2);

        fread(reg_sys,                     16 * sizeof(uInt32),               1, fp);
        fread(&cFlag,                      sizeof(cFlag),                     1, fp);
        fread(&cStack,                     sizeof(cStack),                    1, fp);
        fread(&cBase,                      sizeof(cBase),                     1, fp);
        fread(&cStart,                     sizeof(cStart),                    1, fp);

        fread(&myFastFetch,                sizeof(myFastFetch),               1, fp);
        fread(&myDPCPRandomNumber,         sizeof(myDPCPRandomNumber),        1, fp);
        fread(&myDPCPCycles,               sizeof(myDPCPCycles),              1, fp);
        fread(&myParameterPointer,         sizeof(myParameterPointer),        1, fp);
        fread(myFractionalCounters,        8 * sizeof(uInt32),                1, fp);
        fread(myFractionalIncrements,      8 * sizeof(uInt32),                1, fp);
        fread(myTops,                      8 * sizeof(uInt32),                1, fp);
        fread(myTopsMinusBottoms,          8 * sizeof(uInt32),                1, fp);
        fread(myBottoms,                   8 * sizeof(uInt32),                1, fp);
        fread(myCounters,                  8 * sizeof(uInt32),                1, fp);
        fread(myMusicCounters,             3 * sizeof(uInt32),                1, fp);
        fread(myMusicFrequencies,          3 * sizeof(uInt32),                1, fp);
        fread(myMusicWaveforms,            3 * sizeof(uInt32),                1, fp);
        fread(myMusicCountersShifted,      3 * sizeof(uInt32),                1, fp);
        fread(myParameter,                 8 * sizeof(uInt8),                 1, fp);
        
        fread(&myAmplitudeStream,          sizeof(myAmplitudeStream),         1, fp);
        fread(&myDataStreamFetch,          sizeof(myDataStreamFetch),         1, fp);    
        fread(&peekvalue,                  sizeof(peekvalue),                 1, fp);
        fread(&myMode,                     sizeof(myMode),                    1, fp);
        fread(&myLDXenabled,               sizeof(myLDXenabled),              1, fp);
        fread(&myLDYenabled,               sizeof(myLDYenabled),              1, fp);
        fread(&myFastFetcherOffset,        sizeof(myFastFetcherOffset),       1, fp);        
        fread(myMusicWaveformSize,         3 * sizeof(uInt16),                1, fp);
        
        fread(&bWriteOrLoadPossibleAR,     sizeof(bWriteOrLoadPossibleAR),    1, fp);
        
        if (bSaveStateXL && xl_ram_buffer)
        {
            fread(xl_ram_buffer,           32*1024,                           1, fp);   
        }

        fread(&myTunePosition,             sizeof(myTunePosition),            1, fp);
        fread(&myAudioCycles,              sizeof(myAudioCycles),             1, fp);
        fread(&deltaCyclesX10,             sizeof(deltaCyclesX10),            1, fp);
        fread(&myOperationType,            sizeof(myOperationType),           1, fp);            
        
        fclose(fp);

        for (int i=0; i<64; i++)
        {
            if (myPageOffsets[i].peek_type == TYPE_RAM)
            {
                myPageAccessTable[i].directPeekBase = &myRAM[myPageOffsets[i].peek_offset];
            }
            else if (myPageOffsets[i].peek_type == TYPE_CART)
            {
                myPageAccessTable[i].directPeekBase = &cart_buffer[myPageOffsets[i].peek_offset];
            }
            else if (myPageOffsets[i].peek_type == TYPE_FASTCART)
            {
                myPageAccessTable[i].directPeekBase = &fast_cart_buffer[myPageOffsets[i].peek_offset];
            }
            else if (myPageOffsets[i].peek_type == TYPE_XLRAM)
            {
                myPageAccessTable[i].directPeekBase = &xl_ram_buffer[myPageOffsets[i].peek_offset];
            }
            else
            {
                myPageAccessTable[i].directPeekBase = (uInt8 *)(uintptr_t)myPageOffsets[i].peek_offset;
            }

            if (myPageOffsets[i].poke_type == TYPE_RAM)
            {
                myPageAccessTable[i].directPokeBase = &myRAM[myPageOffsets[i].poke_offset];
            }
            else if (myPageOffsets[i].poke_type == TYPE_CART)
            {
                myPageAccessTable[i].directPokeBase = &cart_buffer[myPageOffsets[i].poke_offset];
            }
            else if (myPageOffsets[i].poke_type == TYPE_FASTCART)
            {
                myPageAccessTable[i].directPokeBase = &fast_cart_buffer[myPageOffsets[i].poke_offset];
            }
            else if (myPageOffsets[i].poke_type == TYPE_XLRAM)
            {
                myPageAccessTable[i].directPokeBase = &xl_ram_buffer[myPageOffsets[i].poke_offset];
            }
            else
            {
                myPageAccessTable[i].directPokeBase = (uInt8 *)(uintptr_t)myPageOffsets[i].poke_offset;
            }
        }

        bInitialDiffSet = true;
    }
}

void dsSaveStateHandler() {}
