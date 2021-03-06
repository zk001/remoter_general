/********************************************************************************************************
 * @file     apt8l08_key.c
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
#if defined(APT8_KEY)
#include "common.h"
#include "apt8l08_key.h"

//general
#include "n_timer.h"
#include "gpio_set.h"

//vendor
#include "board.h"
#include "main.h"

static const u8 APTT8L16ArrySensing [8] = {0x03, 0x04, 0x04, 0x03, 0x03, 0x04, 0x04, 0x04};
static const u8 APTTouchRegAdd [14] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d};
static const u8 APTTouchRegDat [14] = {0x03, 0x50, 0x20, 0x00, 0x00, 0x00, 0x08, 0x02, 0x02, 0x10, 0x10, 0x04, 0x00, 0x00};
_attribute_data_retention_ static const u8* apt8_cap_sense = APTT8L16ArrySensing;
_attribute_data_retention_ static const u8* apt8_reg_data  = APTTouchRegDat;

static bool stable_status [MAX_TOUCH_KEYS];
_attribute_data_retention_ static u8 apt8_first_key;
_attribute_data_retention_ static u8 apt8_last_key;
_attribute_data_retention_ static bool touch_key_set_sleep = true;

#if defined(APT_DEBOUNCE)
static u32 debounce_time [MAX_TOUCH_KEYS];
#endif

/**
 * @brief      This function serves to write one byte from the master device at the specified slave address
 * @param[in]  addr - i2c slave address where the one byte data will be written
 * @param[in]  data - the one byte data will be written via I2C interface
 * @return     none
 */
static void apt8_set_reg (u8 addr, u8 data)
{
  u8 rd_data; 
  u8 cnt = 10;

  do {
    i2c_write_series (addr, 1, (u8*)&data, 1);
    WaitUs (5);
    i2c_read_series (addr, 1, (u8*)&rd_data, 1);
    if (rd_data != data) {
      WaitMs (20);
      cnt--;
    }
  } while (rd_data != data && cnt);
}

/**
 * @brief     This function serves to enter apt8 configured mode
 * @param[in] none.
 * @return    none
 */
static void apt8_set_cfg ()
{
  apt8_set_reg (SYS_CON, 0x5a);
}

/**
 * @brief     This function serves to enter apt8 active mode
 * @param[in] none.
 * @return    none
 */
static void apt_set_active ()
{
  apt8_set_reg (SYS_CON, 0);
}

/**
 * @brief      This function serves to translate global key to local key
 * @param[in]  key - the global key
 * @return the local key translated from the global key
 */
static key_index_t local_touch_key (key_index_t key)
{
  if (key < apt8_first_key)
    return 0;

  if (apt8_last_key < key)
    return apt8_last_key - apt8_first_key;

  return key - apt8_first_key;
}

/**
 * @brief      This function serves to check the local key is the last local key
 * @param[in]  key   - the local key
 * @return if the last local key or not
 */
static inline bool is_last_local_key (u8 key)
{
  return key == apt8_last_key - apt8_first_key;
}

/**
 * @brief     This function serves to enter apt8 sleep mode
 * @param[in] none.
 * @return    none
 */
void apt8_enter_sleep ()
{
  i2c_gpio_set (APT8L08_I2C_PORT);  	//SDA/CK : B6/D7

  i2c_master_init (APT8_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*200000)));

  apt8_set_cfg ();
}

/**
 * @brief     This function serves to exit apt8 sleep mode
 * @param[in] none.
 * @return    none
 */
void apt8_exit_sleep ()
{
  i2c_gpio_set (APT8L08_I2C_PORT);  	//SDA/CK : B6/D7

  i2c_master_init (APT8_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*200000)));

  apt_set_active ();
}

/**
 * @brief      This function serves to write one byte from the slave device at the specified address
 * @param[in]  cap_sense     - the address of the cap_sense value array
 * @param[in]  cap_sense_cnt - the length of the cap_sense value array
 * @param[in]  reg_data      - the address of the apt8 register data array
 * @param[in]  reg_data_cnt  - the length of the reg_data value array
 * @return     none
 */
