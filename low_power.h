#if (defined LOW_POWER_DETECT)
#ifndef __LOW_POWER__
#define __LOW_POWER__

typedef void low_bat_warn(void);
typedef void low_bat_finish(void);

extern bool low_bat_chk(u32 threshold, low_bat_warn* cb_warn, low_bat_finish* cb_finsish, u32 low_bat_time);
extern void low_bat_update();
extern bool is_bat_warn();
extern u32 low_power_threshold();

#endif
#endif
