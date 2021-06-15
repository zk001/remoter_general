#if (defined LOW_POWER_DETECT)
#ifndef __LOW_POWER__
#define __LOW_POWER__

#define THRESHOLD 2490

extern void vbat_init();
extern bool is_low_bat();

#endif
#endif
