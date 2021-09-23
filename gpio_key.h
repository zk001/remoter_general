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
#ifndef __MECHANICAL_H_
#define __MECHANICAL_H_

#define SET_ROW_GPIO_OUTPUT_HIGH(row) st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_set_input_en(row, 0);\
    gpio_set_output_en(row, 1);\
    gpio_write(row, 1);)

#define SET_ROW_GPIO_OUTPUT_LOW(row) st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_set_input_en(row, 0);\
    gpio_set_output_en(row, 1);\
    gpio_write(row, 0);)

#define SET_ROW_GPIO_OUTPUT_HIGH_Z(row) st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_shutdown(row);)

#define SET_ROW_GPIO_INPUT(row)  st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_set_input_en(row, 1);\
    gpio_set_output_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLUP_1M);)

#define SET_COL_GPIO_INPUT(col) st(\
    gpio_set_func(col, AS_GPIO);\
    gpio_set_input_en(col, 1);\
    gpio_set_output_en(col, 0);\
    gpio_setup_up_down_resistor(col, PM_PIN_PULLUP_1M);)

#define SET_ROW_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(row) st(\
    gpio_set_output_en(row, 0);\
    gpio_set_input_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLDOWN_100K);)

#define SET_COL_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(col) st(\
    gpio_set_output_en(col, 0);\
    gpio_set_input_en(col, 0);\
    gpio_setup_up_down_resistor(col, PM_PIN_PULLUP_1M);\
    cpu_set_gpio_wakeup(col, 0, 1);)

#define SET_ROW_GPIO_WITH_DEEPSLEEP_HIGH_WAKEUP(row) st(\
    gpio_set_output_en(row, 0);\
    gpio_set_input_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLUP_1M);)

#define SET_COL_GPIO_WITH_DEEPSLEEP_HIGH_WAKEUP(col) st(\
    gpio_set_output_en(col, 0);\
    gpio_set_input_en(col, 0);\
    gpio_setup_up_down_resistor(col, PM_PIN_PULLUP_1M);\
    cpu_set_gpio_wakeup(col, 1, 1);)

#define SET_ROW_GPIO_WITH_DEEPSLEEP_HIGH(row) st(\
    gpio_set_output_en(row, 0);\
    gpio_set_input_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLUP_10K);)

enum wake_up_pin_t{
  IS_WAKE_UP = 1,
  NO_WAKE_UP = 2
};

typedef struct {
  u32 row;
  u32 col;
  enum wake_up_pin_t is_wake_up_pin;
}key_map_t;

extern void gpio_key_alloc(key_map_t* key_arry, u8 num);
extern void gpio_key_init(u8 first_key, u8 last_key);
extern void gpio_key_sleep_setup();
extern void gpio_key_enable_sleep(u8 key);
extern void gpio_key_disable_sleep(u8 key);
extern void gpio_key_low_scan(key_status_t* key_s, key_index_t key);
#endif
#endif
