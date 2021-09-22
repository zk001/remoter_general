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

extern void apt8_reset();
extern void apt8_init(u8 first_key, u8 last_key);
extern void apt8_read(key_status_t* key_s, key_index_t key);
extern void apt8_set_cap_sense_and_reg_data(const u8* cap_sense, u8 cap_sense_cnt, const u8* reg_data, u8 reg_data_cnt);
extern void apt8_enter_sleep();
extern void apt8_exit_sleep();
extern void apt8_touch_key_enable_sleep(u8 key);
extern void apt8_touch_key_disable_sleep(u8 key);
extern void apt8_touch_key_sleep_setup();
#endif
#endif
