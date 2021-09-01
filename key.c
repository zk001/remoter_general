#include "n_timer.h"
#include "../../common/mempool.h"
#include "main.h"
#include "board.h"
#include "gpio_key.h"
#include "apt8l08_key.h"
#include "power_saving.h"
#include "key.h"
#include "led.h"
#include "wakeup.h"

u8 wakeup_key = 255;
_attribute_data_retention_ u8 leader_key    = 255;
_attribute_data_retention_ u8 pre_key       = 255;
_attribute_data_retention_ u8 cur_key       = 255;

_attribute_data_retention_ static u8 first_key   = 255;
_attribute_data_retention_ static u8 second_key  = 255;
_attribute_data_retention_ static handler       stuck_key_handler;
_attribute_data_retention_ static key_state_t   key_status[MAX_KEYS];
_attribute_data_retention_ static key_process_t key_event[MAX_KEYS];
_attribute_data_retention_ static key_table_t   key_table;
_attribute_data_retention_ handler normal_handler = NULL;

static void key_status_init()
{
  for(u8 i = 0; i < key_table.num; i++){
    key_status[i].flag          = 0;
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

static void cal_key_press_time(key_state_t * key_result, key_status_t key_s)
{
  key_result->pre_status      = key_result->cur_status;//save current status for nex check
  key_result->cur_status      = key_s;
  key_result->pressing_time   = (u32)((int)clock_time() - (int)key_result->sys_time);//caul the pressing time

  switch(key_result->pre_status){
    case RELEASE://the prev status is RELEASE
      if(key_s == RELEASE)//the current status is RELEASE
        goto released_process;
      else//the current status is PRESSING
        goto normal_process;
      break;
    case PRESSING://the prev status is PRESSING
      goto normal_process;
      break;
    default: break;
  }

released_process:
  key_result->flag            = 0;
  key_result->sys_time        = clock_time();//when scan, store the system time
  key_result->pressing_time   = 0;
normal_process:
  ;
}

static void key_read(key_state_t* key_result, const key_type_t* key)
{
  key_status_t key_s;//the current key status

  key_index_t key_index = key->key;//the key number

  key->key_scan(&key_s, key_index);//check the key is pressing or not

  cal_key_press_time(key_result, key_s);//cal the key pressing time
}

static bool is_stuck_key_released(key_state_t* key_result, const key_type_t* key)
{
  key_status_t key_s;//the current key status

  key_index_t key_index = key->key;//the key number

  key->stuck_scan(&key_s, key_index);//check the key is pressing or not

  return (key_s == RELEASE);
}

static inline void clr_first_key()
{
  first_key = 255;
}

static inline void clr_second_key()
{
  second_key = 255;
}

static inline bool is_first_key_valid()
{
  return (first_key != 255);
}

static inline u8 get_first_key()
{
  return first_key;
}

static inline u8 get_second_key()
{
  return second_key;
}

static inline void set_first_key(u8 key)
{
  first_key = key;
}

static inline void set_second_key(u8 key)
{
  second_key = key;
}

static inline bool is_diff_with_first_key(u8 key)
{
  return (first_key != key);
}

static bool is_key_pressing(u8 key, u32 *time)
{
  key_state_t key_f;

  key_f = key_status[key];

  key_read(&key_f, &key_table.key[key]);

  key_status[key] = key_f;

  if(time != NULL)
    *time = key_f.pressing_time;

  return (key_f.cur_status == PRESSING);
}

static bool is_key_released(u8 key, u32 *time)
{
  key_state_t key_f;

  key_f = key_status[key];

  key_read(&key_f, &key_table.key[key]);

  key_status[key] = key_f;

  if(time != NULL)
    *time = key_f.pressing_time;

  return (key_f.cur_status == RELEASE);
}

static inline bool is_key_pressing_exceed_time(u8 key, u32 time)
{
  u32 pressing_time;

  return (is_key_pressing(key, &pressing_time))? (pressing_time > time):0;
}

static inline bool is_key_pressing_less_than_time(u8 key, u32 time)
{
  u32 pressing_time;

  return (is_key_pressing(key, &pressing_time))? (pressing_time < time):0;
}

static inline bool is_two_key_released(u8 first, u8 second)
{
  u32 first_key_time;
  u32 second_key_time;

  return (is_key_released(first, &first_key_time) && is_key_released(second, &second_key_time))?\
    ((first_key_time == 0) && (second_key_time == 0)):0;
}

static inline bool is_single_key_released(u8 key)
{
  u32 time;

  return (is_key_released(key, &time))? (time == 0):0;
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

  *st_time = setup_time ? setup_time:COMBIN_KEY_SETUP_TIME;//setup time

  *cmb_time = combin_time? combin_time:COMBIN_TIME;//last time
}

static void get_register_key_comb_action(u8 first_key, u8 second_key, key_action_t *action)
{
  event_handler_t *pos_ptr = NULL;

  list_for_each_entry(pos_ptr, &key_event[first_key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[first_key].list){
      if(((pos_ptr->key_ac == COMBIN_KEY) || (pos_ptr->key_ac == COMBIN_KEY_IN_TIME)) && (pos_ptr->second_key == second_key)){
        *action = pos_ptr->key_ac;
        break;
      }
    }
  }
}

static inline bool is_leader_key_on()
{
  if(get_first_key() == leader_key)
    return is_key_pressing(leader_key, NULL);
  else
    return 0;
}

static inline void set_key_event(u8 key, key_action_t key_ac)
{
  key_event[key].key_current_action  |= key_ac;
}

static inline void clr_key_flag(u8 key, u32 flag)
{
  key_status[key].flag &= ~flag;
}

static inline void set_key_flag(u8 key, u32 flag)
{
  key_status[key].flag |= flag;
}

static inline bool is_stuck_key(u8 key)
{
  return (key_status[key].flag & STUCK_KEY_FLAG) ? 1:0;
}

static inline void set_current_key_flag(key_state_t *key_s, u32 flag)
{
  key_s->flag |= flag;
}

static inline bool is_current_key_stuck(const key_state_t* key_s)
{
  return (key_s->flag & STUCK_KEY_FLAG) ? 1:0;
}

static inline bool is_current_key_pressing(const key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) && (key_s->pressing_time));
}

