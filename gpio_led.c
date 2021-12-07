/********************************************************************************************************
 * @file     gpio_led.c
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
#if defined (GPIO_LED)
#include "common.h"
#include "gpio_led.h"

//general
#include "led.h"
#include "timer_clock.h"

//vendor
#include "board.h"
#include "app.h"

typedef struct {
  u8 pwm;
  u8 id;
}pwm_t;

_attribute_data_retention_ static low_led_t low_led;

/**
 * @brief      This function serves to register gpio led port array
 * @param[in]  key_arry  - the gpio led port relative register array
 * @param[in]  num       - the length of the key_arry
 * @return     none
 */
void gpio_led_alloc (const u32* led_arry, u8 num)
{
  low_led.led_table = led_arry;
  low_led.num       = num;
}

/**
 * @brief      This function serves to set led on or off
 * @param[in]  leds  - the leds which will be set on or off
 * @param[in]  mode  - the led mode, HAL_LED_MODE_ON or HAL_LED_MODE_OFF
 * @return     none
 */
void gpio_led_on_off (u32 leds, u8 mode)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while (leds) {
    if (leds & led) {
      if (mode == HAL_LED_MODE_ON)
        GPIO_HAL_TURN_ON(low_led.led_table[num]);
      else
        GPIO_HAL_TURN_OFF(low_led.led_table[num]);
      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}

/**
 * @brief      This function serves to choose which pwm module to use
 * @param[in]  led  - the leds which will be used with pwm
 * @param[out] pwm  - the pwm module will be used with led
 * @return     none
 */
static void choose_pwm (u32 led, pwm_t* pwm)
{
  switch (led) {
    case GPIO_PD4: pwm->pwm = AS_PWM2_N; pwm->id = PWM2_ID;break;
    case GPIO_PA0: pwm->pwm = AS_PWM0_N; pwm->id = PWM0_ID;break;
    case GPIO_PB1: pwm->pwm = AS_PWM4;   pwm->id = PWM4_ID;break;
    case GPIO_PB4: pwm->pwm = AS_PWM4;   pwm->id = PWM4_ID;break;
    case GPIO_PB5: pwm->pwm = AS_PWM5;   pwm->id = PWM5_ID;break;
    case GPIO_PC0: pwm->pwm = AS_PWM4_N; pwm->id = PWM4_ID;break;
    case GPIO_PC1: pwm->pwm = AS_PWM1_N; pwm->id = PWM1_ID;break;
    case GPIO_PC4: pwm->pwm = AS_PWM2;   pwm->id = PWM2_ID;break;
    case GPIO_PC5: pwm->pwm = AS_PWM3_N; pwm->id = PWM3_ID;break;
    case GPIO_PC6: pwm->pwm = AS_PWM4_N; pwm->id = PWM4_ID;break;
    case GPIO_PD3: pwm->pwm = AS_PWM1_N; pwm->id = PWM1_ID;break;
    default:break;
  }
}

/**
 * @brief      This function serves to turn on led with pwm
 * @param[in]  led  - the leds which will be turn on with pwm
 * @return     none
 */
static void pwm_turn_on (u32 led)
{
  pwm_t pwm;

  choose_pwm (led, &pwm);

  gpio_set_func (led, pwm.pwm);
  pwm_set_mode (pwm.id, PWM_NORMAL_MODE);
  pwm_set_phase (pwm.id, 0);   //no phase at pwm beginning

  if(pwm.pwm == AS_PWM0_N || pwm.pwm == AS_PWM1_N ||\
      pwm.pwm == AS_PWM2_N || pwm.pwm == AS_PWM3_N ||\
      pwm.pwm == AS_PWM4_N)
    pwm_set_cycle_and_duty (pwm.id, (u16)(PWM_PERIOD * CLOCK_SYS_CLOCK_1US),  (u16)((PWM_PERIOD - PWM_ON_DUTY) * CLOCK_SYS_CLOCK_1US));
  else
    pwm_set_cycle_and_duty (pwm.id, (u16)(PWM_PERIOD * CLOCK_SYS_CLOCK_1US),  (u16)(PWM_ON_DUTY * CLOCK_SYS_CLOCK_1US));

  pwm_start (pwm.id);
}

/**
 * @brief      This function serves to turn off led with pwm
 * @param[in]  led  - the leds which will be turn off with pwm
 * @return     none
 */
static void pwm_turn_off (u32 led)
{
  GPIO_HAL_TURN_OFF(led);
}

/**
 * @brief      This function serves to set led on or off with pwm
 * @param[in]  leds  - the leds which will be set on or off
 * @param[in]  mode  - the led mode, HAL_LED_MODE_ON or HAL_LED_MODE_OFF
 * @return     none
 */
void pwm_gpio_led_on_off (u32 leds, u8 mode)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while (leds) {
    if (leds & led) {
      if (mode == HAL_LED_MODE_ON)
        pwm_turn_on (low_led.led_table[num]);
      else
        pwm_turn_off (low_led.led_table[num]);
      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}

#endif
