#include "../drivers.h"
#include "timer_clock.h"

void timer_clock_init(TIMER_ModeTypeDef timer, u32 ms)
{
	timer0_set_mode(TIMER_MODE_SYSCLK, 0, ms * CLOCK_SYS_CLOCK_1MS);
	timer_start(timer);
	irq_enable();
}