static inline bool is_key_from_release_to_pressing(const key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) && (key_s->pre_status == RELEASE));
}

static inline bool is_current_key_short_immediately(const key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) &&\
      (key_s->pressing_time) &&\
      !(key_s->flag & SHORT_KEY_IMMEDIATELY_FLAG));
}

static inline bool is_current_key_no_time_limit_released(const key_state_t* key_s)
{
  return ((key_s->cur_status == RELEASE) && (key_s->pressing_time));
}

static inline bool is_current_key_no_time_limit_on(const key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) && (key_s->pressing_time));
}

static bool is_short_key_released(u8 i, const key_state_t* key_s)
{
  event_handler_t *pos_ptr = NULL;
  u32  key_handler_time1 = 0;
  u32 time;

  list_for_each_entry(pos_ptr, &key_event[i].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[i].list){
      if((pos_ptr->key_ac == SHORT_KEY)){
        key_handler_time1 = pos_ptr->time1;
        break;
      }
    }
  }

  time = key_handler_time1 ? key_handler_time1:SHORT_TIME;

  return ((key_s->cur_status == RELEASE) && (key_s->pressing_time) && (key_s->pressing_time < time));
}

static bool is_long_key(u8 i, key_state_t* key_s)
{
  event_handler_t *pos_ptr = NULL;
  u32  key_handler_time1 = 0;
  u32 time;

  list_for_each_entry(pos_ptr, &key_event[i].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[i].list){
      if((pos_ptr->key_ac == LONG_KEY)){
        key_handler_time1 = pos_ptr->time1;
        break;
      }
    }
  }

  time = key_handler_time1 ? key_handler_time1:LONG_TIME;

  return ((key_s->cur_status == PRESSING) && \
      (key_s->pressing_time > time) && \
      (key_s->pressing_time < STUCK_TIME) && \
      !(key_s->flag & LONG_KEY_FLAG));//bit31 is the flag that indicaction the long key only once
}

