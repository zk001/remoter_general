#include "../drivers.h"
#include "timer_clock.h"

timer_handler_t timer_handler;

void clr_timer_handler()
{
  timer_handler.t_handler = NULL;
  timer_handler.para      = 0;
}

void set_timer_handler(timer_handler_t* timer_hand)
{
  timer_handler.t_handler = timer_hand->t_handler;
  timer_handler.para      = timer_hand->para;
}

void set_timer_handler_para(timer_handler_t* timer_hand)
{
  timer_handler.para = timer_hand->para;
}

void run_timer_handler()
{
  timer_event_handler t_handler;
  u32 para;

  t_handler  = timer_handler.t_handler;
  para       = timer_handler.para;

  if(t_handler)
    t_handler(para);
}

void timer0_clock_init(u32 us)
{
  timer0_set_mode(TIMER_MODE_SYSCLK, 0, us*CLOCK_SYS_CLOCK_1US);
  timer_start(TIMER0);
  irq_enable();
}

void timer0_clock_stop()
{
  timer_stop(TIMER0);
}
