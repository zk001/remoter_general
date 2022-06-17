#ifndef __SLEEP_GPIO_SET_H__
#define __SLEEP_GPIO_SET_H__

#include "common.h"
////////////////////////output & input ///////////////////
#define SET_GPIO_OUTPUT_HIGH(gpio) st(\
    gpio_set_func (gpio, AS_GPIO);\
    gpio_set_input_en (gpio, 0);\
    gpio_set_output_en (gpio, 1);\
    gpio_write (gpio, 1);)

#define SET_GPIO_OUTPUT_LOW(gpio) st(\
    gpio_set_func (gpio, AS_GPIO);\
    gpio_set_input_en (gpio, 0);\
    gpio_set_output_en (gpio, 1);\
    gpio_write (gpio, 0);)

#define SET_GPIO_OUTPUT_HIGH_Z(gpio) st(\
    gpio_set_func (gpio, AS_GPIO);\
    gpio_shutdown (gpio);)

#define SET_GPIO_INPUT_WITH_PULLUP(gpio)  st(\
    gpio_set_func (gpio, AS_GPIO);\
    gpio_set_input_en (gpio, 1);\
    gpio_set_output_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLUP_1M);)

#define SET_GPIO_INPUT_WITH_PULLDOWN(gpio) st(\
    gpio_set_func (gpio, AS_GPIO);\
    gpio_set_input_en (gpio, 1);\
    gpio_set_output_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLDOWN_100K);)

///////////////////////////low wakeup////////////////////////////
#define SET_DRIVER_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio) st(\
	gpio_set_func (gpio, AS_GPIO);\
    gpio_set_output_en (gpio, 0);\
    gpio_set_input_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLDOWN_100K);)

#define SET_SCAN_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio) st(\
	gpio_set_func (gpio, AS_GPIO);\
    gpio_set_output_en (gpio, 0);\
    gpio_set_input_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLUP_1M);\
    cpu_set_gpio_wakeup (gpio, 0, 1);)

/////////////////////////high wakeup ////////////////////////////////////////
#define SET_DRIVER_GPIO_WITH_DEEPSLEEP_HIGH_WAKEUP(gpio) st(\
	gpio_set_func (gpio, AS_GPIO);\
    gpio_set_output_en (gpio, 0);\
    gpio_set_input_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLUP_1M);)

#define SET_SCAN_GPIO_WITH_DEEPSLEEP_HIGH_WAKEUP(gpio) st(\
	gpio_set_func (gpio, AS_GPIO);\
    gpio_set_output_en (gpio, 0);\
    gpio_set_input_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLDOWN_100K);\
    cpu_set_gpio_wakeup (gpio, 1, 1);)

/////////////////////////no wakeup//////////////////////////////////////////////
#define SET_DRIVER_GPIO_WITH_DEEPSLEEP_HIGH(gpio) st(\
	gpio_set_func (gpio, AS_GPIO);\
    gpio_set_output_en (gpio, 0);\
    gpio_set_input_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLUP_10K);)

#define SET_DRIVER_GPIO_WITH_DEEPSLEEP_LOW(gpio) st(\
	gpio_set_func (gpio, AS_GPIO);\
    gpio_set_output_en (gpio, 0);\
    gpio_set_input_en (gpio, 0);\
    gpio_setup_up_down_resistor (gpio, PM_PIN_PULLDOWN_100K);)
#endif
