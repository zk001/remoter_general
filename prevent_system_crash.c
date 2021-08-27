#include "common.h"
#include "drivers.h"

void prevent_system_crash()
{
  gpio_set_func(GPIO_SWS, AS_GPIO);
  gpio_set_input_en(GPIO_SWS, 1);
  gpio_set_output_en(GPIO_SWS, 0);
  gpio_setup_up_down_resistor(GPIO_SWS, PM_PIN_PULLUP_1M);
}
