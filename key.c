/********************************************************************************************************
 * @file     key.c
 *
 * @brief    This is the source file for TLSR8258
 *
 * @author	 Driver Group
 * @date     Sep 22, 2021
 *
 * @par      Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 * @par      History:
 * 			 1.initial release(DEC. 26 2018)
 *
 * @version  A001
 *
 *******************************************************************************************************/
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

bool exit_peidui;
u8 wakeup_key = 255;
u8 pre_key    = 255;
u8 cur_key    = 255;

_attribute_data_retention_ static handler       stuck_key_handler;
_attribute_data_retention_ static key_state_t   key_status[MAX_KEYS];
_attribute_data_retention_ static key_process_t key_event[MAX_KEYS];
_attribute_data_retention_ static key_table_t   key_table;
_attribute_data_retention_ handler normal_handler = NULL;

static u8 first_key   = 255;
static u8 second_key  = 255;
static u8 key_start_time_window = 255;
static u8 pressed_times;
static u8 released_times;
static u32 pressed_tick;
static u32 long_key_time;
static process_status_t process_status = INITIAL_PROCESS;

/**
 * @brief      This function serves to set key status structure to initial
 * @param[in]  none
 * @return     none
 */
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

/**
 * @brief      This function serves to set key event structure to initial
 * @param[in]  none
 * @return     none
 */
static void key_event_init()
{
  for(u8 i = 0; i < key_table.num; i++){
    key_event[i].key_current_action = RELEASE_KEY;
    INIT_LIST_HEAD(&key_event[i].list);
  }
}

/**
 * @brief      This function serves to set key state structure
 * @param[out] key_result - get the key state according to the key_s
 * @param[in]  key_s      - indicate the key status, PRESSING or RELEASE
 * @return     none
 */
static void set_key_state(key_state_t* key_result, key_status_t key_s)
{
  key_status_t pre_pre_status;

  pre_pre_status         = key_result->pre_status;
  key_result->pre_status = key_result->cur_status;//save current status for nex check
  key_result->cur_status = key_s;

  if(pre_pre_status == RELEASE && key_result->pre_status == RELEASE && key_result->cur_status == RELEASE)
    return;

  switch(key_result->pre_status){
	case RELEASE://the prev status is RELEASE
	  if(key_result->cur_status == RELEASE){//the current status is RELEASE
	    key_result->flag          = 0;
        key_result->sys_time      = 0;//when scan, store the system time
        key_result->pressing_time = 0;
	  }else//the current status is PRESSING
        key_result->sys_time      = clock_time();
	  break;
	case PRESSING://the prev status is PRESSING
	  key_result->pressing_time   = (u32)((int)clock_time() - (int)key_result->sys_time);//caul the pressing time
	  break;
	default: break;
  }
}

/**
 * @brief      This function serves to read key state structure
 * @param[out] key_result - get the key state according to the key
 * @param[in]  key        - the key handler which to be read
 * @return     none
 */
static void key_read(key_state_t* key_result, const key_type_t* key)
{
  key_status_t key_s;//the current key status

  key_index_t key_index = key->key;//the key number

  key->key_scan(&key_s, key_index);//check the key is pressing or not

  set_key_state(key_result, key_s);//cal the key pressing time
}

/**
 * @brief      This function serves to checkout the key pressing time is exceed than the time
 * @param[in]  status - the status handler of the key,which include pressing time
 * @param[in]  time   - the time will be compared with the pressing time
 * @return the result of the compare with the pressing time and time
 */
static inline bool is_key_pressing_exceed_time(const key_state_t* status, u32 time)
{
  return status->pressing_time > time;
}

/**
 * @brief      This function serves to checkout the key pressing time is less than the time
 * @param[in]  status - the status handler of the key,which include pressing time
 * @param[in]  time   - the time will be compared with the pressing time
 * @return the result of the compare with the pressing time and time
 */
static inline bool is_key_pressing_less_than_time(const key_state_t* status, u32 time)
{
  return status->pressing_time < time;
}

/**
 * @brief      This function serves to set the key action
 * @param[in]  key     - the key will be set
 * @param[in]  action  - the action will be set to the key
 * @return     none
 */
static inline void set_key_action(u8 key, key_action_t action)
{
  key_event[key].key_current_action  |= action;
}

