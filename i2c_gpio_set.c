#include "../../drivers.h"
#include "i2c_gpio_set.h"
//for shuman when enter low power mode,set i2c port to PM_PIN_PULLDOWN_100K
void i2c_gpio_set_deepsleep(u32 pin_sda, u32 pin_clk)
{
  gpio_set_func(pin_sda, AS_GPIO);
  gpio_set_input_en(pin_sda, 1);
  gpio_set_output_en(pin_sda, 0);
  gpio_setup_up_down_resistor(pin_sda, PM_PIN_PULLDOWN_100K);

  gpio_set_func(pin_clk, AS_GPIO);
  gpio_set_input_en(pin_clk, 1);
  gpio_set_output_en(pin_clk, 0);
  gpio_setup_up_down_resistor(pin_clk, PM_PIN_PULLDOWN_100K);
}
