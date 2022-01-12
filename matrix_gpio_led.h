#if defined(MATRIX_GPIO_LED)
#ifndef __MATRIX_GPIO_LED_H__
#define __MATRIX_GPIO_LED_H__
#include "common.h"

typedef struct {
  u32 sel_pin;
  u32 draw_pin;
  u8 mode;
} matrix_led_map_t;

#define DELAY_TIME 10000

extern void matrix_led_alloc (matrix_led_map_t* led_arry, u8 num);
extern void matrix_led_init ();
extern void matrix_led_on_off (u32 leds, u8 mode);
extern void matrix_led_update ();
#endif
#endif
