/**************************************************************************************************
  Filename:       hal_led.h
  Revised:        $Date: 2007-07-06 10:42:24 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

  Description:    This file contains the interface to the LED Service.


  Copyright 2005-2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED ¡°AS IS¡± WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/
#if defined(LED)
#ifndef HAL_LED_H
#define HAL_LED_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "../drivers.h"
#include "gpio_led.h"
#include "aw9523_led.h"
/*********************************************************************
 * MACROS
 */

#define BREATHE_LEDS TRUE
#define BLINK_LEDS
#define HAL_LED  TRUE


/*********************************************************************
 * CONSTANTS
 */

/* LEDS - The LED number is the same as the bit position */
#define HAL_LED_1     0x01
#define HAL_LED_2     0x02
#define HAL_LED_3     0x04
#define HAL_LED_4     0x08
#define HAL_LED_5     0x10
#define HAL_LED_6     0x20
#define HAL_LED_7     0x40
#define HAL_LED_8     0x80
#define HAL_LED_9     0x100
#define HAL_LED_10    0x200
#define HAL_LED_11    0x400
#define HAL_LED_12    0x800
#define HAL_LED_13    0x1000
#define HAL_LED_14    0x2000
#define HAL_LED_15    0x4000
#define HAL_LED_16    0x8000
#define HAL_LED_17    0x10000
#define HAL_LED_18    0x20000
#define HAL_LED_19    0x40000
#define HAL_LED_20    0x80000
#define HAL_LED_21    0x100000


/* Modes */
#define HAL_LED_MODE_OFF              0x00
#define HAL_LED_MODE_ON               0x01
#define HAL_LED_MODE_BLINK            0x02
#define HAL_LED_MODE_FLASH            0x04
#define HAL_LED_MODE_TOGGLE           0x08
#define HAL_LED_MODE_CONTINUE_BREATHE 0x10
#define HAL_LED_MODE_BREATHE          0x20
//#define MAX_BRIGHT_LEVEL 100
/* Defaults */

#define HAL_LED_DEFAULT_DUTY_CYCLE    100
#define HAL_LED_DEFAULT_FLASH_COUNT   10
#define HAL_LED_DEFAULT_FLASH_TIME    MS2TICK(1000)

//#define HAL_LED_DEFAULT_MAX_BREATH_LEDS 16
#define HAL_LED_DEFAULT_BREATHE_STEP    MS2TICK(6)
#define HAL_LED_DEFAULT_BREATHE_COUNT   1
#define HAL_LED_DEFAULT_BREATHE_TIME    MS2TICK(1500)
/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
	u32 leds;
	void (*led_on_off)(u32 leds, u8 mode);
	void (*led_breath)(u32 leds, u8 tim);
}hal_led_t;

typedef struct
{
	const hal_led_t *led;
	u8 num;
}hal_led_arry_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*
 * Initialize LED Service.
 */
extern void HalLedInit( void );

/*
 * Set the LED ON/OFF/TOGGLE.
 */
extern u32 HalLedSet( u32 led, u8 mode );

/*
 * Blink the LED.
 */
extern void HalLedBlink( u32 leds, u8 cnt, u8 duty, u32 time );

extern int HalLedUpdate (void *data);

/*
 * Put LEDs in sleep state - store current values
 */
extern void HalLedEnterSleep( void );

/*
 * Retore LEDs from sleep state
 */
extern void HalLedExitSleep( void );

/*
 * Return LED state
 */
extern u32 HalLedGetState ( void );

extern void HalLedBreathe (u32 leds, u8 numBreathe, u32 step, u32 period);
extern int HalLedUpdateBreath (void *data);
extern void register_led(const hal_led_t *led, u8 num);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
#endif
