/********************************************************************************************************
 * @file     i2c_gpio_set.c
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
#include "../../drivers.h"
#include "i2c_gpio_set.h"

/**
 * @brief      This function serves to turn off i2c when system is entered sleep mode
 * @param[in]  pin_sda  - the i2c sda pin
 * @param[in]  pin_clk  - the i2c clk pin
 * @return     none
 */
void i2c_gpio_set_deepsleep (u32 pin_sda, u32 pin_clk)
{
  gpio_set_func (pin_sda, AS_GPIO);
  gpio_set_input_en (pin_sda, 1);
  gpio_set_output_en (pin_sda, 0);
  gpio_setup_up_down_resistor (pin_sda, PM_PIN_PULLDOWN_100K);

  gpio_set_func (pin_clk, AS_GPIO);
  gpio_set_input_en (pin_clk, 1);
  gpio_set_output_en (pin_clk, 0);
  gpio_setup_up_down_resistor (pin_clk, PM_PIN_PULLDOWN_100K);
}
