/********************************************************************************************************
 * @file     n_timer.c
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
#ifndef __N_TIMER__
#define __N_TIMER__

#include "../common.h"

#define MS2TICK(MS)   (MS*16*1000)
#define TICK2MS(TICK) (TICK/1000/16)
#define US2TICK(US)   (US*16)
#define TICK2US(TICK) (TICK/16)

//time_after(current, timeout) returns true if the time current is after time timeout.
#define time_after(current, timeout) \
  ((int)(timeout) - (int)(current) < 0)

//static inline bool n_clock_time_exceed(u32 ref, u32 us)
//{
//  return time_after(clock_time(), (ref + (us*16)));
//}

static inline bool n_clock_time_exceed(u32 ref, u32 us)
{
  u32 time;
  u32 temp;

  time = clock_time();

  if(time >= ref)
    temp = time - ref;
  else{
    temp = 0xffffffff - ref;
    temp += time;
  }

  return (temp > (us * 16));
}
#endif
