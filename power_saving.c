#include "../common.h"
#include "n_timer.h"
#include "power_saving.h"
#include "app.h"
#include "mac_id.h"

_attribute_data_retention_ static u32 idle_tick;
static u32 sys_tick;
_attribute_data_retention_ static bool wake_up;

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

bool is_wakeup_from_sleep()
{
  return wake_up;
}

void clr_wake_up_flag()
{
  wake_up = 0;
}

void set_wake_up_flag()
{
  wake_up = 1;
}