void apt8_set_cap_sense_and_reg_data (const u8* cap_sense, u8 cap_sense_cnt, const u8* reg_data, u8 reg_data_cnt)
{
  if (cap_sense && cap_sense_cnt == 8)
    apt8_cap_sense = cap_sense;  

  if (reg_data && reg_data_cnt == 14)
    apt8_reg_data = reg_data;
}

/**
 * @brief      This function serves to init apt8
 * @param[in]  first_key - the global first key of apt8
 * @param[in]  last_key  - the global last key of apt8
 * @return     none
 */
void apt8_init (u8 first_key, u8 last_key)
{
  i2c_gpio_set (APT8L08_I2C_PORT);  	//SDA/CK : B6/D7

  i2c_master_init (APT8_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*200000)));

  WaitMs (5);

  apt8_set_cfg ();

  WaitMs (5);

  apt_set_active ();

  WaitMs (5);

  apt8_set_cfg ();

  for (u8 i = 0; i < 14; i++)
    apt8_set_reg (APTTouchRegAdd[i], apt8_reg_data[i]);

  for (u8 i = 0; i < 8; i++)
    apt8_set_reg (i, apt8_cap_sense[i]);

  apt_set_active ();

  WaitMs (300);

  apt8_first_key = first_key;
  apt8_last_key  = last_key;
}

/**
 * @brief     This function serves to reset apt8
 * @param[in] none.
 * @return    none
 */
void apt8_reset ()
{
  i2c_gpio_set (APT8L08_I2C_PORT);  	//SDA/CK : B6/D7

  i2c_master_init (APT8_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*200000)));

  apt8_set_cfg ();

  WaitMs (5);

  apt_set_active ();

  WaitMs (5);
}

#if defined(APT_DEBOUNCE)
static key_status_t key_filter (u8 local_key, bool cur_status)
{
  static bool is_wakeup_filter = false;
  u32 filter_time;
  u32 cur_time;

  cur_time = clock_time ();
  filter_time = debounce_time[local_key];

  if (!is_wakeup_filter) {//no filter
    stable_status[local_key] = cur_status;
    if (is_last_local_key(local_key))//all key scan finished
      is_wakeup_filter = true;
  } else {
    if (filter_time) { //debounce status
      if (((u32)((int)cur_time - (int)filter_time)) >= APT_DEBOUNCE_TIME) { //check if debounce finished
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
#endif

/**
 * @brief      This function serves to read the specified apt8 key whether is pressing or not
 * @param[out]  key_s - specified the key is pressing or not
 * @param[in]   key   - the global key of apt8
 * @return     none
 */
void apt8_read (key_status_t* key_s, key_index_t key)
{
  u8 rd_data;
  u8 touch_key;
  bool cur_status;

  i2c_gpio_set (APT8L08_I2C_PORT);  	//SDA/CK : B6/D7

  i2c_master_init (APT8_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*200000)));

  i2c_read_series (KVR0, 1, (u8*)&rd_data, 1);

  touch_key = local_touch_key (key);

  cur_status = rd_data & (1 << touch_key)? true:false;

#if defined (APT_DEBOUNCE)
  *key_s = key_filter (touch_key, cur_status);
#else
  stable_status[touch_key] = cur_status;
  *key_s = stable_status[touch_key] ? PRESSING:RELEASE;
#endif
}
/**
 * @brief     This function serves to set apt8 if wakeup system or not
 * @param[in] none.
 * @return    none
 */
void apt8_touch_key_sleep_setup ()
{
  if (touch_key_set_sleep)
    SET_SCAN_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP (APT8L08_INT);
}

/**
 * @brief     This function serves to enable apt8 wakeup system
 * @param[in] key - the global key of apt8.
 * @return    none
 */
void apt8_touch_key_enable_sleep (u8 key)
{
  (void) key;
  touch_key_set_sleep = true;
}

/**
 * @brief     This function serves to disable apt8 wakeup system
 * @param[in] key - the global key of apt8.
 * @return    none
 */
void apt8_touch_key_disable_sleep (u8 key)
{
  (void) key;
  touch_key_set_sleep = false;
}

#endif
