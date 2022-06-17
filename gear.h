#ifndef __GEAR_H__
#define __GEAR_H__

#include "common.h"

typedef enum {
  KEEP,
  BACK,
  RESET
} method_t;

typedef enum {
  GEAR_DOWN = 1,
  GEAR_UP,
  GEAR_NONE
} up_or_down_t;

typedef struct {
  char* name;
  u8 value;
  u8 max_value;
  u8 min_value;
  method_t method;
  up_or_down_t direction;
} gear_t;

extern void register_gear (unsigned long flash_addr, u8* flash_buf, gear_t* user_value, u8 region, u8 num);
extern u8 get_gear (u8 user_id, char* name);
extern void update_gear (u8 user_id, char* name, up_or_down_t direction);
extern u8 get_gear_direction (u8 user_id, char* name);
extern void check_sram_data_valid (u8 user_id, char* name, u8 max_value, u8 min_value);
extern void cpy_user_data (u8 is_store_flash, u8 user_id_dst, u8 user_id_src, char* name);
extern void cpy_user_data_with_direction (u8 is_store_flash, u8 user_id_dst, u8 user_id_src, char* name);

#endif
