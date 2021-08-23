#include "../common.h"
#include "n_timer.h"
#include "power_saving.h"
#include "app.h"

static u32 idle_tick;
static u32 sys_tick;

void idle_time_for_sleep(u32 ms)
{
  idle_tick = MS2TICK(ms);
}

bool poll_idle_time()
{
  return n_clock_time_exceed(sys_tick, TICK2US(idle_tick));
}

void reload_sys_time()
{
  sys_tick = clock_time();
}

