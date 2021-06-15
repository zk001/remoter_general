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
#define HAL_LED_DEFAULT_BREATHE_STEP    MS2TICK(50)
#define HAL_LED_DEFAULT_BREATHE_COUNT   1
#define HAL_LED_DEFAULT_BREATHE_TIME    MS2TICK(2500)
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
