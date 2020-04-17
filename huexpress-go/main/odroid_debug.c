#include "odroid_debug.h"
#include "esp_system.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#ifdef ODROID_DEBUG_PERF_USE

#define ODROID_DEBUG_PERF_DATA_MAX 512

int odroid_debug_perf_data_calls[512];
int odroid_debug_perf_data_time[512];

void odroid_debug_perf_init()
{
    memset(odroid_debug_perf_data_calls, 0, sizeof(int)*ODROID_DEBUG_PERF_DATA_MAX);
    memset(odroid_debug_perf_data_time, 0, sizeof(int)*ODROID_DEBUG_PERF_DATA_MAX);
}

#define time_div 1000

void odroid_debug_perf_log()
{
#ifdef ODROID_DEBUG_PERF_LOG_ALL
   int total = odroid_debug_perf_data_time[0];
   
   printf("##########################################################################################\n");
   
   for (int i = 0; i<ODROID_DEBUG_PERF_DATA_MAX/16;i++)
   {
    int offset = i*16;
    #define TIME_MIN (total/1000)
    
    printf(LOG_COLOR(LOG_COLOR_BLACK));
    
    #define cpu_calls odroid_debug_perf_data_time
    bool tmp = cpu_calls[offset+0]>TIME_MIN || cpu_calls[offset+1]>TIME_MIN || cpu_calls[offset+2]>TIME_MIN || cpu_calls[offset+3]>TIME_MIN ||
      cpu_calls[offset+4]>TIME_MIN || cpu_calls[offset+5]>TIME_MIN || cpu_calls[offset+6]>TIME_MIN || cpu_calls[offset+7]>TIME_MIN ||
      cpu_calls[offset+8]>TIME_MIN || cpu_calls[offset+9]>TIME_MIN || cpu_calls[offset+10]>TIME_MIN || cpu_calls[offset+11]>TIME_MIN ||
      cpu_calls[offset+12]>TIME_MIN || cpu_calls[offset+13]>TIME_MIN || cpu_calls[offset+14]>TIME_MIN || cpu_calls[offset+15]>TIME_MIN;
    if (!tmp) continue;
    
    #undef cpu_calls
    #define cpu_calls odroid_debug_perf_data_calls
    
    printf("0x%02X %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d COUNT\n",
      offset, cpu_calls[offset+0], cpu_calls[offset+1], cpu_calls[offset+2], cpu_calls[offset+3],
      cpu_calls[offset+4], cpu_calls[offset+5], cpu_calls[offset+6], cpu_calls[offset+7],
      cpu_calls[offset+8], cpu_calls[offset+9], cpu_calls[offset+10], cpu_calls[offset+11],
      cpu_calls[offset+12], cpu_calls[offset+13], cpu_calls[offset+14], cpu_calls[offset+15]
    );
    #undef cpu_calls
    #define cpu_calls odroid_debug_perf_data_time
    printf("0x%02X %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d TIME\n",
      offset, cpu_calls[offset+0]/time_div, cpu_calls[offset+1]/time_div, cpu_calls[offset+2]/time_div, cpu_calls[offset+3]/time_div,
      cpu_calls[offset+4]/time_div, cpu_calls[offset+5]/time_div, cpu_calls[offset+6]/time_div, cpu_calls[offset+7]/time_div,
      cpu_calls[offset+8]/time_div, cpu_calls[offset+9]/time_div, cpu_calls[offset+10]/time_div, cpu_calls[offset+11]/time_div,
      cpu_calls[offset+12]/time_div, cpu_calls[offset+13]/time_div, cpu_calls[offset+14]/time_div, cpu_calls[offset+15]/time_div
    );
    /*
    printf("0x%02X %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f TIME PROZ\n",
      offset, 100 * ((float)cpu_calls[offset+0])/total, 100 * ((float)cpu_calls[offset+1])/total, 100 * ((float)cpu_calls[offset+2])/total, 100 * ((float)cpu_calls[offset+3])/total,
      100 * ((float)cpu_calls[offset+4])/total, 100 * ((float)cpu_calls[offset+5])/total, 100 * ((float)cpu_calls[offset+6])/total, 100 * ((float)cpu_calls[offset+7])/total,
      100 * ((float)cpu_calls[offset+8])/total, 100 * ((float)cpu_calls[offset+9])/total, 100 * ((float)cpu_calls[offset+10])/total, 100 * ((float)cpu_calls[offset+11])/total,
      00 * ((float)cpu_calls[offset+12])/total, 100 * ((float)cpu_calls[offset+13])/total, 100 * ((float)cpu_calls[offset+14])/total, 100 * ((float)cpu_calls[offset+15])/total
    );
    */
    printf("0x%02X ", offset);
    bool col_other = false;
    for (int j = 0;j < 16; j++)
    {
        float f = 100 * ((float)cpu_calls[offset+j])/total;
        if (f>30)
        {
            printf(LOG_COLOR(LOG_COLOR_RED));
            col_other = true;
        }
        else
        if (f>0.3) // 20
        {
            printf(LOG_COLOR(LOG_COLOR_BROWN));
            col_other = true;
        }
        else
        if (f>=0.1)// 1
        {
            printf(LOG_COLOR(LOG_COLOR_GREEN));
            col_other = true;
        }
        else
        {
            if (col_other)
            {
                col_other = false;
                printf(LOG_COLOR(LOG_COLOR_BLACK));
            }
        }
        printf("%8.2f ", f);
    }
    printf("TIME PROZ\n");
   }
   printf(LOG_COLOR(LOG_COLOR_BLACK));
#endif
   odroid_debug_perf_init();
}

void odroid_debug_perf_log_specific(int call, int base)
{
    float f = 100 * ((float)base)/((float)odroid_debug_perf_data_time[call]);
    printf("0x%02X %8.2f%% (%16d)\n", call, f, odroid_debug_perf_data_time[call]);
}

void odroid_debug_perf_log_one(const char *text, int call)
{
    int total = odroid_debug_perf_data_time[0];
    float f = 100 * ((float)odroid_debug_perf_data_time[call])/total;
    printf("%-24s: %16d %16d    %8.2f%%\n",
        text,
        odroid_debug_perf_data_calls[call],
        odroid_debug_perf_data_time[call]/time_div,
        f
        ); 
}

#endif
