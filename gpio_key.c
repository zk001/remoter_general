#if defined(GPIO_KEY)
#include "../../common.h"
#include "n_timer.h"
#include "key.h"
#include "board.h"
#include "gpio_key.h"
#include "app.h"

//init
//row set input with 1m pull_up resistor
//col set input with 1m pull_pu resistor

//sleep
//row set 10k pull_down resistor
//col set 1m pull_up resistor low_level wakeup 

//scan
//set scan row output low other row key input
//col input 1m pull_down resistor
//if col is low_level ,the key is pressing

static u32 debounce_time[MAX_GPIO_KEYS];
_attribute_data_retention_ static u8 gpio_first_key;
_attribute_data_retention_ static u8 gpio_last_key;

_attribute_data_retention_ static struct {
  key_map_t *map;
  u8 num;
}gpio_key_map;

static void local_gpio_key(u8 key, u8* local_key)
{
  if(key < gpio_first_key)
    *local_key = 0;
  else if(key > gpio_last_key)
    *local_key = gpio_last_key - gpio_first_key;
  else
    *local_key = key - gpio_first_key;
}

static inline bool is_last_local_key(u8 key)
{
  return key == gpio_last_key - gpio_first_key;
}

static void gpio_key_for_scan(const key_map_t* key)
{
  SET_ROW_GPIO_OUTPUT_LOW(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

static void gpio_key_for_no_scan(const key_map_t* key)
{
  SET_ROW_GPIO_INPUT(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

static inline key_map_t* key_map(u8 key)
{
  return (key_map_t*)&gpio_key_map.map[key];
}

static inline bool read_col_gpio(u32 col)
{
  return gpio_read(col);
}

bool key_low_level_scan(const key_map_t* key)
{
  bool status;

  gpio_key_for_scan(key);

  status = read_col_gpio(key->col) ? 0:1;

  gpio_key_for_no_scan(key);

  return status;
}

void gpio_key_alloc(key_map_t* key_arry, u8 num)
{
  gpio_key_map.map = key_arry;
  gpio_key_map.num = num;
}

void gpio_key_init(u8 first_key, u8 last_key)
{
  if(gpio_key_map.map && gpio_key_map.num){
    for(u8 i = 0; i < gpio_key_map.num; i++){
      SET_ROW_GPIO_INPUT(gpio_key_map.map[i].row);
      SET_COL_GPIO_INPUT(gpio_key_map.map[i].col);
    }
    gpio_first_key  = first_key;
    gpio_last_key   = last_key;
  }
}

void gpio_key_enable_sleep(u8 key)
{
  u8 local_key;

  if(gpio_key_map.map && gpio_key_map.num){
    local_gpio_key(key, &local_key);
    gpio_key_map.map[local_key].is_wake_up_pin = IS_WAKE_UP;
  }
}

void gpio_key_disable_sleep(u8 key)
{
  u32 col;
  u8 local_key;

  if(gpio_key_map.map && gpio_key_map.num){
    local_gpio_key(key, &local_key);

    gpio_key_map.map[local_key].is_wake_up_pin = NO_WAKE_UP;

    col = gpio_key_map.map[local_key].col;

    for(u8 i = 0; i < gpio_key_map.num; i++){
      if(gpio_key_map.map[i].col == col && i != local_key)
        gpio_key_map.map[i].is_wake_up_pin = NO_WAKE_UP;
    }
  }
}

void gpio_key_sleep_setup()
{
  if(gpio_key_map.map && gpio_key_map.num){
    for(u8 i = 0; i < gpio_key_map.num; i++){
      if(gpio_key_map.map[i].is_wake_up_pin == NO_WAKE_UP)
        SET_ROW_GPIO_WITH_DEEPSLEEP_HIGH(gpio_key_map.map[i].row);
    }

    for(u8 i = 0; i < gpio_key_map.num; i++){
      if(gpio_key_map.map[i].is_wake_up_pin == IS_WAKE_UP){
        SET_COL_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].col);
        SET_ROW_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].row);
      }
    }
  }
}

void gpio_key_low_scan(key_status_t* key_s, key_index_t key)//key scan rate too low
{
  static bool wakeup_fast_scan = 1;
  u32 time;
  u32 cur_time;
  key_map_t* key_row_col;
  u8 local_key;

  if(gpio_key_map.map && gpio_key_map.num){
    local_gpio_key(key, &local_key);

    key_row_col = key_map(local_key);

    time = debounce_time[local_key];
    cur_time = clock_time();

    if(wakeup_fast_scan){
      if(key_low_level_scan(key_row_col)){
        if(!time)
          debounce_time[local_key] = clock_time();
        *key_s = PRESSING;
      }else{
        debounce_time[local_key] = 0;
        *key_s = RELEASE;
      }
      if(is_last_local_key(local_key)){
        wakeup_fast_scan = 0;
        WaitMs(20);//wait for debounce scan
      }
    }else{
      if(key_low_level_scan(key_row_col)){
        if(!time){
          debounce_time[local_key] = clock_time();
          *key_s = RELEASE;
        }else if(((u32)((int)cur_time - (int)time)) >= DEBOUNCE_TIME)
          *key_s = PRESSING;
        else
          *key_s = RELEASE;
      }else{
        debounce_time[local_key] = 0;
        *key_s = RELEASE;
      }
    }
  }
}

#endif
