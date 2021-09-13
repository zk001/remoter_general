#include "../../common.h"
#include "dc.h"
#include "board.h"

//set dc chip select pin to output high
void dc_power_on()
{
  gpio_set_func(DCDC_CE, AS_GPIO);
  gpio_set_output_en(DCDC_CE, 1);
  gpio_set_input_en(DCDC_CE, 0);
  gpio_write(DCDC_CE, DC_ENABLE_LEVEL);
}

void dc_shutdown()
{
  gpio_set_func(DCDC_CE, AS_GPIO);
  gpio_set_output_en(DCDC_CE, 1);
  gpio_set_input_en(DCDC_CE, 0);
  gpio_write(DCDC_CE, ~DC_ENABLE_LEVEL);
}
