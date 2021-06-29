#include "../../common.h"
#include "dc.h"
#include "board.h"

//set dc chip select pin to output high
void dc_power_on()
{
  gpio_set_func(DCDC_CE, AS_GPIO);
  gpio_set_output_en(DCDC_CE, 1);
  gpio_set_input_en(DCDC_CE, 0);
#if defined(SHUMAN)
  gpio_write(DCDC_CE, 0);
#else
  gpio_write(DCDC_CE, 1);
#endif
}

void dc_shutdown()
{
  gpio_set_func(DCDC_CE, AS_GPIO);
  gpio_set_output_en(DCDC_CE, 1);
  gpio_set_input_en(DCDC_CE, 0);
#if defined(SHUMAN)
  gpio_write(DCDC_CE, 1);
#else
  gpio_write(DCDC_CE, 0);
#endif
}
