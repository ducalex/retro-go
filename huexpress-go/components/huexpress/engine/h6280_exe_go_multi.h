TaskHandle_t h6280_int_TaskHandle;
QueueHandle_t h6280_int_Queue;

uchar touch_global;
volatile uchar I_global;

#define USE_QUEUE

void h6280_int_Task(void *arg)
{
    gfx_init();
    #define I I_global
    GFX_Loop6502_Init
    uchar old = touch_global;
    uchar *param;
        
    while (true)
    {
#ifdef USE_QUEUE
        xQueuePeek(h6280_int_Queue, &param, portMAX_DELAY);
#endif

#ifdef MY_INLINE_GFX_Loop6502
{
            #include "gfx_Loop6502.h"
            //I = Loop6502_2();     /* Call the periodic handler */
}
#else
            I = Loop6502();     /* Call the periodic handler */
#endif
#ifdef USE_QUEUE
        xQueueReceive(h6280_int_Queue, &param, portMAX_DELAY);
#else
        usleep(5000);
#endif
    }
    #undef I
}

void
exe_go(void)
{
    touch_global = 0;
    I_global = 0;
#ifdef USE_QUEUE
    h6280_int_Queue = xQueueCreate(1, sizeof(uchar*));
#endif
    xTaskCreatePinnedToCore(h6280_int_Task, "intTask", 1024 * 4, NULL, 5, &h6280_int_TaskHandle, 1);
/*    flnz_list = (uchar *)my_special_alloc(false, 1,256);
    {
        memset(flnz_list,0,256);
        flnz_list[0] = FL_Z;
        for (int i = 0x80;i<256;i++)
            flnz_list[i] = FL_N;
    }
  */
    
    uint32 CycleNew = 0;
#ifndef MY_h6280_INT_cycle_counter
    uint32 cyclecountold = 0;
#endif
    //uint32 cycles = 0;
    uint32 cycles = 0;
    uchar I;

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

#ifdef USE_QUEUE
            xQueueSend(h6280_int_Queue, &touch_global, portMAX_DELAY);            
#endif

            ODROID_DEBUG_PERF_START2(debug_perf_int)
            // _ICount += _IPeriod;
            /* Reset the cycle counter */
            cycles = 0;

            // Log("Requested interrupt is %d\n",I);

            /* if (I == INT_QUIT) return; */
            I = I_global;
            I_global = 0;
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