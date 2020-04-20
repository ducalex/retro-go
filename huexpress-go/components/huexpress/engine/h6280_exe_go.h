
void
exe_go(void)
{
    gfx_init();
    uchar I;

    uint32 CycleNew = 0;
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
            CycleNew += cycles;
            // cycles -= 455;
            // scanline++;

            // Log("Calling periodic handler\n");

            I = Loop6502();     /* Call the periodic handler */

            ODROID_DEBUG_PERF_START2(debug_perf_int)
            // _ICount += _IPeriod;
            /* Reset the cycle counter */
            cycles = 0;

            // Log("Requested interrupt is %d\n",I);

            /* if (I == INT_QUIT) return; */

            if (I)
                Int6502(I);     /* Interrupt if needed  */

            if ((unsigned int) (CycleNew - cyclecountold) >
                (unsigned int) TimerPeriod * 2) cyclecountold = CycleNew;

            ODROID_DEBUG_PERF_INCR2(debug_perf_int, ODROID_DEBUG_PERF_INT)
        } /*else*/ {
            ODROID_DEBUG_PERF_START2(debug_perf_int2)
            if (CycleNew - cyclecountold >= TimerPeriod) {
                cyclecountold += TimerPeriod;
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