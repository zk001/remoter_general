/********************************************************************************************************
 * @file     interrupt.h
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
extern u8 rf_channel_select[];

extern volatile unsigned char timer0_expire_flg;
extern unsigned int timer0_irq_cnt;

#endif
