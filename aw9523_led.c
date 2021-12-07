/********************************************************************************************************
 * @file     aw9523_led.c
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
#if defined (AW9523_LED)
#include "common.h"
#include "aw9523_led.h"

//general
#include "led.h"

//vendor
#include "main.h"


//led
//should we set gpio direction???
//shuld we set gcr register???
//P1 ports is output mode as default when power on and P0 ports is high-z

_attribute_data_retention_ static u8 aw9523_led_bright [16];

static const u8 aw9523_led_table [] = {
  PORT0_BIT_0, PORT0_BIT_1, PORT0_BIT_2, PORT1_BIT_2, PORT1_BIT_3,
  PORT0_BIT_3, PORT0_BIT_4, PORT1_BIT_1, PORT0_BIT_5, PORT1_BIT_7,
  PORT1_BIT_5, PORT1_BIT_6, PORT1_BIT_0, PORT1_BIT_4, PORT0_BIT_6,
  PORT0_BIT_7
};

/**
 * @brief      This function serves to write one byte to the GCR register
 * @param[in]  conf - the data will be written to the GCR register
 * @return     none
 */
static void aw9523_set_conf (u8 conf)
{
  i2c_write_byte (GCR, 1, conf);
}

/**
 * @brief      This function serves to select port which to be configured to be led mode
 * @param[in]  aw_port - the port will be configured to be the led mode
 * @return     none
 */
static void aw9523_set_led_mode (u8 aw_port)
{
  u8 port;

  port = aw_port ? P1_MOD_SWITCH:P0_MOD_SWITCH;

  i2c_write_byte (port, 1, 0);
}

/**
 * @brief      This function serves to select port which to be configured to be gpio mode
 * @param[in]  aw_port - the port will be configured to be the gpio mode
 * @return     none
 */
void aw9523_set_gpio_mode (u8 aw_port)
{
  u8 port;

  port = aw_port ? P1_MOD_SWITCH:P0_MOD_SWITCH;

  i2c_write_byte (port, 1, 0xff);
}

/**
 * @brief      This function serves to set led dim register
 * @param[in]  port_bit - the port_bit which will be set by level parameter
 * @param[in]  level    - the led brightness level,ranger 0~255
 * @return     none
 */
static void aw9523_set_led_dim (u8 port_bit, u8 level)
{
  u8 port;
  u8 local_level = 0;

  switch (port_bit) {
    case PORT0_BIT_0: port = STEP_DIM_SET4;  local_level = aw9523_led_bright[0];  break;
    case PORT0_BIT_1: port = STEP_DIM_SET5;  local_level = aw9523_led_bright[1];  break;
    case PORT0_BIT_2: port = STEP_DIM_SET6;  local_level = aw9523_led_bright[2];  break;
    case PORT0_BIT_3: port = STEP_DIM_SET7;  local_level = aw9523_led_bright[5];  break;
    case PORT0_BIT_4: port = STEP_DIM_SET8;  local_level = aw9523_led_bright[6];  break;
    case PORT0_BIT_5: port = STEP_DIM_SET9;  local_level = aw9523_led_bright[8];  break;
    case PORT0_BIT_6: port = STEP_DIM_SET10; local_level = aw9523_led_bright[14]; break;
    case PORT0_BIT_7: port = STEP_DIM_SET11; local_level = aw9523_led_bright[15]; break;
    case PORT1_BIT_0: port = STEP_DIM_SET0;  local_level = aw9523_led_bright[12]; break;
    case PORT1_BIT_1: port = STEP_DIM_SET1;  local_level = aw9523_led_bright[7];  break;
    case PORT1_BIT_2: port = STEP_DIM_SET2;  local_level = aw9523_led_bright[3];  break;
    case PORT1_BIT_3: port = STEP_DIM_SET3;  local_level = aw9523_led_bright[4];  break;
    case PORT1_BIT_4: port = STEP_DIM_SET12; local_level = aw9523_led_bright[13]; break;
    case PORT1_BIT_5: port = STEP_DIM_SET13; local_level = aw9523_led_bright[10]; break;
    case PORT1_BIT_6: port = STEP_DIM_SET14; local_level = aw9523_led_bright[11]; break;
    case PORT1_BIT_7: port = STEP_DIM_SET15; local_level = aw9523_led_bright[9];  break;
    default:port = 0;break;
  }

  i2c_gpio_set (AW9523_I2C_PORT);  	//SDA/CK : C0/C1

  i2c_master_init (AW9523_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*200000)));

  if (local_level && level)
    i2c_write_byte (port, 1, local_level);
  else
    i2c_write_byte (port, 1, level);
}

