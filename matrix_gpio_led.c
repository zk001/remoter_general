#if defined(MATRIX_GPIO_LED)
#include "common.h"
#include "matrix_gpio_led.h"
#include "led.h"
#include "gpio_set.h"
#include "n_timer.h"

_attribute_data_retention_ static struct {
  matrix_led_map_t* map;
  u8 num;
} matrix_led_map;

_attribute_data_retention_ static u8 total;
static u32 start_time;

static u8 get_sel_pin_num ()
{
  u8 num = 0;
  u32 sel_pin = 0;

  for (u8 i = 0; i < matrix_led_map.num; i++) {
    if (sel_pin != matrix_led_map.map[i].sel_pin) {
      sel_pin = matrix_led_map.map[i].sel_pin;
      num++;
    }
  }
  return num;
}

static u32 get_sel_pin (u8 num)
{
  u8 local_num = 0;
  u32 sel_pin = 0;

  for (u8 i = 0; i < matrix_led_map.num; i++) {
    if (sel_pin != matrix_led_map.map[i].sel_pin) {
      sel_pin = matrix_led_map.map[i].sel_pin;
      local_num++;
      if (local_num == num) {
        return sel_pin;
      }
    }
  }
  return 0;
}

static inline void driver_pin (u32 pin)
{
  SET_GPIO_OUTPUT_HIGH(pin);
}

static inline void shutdown_pin (u32 pin)
{
  SET_GPIO_OUTPUT_LOW(pin);
}

static void driver_draw_pin (u32 sel_pin)
{
  for (u8 i = 0; i < matrix_led_map.num; i++) {
    if (matrix_led_map.map[i].sel_pin == sel_pin) {
      if (matrix_led_map.map[i].mode == HAL_LED_MODE_ON)
	    SET_GPIO_OUTPUT_HIGH(matrix_led_map.map[i].draw_pin);
	  else
        SET_GPIO_OUTPUT_LOW(matrix_led_map.map[i].draw_pin);
    }
  }
}

static void set_start_time ()
{
  start_time = clock_time ();
}

static bool is_stable_time_exceed ()
{
  if (start_time) {
    if (n_clock_time_exceed (start_time, DELAY_TIME))
      return true;
    return false;
  } else
    return true;
}

static void driver_matrix_led (u8 cur)
{
  u32 sel_pin;

  sel_pin = get_sel_pin (cur);
  driver_draw_pin (sel_pin);
  driver_pin (sel_pin);
}

static void shutdown_matrix_led (u8 cur)
{
  u32 sel_pin;

  sel_pin = get_sel_pin (cur);
  shutdown_pin (sel_pin);
}

void matrix_led_update ()
{
  static u8 cur = 1;
  static u8 pre = 1;

  if (matrix_led_map.map && matrix_led_map.num) {
    if (is_stable_time_exceed ()) {
	  shutdown_matrix_led (pre);

	  if (cur++ == total)
        cur = 1;

	  driver_matrix_led (cur);
	  pre = cur;
	  set_start_time ();
    }
  }
}

void matrix_led_alloc (matrix_led_map_t* led_arry, u8 num)
{
  matrix_led_map.map = led_arry;
  matrix_led_map.num = num;
}

void matrix_led_init ()
{
  if (matrix_led_map.map && matrix_led_map.num) {
    for (u8 i = 0; i < matrix_led_map.num; i++) {
      SET_GPIO_INPUT_WITH_PULLDOWN(matrix_led_map.map[i].sel_pin);
      SET_GPIO_INPUT_WITH_PULLDOWN(matrix_led_map.map[i].draw_pin);
	  matrix_led_map.map[i].mode = HAL_LED_MODE_OFF;
	}
    total = get_sel_pin_num ();
  }
}

void matrix_led_on_off (u32 leds, u8 mode)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while (leds) {
	if (leds & led) {
	  if (mode == HAL_LED_MODE_ON)
	    matrix_led_map.map[num].mode = HAL_LED_MODE_ON;
	  else
	    matrix_led_map.map[num].mode = HAL_LED_MODE_OFF;
	  leds ^= led;
	}
	num++;
	led <<= 1;
  }
}
#endif
