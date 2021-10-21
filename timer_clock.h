/********************************************************************************************************
 * @file     timer_clock.h
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
#ifndef TIMER_CLOCK
#define TIMER_CLOCK

#include "../drivers.h"

#define CLOCK_SYS_CLOCK_HZ  	24000000		// define system clock

enum{
  CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,				///< system tick per 1 second
  CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),		///< system tick per 1 millisecond
  CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),   ///< system tick per 1 microsecond
};

typedef void (*timer_event_handler)(u32);

typedef struct {
  timer_event_handler t_handler;
  u32 para;
}timer_handler_t;

extern void timer0_clock_init (u32 us);
extern void timer0_clock_stop ();
extern void run_timer_handler ();
extern void set_timer_handler (timer_handler_t *timer_hand);
extern void clr_timer_handler ();

#endif
