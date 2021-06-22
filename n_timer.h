#ifndef __N_TIMER__
#define __N_TIMER__

#include "../common.h"

#define MS2TICK(MS)   (MS*16*1000)
#define TICK2MS(TICK) (TICK/1000/16)
#define US2TICK(US)   (US*16)
#define TICK2US(TICK) (TICK/16)

//time_after(current, timeout) returns true if the time current is after time timeout.
#define time_after(current, timeout) \
  ((int)(timeout) - (int)(current) < 0)

static inline bool n_clock_time_exceed(u32 ref, u32 us)
{
  return time_after(clock_time(), (ref + (us*16)));
}

#endif
