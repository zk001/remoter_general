/********************************************************************************************************
 * @file     rf.h
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
#ifndef __RF_H__
#define __RF_H__

extern void send_rf_data(void *addr, u32 len);
extern u8 receive_rf_data(void *addr);
extern void rf_8359_set_tx(void);
extern void rf_8359_set_rx(void);
extern void decrease_rf_power_tx();
extern void send_rf_data_kemu(void *addr, u32 len);
extern void send_rf_data_ruierte(void *addr, u32 len);
extern void send_rf_data_yihe(void *addr, u32 len);
extern bool receive_from_peer(u32 uid);

#endif
