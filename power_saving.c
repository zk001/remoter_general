/********************************************************************************************************
 * @file     power_saving.c
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
//#include "../common.h"
#include "common.h"
#include "n_timer.h"
#include "power_saving.h"
#include "app.h"

static u32 idle_tick;
static u32 sys_tick;

/**
 * @brief      This function serves to set time interval for sleep
 * @param[in]  ms   - the millisecond
 * @return     none
 */
void idle_time_for_sleep (u32 ms)
{
  idle_tick = MS2TICK(ms);
}

/**
 * @brief      This function serves to poll the idle time when no key is pressing
 * @param[in]  none
 * @return true if the system time is more than idle time
 */
bool poll_idle_time ()
{
  return n_clock_time_exceed (sys_tick, TICK2US(idle_tick));
}

/**
 * @brief      This function serves to record the system time
 * @param[in]  none
 * @return     none
 */
void reload_sys_time ()
{
  sys_tick = clock_time ();
}

