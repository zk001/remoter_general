#include "n_timer.h"
#include "../../common/mempool.h"
#include "main.h"
#include "board.h"
#include "gpio_key.h"
#include "apt8l08_key.h"
#include "power_saving.h"
#include "key.h"

_attribute_data_retention_ u8 first_key_r   = 255;
_attribute_data_retention_ u8 second_key_r  = 255;
_attribute_data_retention_ u8 leader_key    = 255;
_attribute_data_retention_ u8 pre_key       = 255;
_attribute_data_retention_ u8 cur_key       = 255;

_attribute_data_retention_ static handler stuck_key_handler;

_attribute_data_retention_ static key_state_t   key_status[MAX_KEY];
_attribute_data_retention_ static key_process_t key_event[MAX_KEY];

_attribute_data_retention_ static key_table_t key_table;

static void key_status_init()
{
  for(u8 i = 0; i < key_table.num; i++){
    key_status[i].key           = 0;
    key_status[i].cur_status    = RELEASE;
    key_status[i].pre_status    = RELEASE;
    key_status[i].sys_time      = 0;
    key_status[i].pressing_time = 0;
  }
}

static void key_event_init()
{
  for(u8 i = 0; i < key_table.num; i++){
    key_event[i].key_current_action = RELEASE_KEY;
    INIT_LIST_HEAD(&key_event[i].list);
  }
}

static void cal_key_press_time(key_state_t * key_result, key_status_t key_s, key_index_t key)
{
  u32 current_time = clock_time();

  key_result->cur_status      = key_s;
  key_result->pressing_time   = (u32)((int)current_time - (int)key_result->sys_time);//caul the pressing time

  switch(key_result->pre_status)
  {
    case RELEASE://the prev status is RELEASE
      if(key_s == RELEASE)//the current status is RELEASE
        goto released_process;
      else//the current status is PRESSING
        goto normal_process;
      break;
    case PRESSING://the prev status is PRESSING
      goto normal_process;
      break;
    default:break;
  }

released_process:
  key_result->key             = key;
  key_result->sys_time        = clock_time();//when scan, store the system time
  key_result->pressing_time   = 0;
normal_process:
  key_result->pre_status      = key_result->cur_status;//save current status for nex check
}

static void key_read(key_state_t * key_result, const key_type_t* key)
{
  key_status_t key_s;//the current key status
  key_index_t key_index = key->key;//the key number

  key->key_scan(&key_s, key_index);//check the key is pressing or not

  cal_key_press_time(key_result, key_s, key_index);//check the key pressing time
}

static inline bool is_different_key(u8 key)
{
  if(first_key_r == 255){//if the first key is not recorded,then record it
    first_key_r = key;//record first key
    return 0;
  }else if(key != first_key_r){//if first key is recorded,check if it is the first key
    second_key_r = key;//record second key
    return 1;
  }else{//still the first key
    return 0;
  }
}

static inline bool is_key_pressing(u8 key, u32 *time)
{
  key_state_t key_f;

  key_f = key_status[key];

  key_read(&key_f, &key_table.key[key]);

  if(time != NULL)
    *time = key_f.pressing_time;

  return (key_f.cur_status == PRESSING)? 1:0;
}

static inline bool is_key_released(u8 key, u32 *time)
{
  key_state_t key_f;

  key_f = key_status[key];

  key_read(&key_f, &key_table.key[key]);

  if(time != NULL)
    *time = key_f.pressing_time;

  return (key_f.cur_status == RELEASE)? 1:0;
}

static inline bool is_key_on(u8 key, u32 time)
{
  key_state_t key_f;

  key_f = key_status[key];

  key_read(&key_f, &key_table.key[key]);

  return ((key_f.cur_status == PRESSING) && (key_f.pressing_time > time))? 1:0;
}

