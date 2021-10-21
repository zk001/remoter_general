/********************************************************************************************************
 * @file     low_power.h
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
#if (defined LOW_POWER_DETECT)
#ifndef __LOW_POWER__
#define __LOW_POWER__

typedef void low_bat_warn (void);
typedef void low_bat_finish (void);

extern bool is_low_power (u32 threshold);
extern bool low_bat_chk (u32 threshold, low_bat_warn* cb_warn, low_bat_finish* cb_finsish, u32 low_bat_time);
extern void low_bat_update ();
extern bool is_bat_warn ();
extern u32 low_power_threshold ();
extern void clr_bat_warn ();

#endif
#endif