static bool is_long_key_8s(u8 i, key_state_t* key_s)
{
  event_handler_t *pos_ptr = NULL;

  list_for_each_entry(pos_ptr, &key_event[i].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[i].list){
      if((pos_ptr->key_ac == LONG_KEY_IN_8S)){
        break;
      }
    }
  }

  return ((key_s->cur_status == PRESSING) && \
      (key_s->pressing_time > LONG_KEY_8S) && \
      (key_s->pressing_time < STUCK_TIME) && \
      !(key_s->flag & LONG_8S_KEY_FLAG));//bit31 is the flag that indicaction the long key only once
}

static inline bool is_key_stuck(key_state_t* key_s)
{
  return ((key_s->cur_status == PRESSING) && \
      (key_s->pressing_time > STUCK_TIME) && \
      !(key_s->flag & STUCK_KEY_FLAG));//bit30 is the flag that indication the stuck only once
}

static inline key_action_t get_key_action(u8 key)
{
  return key_event[key].key_current_action;
}

static inline void clr_key_action(u8 key, u32 action)
{
  key_event[key].key_current_action &= ~action;
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

static void set_wakeup_pin(u8 key)
{
  if(key_table.key[key].type == MECHANICAL_KEY)
    gpio_key_sleep_set(key);
  else if(key_table.key[key].type == TOUCH_KEY){
#if (defined AW9523_LED)
    touch_key_sleep_set(key);
#endif
  }
}

static void clr_wakeup_pin(u8 key)
{
  if(key_table.key[key].type == MECHANICAL_KEY)
    gpio_key_sleep_unset(key);
  else if(key_table.key[key].type == TOUCH_KEY){
#if (defined AW9523_LED)
    touch_key_sleep_unset(key);
#endif
  }
}

void key_wakeup_init()
{
  u8 type = 0;
  u8 first_key_of_type = 0;
  u8 num_keys_of_type  = 0;

  for(u8 i = 0; i < key_table.num; i++){
    if(type != key_table.key[i].type){
      type = key_table.key[i].type;
      if(num_keys_of_type){
        if(key_table.key[first_key_of_type].type != TOUCH_KEY)
          key_table.key[first_key_of_type].key_init(first_key_of_type, first_key_of_type + num_keys_of_type);
        num_keys_of_type = 0;
      }
      first_key_of_type = i;
    }else
      num_keys_of_type++;
  }

  if(key_table.key[first_key_of_type].type != TOUCH_KEY)
    key_table.key[first_key_of_type].key_init(first_key_of_type, first_key_of_type + num_keys_of_type);

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

bool get_other_map_key(u32 *key_map, u8 *key)
{
  u32  key_iterator = 1;
  u32  tmp_itor = 0;
  bool has_key = 0;
  u8 real_key = 0;

  while(*key_map){
    if(*key_map & key_iterator){
      *key_map ^= key_iterator;

      while(key_iterator != tmp_itor){
        tmp_itor = 1 << real_key;
        real_key++;
      }
      real_key--;
      *key = real_key;
      has_key = 1;
      break;
    }
    key_iterator <<= 1;
  }
  return has_key;
}

static bool is_all_other_key_stuck(u32 key_map)
{
  u32  key_iterator = 1;
  u32  tmp_itor = 0;
  u32  is_stuck = 0;
  u32  tmp_key = key_map;
  u8  real_key = 0;

  while(key_map){
    if(key_map & key_iterator){
      key_map ^= key_iterator;

      while(key_iterator != tmp_itor){
        tmp_itor = 1 << real_key;
        real_key++;
      }
      real_key--;
      if(is_stuck_key(real_key))
        is_stuck |= 1 << real_key;

      tmp_itor = 0;
      real_key = 0;
    }

    key_iterator <<= 1;
  }
  return (is_stuck ^ tmp_key)? 0:1;
}

static void process_stuck_key(u8 key)
{
  if(!is_stuck_key(key)){
    if(is_key_pressing_exceed_time(key, STUCK_TIME)){//if second key stuck?
      clr_wakeup_pin(key);
      set_key_event(key, STUCK_KEY);
      set_key_flag(key, STUCK_KEY_FLAG);
    }
  }
}

int key_process(void *data)
{
  key_state_t *key_s;
  key_process_t *key_e;

  static bool is_more_one_key_pressing;
  static bool setup_time_valid;

  static u32 pressed_tick;
  static u8 key_start_time_window = 255;
  static u8 released_times;
  static u8 pressed_times;
  static u32 other_key_map;
  static bool is_break_combin_key;

  u32 st_time;
  u32 cmb_time;
  u32 interval;

  key_s = &key_status[0];
  key_e = &key_event[0];

  for(u8 i = 0; i < MAX_KEYS; i++){

    if(!key_table.key)
      goto return_0;

    if(is_current_key_stuck(key_s))//current key is stuck key,then goto stuck_key_process
      goto stuck_key_process;

    //when two key is pressing, current key is not first key or second key, then check next key
    if(is_more_one_key_pressing){
      if((get_first_key() == i) || (get_second_key() == i))
        ;
      else{
        if(is_key_pressing(i, NULL)){//when two keys are pressing,check if other key is pressing
          other_key_map |= 1 << i;
          is_break_combin_key = 1;
        }
        else
          other_key_map &= ~(1 << i);
        goto the_next;
      }
    }

    key_read(key_s, &key_table.key[i]);

    //t twice s short
    //pressed_times  0  1 1 2 2
    //released_times 0  0 1 1 2
    //               x  x s x t
    if(key_start_time_window != 255){
      interval = (int)clock_time() - (int)pressed_tick;
      if(interval <= ONE_KEY_PRESSING_TWICE_INTERVAL){
        if(released_times == 2){
          released_times = 0;
          pressed_times  = 0;
          key_start_time_window = 255;//stop
          key_e->key_current_action |= ONE_KEY_TWICE;
        }
      }else{//over ONE_KEY_PRESSING_TWICE_INTERVAL
        if(pressed_times == 1 && released_times == 1)
          key_e->key_current_action |= ONE_KEY_TWICE_ONLY_ONCE;
        released_times = 0;
        pressed_times  = 0;
        key_start_time_window = 255;//stop
      }
    }

    if(is_wakeup_from_sleep()){
      if(wakeup_key == 255){
        if(is_current_key_pressing(key_s)){//if current key is pressing, reload system tick
          wakeup_key = i;
        }
      }
    }

    if(is_current_key_pressing(key_s))//if current key is pressing, reload system tick
      reload_sys_time();

    if(get_second_key() != i){//if two keys are pressing, not set the second key NO_TIME_LIMIT_KEY_RELEASED action and NO_TIME_LIMIT_KEY_ON action
      if(is_current_key_no_time_limit_released(key_s))//if current key is been pressed and now been released, set NO_TIME_LIMIT_KEY_RELEASED action
        key_e->key_current_action |= NO_TIME_LIMIT_KEY_RELEASED;

      if(is_current_key_no_time_limit_on(key_s))//if current key is pressing, set NO_TIME_LIMIT_KEY_ON action
        key_e->key_current_action |= NO_TIME_LIMIT_KEY_ON;
    }

    if(!is_more_one_key_pressing){//check if there is two key pressing or not
      if(is_current_key_pressing(key_s)){//if current key is pressing
        if(is_current_key_short_immediately(key_s)){//if current key is pressing,set SHORT_KEY_IMMEDIATELY_FLAG action
          pre_key = cur_key;
          cur_key = i;
          key_e->key_current_action |= SHORT_KEY_IMMEDIATELY;
          set_current_key_flag(key_s, SHORT_KEY_IMMEDIATELY_FLAG);
        }

        if(is_first_key_valid()){//check if first key is record or not
          if(is_diff_with_first_key(i)){//check current key is diff with prev key
            //current key is different with prev key
            if(is_key_pressing(get_first_key(), NULL)){//is first key pressing?
              set_second_key(i);
              goto two_key_pressing;//first key is pressing, goto two_key_pressing
            }//first key is released
          }//current key is same with prev key
        }//first key is not record, then record it
        set_first_key(i);//record current key as the first_key
        goto single_key_pressing;//current key is pressing, goto one_key_pressed process
      }//the current key is not pressing 1995076
      goto single_released_key_check;//goto single_released_key_check process to check
    }//two key is pressing

    if(setup_time_valid)
      goto two_key_pressed_in_time;//check COMBIN_KEY_IN_TIME action

    goto wait_key_reset;//COMBIN_KEY_IN_TIME action is finish, wait two key released

single_key_pressing:
    if(is_long_key_8s(i, key_s)){
      key_e->key_current_action |= LONG_KEY_IN_8S;
      set_current_key_flag(key_s, LONG_8S_KEY_FLAG);
    }
    if(is_long_key(i, key_s)){
      key_e->key_current_action |= LONG_KEY;
      set_current_key_flag(key_s, LONG_KEY_FLAG);
    }else if(is_key_stuck(key_s)){
      key_e->key_current_action |= STUCK_KEY;
      set_current_key_flag(key_s, STUCK_KEY_FLAG);
      clr_wakeup_pin(i);
      clr_first_key();
    }
    //key from released to pressing && time window is not start, then start time windows
    //key from released to pressing && is other key is pressing, then start time windows
    if(is_key_from_release_to_pressing(key_s) && (key_start_time_window == 255 || key_start_time_window != i)){
      pressed_times = 1;
      key_start_time_window = i;//which key starts the time windows
      pressed_tick = clock_time();
    }

    goto the_next;//continue check the next key

single_released_key_check:
    if(is_short_key_released(i, key_s)){
      key_e->key_current_action |= SHORT_KEY;

      //time window is valid
      if(key_start_time_window != 255)
        released_times++;
    }

    goto the_next;//continue check the next key

two_key_pressing:
    is_more_one_key_pressing = 1;
    clr_key_action(get_second_key(), SHORT_KEY_IMMEDIATELY);
    clr_key_action(get_second_key(), NO_TIME_LIMIT_KEY_ON);
    get_key_combin_setup_time_and_combin_time(get_first_key(), &st_time, &cmb_time);
    if(is_key_pressing_less_than_time(get_first_key(), cmb_time)){//two key is pressing, and first key pressing time is less than cmb_time
      set_key_event(get_first_key(),  COMBIN_KEY | FIRST_KEY_FLAG);
      set_key_event(get_second_key(), COMBIN_KEY | SECOND_KEY_FLAG);
    }

    if(is_key_pressing_less_than_time(get_first_key(), st_time))//two key is pressing, and first key pressing time is less than st_time
      setup_time_valid = 1;

    if(key_start_time_window != 255){
      released_times = 0;
      pressed_times  = 0;
      key_start_time_window = 255;//stop
    }
    goto the_next;

two_key_pressed_in_time:
    if(!is_break_combin_key){
      get_key_combin_setup_time_and_combin_time(get_first_key(), &st_time, &cmb_time);
      if(is_key_pressing(get_first_key(), NULL) && is_key_pressing(get_second_key(), NULL)){
        if(is_key_pressing_exceed_time(get_second_key(), cmb_time)){//if pressing time is more than cmb_time
          setup_time_valid = 0;
          set_key_event(get_first_key(),   COMBIN_KEY_IN_TIME | FIRST_KEY_FLAG );//bit31 used as firstkey flag
          set_key_event(get_second_key(),  COMBIN_KEY_IN_TIME | SECOND_KEY_FLAG);//bit30 used as secondkey flag
        }
      }else//key is released, clr the flag
        setup_time_valid = 0;
    }

wait_key_reset:
    process_stuck_key(get_first_key());

    process_stuck_key(get_second_key());

    u32 tmp_other_key_map;
    tmp_other_key_map = other_key_map;
    while(tmp_other_key_map){
      u8 other_key;
      if(get_other_map_key(&tmp_other_key_map, &other_key)){
        process_stuck_key(other_key);
      }
    }

    //two keys are released,then reset
    //two keys are stuck,then reset
    //first key is stuck,second key is released,then reset
    //second key is stuck,fist key is released, then reset
    //if first key is leader key and second key is released,then reset
    if((is_two_key_released(get_first_key(), get_second_key()) ||\
          (is_stuck_key(get_first_key()) && is_stuck_key(get_second_key())) ||\
          (is_stuck_key(get_first_key())  && is_single_key_released(get_second_key())) ||\
          (is_stuck_key(get_second_key()) && is_single_key_released(get_first_key())) ||\
          (is_leader_key_on(get_first_key()) && is_single_key_released(get_second_key()))) &&\
        (is_all_other_key_stuck(other_key_map)))
      goto reset_key_val;

    goto the_next;

stuck_key_process:
    if(is_stuck_key_released(key_s, &key_table.key[i])){
      clr_key_flag(i, STUCK_KEY_FLAG);
      set_wakeup_pin(i);
    }

    goto the_next;

reset_key_val:
    clr_first_key();
    clr_second_key();
    is_more_one_key_pressing = 0;
    other_key_map = 0;
    is_break_combin_key = 0;
the_next:
    key_s++;
    key_e++;
  }

return_0:
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

void register_normal_sys_event(handler normal_cb)
{
  normal_handler = normal_cb;
}

void poll_key_event()
{
  handler key_handler = NULL;
  key_action_t key_action;
  key_action_t action;
  key_action_t key;
  u8 first  = 255;
  u8 second = 255;

  for(u8 i = 0; i < MAX_KEYS; i++){
    key_action = get_key_action(i);//get key status from key_event
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
          key_handler = get_key_handler(first, action, second);
          clr_key_action(first,  0xffffffff);
          clr_key_action(second, 0xffffffff);
          if(key_handler)
            key_handler();
          if(normal_handler)
            normal_handler();
        }
      }else{
        action =  key_action & key;

        if(action == STUCK_KEY){
          key_action ^= key;
          clr_key_action(i, STUCK_KEY);
          key_handler = get_stuck_key_handler();
          if(key_handler)
            key_handler();
        }else if(action){//short key short immedia key long key process
          clr_key_action(i, action);
          key_handler = get_key_handler(i, action, 0);
          if(key_handler)
            key_handler();
          if(normal_handler)
            normal_handler();
          key_action ^= key;
        }
        key <<= 1;
      }
    }
  }
}