static void get_key_combin_setup_time_and_combin_time(u8 key, u32 *st_time, u32 *cmb_time)
{
  event_handler_t *pos_ptr = NULL;
  u32 setup_time  = 0;
  u32 combin_time = 0;

  list_for_each_entry(pos_ptr, &key_event[key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[key].list){
      if((pos_ptr->key_ac == COMBIN_KEY) || (pos_ptr->key_ac == COMBIN_KEY_IN_TIME)){
        setup_time  = pos_ptr->time1;
        combin_time = pos_ptr->time2;
        break;
      }
    }
  }

  *st_time = setup_time ? setup_time:COMBIN_KEY_SETUP_TIME;

  *cmb_time = combin_time? combin_time:COMBIN_TIME;
}

static inline bool is_key_valid(u8 key, u32 time)
{
  key_state_t key_f;

  key_f = key_status[key];

  key_read(&key_f, &key_table.key[key]);

  return ((key_f.cur_status == PRESSING) && (key_f.pressing_time < time))? 1:0;
}

static inline bool is_key_stuck()
{
  u32 first_key_time;
  u32 second_key_time;

  if(is_key_released(first_key_r, &first_key_time) && is_key_released(second_key_r, &second_key_time))//wait key release
    return ((first_key_time == 0) && (second_key_time == 0)) ? 1:0;//wait key pressing time reset to 0
  else if(is_key_released(first_key_r, &first_key_time) && is_key_on(second_key_r, STUCK_TIME))//first key is released second key is stuck
    return first_key_time ? 0:1;
  else if(is_key_on(first_key_r, STUCK_TIME) && is_key_released(second_key_r, &second_key_time))//first key is stuck second key is released
    return second_key_time ? 0:1;
  else
    //first key and second key is stuck
    return (is_key_on(first_key_r, STUCK_TIME) && is_key_on(second_key_r, STUCK_TIME)) ? 1:0;
}

static inline bool is_leader_key()
{
  return (first_key_r == leader_key) ? 1:0;
}

static inline bool is_leader_key_event_finish()
{
  u32 second_key_time;

  if(is_key_on(first_key_r, STUCK_TIME) && is_key_released(second_key_r, &second_key_time))//first key is stuck second key is released
    return second_key_time ? 0:1;
  else
    return 0;
}

static inline void set_key_event(u8 key, key_action_t key_ac)
{
  key_event[key].key_current_action  |= key_ac;
}

static inline bool is_key_pressing_now(key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) && (key_s->pressing_time)) ? 1:0;
}

static inline bool is_short_immediately_key(key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) &&\
      (key_s->pressing_time) &&\
      !(key_s->key & SHORT_KEY_IMMEDIATELY_FLAG)) ? 1:0;
}

static inline bool is_no_time_limit_key_released(const key_state_t* key_s)
{
  return ((key_s->cur_status == RELEASE) && (key_s->pressing_time))? 1:0;
}

static inline bool is_no_time_limit_key_on(const key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) && (key_s->pressing_time))? 1:0;
}

static bool is_short_released_key(u8 i, const key_state_t* key_s)
{
  event_handler_t *pos_ptr = NULL;
  u32  key_handler_time1 = 0;
  u32 default_time;

  list_for_each_entry(pos_ptr, &key_event[i].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[i].list){
      if((pos_ptr->key_ac == SHORT_KEY)){
        key_handler_time1 = pos_ptr->time1;
        break;
      }
    }
  }

  default_time = key_handler_time1 ? key_handler_time1:SHORT_TIME;

  return ((key_s->cur_status == RELEASE) && (key_s->pressing_time) && (key_s->pressing_time < default_time))? 1:0;
}

static bool is_long_key(u8 i, key_state_t* key_s)
{
  event_handler_t *pos_ptr = NULL;
  u32  key_handler_time1 = 0;
  u32 default_time;

  list_for_each_entry(pos_ptr, &key_event[i].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[i].list){
      if((pos_ptr->key_ac == LONG_KEY)){
        key_handler_time1 = pos_ptr->time1;
        break;
      }
    }
  }

  default_time = key_handler_time1 ? key_handler_time1:LONG_TIME;

  return ((key_s->cur_status == PRESSING) && \
      (key_s->pressing_time > default_time) && \
      (key_s->pressing_time < STUCK_TIME) && \
      !(key_s->key & LONG_KEY_FLAG))? 1:0;//bit31 is the flag that indicaction the long key only once

}

