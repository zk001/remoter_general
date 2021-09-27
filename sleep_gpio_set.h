#ifndef __SLEEP_GPIO_SET_H__
#define __SLEEP_GPIO_SET_H__
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

#define SET_ROW_GPIO_WITH_DEEPSLEEP_HIGH(row) st(\
    gpio_set_output_en(row, 0);\
    gpio_set_input_en(row, 0);\
    gpio_setup_up_down_resistor(row, PM_PIN_PULLUP_10K);)
#endif
