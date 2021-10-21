/********************************************************************************************************
 * @file     timer_clock.c
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
#include "../drivers.h"
#include "timer_clock.h"

timer_handler_t timer_handler;

/**
 * @brief      This function serves to clear timer handler
 * @param[in]  none
 * @return     none
 */
void clr_timer_handler ()
{
  timer_handler.t_handler = NULL;
  timer_handler.para      = 0;
}

/**
 * @brief      This function serves to set timer handler
 * @param[in]  timer_hand - the timer handler
 * @return     none
 */
void set_timer_handler (timer_handler_t* timer_hand)
{
  timer_handler.t_handler = timer_hand->t_handler;
  timer_handler.para      = timer_hand->para;
}

/**
 * @brief      This function serves to set timer handler parameter
 * @param[in]  timer_hand - the timer handler
 * @return     none
 */
void set_timer_handler_para (timer_handler_t* timer_hand)
{
  timer_handler.para = timer_hand->para;
}

/**
 * @brief      This function serves to run timer handler, it will be invoked in interrupt function
 * @param[in]  none
 * @return     none
 */
void run_timer_handler ()
{
  timer_event_handler t_handler;
  u32 para;

  t_handler  = timer_handler.t_handler;
  para       = timer_handler.para;

  if (t_handler)
    t_handler (para);
}

/**
 * @brief      This function serves to configure timer clock
 * @param[in]  us  - the timer interrupt interval,microsecond
 * @return     none
 */
void timer0_clock_init (u32 us)
{
  timer0_set_mode (TIMER_MODE_SYSCLK, 0, us*CLOCK_SYS_CLOCK_1US);
  timer_start (TIMER0);
  irq_enable ();
}

/**
 * @brief      This function serves to stop timer clock
 * @param[in]  none
 * @return     none
 */
void timer0_clock_stop ()
{
  timer_stop (TIMER0);
}