static inline bool is_stuck_key(key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) && \
      (key_s->pressing_time > STUCK_TIME) && \
      !(key_s->key & STUCK_KEY_FLAG))? 1:0;//bit30 is the flag that indication the stuck only once
}

static bool is_combin_key(u8 i, key_state_t* key_s)
{
  event_handler_t *pos_ptr = NULL;
  u32  key_handler_time1 = 0;
  u32  key_handler_time2 = 0;
  u32 default_time;
  u32 default_time2;

  list_for_each_entry(pos_ptr, &key_event[i].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[i].list){
      if((pos_ptr->key_ac == COMBIN_KEY) || (pos_ptr->key_ac == COMBIN_KEY_IN_TIME)){
        key_handler_time1 = pos_ptr->time1;
        key_handler_time2 = pos_ptr->time2;
        break;
      }
    }
  }

  default_time = key_handler_time1? key_handler_time1:COMBIN_KEY_SETUP_TIME;

  default_time2 = key_handler_time2? key_handler_time2:COMBIN_TIME;

  return ((key_s->cur_status == PRESSING) && \
      ((key_s->pressing_time < default_time ) || \
       (key_s->pressing_time < default_time2)))? 1:0;//combin_immediately_occured
}

static inline key_action_t get_key_status(u8 key)
{
  return key_event[key].key_current_action;
}

static inline void clr_key_status(u8 key, u32 status)
{
  key_event[key].key_current_action &= ~status;
}

static handler get_key_handler(u8 key, key_action_t key_ac, u8 second_key)
{
  event_handler_t *pos_ptr = NULL;
  handler key_handler = NULL;

  list_for_each_entry(pos_ptr, &key_event[key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[key].list){
      if((pos_ptr->key_ac == key_ac)){
        if((key_ac == COMBIN_KEY) || (key_ac  == COMBIN_KEY_IN_TIME)){
          if(pos_ptr->second_key == second_key){
            key_handler = pos_ptr->key_handler;
            break;
          }
        }else{
          key_handler = pos_ptr->key_handler;
          break;
        }
      }
    }
  }
  return key_handler;
}
static inline handler get_stuck_key_handler()
{
  return stuck_key_handler;
}

void key_wake_up_init()
{
  u8 type = 0;
  u8 first_key_of_type = 0;
  u8 num_keys_of_type  = 0;

  for(u8 i = 0; i < key_table.num; i++){
    if(type != key_table.key[i].type){
      type = key_table.key[i].type;
      if(num_keys_of_type){
        if((!is_wakeup_from_sleep()) && (key_table.key[first_key_of_type].type == TOUCH_KEY))
          key_table.key[first_key_of_type].key_init(first_key_of_type, first_key_of_type + num_keys_of_type);
        num_keys_of_type = 0;
      }
      first_key_of_type = i;
    }else
      num_keys_of_type++;
  }

  if((!is_wakeup_from_sleep()) && (key_table.key[first_key_of_type].type == TOUCH_KEY))
    key_table.key[first_key_of_type].key_init(first_key_of_type, first_key_of_type + num_keys_of_type);

  if(!is_wakeup_from_sleep()){
    key_status_init();

  }

  key_event_init();
}

void key_init()
{
  u8 type = 0;
  u8 first_key_of_type = 0;
  u8 num_keys_of_type  = 0;

  for(u8 i = 0; i < key_table.num; i++){
    if(type != key_table.key[i].type){
      type = key_table.key[i].type;
      if(num_keys_of_type){
        key_table.key[first_key_of_type].key_init(first_key_of_type, first_key_of_type + num_keys_of_type);
        num_keys_of_type = 0;
      }
      first_key_of_type = i;
    }else
      num_keys_of_type++;
  }

  key_table.key[first_key_of_type].key_init(first_key_of_type, first_key_of_type + num_keys_of_type);

  key_status_init();

  key_event_init();
}

