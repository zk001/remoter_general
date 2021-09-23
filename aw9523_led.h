/********************************************************************************************************
 * @file     aw9523_led.h
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
#if defined(AW9523_LED)
#ifndef _AW9523_LED_H_
#define _AW9523_LED_H_

#include "../drivers.h"
#include "../common.h"
#include "board.h"

#define RSTN_OUT_LOW()  st(\
    gpio_set_output_en(AW9523_RSTN, 1);\
    gpio_set_input_en(AW9523_RSTN, 0);\
    gpio_write(AW9523_RSTN, 0);)

#define RSTN_OUT_HIGH()  st(\
    gpio_set_output_en(AW9523_RSTN, 1);\
    gpio_set_input_en(AW9523_RSTN, 0);\
    gpio_write(AW9523_RSTN, 1);)

#define P0_IN_STATE    0x00
#define P1_IN_STATE    0x01
#define P0_OUT_STATE   0x02
#define P1_OUT_STATE   0x03
#define P0_CONF        0x04
#define P1_CONF        0x05
#define P0_INT_ENABLE  0x06
#define P1_INT_ENABLE  0x07
#define ID             0x10
#define GCR            0x11
#define P0_MOD_SWITCH  0x12
#define P1_MOD_SWITCH  0x13
#define STEP_DIM_SET0  0x20
#define STEP_DIM_SET1  0x21
#define STEP_DIM_SET2  0x22
#define STEP_DIM_SET3  0x23
#define STEP_DIM_SET4  0x24
#define STEP_DIM_SET5  0x25
#define STEP_DIM_SET6  0x26
#define STEP_DIM_SET7  0x27
#define STEP_DIM_SET8  0x28
#define STEP_DIM_SET9  0x29
#define STEP_DIM_SET10 0x2a
#define STEP_DIM_SET11 0x2b
#define STEP_DIM_SET12 0x2c
#define STEP_DIM_SET13 0x2d
#define STEP_DIM_SET14 0x2e
#define STEP_DIM_SET15 0x2f
#define SOFT_RESET     0x7f

#define AW9523_ADDRESS 0xb6//write address 0xb7 read address

#define PORT0_BIT_0  0
#define PORT0_BIT_1  1
#define PORT0_BIT_2  2
#define PORT0_BIT_3  3
#define PORT0_BIT_4  4
#define PORT0_BIT_5  5
#define PORT0_BIT_6  6
#define PORT0_BIT_7  7
#define PORT1_BIT_0  8
#define PORT1_BIT_1  9
#define PORT1_BIT_2  10
#define PORT1_BIT_3  11
#define PORT1_BIT_4  12
#define PORT1_BIT_5  13
#define PORT1_BIT_6  14
#define PORT1_BIT_7  15

extern void aw9523_init();
extern void aw9523_led_on_off(u32 leds, u8 mode);
extern void aw9523_led_breath(u32 leds, u8 tim);
extern void aw9523_set_led_bright(u32 leds, u8 level);
#endif
#endif