bool app_read_key(u8 first_key, u8 second_key)
{
  key_action_t key_action;
  key_action_t action;
  key_action_t key;
  key_action_t register_action = RELEASE_KEY;
  u8 first  = 255;
  u8 second = 255;

  key_process(NULL);

  for(u8 i = 0; i < MAX_KEYS; i++){
    key_action = get_key_action(i);//get key status from key_event
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
          clr_key_action(first,  0xffffffff);
          clr_key_action(second, 0xffffffff);

          get_register_key_comb_action(first_key, second_key, &register_action);
          if((register_action == action) && (first == first_key) && (second == second_key))
            return 1;
        }
      }else{
        action =  key_action & key;
        if(action == STUCK_KEY){
          key_action ^= key;
          clr_key_action(i, STUCK_KEY);
        }else if(action){//short key short immedia key long key process
          clr_key_action(i, action);
          key_action ^= key;
        }
        key <<= 1;
      }
    }
  }
  return 0;
}

bool app_read_single_key(u8 first_key)
{
  key_action_t key_action;
  key_action_t action;
  key_action_t key;
  u8 first  = 255;
  u8 second = 255;

  key_process(NULL);

  for(u8 i = 0; i < MAX_KEYS; i++){
    key_action = get_key_action(i);//get key status from key_event
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
          clr_key_action(first,  0xffffffff);
          clr_key_action(second, 0xffffffff);
        }
      }else{
        action =  key_action & key;
        if(action == STUCK_KEY){
          key_action ^= key;
          clr_key_action(i, STUCK_KEY);
        }else if(action){//short key short immedia key long key process
          clr_key_action(i, action);
          key_action ^= key;
          if((first_key == i) && (action == SHORT_KEY_IMMEDIATELY))
            return 1;
        }
        key <<= 1;
      }
    }
  }
  return 0;
}