/**
 * @brief      This function serves to get the key action
 * @param[in]  key     - the key will be get
 * @return the action which will be get from the key
 */
static inline key_action_t get_key_action(u8 key)
{
  return key_event[key].key_current_action;
}

/**
 * @brief      This function serves to clear the key action
 * @param[in]  key     - the key will be clear
 * @param[in]  action  - the action will be clear
 * @return     none
 */
static inline void clr_key_action(u8 key, u32 action)
{
  key_event[key].key_current_action &= ~action;
}

/**
 * @brief      This function serves to set the key flag
 * @param[out] key_s - the key handler
 * @param[in]  flag  - the key flag will be set to the key_s
 * @return     none
 */
static inline void set_current_key_flag(key_state_t* key_s, u32 flag)
{
  key_s->flag |= flag;
}

/**
 * @brief      This function serves to get the key flag
 * @param[in]  key_s - the key flag which will be get from the key_s
 * @return the key flag
 */
static inline u32 get_current_key_flag(const key_state_t* key_s)
{
  return key_s->flag;
}

/**
 * @brief      This function serves to get the key pressing time
 * @param[in]  key_s - the key pressing time which will be get form the key_s
 * @return the key pressing time
 */
static inline u32 get_pressing_time(const key_state_t* key_s)
{
  return key_s->pressing_time;
}

/**
 * @brief      This function serves to checkout if the key status is from release to pressing
 * @param[in]  key_s - the key handler which will be checkout
 * @return true if the key status is from release to pressing
 */
static inline bool is_key_from_release_to_pressing(const key_state_t* key_s)
{
  return key_s->cur_status == PRESSING && key_s->pre_status == RELEASE;
}

/**
 * @brief      This function serves to checkout if the key status is from pressing to release ignore the pressing time
 * @param[in]  key_s - the key handler which will be checkout
 * @return true if the key status is from pressing to release
 */
static inline bool is_current_key_no_time_limit_released(const key_state_t* key_s)
{
  return key_s->cur_status == RELEASE && key_s->pressing_time;
}

/**
 * @brief      This function serves to get the key state structure
 * @param[in]  key - the key number
 * @return the key state structure
 */
static inline key_state_t* get_key_status(u8 key)
{
  return &key_status[key];
}

/**
 * @brief      This function serves to get the key setup time and combin time
 * @param[in]  key      - which key will be getting the setup time and combin time
 * @param[out] st_time  - key setup time
 * @param[out] cmb_time - key combin time
 * @return     none
 */
static void get_key_combin_setup_time_and_combin_time(u8 key, u32* st_time, u32* cmb_time)
{
  event_handler_t* pos_ptr = NULL;
  u32 setup_time  = 0;
  u32 combin_time = 0;

  list_for_each_entry(pos_ptr, &key_event[key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[key].list){
      if(pos_ptr->key_ac == COMBIN_KEY || pos_ptr->key_ac == COMBIN_KEY_IN_TIME){
        setup_time  = pos_ptr->time1;
        combin_time = pos_ptr->time2;
        break;
      }
    }
  }

  *st_time = setup_time ? setup_time:COMBIN_KEY_SETUP_TIME;//setup time

  *cmb_time = combin_time? combin_time:COMBIN_TIME;//last time
}

/**
 * @brief      This function serves to get the two key action
 * @param[in]  first_key   - the first  key of the two key action
 * @param[in]  second_key  - the second key of the two key action
 * @param[out] action      - get the first key and second key action
 * @return     none
 */
static void get_register_key_comb_action(u8 first_key, u8 second_key, key_action_t* action)
{
  event_handler_t* pos_ptr = NULL;

  list_for_each_entry(pos_ptr, &key_event[first_key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[first_key].list){
      if((pos_ptr->key_ac == COMBIN_KEY || pos_ptr->key_ac == COMBIN_KEY_IN_TIME) && pos_ptr->second_key == second_key){
        *action = pos_ptr->key_ac;
        break;
      }
    }
  }
}

/**
 * @brief      This function serves to checkout if the key is short key
 * @param[in]  key    - the key will be checkout
 * @param[in]  key_s  - the key handler
 * @return the result if it is short key or not
 */
