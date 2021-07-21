#if defined(GPIO_KEY)
#ifndef __MECHANICAL_H_
#define __MECHANICAL_H_

#define SET_ROW_GPIO_OUTPUT_HIGH(row) st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_set_input_en(row, 0);\
    gpio_set_output_en(row, 1);\
    gpio_write(row, 1);)

#define SET_ROW_GPIO_OUTPUT_LOW(row) st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_set_input_en(row, 0);\
    gpio_set_output_en(row, 1);\
    gpio_write(row, 0);)

#define SET_ROW_GPIO_OUTPUT_HIGH_Z(row) st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_shutdown(row);)

#define SET_ROW_GPIO_INPUT(row)  st(\
    gpio_set_func(row, AS_GPIO);\
    gpio_set_input_en(row, 1);\
    gpio_set_output_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLUP_1M);)

#define SET_COL_GPIO_INPUT(col) st(\
    gpio_set_func(col, AS_GPIO);\
    gpio_set_input_en(col, 1);\
    gpio_set_output_en(col, 0);\
    gpio_setup_up_down_resistor(col, PM_PIN_PULLUP_1M);)

#define SET_ROW_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(row) st(\
    gpio_set_output_en(row, 0);\
    gpio_set_input_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLDOWN_100K);)

#define SET_COL_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(col) st(\
    gpio_set_output_en(col, 0);\
    gpio_set_input_en(col, 0);\
    gpio_setup_up_down_resistor(col, PM_PIN_PULLUP_1M);\
    cpu_set_gpio_wakeup(col, 0, 1);)

#define SET_ROW_GPIO_WITH_DEEPSLEEP_HIGH_WAKEUP(row) st(\
    gpio_set_output_en(row, 0);\
    gpio_set_input_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLUP_1M);)

#define SET_COL_GPIO_WITH_DEEPSLEEP_HIGH_WAKEUP(col) st(\
    gpio_set_output_en(col, 0);\
    gpio_set_input_en(col, 0);\
    gpio_setup_up_down_resistor(col, PM_PIN_PULLUP_1M);\
    cpu_set_gpio_wakeup(col, 1, 1);)

#if (defined YIHE_0932)
#define DEBOUNCE_TIME 20*16*1000
#else
#define DEBOUNCE_TIME 20*16*1000
#endif

enum wake_up_pin_t{
  IS_WAKE_UP = 1,
  NO_WAKE_UP = 2
};

typedef struct {
  u32 row;
  u32 col;
  enum wake_up_pin_t is_wake_up_pin;
}key_map_t;

extern void gpio_key_init(u8 first_key, u8 last_key);
extern void gpio_key_low_scan(key_status_t* key_s, key_index_t key);
extern void gpio_key_alloc(key_map_t *key_arry, u8 num);
extern void gpio_key_sleep_setup();
extern void gpio_key_sleep_unset(u8 key);
extern void gpio_stuck_key_low_scan(key_status_t* key_s, key_index_t key);
extern void pwm_gpio_led_on_off(u32 leds, u8 mode);
extern void gpio_key_sleep_set(u8 key);
extern void gpio_wakeup_fast_read(key_status_t* key_s, key_index_t key);

#endif
#endif
