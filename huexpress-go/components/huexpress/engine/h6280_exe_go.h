
void
exe_go(void)
{
#ifdef BENCHMARK
    static int countNb = 8;     /* run for 8 * 65536 scan lines */
    static int countScan = 0;   /* scan line counter */
    static double lastTime;
    static double startTime;
    startTime = lastTime = osd_getTime();
#endif
/*    flnz_list = (uchar *)my_special_alloc(false, 1,256);
    {
        memset(flnz_list,0,256);
        flnz_list[0] = FL_Z;
        for (int i = 0x80;i<256;i++)
            flnz_list[i] = FL_N;
    }
  */  gfx_init();
    uchar I;
    GFX_Loop6502_Init
    
    uint32 CycleNew = 0;
#ifndef MY_h6280_INT_cycle_counter
    uint32 cyclecountold = 0;
#endif
    //uint32 cycles = 0;
    uint32 cycles = 0;

// Slower!
//#undef reg_pc
  //  WORD_ALIGNED_ATTR uint16 reg_pc = reg_pc_;

//#undef reg_a
 //   uchar reg_a = 0;

    while (true) {
      ODROID_DEBUG_PERF_START2(debug_perf_total)

      ODROID_DEBUG_PERF_START2(debug_perf_part1)
      while (cycles<=455)
      {
#ifdef USE_INSTR_SWITCH
        #include "h6280_instr_switch.h"
#else
        /*err =*/ (*optable_runtime[PageR[reg_pc >> 13][reg_pc]].func_exe) ();
#endif
      }

      ODROID_DEBUG_PERF_INCR2(debug_perf_part1, ODROID_DEBUG_PERF_CPU)

        // HSYNC stuff - count cycles:
        /*if (cycles > 455) */ {
#ifdef BENCHMARK
            countScan++;
            if ((countScan & 0xFFFF) == 0) {
                double currentTime = osd_getTime();
                printf("%f\n", currentTime - lastTime);
                lastTime = currentTime;
                countNb--;
                if (countNb == 0)
                {
                    printf("RESULT: %f\n", currentTime - startTime);
                    esp_restart();
                    //abort();
                }
            }
#endif

            CycleNew += cycles;
            // cycles -= 455;
            // scanline++;

            // Log("Calling periodic handler\n");
#ifdef MY_INLINE_GFX_Loop6502
{
            #include "gfx_Loop6502.h"
            //I = Loop6502_2();     /* Call the periodic handler */
}
#else
            I = Loop6502();     /* Call the periodic handler */
#endif
            ODROID_DEBUG_PERF_START2(debug_perf_int)
            // _ICount += _IPeriod;
            /* Reset the cycle counter */
            cycles = 0;

            // Log("Requested interrupt is %d\n",I);

            /* if (I == INT_QUIT) return; */

            if (I)
                Int6502(I);     /* Interrupt if needed  */

#ifdef MY_h6280_INT_cycle_counter
            if ((unsigned int) (CycleNew ) > (unsigned int) TimerPeriod * 2) CycleNew = 0;
#else
            if ((unsigned int) (CycleNew - cyclecountold) >
                (unsigned int) TimerPeriod * 2) cyclecountold = CycleNew;
#endif

            ODROID_DEBUG_PERF_INCR2(debug_perf_int, ODROID_DEBUG_PERF_INT)
        } /*else*/ {
            ODROID_DEBUG_PERF_START2(debug_perf_int2)
#ifdef MY_h6280_INT_cycle_counter
            if (CycleNew  >= TimerPeriod) {
                CycleNew-= TimerPeriod;
#else
            if (CycleNew - cyclecountold >= TimerPeriod) {
                cyclecountold += TimerPeriod;
#endif
#ifdef MY_INLINE
                if (io.timer_start) {
                    io.timer_counter--;
                    if (io.timer_counter > 128) {
                        io.timer_counter = io.timer_reload;
                        if (!(io.irq_mask & TIRQ)) {
                            io.irq_status |= TIRQ;
                            I = INT_TIMER;
                        } else I = INT_NONE;
                    } else I = INT_NONE;
                } else I = INT_NONE;
#else
                I = TimerInt();
#endif
                if (I)
                    Int6502(I);
            }
            ODROID_DEBUG_PERF_INCR2(debug_perf_int2, ODROID_DEBUG_PERF_INT2)
        }
        ODROID_DEBUG_PERF_INCR2(debug_perf_total, ODROID_DEBUG_PERF_TOTAL)
    }
    MESSAGE_ERROR("Abnormal exit from the cpu loop\n");
}