int key_process(void *data)
{
  key_state_t *key_s;
  key_process_t *key_e;

  _attribute_data_retention_  static u8 is_two_key_pressed = 0;
  _attribute_data_retention_  static u8 setup_time_valid = 0;
  _attribute_data_retention_  static u8 invalid_key = 0;

  u32 st_time;
  u32 cmb_time;

  key_s = &key_status[0];
  key_e = &key_event[0];

  for(u8 i = 0; i < MAX_KEY; i++){

    if(!key_table.key)
      return 0;

    key_read(key_s, &key_table.key[i]);

    if(is_key_pressing_now(key_s))
      reload_sys_time();

    if(is_no_time_limit_key_released(key_s))
      key_e->key_current_action |= NO_TIME_LIMIT_KEY_RELEASED;

    if(is_no_time_limit_key_on(key_s))
      key_e->key_current_action |= NO_TIME_LIMIT_KEY_ON;

    if(is_short_immediately_key(key_s)){
      pre_key = cur_key;//if key is pressing,then record it
      cur_key = i;
      set_key_event(i, SHORT_KEY_IMMEDIATELY);
      key_s->key |= SHORT_KEY_IMMEDIATELY_FLAG;//bit29 is the flag that indication the short key only once
    }

    if(!is_two_key_pressed){//there is only one key pressing
      if(is_combin_key(i, key_s)){//combin_immediately_occured
        if(is_different_key(i)){
          get_key_combin_setup_time_and_combin_time(first_key_r, &st_time, &cmb_time);
          if(is_key_valid(first_key_r, st_time)){//first_key is still on and time is less than COMBIN_KEY_SETUP_TIME
            goto setup_valid;
          }else if(is_key_valid(first_key_r, cmb_time)){//first_key is still on and time is less than COMBIN_TIME
            goto two_key_pressed;
          }else if(is_key_released(first_key_r, NULL)){//first key is released
            first_key_r = i;
            goto the_next;
          }else if(is_leader_key())//first key is still on and is the leader_key
            goto two_key_pressed;
          else{//first key is not leader_key ,but still on and time is more than COMBIN_TIME
            is_two_key_pressed = 1;
            invalid_key = 1;
            goto the_next;
          }
setup_valid:
          setup_time_valid = 1;
two_key_pressed:
          is_two_key_pressed = 1;
          set_key_event(first_key_r,  COMBIN_KEY | FIRST_KEY_FLAG);//bit31 is used as firstkey flag
          set_key_event(second_key_r, COMBIN_KEY | SECOND_KEY_FLAG);//bit30 is used as secondkey flag
        }
      }else if(is_long_key(i, key_s)){//bit31 is the flag that indicaction the long key only once
        key_e->key_current_action |= LONG_KEY;
        key_s->key |= LONG_KEY_FLAG;
      }else if(is_stuck_key(key_s)){//bit30 is the flag that indication the stuck only once
        key_e->key_current_action |= STUCK_KEY;
        key_s->key |= STUCK_KEY_FLAG;
      }else if(is_short_released_key(i, key_s)){
        key_e->key_current_action |= SHORT_KEY;
      }
    }else{//there is two key pressing
      if(invalid_key){//two key is pressing ,but invalid
        if(is_key_stuck()){
          first_key_r        = 255;
          second_key_r       = 255;
          invalid_key        = 0;
          is_two_key_pressed = 0;
        }
      }else{//two key is pressing and valid
        get_key_combin_setup_time_and_combin_time(first_key_r, &st_time, &cmb_time);
        if(setup_time_valid && is_key_on(second_key_r, cmb_time)){//two key is pressing and time is more than COMBIN_KEY_LAST_TIME
          setup_time_valid = 0;
          set_key_event(first_key_r,   COMBIN_KEY_IN_TIME | FIRST_KEY_FLAG );//bit31 used as firstkey flag  
          set_key_event(second_key_r,  COMBIN_KEY_IN_TIME | SECOND_KEY_FLAG);//bit30 used as secondkey flag
        }else if(is_key_stuck()) {//check if stuck key
          if(is_leader_key() && is_leader_key_event_finish()){//check if leader_key
            is_two_key_pressed = 0;
            setup_time_valid   = 0;
            second_key_r       = 255;
          }else{
            is_two_key_pressed = 0;
            setup_time_valid   = 0;
            first_key_r        = 255;
            second_key_r       = 255;
          }
        }
      }
    }
the_next:
    key_s++;
    key_e++;
  }
  return 0;
}

