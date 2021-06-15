#if defined(APT8_KEY)
#ifndef _APT8L08_KEY_H
#define _APT8L08_KEY_H
#include "key.h"

#define   APT8_ADDRESS    0xac
#define   SYS_CON         0x3a
#define   MCON            0x21
#define   BUR             0x22
#define   DMR0            0x2d
#define   DMR1            0x2a
#define   KDR0            0x23
#define   GSR             0x20
#define   KOR             0x00
#define   KVR0            0x34

#define MAX_TOUCH_KEY 8
#define APT8_INT_GPIO GPIO_PD3

extern void apt8_reset();
extern void apt8_init(u8 first_key, u8 last_key);
extern void apt8_read(key_status_t* key_s, key_index_t key);
extern void apt_enter_sleep();
extern void apt_exit_sleep();
extern void touch_key_sleep_unset(u8 key);
#endif
#endif
