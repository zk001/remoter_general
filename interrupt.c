#include "../drivers.h"
#include "led.h"
#include "../../esb_ll/esb_ll.h"
#include "timer_clock.h"

volatile unsigned int tx_irq_cnt_tx = 0;
volatile unsigned int tx_irq_cnt_invalid_pid = 0;
volatile unsigned int tx_irq_cnt_max_retry = 0;
volatile unsigned int tx_irq_cnt_tx_ds = 0;
volatile unsigned int tx_irq_cnt_rx_dr = 0;

volatile unsigned char rx_flag = 0;
volatile unsigned char ds_flag = 0;
volatile unsigned char maxretry_flag = 0;
const u8 rf_channel_select[] = {10, 44, 100, 140};

volatile unsigned char timer0_expire_flg = 0;
unsigned int timer0_irq_cnt = 0;

__attribute__((section(".ram_code")))__attribute__((optimize("-Os"))) void irq_handler(void)
{
  unsigned short src_rf = rf_irq_src_get();
  unsigned char pipe = ESB_GetTXPipe();

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

  if(reg_tmr_sta & FLD_TMR_STA_TMR0){
    reg_tmr_sta = FLD_TMR_STA_TMR0; //clear irq status
    timer0_irq_cnt ++;;
    timer0_expire_flg = 1;
    run_timer_handler();
  }

  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);
}

