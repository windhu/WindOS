#include "util.h"
#include "core_cm4.h"

// Can get from HAL_RCC_GetSysClockFreq()
#define SYSTEM_CLK_FREQ  168000000U

#if 0
void delay_us(uint32_t us) {
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
  
    ticks = us * (SYSTEM_CLK_FREQ / 1000000); 
  
    told = SysTick->VAL;
    while (1) {
        tnow = SysTick->VAL;
        if (tnow != told) {
            tcnt += (told < tnow) ? (told - tnow) : (SysTick->LOAD - tnow + told);
            told = tnow;
            if (tcnt >= ticks) break;
        }
    }
  }
#endif

void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD; 
    ticks = nus * (SYSTEM_CLK_FREQ / 1000000);
    
#if SYS_SUPPORT_OS 
    delay_osschedlock();
#endif

    told = SysTick->VAL; 
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow; 
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) 
            {
                break;
            }
        }
    }

#if SYS_SUPPORT_OS 
    delay_osschedunlock();
#endif 
}