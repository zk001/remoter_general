#ifndef _APT8L08_KEY_SIMULATION_H
#define _APT8L08_KEY_SIMULATION_H
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

extern void apt8_reset();
extern void apt8_init();
extern void apt8_set_cfg();
extern void apt8_set_reg(u8 addr, u8 data);
extern void apt8_read(key_status_t* key_result, key_index_t key);
#endif
