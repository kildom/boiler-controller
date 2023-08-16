
#include "stm32l5xx_hal.h"
#include "stm32l5xx_ll_tim.h"
#include "main.h"
#include "lowlevel.hh"

static volatile bool timerCapturedIntr = true;

uint32_t get_time()
{
    uint32_t cnt = LL_TIM_GetCounter(htim2.Instance);
    return cnt >> 1;
}

void timeout(uint32_t t)
{
    uint32_t cnt;
    uint32_t new_cnt = LL_TIM_GetCounter(htim2.Instance);

    t <<= 1;

    do {
        cnt = new_cnt;
        int32_t diff = t - cnt;
        if (diff < 1) {
            t = cnt + 1;
        } else if (diff > PERIODIC_TIMEOUT) {
            return;
        }
        LL_TIM_OC_SetCompareCH1(htim2.Instance, t);
        new_cnt = LL_TIM_GetCounter(htim2.Instance);
    } while (cnt != new_cnt);
}

void init_time()
{
    HAL_TIM_Base_Start(&htim2);
    LL_TIM_OC_SetCompareCH1(htim2.Instance, (uint32_t)(-1));
    LL_TIM_EnableIT_CC1(htim2.Instance);
}


bool is_timeout()
{
    if (LL_TIM_IsActiveFlag_CC1(htim2.Instance) || timerCapturedIntr) {
        LL_TIM_ClearFlag_CC1(htim2.Instance);
        timerCapturedIntr = false;
        return true;
    }
    return false;
}


extern "C"
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    timerCapturedIntr = true;
}
