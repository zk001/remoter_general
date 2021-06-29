#if defined(GPIO_LED)
#ifndef _GPIO_LED_H__
#define _GPIO_LED_H__

typedef struct {
  const u32 *led_table;
  u8 num;
}low_led_t;

#define GPIO_HAL_TURN_ON(led)	st(\
    gpio_set_func(led, AS_GPIO);\
    gpio_set_output_en(led, 1);\
    gpio_set_input_en(led, 0);\
    gpio_write(led, 1);)

#define GPIO_HAL_TURN_OFF(led)	st(\
    gpio_set_func(led, AS_GPIO);\
    gpio_set_output_en(led, 1);\
    gpio_set_input_en(led, 0);\
    gpio_write(led, 0);)

extern void gpio_led_alloc(const u32 *led_arry, u8 num);
extern void gpio_led_on_off(u32 leds, u8 mode);
extern void pwm_gpio_sim_led_on_off(u32 leds, u8 mode);

#endif
#endif
