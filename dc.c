#include "dc.h"
#include "board.h"
#include "../drivers.h"

//set dc chip select pin to output high
void dc_init()
{
	gpio_set_func(DCDC_CE, AS_GPIO);
	gpio_set_output_en(DCDC_CE, 1);
	gpio_set_input_en(DCDC_CE, 0);
	gpio_write(DCDC_CE, 1);
}

void dc_shutdown()
{
	gpio_set_func(DCDC_CE, AS_GPIO);
	gpio_set_output_en(DCDC_CE, 1);
	gpio_set_input_en(DCDC_CE, 0);
	gpio_write(DCDC_CE, 0);
}
