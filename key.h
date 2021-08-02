#ifndef __KEY_H__
#define __KEY_H__
#include "../common.h"
#include "list.h"

typedef enum {
  RELEASE = 1,
  PRESSING
}key_status_t;

typedef enum {
  NO_STUCK = 1,
  STUCK
}key_stuck_t;

typedef enum {
  SHORT_KEY_IMMEDIATELY_FLAG =  0x01,
  LONG_KEY_FLAG              =  0x02,
  STUCK_KEY_FLAG             =  0x04,
  LONG_8S_KEY_FLAG           =  0x08,
}key_flag_t;

typedef struct {
  key_flag_t flag;
  key_status_t cur_status;//current status
  key_status_t pre_status;//prev status
  u32 sys_time;//store the system time
  u32 pressing_time;
}key_state_t;

typedef void (*handler)(void);

typedef enum {
  RELEASE_KEY              = 0,
  SHORT_KEY                = 0x01,//active when released in short time
  SHORT_KEY_IMMEDIATELY    = 0x02,//active when pressed
  LONG_KEY                 = 0x04,//active when pressed over long time
  COMBIN_KEY               = 0x08,//active when two keys are pressed in combin time 
  COMBIN_KEY_IN_TIME       = 0x10,//active when two keys are pressed in combin time and over a spec time
  STUCK_KEY                = 0x20,//active when pressed over long long time 
  NO_TIME_LIMIT_KEY_RELEASED  = 0x40,
  NO_TIME_LIMIT_KEY_ON        = 0x80,
  ONE_KEY_TWICE               = 0x100,
  ONE_KEY_TWICE_ONLY_ONCE     = 0x200,
  LONG_KEY_IN_8S              = 0x400,
  RESERVED_KEY5            = 0x800,
  RESERVED_KEY6            = 0x1000,
  RESERVED_KEY7            = 0x2000,
  RESERVED_KEY8            = 0x4000,
  RESERVED_KEY9            = 0x8000,
  RESERVED_KEY10           = 0x10000,
  RESERVED_KEY11           = 0x20000,
  RESERVED_KEY12           = 0x40000,
  RESERVED_KEY13           = 0x80000,
  RESERVED_KEY14           = 0x100000,
  RESERVED_KEY15           = 0x200000,
  RESERVED_KEY16           = 0x400000,
  RESERVED_KEY17           = 0x800000,
  RESERVED_KEY18           = 0x1000000,
}key_action_t;

typedef struct {
  struct g_list_head list;
  u8 second_key;
  key_action_t key_ac;
  u32 time1;
  u32 time2;
  handler key_handler;
}event_handler_t __attribute__ ((aligned (4)));

typedef struct {
  struct g_list_head list;
  key_action_t key_current_action;
}key_process_t __attribute__ ((aligned (4)));

typedef enum {
  KEY0 = 0,
  KEY1,
  KEY2,
  KEY3,
  KEY4,
  KEY5,
  KEY6,
  KEY7,
  KEY8,
  KEY9,
  KEY10,
  KEY11,
  KEY12,
  KEY13,
  KEY14,
  KEY15,
  KEY16,
  KEY17,
  KEY18,
  KEY19,
  KEY20,
  KEY21,
  KEY22,
  KEY23,
  KEY24,
  KEY25,
  KEY26,
  KEY27,
  KEY28,
  KEY29,
  KEY30,
  KEY31
}key_index_t;

enum key_type_e{
  MECHANICAL_KEY = 1,
  TOUCH_KEY      = 2
};

typedef struct {
  u8 key;
  enum key_type_e type;
  void (*key_init)(u8 first_key, u8 last_key);
  void (*key_scan)(key_status_t* key_s, key_index_t key);
  void (*stuck_scan)(key_status_t* key_s, key_index_t key);
}key_type_t;

typedef struct {
  const key_type_t *key;
  u8 num;
}key_table_t;

#define FIRST_KEY_FLAG   0x80000000
#define SECOND_KEY_FLAG  0x40000000

#define ONE_KEY_PRESSING_TWICE_INTERVAL MS2TICK(1000)
#define LONG_KEY_8S (8000*16*1000)

extern u8 cur_key;
extern u8 pre_key;
extern u8 leader_key;

extern void key_init();
extern void key_wakeup_init();
extern void poll_key_event();
extern int key_process();
extern void set_leader_key(u8 key);
extern void register_key_event(u8 first_key, u8 second_key, u32 time1, u32 time2, key_action_t key_ac, handler key_handler);
extern void register_key(const key_type_t *key, u8 num);
extern u8 app_read_single_key(u8 key);
extern u8 app_read_key(u8 first_key, u8 second_key);
extern void set_stuck_key_handler(handler stuck_handler);
extern void key_gpio_sleep_init();
extern void clr_app_read_key_flag();
#endif