static bool is_short_key_released(u8 key, const key_state_t* key_s)
{
  event_handler_t* pos_ptr = NULL;
  u32  key_handler_time1 = 0;
  u32 time;

  list_for_each_entry(pos_ptr, &key_event[key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[key].list){
      if((pos_ptr->key_ac == SHORT_KEY)){
        key_handler_time1 = pos_ptr->time1;
        break;
      }
    }
  }

  time = key_handler_time1 ? key_handler_time1:SHORT_TIME;

  return key_s->pressing_time < time;
}

/**
 * @brief      This function serves to checkout if the key is long key
 * @param[in]  key    - the key will be checkout
 * @param[in]  key_s  - the key handler
 * @return the result if it is long key or not
 */
static u8 is_long_key(u8 key, const key_state_t* key_s)
{
  event_handler_t* pos_ptr = NULL;
  u32 key_handler_time1 = 0;
  u32 time;
  u8 total_cnt   = 0;
  u8 resovle_cnt = 0;

  list_for_each_entry(pos_ptr, &key_event[key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[key].list){
      if(pos_ptr->key_ac == LONG_KEY){
        key_handler_time1 = pos_ptr->time1;
        time = key_handler_time1 ? key_handler_time1:LONG_TIME;
        total_cnt++;
        if(key_s->pressing_time > time && long_key_time != time){
          resovle_cnt++;
          long_key_time = time;
        }
      }
    }
  }
  if(total_cnt == 1 && total_cnt == resovle_cnt)
    return LONG_KEY_FINISH | LONG_KEY_OCCURED;
  else if(total_cnt == resovle_cnt && total_cnt != 0)
    return LONG_KEY_FINISH;
  else if(resovle_cnt)
    return LONG_KEY_OCCURED;
  else
    return NO_LONG_KEY;
}

/**
 * @brief      This function serves to get the registered key handler
 * @param[in]  first_key   - the first key
 * @param[in]  second_key  - the second key
 * @param[in]  action      - the key handler
 * @return the key handler of the first key and second key
 */
