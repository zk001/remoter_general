/********************************************************************************************************
 * @file     gpio_key.c
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
#if defined (GPIO_KEY)
#include "../../common.h"
#include "n_timer.h"
#include "key.h"
#include "board.h"
#include "matrix_gpio_key.h"
#include "app.h"
#include "gpio_set.h"

//init
//row set input with 1m pull_up resistor
//col set input with 1m pull_pu resistor

//sleep
//row set 10k pull_down resistor
//col set 1m pull_up resistor low_level wakeup 

//scan
//set scan row output low other row key input
//col input 1m pull_down resistor
//if col is low_level ,the key is pressing

static u32 debounce_time [MAX_GPIO_KEYS];
_attribute_data_retention_ static u8 gpio_first_key;
_attribute_data_retention_ static u8 gpio_last_key;

_attribute_data_retention_ static struct {
  key_map_t* map;
  u8 num;
} gpio_key_map;

/**
 * @brief      This function serves to translate global key to local key
 * @param[in]  key        - the global key
 * @param[out] local_key  - the local key
 * @return     none
 */
static void local_gpio_key (u8 key, u8* local_key)
{
  if (key < gpio_first_key)
    *local_key = 0;
  else if (gpio_last_key < key)
    *local_key = gpio_last_key - gpio_first_key;
  else
    *local_key = key - gpio_first_key;
}

/**
 * @brief      This function serves to check the local key is the last local key
 * @param[in]  key   - the local key
 * @return if the last local key or not
 */
static inline bool is_last_local_key (u8 key)
{
  return key == gpio_last_key - gpio_first_key;
}

/**
 * @brief      This function serves to set matrix gpio key relative registers to scan mode
 * @param[in]  key   - the row gpio relative registers and col gpio relative registers of the key
 * @return     none
 */
static void gpio_key_for_scan (const key_map_t* key)
{
  SET_ROW_GPIO_OUTPUT_LOW(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

/**
 * @brief      This function serves to set matrix gpio key relative registers to normal mode
 * @param[in]  key   - the row gpio register and col gpio relative registers of the key
 * @return     none
 */
static void gpio_key_for_no_scan (const key_map_t* key)
{
  SET_ROW_GPIO_INPUT(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

/**
 * @brief      This function serves to translate local key to local key relative registers
 * @param[in]  key   - the local key
 * @return the local key relative register
 */
static inline key_map_t* key_map (u8 key)
{
  return (key_map_t*)&gpio_key_map.map[key];
}

/**
 * @brief      This function serves to read col key level-register
 * @param[in]  key   - the local key
 * @return the local key level, 0 low level,1 high level
 */
static inline bool read_col_gpio (u32 col)
{
  return gpio_read (col);
}

/**
 * @brief      This function serves to scan matrix gpio key
 * @param[in]  key   - the local key relative register
 * @return the local key level, 0 low level,1 high level
 */
bool key_low_level_scan (const key_map_t* key)
{
  bool status;

  gpio_key_for_scan (key);

  status = read_col_gpio (key->col) ? false:true;

  gpio_key_for_no_scan (key);

  return status;
}

/**
 * @brief      This function serves to register gpio key port array
 * @param[in]  key_arry  - the gpio key port relative register array
 * @param[in]  num       - the length of the key_arry
 * @return     none
 */
void gpio_key_alloc (key_map_t* key_arry, u8 num)
{
  gpio_key_map.map = key_arry;
  gpio_key_map.num = num;
}

/**
 * @brief      This function serves to init gpio key
 * @param[in]  first_key - the global first key of gpio key
 * @param[in]  last_key  - the global last key of gpio key
 * @return     none
 */
void gpio_key_init (u8 first_key, u8 last_key)
{
  if (gpio_key_map.map && gpio_key_map.num) {
    for (u8 i = 0; i < gpio_key_map.num; i++) {
      SET_ROW_GPIO_INPUT(gpio_key_map.map[i].row);
      SET_COL_GPIO_INPUT(gpio_key_map.map[i].col);
    }
    gpio_first_key  = first_key;
    gpio_last_key   = last_key;
  }
}

/**
 * @brief     This function serves to enable gpio key wakeup system
 * @param[in] key - the global key of gpio key
 * @return    none
 */
void gpio_key_enable_sleep (u8 key)
{
  u8 local_key;

  if (gpio_key_map.map && gpio_key_map.num) {
    local_gpio_key (key, &local_key);
    gpio_key_map.map[local_key].is_wake_up_pin = IS_WAKE_UP;
  }
}

/**
 * @brief     This function serves to disable gpio key wakeup system
 * @param[in] key - the global key of gpio key
 * @return    none
 */
void gpio_key_disable_sleep (u8 key)
{
  u32 col;
  u8 local_key;

  if (gpio_key_map.map && gpio_key_map.num) {
    local_gpio_key (key, &local_key);

    gpio_key_map.map[local_key].is_wake_up_pin = NO_WAKE_UP;

    col = gpio_key_map.map[local_key].col;

    for (u8 i = 0; i < gpio_key_map.num; i++) {
      if (gpio_key_map.map[i].col == col && i != local_key)
        gpio_key_map.map[i].is_wake_up_pin = NO_WAKE_UP;
    }
  }
}


/**
 * @brief     This function serves to set gpio key if wakeup system or not
 * @param[in] none
 * @return    none
 */
void gpio_key_sleep_setup ()
{
  if (gpio_key_map.map && gpio_key_map.num) {
    for (u8 i = 0; i < gpio_key_map.num; i++) {
      if (gpio_key_map.map[i].is_wake_up_pin == NO_WAKE_UP)
        SET_ROW_GPIO_WITH_DEEPSLEEP_HIGH(gpio_key_map.map[i].row);
    }

    for (u8 i = 0; i < gpio_key_map.num; i++) {
      if (gpio_key_map.map[i].is_wake_up_pin == IS_WAKE_UP) {
        SET_COL_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].col);
        SET_ROW_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].row);
      }
    }
  }
}

/**
 * @brief       This function serves to read the specified gpio key whether is pressing or not
 * @param[out]  key_s - specified the key is pressing or not
 * @param[in]   key   - the global key of gpio key
 * @return      none
 */
void gpio_key_low_scan (key_status_t* key_s, key_index_t key)//key scan rate too low
{
  static bool wakeup_fast_scan = true;
  u8 local_key;
  u32 time;
  u32 cur_time;
  key_map_t* key_row_col;

  if (gpio_key_map.map && gpio_key_map.num) {
    local_gpio_key(key, &local_key);

    key_row_col = key_map(local_key);

    time = debounce_time[local_key];
    cur_time = clock_time();

    if (wakeup_fast_scan) {
      if (key_low_level_scan(key_row_col)) {
        if (!time)
          debounce_time[local_key] = clock_time();
        *key_s = PRESSING;
      } else {
        debounce_time[local_key] = 0;
        *key_s = RELEASE;
      }
      if (is_last_local_key(local_key)) {
        wakeup_fast_scan = false;
        WaitMs(20);//wait for debounce scan
      }
    } else {
      if (key_low_level_scan(key_row_col)) {
        if (!time) {
          debounce_time[local_key] = clock_time();
          *key_s = RELEASE;
        } else if (((u32)((int)cur_time - (int)time)) >= DEBOUNCE_TIME)
          *key_s = PRESSING;
        else
          *key_s = RELEASE;
      } else {
        debounce_time[local_key] = 0;
        *key_s = RELEASE;
      }
    }
  }
}

#endif
