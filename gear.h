#ifndef __GEAR_H__
#define __GEAR_H__

#include "common.h"

typedef enum {
  KEEP,
  BACK,
  RESET
} method_t;

typedef enum {
  GEAR_DOWN,
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

extern void register_gear (gear_t* user_value, u8 region, u8 num);
extern u8 get_gear (u8 user_id, char* name);
extern void update_gear (u8 user_id, char* name, up_or_down_t direction);

#endif
