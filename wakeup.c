/********************************************************************************************************
 * @file     wakeup.c
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
#include "../common.h"
#include "wakeup.h"
#include "app.h"

_attribute_data_retention_ static bool wake_up;

/**
 * @brief      This function serves to checkout if the system is wakeup from sleep
 * @param[in]  none
 * @return true if the system is wakeup from sleep
 */
bool is_wakeup_from_sleep ()
{
  return wake_up;
}

/**
 * @brief      This function serves to clear wakeup system flag
 * @param[in]  none
 * @return     none
 */
void clr_wakeup_flag ()
{
  wake_up = false;
}

/**
 * @brief      This function serves to set wakeup system flag
 * @param[in]  none
 * @return     none
 */
void set_wakeup_flag ()
{
  wake_up = true;
}
