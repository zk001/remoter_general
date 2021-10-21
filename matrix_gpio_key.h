/********************************************************************************************************
 * @file     gpio_key.h
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
#if defined(GPIO_KEY)
#ifndef __MATRIX_GPIO_KEY_H__
#define __MATRIX_GPIO_KEY_H__

enum wake_up_pin_t {
  IS_WAKE_UP = 1,
  NO_WAKE_UP = 2
};

typedef struct {
  u32 row;
  u32 col;
  enum wake_up_pin_t is_wake_up_pin;
}key_map_t;

extern void gpio_key_alloc (key_map_t* key_arry, u8 num);
extern void gpio_key_init (u8 first_key, u8 last_key);
extern void gpio_key_sleep_setup ();
extern void gpio_key_enable_sleep (u8 key);
extern void gpio_key_disable_sleep (u8 key);
extern void gpio_key_low_scan (key_status_t* key_s, key_index_t key);
#endif
#endif
