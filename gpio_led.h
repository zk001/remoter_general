/********************************************************************************************************
 * @file     gpio_led.h
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
#if defined(GPIO_LED)
#ifndef _GPIO_LED_H__
#define _GPIO_LED_H__

#include "common.h"

typedef struct {
  const u32* led_table;
  u8 num;
}low_led_t;

#define GPIO_HAL_TURN_ON(led)	st(\
    gpio_set_func (led, AS_GPIO);\
    gpio_set_output_en (led, 1);\
    gpio_set_input_en (led, 0);\
    gpio_write (led, 1);)

#define GPIO_HAL_TURN_OFF(led)	st(\
    gpio_set_func (led, AS_GPIO);\
    gpio_set_output_en (led, 1);\
    gpio_set_input_en (led, 0);\
    gpio_write (led, 0);)

extern void gpio_led_alloc (const u32* led_arry, u8 num);
extern void gpio_led_on_off (u32 leds, u8 mode);
extern void pwm_gpio_led_on_off (u32 leds, u8 mode);
extern void timer_pwm_led_on_off (u32 leds, u8 mode);

#endif
#endif