static handler get_key_handler(u8 first_key, u8 second_key, key_action_t action)
{
  event_handler_t* pos_ptr = NULL;
  handler key_handler = NULL;

  list_for_each_entry(pos_ptr, &key_event[first_key].list, list){
    if(pos_ptr != (event_handler_t*)&key_event[first_key].list){
      if((pos_ptr->key_ac == action)){
        if(action == COMBIN_KEY || action  == COMBIN_KEY_IN_TIME || action == LEADER_KEY){
          if(pos_ptr->second_key == second_key){
            key_handler = pos_ptr->key_handler;
            break;
          }
        }else if(action == LONG_KEY){
          if(pos_ptr->time1 == long_key_time){
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

/**
 * @brief      This function serves to get the registered stuck key handler
 * @param[in]  none
 * @return     none
 */
static inline handler get_stuck_key_handler()
{
  return stuck_key_handler;
}

/**
 * @brief      This function serves to set the key which can wakeup the system
 * @param[in]  key - the key number
 * @return     none
 */
static inline void set_wakeup_key(u8 key)
{
  key_table.key[key].key_enable_sleep(key);
}

/**
 * @brief      This function serves to clear the key which can't wakeup the system
 * @param[in]  key - the key number
 * @return     none
 */
static inline void clr_wakeup_key(u8 key)
{
  key_table.key[key].key_disable_sleep(key);
}

/**
 * @brief      This function serves to set the key which wakeup the system
 * @param[in]  key - the key number
 * @return     none
 */
static void wakeup_system_key(u8 key)
{
  if(is_wakeup_from_sleep()){
    if(wakeup_key == 255)//if current key is pressing, reload system tick
      wakeup_key = key;
  }
}

/**
 * @brief      This function serves to get the key number from key bit value
 * @param[out]  bit - the key bit value
 * @param[out]  key - the key number
 * @return      none
 */
static bool get_key_from_bit(u32* bit, u8* key)
{
  u32  bit_indicate = 1;
  u32  tmp_itor = 0;
  bool has_key = 0;
  u8 real_key = 0;

  while(*bit){
    if(*bit & bit_indicate){
      *bit ^= bit_indicate;

      while(bit_indicate != tmp_itor){
        tmp_itor = 1 << real_key;
        real_key++;
      }
      real_key--;
      *key = real_key;
      has_key = 1;
      break;
    }
    bit_indicate <<= 1;
  }
  return has_key;
}

/**
 * @brief      This function serves to get the two key numbers from key bit value
 * @param[in]  bit - the key bit value
 * @param[out] key - the key number
 * @return     none
 */
static void two_bit2key(u32 bit, two_key_t* key)
{
  for(u8 i = 0; i < 32; i++){
    if(bit & (1 << i)){
      if(key->key1 == 255){
        key->key1 = i;
      }else{
        key->key2 = i;
        break;
      }
    }
  }
}

/**
 * @brief      This function serves to get the one key numbers from key bit value
 * @param[in]  bit - the key bit value
 * @param[out] key - the key number
 * @return     none
 */
static void one_bit2key(u32 bit, u8* key)
{
  for(u8 i = 0; i < 32; i++){
    if(bit & (1 << i)){
      *key = i;
      break;
    }
  }
}

/**
 * @brief      This function serves to scan matrix key
 * @param[in]  none
 * @return     none
 */
static void matrix_key_read()
{
  for(u8 i = 0; i < key_table.num; i++)
    key_read(&key_status[i], &key_table.key[i]);
}

/**
 * @brief      This function serves to read how many keys are pressing
 * @param[out] key_cnt - how many keys are pressing
 * @return the pressing key bit value
 */
static u32 matrix_key_read_pressing(u8* key_cnt)
{
  u32 key_bit = 0;

  for(u8 i = 0; i < key_table.num; i++){
    if(key_status[i].cur_status == PRESSING && !(key_status[i].flag & STUCK_KEY_FLAG)){
      key_bit |= (1 << i);
      (*key_cnt)++;
    }
  }
  if(!key_bit)
    *key_cnt = 0;
  return key_bit;
}

/**
 * @brief      This function serves to read how many keys are from pressing to releasing
 * @param[out] key_cnt - how many keys are from pressing to releasing
 * @return the releasing key bit value
 */
static u32 matrix_key_read_released(u8* key_cnt)
{
  u32 key_bit = 0;

  for(u8 i = 0; i < key_table.num; i++){
    if(key_status[i].cur_status == RELEASE && key_status[i].pressing_time && !(key_status[i].flag & STUCK_KEY_FLAG)){
      key_bit |= (1 << i);
      (*key_cnt)++;
    }
  }
  if(!key_bit)
    *key_cnt = 0;
  return key_bit;
}

/**
 * @brief      This function serves to read how many keys are from stucking to releasing
 * @param[in]  none
 * @return the releasing key bit value
 */
static u32 matrix_key_read_stuck_key_released()
{
  u32 key_stuck_bit = 0;

  for(u8 i = 0; i < key_table.num; i++){
    if(key_status[i].cur_status == RELEASE && key_status[i].flag & STUCK_KEY_FLAG)
      key_stuck_bit |= (1 << i);
  }
  return key_stuck_bit;
}

/**
 * @brief      This function serves to processing one key pressing twice when the key is pressing
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void one_key_twice_pressing_update(u32 bit)
{
  u8 key = 255;
  key_state_t* status;

  one_bit2key(bit, &key);

  status = get_key_status(key);

  //single key pressing
  //key from released to pressing && time window is not start, then start time windows
  //key from released to pressing && is other key is pressing, then start time windows
  if(is_key_from_release_to_pressing(status)){
    if(key_start_time_window == 255 || key_start_time_window != key){
      key_start_time_window = key;//which key starts the time windows
      pressed_tick = clock_time();
    }
    pressed_times++;
  }
}

/**
 * @brief      This function serves to processing one key pressing twice when the key is pressing
 * @param[in]  bit - the key bit value
 * @return     none
 */
static inline void one_key_twice_released_update()
{
  released_times = pressed_times;
}

/**
 * @brief      This function serves to processing one key pressing twice when the key is release
 * @param[in]  none
 * @return     none
 */
void one_key_twice_normal_update()
{
  u32 interval;

  //t twice s short
  //pressed_times  0  1 1 2 2
  //released_times 0  0 1 1 2
  //               x  x s x t

  if(key_start_time_window != 255){
    interval = (int)clock_time() - (int)pressed_tick;
    if(interval <= ONE_KEY_PRESSING_TWICE_INTERVAL){
      if(released_times == 2){
        set_key_action(key_start_time_window, ONE_KEY_TWICE);
        key_start_time_window = 255;//stop
        released_times = 0;
        pressed_times  = 0;
      }
    }else{//over ONE_KEY_PRESSING_TWICE_INTERVAL
      if(pressed_times == 1 && released_times == 1)
        set_key_action(key_start_time_window, ONE_KEY_TWICE_ONLY_ONCE);
      key_start_time_window = 255;//stop
      released_times = 0;
      pressed_times  = 0;
    }
  }
}

/**
 * @brief      This function serves to finish one key pressing twice when the key is entered two key processing
 * @param[in]  none
 * @return     none
 */
static void one_key_twice_finish()
{
  released_times = 0;
  pressed_times  = 0;
  key_start_time_window = 255;//stop
}

/**
 * @brief      This function serves to checkout whether the key is stuck
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void stuck_key_trigger_process(u32 bit)
{
  u8 key;
  key_state_t* status;

  while(bit){
    if(get_key_from_bit(&bit, &key)){
      status = get_key_status(key);
      if(is_key_pressing_exceed_time(status, STUCK_TIME)){
        clr_wakeup_key(key);
        set_key_action(key, STUCK_KEY);
        set_current_key_flag(status, STUCK_KEY_FLAG);
      }
    }
  }
}

/**
 * @brief      This function serves to clear key stuck status when stuck key is released
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void stuck_key_released_process(u32 bit)
{
  u8 key;

  while(bit){
    if(get_key_from_bit(&bit, &key))
      set_wakeup_key(key);
  }
}

/**
 * @brief      This function serves to handler when two keys are pressing
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void two_key_process(u32 bit)
{
  static bool setup_time_valid;
  two_key_t two_key = {255, 255};
  key_state_t* status1;
  key_state_t* status2;
  u32 st_time;
  u32 cmb_time;

  two_bit2key(bit, &two_key);

  if((first_key == two_key.key1 && second_key == two_key.key2) || (first_key == two_key.key2 && second_key == two_key.key1)){
    status1 = get_key_status(first_key);
    status2 = get_key_status(second_key);
    get_key_combin_setup_time_and_combin_time(first_key, &st_time, &cmb_time);
  }else{
    setup_time_valid = 0;

    if(first_key == two_key.key1)
      second_key = two_key.key2;
    else
      second_key = two_key.key1;

    status1 = get_key_status(first_key);
    status2 = get_key_status(second_key);

    get_key_combin_setup_time_and_combin_time(first_key, &st_time, &cmb_time);

    if(is_key_pressing_less_than_time(status1, st_time))//two key is pressing, and first key pressing time is less than st_time
      setup_time_valid = 1;

    if(is_key_pressing_less_than_time(status1, cmb_time)){//two key is pressing, and first key pressing time is less than cmb_time
      set_key_action(first_key,  COMBIN_KEY | FIRST_KEY_FLAG);
      set_key_action(second_key, COMBIN_KEY | SECOND_KEY_FLAG);
    }
  }

  if(setup_time_valid){
    if(is_key_pressing_exceed_time(status2, cmb_time)){//if pressing time is more than cmb_time
      setup_time_valid = 0;
      set_key_action(first_key,   COMBIN_KEY_IN_TIME | FIRST_KEY_FLAG );//bit31 used as firstkey flag
      set_key_action(second_key,  COMBIN_KEY_IN_TIME | SECOND_KEY_FLAG);//bit30 used as secondkey flag
    }
  }
}

/**
 * @brief      This function serves to wait leader key action processing
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void wait_leader_key_process(u32 bit)
{
  u8 key = 255;

  one_bit2key(bit, &key);

  first_key = key;
}

/**
 * @brief      This function serves to process leader key action
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void leader_key_process(u32 bit)
{
  two_key_t two_key = {255, 255};
  key_state_t* status1;
  key_state_t* status2;

  two_bit2key(bit, &two_key);

  if(first_key == two_key.key1)
    second_key = two_key.key2;
  else
    second_key = two_key.key1;

  status1 = get_key_status(first_key);
  status2 = get_key_status(second_key);

  if(!(get_current_key_flag(status2) & LEADER_KEY_FLAG)){
    set_key_action(first_key,  LEADER_KEY | FIRST_KEY_FLAG);
    set_key_action(second_key, LEADER_KEY | SECOND_KEY_FLAG);
    set_current_key_flag(status1, LEADER_KEY_FLAG);
    set_current_key_flag(status2, LEADER_KEY_FLAG);
  }
}

/**
 * @brief      This function serves to process when one key is pressing
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void one_key_process(u32 bit)
{
  key_state_t* status;
  u8 key = 255;
  u8 long_key_event;

  one_bit2key(bit, &key);

  status = get_key_status(key);

  if(is_key_from_release_to_pressing(status)){
    if(key != cur_key){
      wakeup_system_key(key);
      pre_key = cur_key;
      cur_key = key;
    }
    first_key = key;
    set_key_action(key, SHORT_KEY_IMMEDIATELY);
    long_key_time = 0;
  }

  if(get_pressing_time(status))
    set_key_action(key, NO_TIME_LIMIT_KEY_ON);

  if(!(get_current_key_flag(status) & LONG_KEY_FLAG)){
    long_key_event = is_long_key(key, status);

    if(long_key_event & LONG_KEY_FINISH)
      set_current_key_flag(status, LONG_KEY_FLAG);

    if(long_key_event & LONG_KEY_OCCURED)
      set_key_action(key, LONG_KEY);
  }
}

/**
 * @brief      This function serves to process when key is from pressing to releasing
 * @param[in]  bit - the key bit value
 * @return     none
 */
static void released_key_process(u32 bit)
{
  u8 key = 255;
  key_state_t* status;

  one_bit2key(bit, &key);

  status = get_key_status(key);

  set_key_action(key, NO_TIME_LIMIT_KEY_RELEASED);

  if(is_short_key_released(key, status))
    set_key_action(key, SHORT_KEY);
}

/**
 * @brief      This function serves to get the process status
 * @param[in]  none
 * @return the current key process status
 */
static inline process_status_t get_process_status()
{
  return process_status;
}

/**
 * @brief      This function serves to change the status with previous status and pressing key numbers
 * @param[in]  pre_status - the previous process status
 * @param[in]  key_num    - the current pressing key numbers
 * @return the adjusted current key process status
 */
static  process_status_t change_current_process(process_status_t pre_status, u8 key_num)
{
  switch(pre_status){
    case INITIAL_PROCESS:
      if(key_num == 1)
        process_status = ONE_KEY_PROCESS;
      break;
    case ONE_KEY_PROCESS:
      if(!key_num)
        process_status = ONE_KEY_RELEASED_PROCESS;
      else if(key_num == 2)
        process_status = TWO_KEY_PROCESS;
      break;
    case ONE_KEY_RELEASED_PROCESS:
      if(key_num == 1)
        process_status = ONE_KEY_PROCESS;
      else
        process_status = IDLE_PROCESS;
    case TWO_KEY_PROCESS:
      if(key_num == 1)
        process_status = WAIT_LEADER_KEY_PROCESS;
      else if(key_num != 2)
        process_status = WAIT_KEY_RELEASED_PROCESS;
      break;
    case WAIT_LEADER_KEY_PROCESS:
      if(key_num == 2)
        process_status = LEADER_KEY_PROCESS;
      else if(key_num != 1)
        process_status = WAIT_KEY_RELEASED_PROCESS;
      break;
    case LEADER_KEY_PROCESS:
      if(key_num == 1)
        process_status = WAIT_LEADER_KEY_PROCESS;
      else if(key_num != 2)
        process_status = WAIT_KEY_RELEASED_PROCESS;
      break;
    case WAIT_KEY_RELEASED_PROCESS:
      if(!key_num)
        process_status = IDLE_PROCESS;
      break;
    case IDLE_PROCESS:
      if(!key_num)
        process_status = INITIAL_PROCESS;
      else
        process_status = WAIT_KEY_RELEASED_PROCESS;
      break;
    default:break;
  }
  return process_status;
}

/**
 * @brief      This function serves to set high level key initial status
 * @param[in]  none
 * @return     none
 */
void key_init()
{
  u8 type = 0;
  u8 first_key_of_type = 0;
  u8 num_keys_of_type  = 0;

  if(key_table.key && key_table.num){
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
}

/**
 * @brief      This function serves to set initial status when system is wakeup
 * @param[in]  none
 * @return     none
 */
void key_wakeup_init()
{
  u8 type = 0;
  u8 first_key_of_type = 0;
  u8 num_keys_of_type  = 0;

  if(key_table.key && key_table.num){
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
   }
}

/**
 * @brief      This function serves to process high level key numbers and actions
 * @param[in]  data - if you want to process something, you should register a callback function when all key process finished.
 * @return the key numbers are processing
 */
int key_process(void* data)
{
  u32 pressing_key_bit;
  u32 released_key_bit;

  u8 pressing_key_num = 0;
  u8 released_key_num = 0;

  process_status_t pre_status;
  process_status_t cur_status;

  if(key_table.key && key_table.num){
    matrix_key_read();

    pressing_key_bit = matrix_key_read_pressing(&pressing_key_num);//key_cur_status:pressing
    released_key_bit = matrix_key_read_released(&released_key_num);//key_cur_status:released pressing_time valid

    stuck_key_trigger_process(pressing_key_bit);

    stuck_key_released_process(matrix_key_read_stuck_key_released());

    if(pressing_key_num)
      reload_sys_time();

    pre_status = get_process_status();

    cur_status = change_current_process(pre_status, pressing_key_num);

    one_key_twice_normal_update();

    if(cur_status == ONE_KEY_RELEASED_PROCESS){
      one_key_twice_released_update();
      released_key_process(released_key_bit);
    }else if(cur_status == ONE_KEY_PROCESS){
      one_key_twice_pressing_update(pressing_key_bit);
      one_key_process(pressing_key_bit);
    }else if(cur_status == TWO_KEY_PROCESS){
      one_key_twice_finish();
      two_key_process(pressing_key_bit);
    }else if(cur_status == WAIT_LEADER_KEY_PROCESS)
      wait_leader_key_process(pressing_key_bit);
    else if(cur_status == LEADER_KEY_PROCESS)
      leader_key_process(pressing_key_bit);
    else if(cur_status == IDLE_PROCESS){
      first_key  = 255;
      second_key = 255;
    }
  }
  return pressing_key_num || released_key_num;
}

/**
 * @brief      This function serves to register key callback which can be invoked when specified action and time are occured
 * @param[in]  first_key   - the first  key number
 * @param[in]  second_key  - the second key number
 * @param[in]  time1       - if action is short action or long action,the time1 specified pressing time
 * @param[in]  time2       - if action is combin action, the time1 is setup time, the time2 is combin time
 * @param[in]  action      - the key action which should be registerd to the key event structure
 * @param[in]  key_handler - the callback will be invoked when above statement are occured
 * @return     none
 */
void register_key_event(u8 first_key, u8 second_key, u32 time1, u32 time2, key_action_t action, handler key_handler)
{
  event_handler_t* key_hand;

  key_hand = (event_handler_t *)mempool_alloc(&KEY_EVENT_POOL);

  if(!key_hand)
    return;

  key_hand->second_key  = second_key;
  key_hand->key_ac      = action;
  key_hand->key_handler = key_handler;
  key_hand->time1       = time1;
  key_hand->time2       = time2;

  INIT_LIST_HEAD(&key_hand->list);

  g_list_add_tail(&key_hand->list, &key_event[first_key].list);
}

/**
 * @brief      This function serves to register low level key process function
 * @param[in]  key - the low level key process function array
 * @param[in]  num - the length of the key process funciton array
 * @return     none
 */
void register_key(const key_type_t* key, u8 num)
{
  key_table.key = key;
  key_table.num = num;
}

/**
 * @brief      This function serves to register the stuck key handler
 * @param[in]  stuck_handler - when stuck key is occured, the registered callback will be invoked
 * @return     none
 */
void set_stuck_key_handler(handler stuck_handler)
{
  stuck_key_handler = stuck_handler;
}

/**
 * @brief      This function serves to register something you want to do,when key action occured
 * @param[in]  normal_cb - when key action occured, the registered callback will be invoked
 * @return     none
 */
void register_normal_sys_event(handler normal_cb)
{
  normal_handler = normal_cb;
}

/**
 * @brief      This function serves to poll key event, if key action is occured,it will invoked registered key handler with relative key action
 * @param[in]  none
 * @return     none
 */
void poll_key_event()
{
  handler key_handler = NULL;
  key_action_t key_action;
  key_action_t action;
  key_action_t key;
  u8 first  = 255;
  u8 second = 255;

  for(u8 i = 0; i < key_table.num; i++){
    key_action = get_key_action(i);//get key status from key_event
    key = SHORT_KEY;

    while(key_action){
      if(key_action & COMBIN_KEY || key_action & COMBIN_KEY_IN_TIME || key_action & LEADER_KEY){
        action =  key_action & (COMBIN_KEY | COMBIN_KEY_IN_TIME | LEADER_KEY);
        if(key_action & FIRST_KEY_FLAG){//check if it is the first key
          first = i;
          key_action = 0;
        }else{
          second = i;
          key_action = 0;
        }

        if(first != 255 && second != 255){
          key_handler = get_key_handler(first, second, action);
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
          if(exit_peidui && action == SHORT_KEY){
            exit_peidui = 0;
          }else{
            key_handler = get_key_handler(i, 0, action);
            if(key_handler)
              key_handler();
            if(normal_handler)
              normal_handler();
          }
          key_action ^= key;
        }
        key <<= 1;
      }
    }
  }
}

/**
 * @brief      This function serves to exit peidui mode when first key and second key are pressing
 * @param[in]  first_key  - the first key number
 * @param[in]  second_key - the second key number
 * @return     none
 */
bool app_read_key(u8 first_key, u8 second_key)
{
  key_action_t key_action;
  key_action_t action;
  key_action_t key;
  key_action_t register_action1 = RELEASE_KEY;
  key_action_t register_action2 = RELEASE_KEY;
  u8 first  = 255;
  u8 second = 255;

  key_process(NULL);

  for(u8 i = 0; i < key_table.num; i++){
    key_action = get_key_action(i);//get key status from key_event
    key = SHORT_KEY;

    while(key_action){
      if(key_action & COMBIN_KEY || key_action & COMBIN_KEY_IN_TIME){
        action =  key_action & (COMBIN_KEY | COMBIN_KEY_IN_TIME);
        if(key_action & FIRST_KEY_FLAG){//check if it is the first key
          first = i;
          key_action = 0;
        }else{
          second = i;
          key_action = 0;
        }

        if(first != 255 && second != 255){

          clr_key_action(first,  0xffffffff);
          clr_key_action(second, 0xffffffff);

          get_register_key_comb_action(first_key, second_key, &register_action1);
          get_register_key_comb_action(second_key, first_key, &register_action2);

          if(register_action1 == action && register_action2 == action && first == second_key && second == first_key)
            return 1;

          if(register_action1 == action && first == first_key && second == second_key)
            return 1;
        }
      }else{
        action =  key_action & key;
        if(action == STUCK_KEY){
          key_action ^= key;
          clr_key_action(i, STUCK_KEY);
        }else if(action){//short key short immediately key long key process
          clr_key_action(i, action);
          key_action ^= key;
        }
        key <<= 1;
      }
    }
  }
  return 0;
}

/**
 * @brief      This function serves to exit peidui mode when first key is pressing
 * @param[in]  first_key  - the first key number
 * @param[in]  my_action  - the key action when exit peidui mode
 * @return     none
 */
bool app_read_single_key(u8 first_key, key_action_t my_action)
{
  key_action_t key_action;
  key_action_t action;
  key_action_t key;
  u8 first  = 255;
  u8 second = 255;

  key_process(NULL);

  for(u8 i = 0; i < key_table.num; i++){
    key_action = get_key_action(i);//get key status from key_event
    key = SHORT_KEY;

    while(key_action){
      if(key_action & COMBIN_KEY || key_action & COMBIN_KEY_IN_TIME){
        action =  key_action & (COMBIN_KEY | COMBIN_KEY_IN_TIME);
        if(key_action & FIRST_KEY_FLAG){//check if it is the first key
          first = i;
          key_action = 0;
        }else{
          second = i;
          key_action = 0;
        }

        if(first != 255 && second != 255){
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
          if(first_key == i && action == my_action)
            return 1;
        }
        key <<= 1;
      }
    }
  }
  return 0;
}
