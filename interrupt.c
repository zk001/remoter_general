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
#include "gfsk_rf.h"
#include "../../genfsk_ll/genfsk_ll.h"

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
#if defined (FACTORY_TEST)
#define rec_buff_Len    16
#define trans_buff_Len  16
extern volatile unsigned char uart_rx_flag;
extern __attribute__((aligned(4))) unsigned char rec_buff[rec_buff_Len];
extern volatile unsigned int  uart_ndmairq_cnt;
extern volatile unsigned char uart_ndmairq_index;

_attribute_ram_code_ void irq_handler(void)
{
    static unsigned char uart_ndma_irqsrc;
    uart_ndma_irqsrc = uart_ndmairq_get();  ///get the status of uart irq.
    if(uart_ndma_irqsrc){

    //cycle the four registers 0x90 0x91 0x92 0x93,in addition reading will clear the irq.
        if(uart_rx_flag==0){
            rec_buff[uart_ndmairq_cnt++] = reg_uart_data_buf(uart_ndmairq_index);
            uart_ndmairq_index++;
            uart_ndmairq_index &= 0x03;// cycle the four registers 0x90 0x91 0x92 0x93, it must be done like this for the design of SOC.
            if(uart_ndmairq_cnt%16==0&&uart_ndmairq_cnt!=0){
                uart_rx_flag=1;
            }
        }
        else{
            read_reg8(0x90+ uart_ndmairq_index);
            uart_ndmairq_index++;
            uart_ndmairq_index &= 0x03;
        }
    }


}
#else
#if defined (GFSK_RF)
extern volatile  unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4)));
extern volatile  unsigned char rx_ptr;
extern volatile  unsigned char *rx_packet;
extern volatile  unsigned char rx_flag;
extern volatile  unsigned char *rx_payload;
extern volatile  unsigned char tx_done_flag;
__attribute__((section(".ram_code")))__attribute__((optimize("-Os"))) void irq_handler(void)
{
  unsigned int irq_src = irq_get_src();
  unsigned short src_rf = rf_irq_src_get ();
//  unsigned char pipe = ESB_GetTXPipe ();

  if (irq_src & FLD_IRQ_ZB_RT_EN) {//if rf irq occurs
      if (src_rf & FLD_RF_IRQ_TX) {//if rf tx irq occurs
    	  tx_done_flag = 1;
      }
      rf_irq_clr_src(FLD_RF_IRQ_ALL);
  }

  if (irq_src & FLD_IRQ_ZB_RT_EN) {//if rf irq occurs
      if (src_rf & FLD_RF_IRQ_RX) {//if rf rx irq occurs
//          rx_test_cnt++;
          rx_packet = rx_buf + rx_ptr*RX_BUF_LEN;

          //set to next rx_buf
          rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
          gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr*RX_BUF_LEN), RX_BUF_LEN);

          if (gen_fsk_is_rx_crc_ok((u8*)rx_packet)) {
              rx_flag = 1;
          }
      }
      rf_irq_clr_src(FLD_RF_IRQ_ALL);
  }

//  if (src_rf & FLD_RF_IRQ_TX)
//  {
//    rf_irq_clr_src(FLD_RF_IRQ_TX);
//    tx_irq_cnt_tx++;
//  }

//  if (src_rf & FLD_RF_IRQ_INVALID_PID)
//  {
//    rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
//    tx_irq_cnt_invalid_pid++;
//  }

//  if (src_rf & FLD_RF_IRQ_RETRY_HIT)
//  {
//    rf_irq_clr_src(FLD_RF_IRQ_RETRY_HIT);
//    tx_irq_cnt_max_retry++;
//    maxretry_flag = 1;
//    //adjust rptr
//    ESB_UpdateTXFifoRptr(pipe);
//  }

//  if (src_rf & FLD_RF_IRQ_TX_DS)
//  {
//    rf_irq_clr_src(FLD_RF_IRQ_TX_DS);
//    tx_irq_cnt_tx_ds++;
//    ds_flag = 1;
//
//  }

//  if (src_rf & FLD_RF_IRQ_RX_DR)
//  {
//    rf_irq_clr_src(FLD_RF_IRQ_RX_DR);
//    tx_irq_cnt_rx_dr++;
//    // rx_flag = 1;
//  }

//  if (src_rf & FLD_RF_IRQ_RX)
//  {
//    rf_irq_clr_src(FLD_RF_IRQ_RX);
//    // tx_irq_cnt_rx_dr++;
//    rx_flag = 1;
//  }

  if (reg_tmr_sta & FLD_TMR_STA_TMR0) {
    reg_tmr_sta = FLD_TMR_STA_TMR0; //clear irq status
    timer0_irq_cnt ++;
    timer0_expire_flg = 1;
    run_timer_handler();
  }

//  irq_clr_src();
//  rf_irq_clr_src(FLD_RF_IRQ_ALL);
  irq_clr_src2(FLD_IRQ_ALL);
}
#else
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
    timer0_irq_cnt ++;
    timer0_expire_flg = 1;
    run_timer_handler();
  }

  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);
}
#endif
#endif
