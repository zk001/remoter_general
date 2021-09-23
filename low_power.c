/********************************************************************************************************
 * @file     low_power.c
 *
 * @brief    This is the source file for TLSR8258
 *
 * @author	 Driver Group
 * @date     Sep 22, 2021
 *
 * @par      Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 * @par      History:
 * 			 1.initial release(DEC. 26 2018)
 *
 * @version  A001
 *
 *******************************************************************************************************/
#if (defined LOW_POWER_DETECT)
#include "../drivers.h"
#include "low_power.h"
#include "board.h"
#include "app.h"
#include "n_timer.h"

u32 sample_result[32];
u32 low_bat_process_time;
u32 low_bat_start_time;
low_bat_finish* low_bat_finish_cb;
bool  low_bat_warnning;
u32  power_threshold;

/**
 * @brief      This function serves to checkout if the system is in low power status
 * @param[in]  threshold - the low power threshold
 * @return 1 if it is in low power status, otherwise it is in normal power status
 */
bool is_low_power(u32 threshold)
{
  u32 sum = 0;
  u32 avg;
  u8 len = sizeof(sample_result)/sizeof(sample_result[0]);

  adc_init();
  adc_vbat_init(ADC_INPUT_PIN);
  adc_power_on_sar_adc(1);

  for(u8 i = 0; i < len; i++)
    sample_result[i] = adc_sample_and_get_result();

  for(u8 i = 0; i < len; i++)
    sum += sample_result[i];

  avg = sum/len;

  return (avg < threshold);
}

/**
 * @brief      This function serves to handler something when the system is in low power status during the low power time
 * @param[in]  threshold    - the low power threshold
 * @param[in]  cb_warn      - the callback will be invoked when the system is in low power status immediately
 * @param[in]  cb_finsish   - the callback will be invoked when the warning time overflow the parameter low_bat_time
 * @param[in]  low_bat_time - the low power warning last time
 * @return 1 if it is in low power status, otherwise it is in normal power status
 */
bool low_bat_chk(u32 threshold, low_bat_warn* cb_warn, low_bat_finish* cb_finsish, u32 low_bat_time)
{
  if(is_low_power(threshold)){
    if(cb_warn)
      cb_warn();
    if(cb_finsish)
      low_bat_finish_cb = cb_finsish;
    low_bat_warnning = 1;
    power_threshold = threshold;
    low_bat_process_time = low_bat_time;
    low_bat_start_time = clock_time();
    return 1;
  }
  return 0;
}

/**
 * @brief      This function serves to checkout if the registered low_bat_time is reached
 * @param[in]  none
 * @return     none
 */
void low_bat_update()
{
  if(low_bat_start_time){
    if(n_clock_time_exceed(low_bat_start_time, low_bat_process_time)){
      low_bat_start_time = 0;
      low_bat_warnning = 0;
      if(low_bat_finish_cb)
        low_bat_finish_cb();
    }
  }
}

/**
 * @brief      This function serves to return if the system is in low power status or not
 * @param[in]  none
 * @return 1 if it is in low power status, otherwise it is in normal power status
 */
bool is_bat_warn()
{
  return low_bat_warnning;
}

/**
 * @brief      This function serves to clear systeme low power status
 * @param[in]  none
 * @return     none
 */
void clr_bat_warn()
{
  low_bat_warnning = 0;
}

/**
 * @brief      This function serves to return low power threshold
 * @param[in]  none
 * @return the low power threshold
 */
u32 low_power_threshold()
{
  return power_threshold;
}
#endif
