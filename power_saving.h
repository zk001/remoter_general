#ifndef __POWER_SAVING_H__
#define __POWER_SAVING_H__

extern void idle_time_for_sleep(u32 tick);
extern bool poll_idle_time();
extern void reload_sys_time();
extern bool is_wakeup_from_sleep();
extern void clr_wake_up_flag();
extern void set_wake_up_flag();

#endif
