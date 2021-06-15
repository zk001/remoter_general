#ifndef TIMER_CLOCK
#define TIMER_CLOCK

#define CLOCK_SYS_CLOCK_HZ  	24000000		// define system clock

enum{
	CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,				///< system tick per 1 second
	CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),		///< system tick per 1 millisecond
	CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),   ///< system tick per 1 microsecond
};

extern void timer_clock_init(TIMER_ModeTypeDef timer, u32 ms);

#endif
