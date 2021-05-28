#if defined(GPIO_LED)

#include "led.h"
#include "board.h"
#include "gpio_led.h"

_attribute_data_retention_ static low_led_t low_led;

void led_alloc(const u32 *led_arry, u8 num)
{
  low_led.led_table = led_arry;
  low_led.num       = num;
}

void gpio_led_on_off(u32 leds, u8 mode)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while(leds){
    if(leds & led){
      if(mode == HAL_LED_MODE_ON)
        GPIO_HAL_TURN_ON(low_led.led_table[num]);
      else
        GPIO_HAL_TURN_OFF(low_led.led_table[num]);
      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}
#endif