void register_key_event(u8 first_key, u8 second_key, u32 time1, u32 time2, key_action_t key_ac, handler key_handler)
{
  event_handler_t *key_hand;

  key_hand = (event_handler_t *)mempool_alloc(&KEY_EVENT_POOL);

  if(!key_hand)
    return;

  key_hand->second_key  = second_key;
  key_hand->key_ac      = key_ac;
  key_hand->key_handler = key_handler;
  key_hand->time1       = time1;
  key_hand->time2       = time2;

  INIT_LIST_HEAD(&key_hand->list);

  g_list_add_tail(&key_hand->list, &key_event[first_key].list);
}

void register_key(const key_type_t *key, u8 num)
{
  key_table.key = key;
  key_table.num = num;
}

void set_leader_key(u8 key)
{
  leader_key = key;
}

void set_stuck_key_handler(handler stuck_handler)
{
  stuck_key_handler = stuck_handler;
}

void poll_key_event()
{
  handler key_handler = NULL;
  key_action_t key_action;
  key_action_t action;
  key_action_t key;
  u8 first  = 255;
  u8 second = 255;

  for(u8 i = 0; i < MAX_KEY; i++){

    key_action = get_key_status(i);//get key status from key_event

    key = SHORT_KEY;

    while(key_action){
      if((key_action & COMBIN_KEY) || (key_action & COMBIN_KEY_IN_TIME)){

    	  action =  key_action & (COMBIN_KEY | COMBIN_KEY_IN_TIME);

          if(key_action & FIRST_KEY_FLAG){//check if it is the first key
            first = i;
            key_action = 0;
          }else{
            second = i;
            key_action = 0;
          }

          if((first != 255) && (second != 255)){
            key_handler = get_key_handler(first, action, second);//clear bit7 and bit6 clear first key and second key flag
            if(key_handler)
              key_handler();

            clr_key_status(first,  0xffffffff);
            clr_key_status(second, 0xffffffff);
          }
      }else{
    	  action =  key_action & key;

    	  if(action == STUCK_KEY){
    		  clr_key_status(i, STUCK_KEY);
    		  key_handler = get_stuck_key_handler();
    		  if(key_handler)
    			  key_handler();
    		  key_action ^= key;
        }else if(action){//short key short immedia key long key process
          //warning bit31 and bit30 are combinkey flag short key or long key
          clr_key_status(i, action);
          key_handler = get_key_handler(i, action, 0);
          if(key_handler)
            key_handler();
          key_action ^= key;
        }
        key <<= 1;
      }
    }
  }
}

u8 app_read_key(u8 first_key, u8 second_key)
{
  _attribute_data_retention_ u8 static only_once = 1;

  if(only_once){
    if(is_key_released(first_key, NULL) && is_key_released(second_key, NULL)){
      only_once = 0;
      return 0;
    }
  }else if(is_key_pressing(first_key, NULL) && is_key_pressing(second_key, NULL)){
    only_once = 1;
    return 1;
  }

  return 0;
}

u8 app_read_single_key(u8 key)
{
  _attribute_data_retention_ u8 static only_once = 1;

  if(only_once){
    if(is_key_released(key, NULL)){
      only_once = 0;
      return 0;
    }
  }else if(is_key_pressing(key, NULL)){
    only_once = 1;
    return 1;
  }

  return 0;
}
