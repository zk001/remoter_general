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
#include "common.h"
#include "matrix_gpio_key.h"

//general
#include "n_timer.h"
#include "key.h"
#include "gpio_set.h"

//vendor
#include "board.h"
#include "app.h"
//driver = row
//scan   = col

/*-------------------low level valid-------------------------*/
//init&idle
//driver pins set input with 1m pull_up resistor
//scan   pins set input with 1m pull_up resistor

//scan
//one driver pin output low other driver pins input
//all scan pins set input 1m pull_up resistor
//if scan pins is low_level, the key is pressing

/*-------------------high level valid------------------------*/
//init&idle
//driver pins set input with 100k pull_down resistor
//scan   pins set input with 100k pull_down resistor

//scan
//one driver pin output high other driver pins input
//all scan pins set input with 100k pull_down resistor
//if scan pins is high_level, the key is pressing

/*--------------------wakeup-------------------------------*/
//sleep&low_level wakeup
//driver pins set 100k pull_down resistor
//scan pins set 1m pull_up resistor
//scan pins set low_level wakeup

//sleep&high_level wakeup
//driver pins set 1m pull_down resistor
//scan pins set 100k pull_down resistor
//scan pins set high_level wakeup

static u32  debounce_time [MAX_GPIO_KEYS];
static bool stable_status [MAX_GPIO_KEYS];
_attribute_data_retention_ static u32 key_valid_level;
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
static inline bool read_gpio (u32 scan_pin)
{
  return gpio_read (scan_pin);
}

/**
 * @brief      This function serves to scan matrix gpio key,high level scan
 * @param[in]  key   - the local key relative register
 * @return the local key level, 0 no pressing,1 pressing
 */
static bool key_scan_high_valid (const key_map_t* key)
{
  bool status;

  SET_GPIO_OUTPUT_HIGH(key->driver_pin);
  SET_GPIO_INPUT_WITH_PULLDOWN(key->scan_pin);

  status = read_gpio (key->scan_pin) ? true:false;

  SET_GPIO_INPUT_WITH_PULLDOWN(key->driver_pin);
  SET_GPIO_INPUT_WITH_PULLDOWN(key->scan_pin);

  return status;
}

/**
 * @brief      This function serves to scan matrix gpio key,low level scan
 * @param[in]  key   - the local key relative register
 * @return the local key level, 0 no pressing,1 pressing
 */
static bool key_scan_low_valid (const key_map_t* key)
{
  bool status;

  SET_GPIO_OUTPUT_LOW(key->driver_pin);
  SET_GPIO_INPUT_WITH_PULLUP(key->scan_pin);

  status = read_gpio (key->scan_pin) ? false:true;

  SET_GPIO_INPUT_WITH_PULLUP(key->driver_pin);
  SET_GPIO_INPUT_WITH_PULLUP(key->scan_pin);

  return status;
}

/**
 * @brief     This function serves to get gpio key scan level
 * @param[in] local_key - the local_key to scan
 * @return    the scan level, 0 low level scan, 1 high level scan
 */
static bool scan_level (u8 local_key)
{
  return key_valid_level & (1 << local_key);
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
 * @brief     This function serves to set gpio key scan level
 * @param[in] driver_pin - the driver_pin to set
 * @param[in] valid_level - the scan level
 * @return    none
 */
void gpio_key_level_valid (u32 driver_pin, bool valid_level)
{
  if (gpio_key_map.map && gpio_key_map.num) {
    for (u8 i = 0; i < gpio_key_map.num; i++) {
      if (gpio_key_map.map[i].driver_pin == driver_pin) {
        key_valid_level &= ~(1 << i);
		key_valid_level |= (valid_level << i);
	  }
    }
  }
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
      if ((key_valid_level & (1 << i)) == 0) {
        SET_GPIO_INPUT_WITH_PULLUP(gpio_key_map.map[i].driver_pin);
        SET_GPIO_INPUT_WITH_PULLUP(gpio_key_map.map[i].scan_pin);
      } else {
    	SET_GPIO_INPUT_WITH_PULLDOWN(gpio_key_map.map[i].driver_pin);
    	SET_GPIO_INPUT_WITH_PULLDOWN(gpio_key_map.map[i].scan_pin);
      }
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
  u32 scan_pin;
  u8 local_key;

  if (gpio_key_map.map && gpio_key_map.num) {
    local_gpio_key (key, &local_key);

    gpio_key_map.map[local_key].is_wake_up_pin = NO_WAKE_UP;

    scan_pin = gpio_key_map.map[local_key].scan_pin;

    for (u8 i = 0; i < gpio_key_map.num; i++) {
      if (gpio_key_map.map[i].scan_pin == scan_pin && i != local_key)
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
        SET_DRIVER_GPIO_WITH_DEEPSLEEP_HIGH(gpio_key_map.map[i].driver_pin);
    }

    for (u8 i = 0; i < gpio_key_map.num; i++) {
      if (gpio_key_map.map[i].is_wake_up_pin == IS_WAKE_UP) {
    	SET_SCAN_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].scan_pin);
        SET_DRIVER_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].driver_pin);
      }
    }
  }
}

static key_status_t key_filter (u8 local_key, bool cur_status)
{
  static bool is_filter = false;
  u32 cur_time;
  u32 filter_time;

  cur_time = clock_time ();
  filter_time = debounce_time[local_key];

  if (!is_filter) {//no filter
    stable_status[local_key] = cur_status;
    if (is_last_local_key (local_key))//all key scan finished
      is_filter = true;
  } else {
    if (filter_time) { //debounce status
      if (((u32)((int)cur_time - (int)filter_time)) >= DEBOUNCE_TIME) { //check if debounce finished
        stable_status[local_key] = cur_status;//store cur_status as stable status
        debounce_time[local_key] = 0;//clear debounce time
      }
    } else {//stable status
      if (cur_status != stable_status[local_key])//check if debounce occrued
        debounce_time[local_key] = clock_time();//store debounce start time
    }
  }
  return stable_status[local_key]?PRESSING:RELEASE;
}

/**
 * @brief       This function serves to read the specified gpio key whether is pressing or not
 * @param[out]  key_s - specified the key is pressing or not
 * @param[in]   key   - the global key of gpio key
 * @return      none
 */
void gpio_key_low_scan (key_status_t* key_s, key_index_t key)
{
  bool cur_status;
  bool (*scan) (const key_map_t* key);
  u8 local_key;
  key_map_t* key_row_col;

  if (gpio_key_map.map && gpio_key_map.num) {
    local_gpio_key (key, &local_key);

    if (scan_level(local_key))
      scan = key_scan_high_valid;
    else
      scan = key_scan_low_valid;

    key_row_col = key_map (local_key);

    cur_status = scan (key_row_col);

    *key_s = key_filter (local_key, cur_status);
  }
}
#endif