/**
 * @brief      This function serves to set led on
 * @param[in]  port_bit - the port_bit which  will be set on
 * @return     none
 */
static void aw9523_led_on (u8 port_bit)
{
  aw9523_set_led_dim (port_bit, LED_BRIGHT_LEVEL);
}

/**
 * @brief      This function serves to set led off
 * @param[in]  port_bit - the port_bit which  will be set off
 * @return     none
 */
static void aw9523_led_off (u8 port_bit)
{
  aw9523_set_led_dim (port_bit, 0);
}

/**
 * @brief      This function serves to set led brightness level
 * @param[in]  port_bit - the port_bit which will be set by level parameter
 * @param[in]  level - the led brightness level,ranger 0~255
 * @return     none
 */
static void set_led_bright (u32 port_bit, u8 level)
{
  switch (port_bit) {
    case PORT0_BIT_0: aw9523_led_bright[0]  = level; break;
    case PORT0_BIT_1: aw9523_led_bright[1]  = level; break;
    case PORT0_BIT_2: aw9523_led_bright[2]  = level; break;
    case PORT0_BIT_3: aw9523_led_bright[5]  = level; break;
    case PORT0_BIT_4: aw9523_led_bright[6]  = level; break;
    case PORT0_BIT_5: aw9523_led_bright[8]  = level; break;
    case PORT0_BIT_6: aw9523_led_bright[14] = level; break;
    case PORT0_BIT_7: aw9523_led_bright[15] = level; break;
    case PORT1_BIT_0: aw9523_led_bright[12] = level; break;
    case PORT1_BIT_1: aw9523_led_bright[7]  = level; break;
    case PORT1_BIT_2: aw9523_led_bright[3]  = level; break;
    case PORT1_BIT_3: aw9523_led_bright[4]  = level; break;
    case PORT1_BIT_4: aw9523_led_bright[13] = level; break;
    case PORT1_BIT_5: aw9523_led_bright[10] = level; break;
    case PORT1_BIT_6: aw9523_led_bright[11] = level; break;
    case PORT1_BIT_7: aw9523_led_bright[9]  = level; break;
    default:break;
  }
}

/**
 * @brief      This function serves to initial aw9523
 * @param[in]  none
 * @return     none
 */
void aw9523_init ()
{
  //  u8 id;

  i2c_gpio_set (AW9523_I2C_PORT);  	//SDA/CK : C0/C1
  i2c_master_init (AW9523_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*200000)));

  RSTN_OUT_LOW();

  WaitUs (100);

  RSTN_OUT_HIGH();

  WaitMs (5);

  i2c_write_byte (SOFT_RESET, 1, 0x00);

  WaitMs (1);

  //  id = i2c_read_byte(0x03, 1);

  aw9523_set_conf (0x13);//P0 push_pull mode,led current = max/4

  aw9523_set_led_mode (0);//port0 as led mode

  aw9523_set_led_mode (1);//port1 as led mode
}

/**
 * @brief      This function serves to set led brightness level
 * @param[in]  leds  - the leds which will be set by level parameter
 * @param[in]  level - the led brightness level,ranger 0~255
 * @return     none
 */
void aw9523_set_led_bright (u32 leds, u8 level)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while (leds) {
    if (leds & led) {
      set_led_bright (aw9523_led_table[num], level);
      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}

/**
 * @brief      This function serves to set led on or off
 * @param[in]  leds  - the leds which will be set on or off
 * @param[in]  mode  - the led mode, HAL_LED_MODE_ON or HAL_LED_MODE_OFF
 * @return     none
 */
void aw9523_led_on_off (u32 leds, u8 mode)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while (leds) {
    if (leds & led) {
      if (mode == HAL_LED_MODE_ON)
        aw9523_led_on (aw9523_led_table[num]);
      else
        aw9523_led_off (aw9523_led_table[num]);
      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}

/**
 * @brief      This function serves to set led breathe
 * @param[in]  leds  - the leds which will be set breathe or not
 * @param[in]  tim   - the led level, ranger 0~255
 * @return     none
 */
void aw9523_led_breath (u32 leds, u8 tim)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while (leds) {
    if (leds & led) {
      aw9523_set_led_dim (aw9523_led_table[num], tim);
      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}

#endif
