#ifndef __INTERRUPT_H
#define __INTERRUPT_H

extern volatile unsigned int tx_irq_cnt_tx;
extern volatile unsigned int tx_irq_cnt_invalid_pid;
extern volatile unsigned int tx_irq_cnt_max_retry;
extern volatile unsigned int tx_irq_cnt_tx_ds;
extern volatile unsigned int tx_irq_cnt_rx_dr;

extern volatile unsigned char rx_flag;
extern volatile unsigned char ds_flag;
extern volatile unsigned char maxretry_flag;;
extern u8 rf_channel_select[];//ÆµµÀ

extern volatile unsigned char timer0_expire_flg;
extern unsigned int timer0_irq_cnt;

#endif
