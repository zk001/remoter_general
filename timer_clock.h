#ifndef TIMER_CLOCK
#define TIMER_CLOCK

#include "../drivers.h"

#define CLOCK_SYS_CLOCK_HZ  	24000000		// define system clock

enum{
  CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,				///< system tick per 1 second
  CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),		///< system tick per 1 millisecond
  CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),   ///< system tick per 1 microsecond
};

typedef void (*timer_event_handler)(u32);

typedef struct {
  timer_event_handler t_handler;
  u32 para;
}timer_handler_t;

extern void timer0_clock_init(u32 us);
extern void timer0_clock_stop();
extern void run_timer_handler();
extern void set_timer_handler(timer_handler_t *timer_hand);
extern void clr_timer_handler();

#endif
