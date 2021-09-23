/********************************************************************************************************
 * @file     dc.c
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
#if defined(DC)
#include "../../common.h"
#include "dc.h"
#include "board.h"

/**
 * @brief      This function serves to set dc power on
 * @param[in]  none
 * @return     none
 */
void dc_power_on()
{
  gpio_set_func(DCDC_CE, AS_GPIO);
  gpio_set_output_en(DCDC_CE, 1);
  gpio_set_input_en(DCDC_CE, 0);
  gpio_write(DCDC_CE, DC_ENABLE_LEVEL);
}

/**
 * @brief      This function serves to set dc shutdown
 * @param[in]  none
 * @return     none
 */
void dc_shutdown()
{
  gpio_set_func(DCDC_CE, AS_GPIO);
  gpio_set_output_en(DCDC_CE, 1);
  gpio_set_input_en(DCDC_CE, 0);
  gpio_write(DCDC_CE, ~DC_ENABLE_LEVEL);
}
#endif
