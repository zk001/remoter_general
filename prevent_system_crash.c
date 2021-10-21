/********************************************************************************************************
 * @file     prevent_system_crash.c
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
#include "common.h"
#include "drivers.h"

/**
 * @brief      This function serves to set sws gpio port to input PM_PIN_PULLUP_1M
 * @param[in]  none
 * @return     none
 */
void prevent_system_crash ()
{
  gpio_set_func (GPIO_SWS, AS_GPIO);
  gpio_set_input_en (GPIO_SWS, 1);
  gpio_set_output_en (GPIO_SWS, 0);
  gpio_setup_up_down_resistor (GPIO_SWS, PM_PIN_PULLUP_1M);
}
