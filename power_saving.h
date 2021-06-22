#ifndef __POWER_SAVING_H__
#define __POWER_SAVING_H__

extern void idle_time_for_sleep(u32 ms);
extern bool poll_idle_time();
extern void reload_sys_time();

#endif
