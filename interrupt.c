/********************************************************************************************************
 * @file     interrupt.c
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
#include "common.h"
#include "../../esb_ll/esb_ll.h"
#include "timer_clock.h"
#include "interrupt.h"

volatile unsigned int tx_irq_cnt_tx = 0;
volatile unsigned int tx_irq_cnt_invalid_pid = 0;
volatile unsigned int tx_irq_cnt_max_retry = 0;
volatile unsigned int tx_irq_cnt_tx_ds = 0;
volatile unsigned int tx_irq_cnt_rx_dr = 0;

volatile unsigned char rx_flag = 0;
volatile unsigned char ds_flag = 0;
volatile unsigned char maxretry_flag = 0;

#if defined(YIHE) || defined(YIHE_0932) || defined(RUIERTE)
const u8 rf_channel_select [] = {130, 140, 150, 160};
#elif defined (BIANJIEBAO)
const u8 rf_channel_select [] = {130, 140, 150, 160};
#else
const u8 rf_channel_select [] = {10, 44, 100, 140};
#endif

volatile unsigned char timer0_expire_flg = 0;
unsigned int timer0_irq_cnt = 0;

/**
 * @brief      This function serves to handler irq_handler
 * @param[in]  none
 * @return     none
 */
__attribute__((section(".ram_code")))__attribute__((optimize("-Os"))) void irq_handler(void)
{
  unsigned short src_rf = rf_irq_src_get ();
  unsigned char pipe = ESB_GetTXPipe ();

  if (src_rf & FLD_RF_IRQ_TX)
  {
    rf_irq_clr_src(FLD_RF_IRQ_TX);
    tx_irq_cnt_tx++;
  }

  if (src_rf & FLD_RF_IRQ_INVALID_PID)
  {
    rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
    tx_irq_cnt_invalid_pid++;
  }

  if (src_rf & FLD_RF_IRQ_RETRY_HIT)
  {
    rf_irq_clr_src(FLD_RF_IRQ_RETRY_HIT);
    tx_irq_cnt_max_retry++;
    maxretry_flag = 1;
    //adjust rptr
    ESB_UpdateTXFifoRptr(pipe);
  }

  if (src_rf & FLD_RF_IRQ_TX_DS)
  {
    rf_irq_clr_src(FLD_RF_IRQ_TX_DS);
    tx_irq_cnt_tx_ds++;
    ds_flag = 1;

  }

  if (src_rf & FLD_RF_IRQ_RX_DR)
  {
    rf_irq_clr_src(FLD_RF_IRQ_RX_DR);
    tx_irq_cnt_rx_dr++;
    // rx_flag = 1;
  }

  if (src_rf & FLD_RF_IRQ_RX)
  {
    rf_irq_clr_src(FLD_RF_IRQ_RX);
    // tx_irq_cnt_rx_dr++;
    rx_flag = 1;
  }

  if (reg_tmr_sta & FLD_TMR_STA_TMR0) {
    reg_tmr_sta = FLD_TMR_STA_TMR0; //clear irq status
    timer0_irq_cnt ++;;
    timer0_expire_flg = 1;
    run_timer_handler();
  }

  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);
}
