#if defined(GPIO_KEY)
#include "../../common.h"
#include "key.h"
#include "board.h"
#include "gpio_key.h"

//init
//row set input with 1m pull_up resistor
//col set input with 1m pull_pu resistor

//sleep
//row set 10k pull_down resistor
//col set 1m pull_up resistor low_level wakeup 

//scan
//set scan row output low other row key input
//col input 1m pull_down resistor
// if col is low_level ,the key is pressing 

static u32 debounce_time[MAX_GPIO_KEY];
_attribute_data_retention_ static u8 gpio_first_key;
_attribute_data_retention_ static u8 gpio_last_key;

_attribute_data_retention_ static struct {
  key_map_t *map;
  u8 num;
}gpio_key_map;

static void gpio_key_for_scan(const key_map_t *key)
{
  SET_ROW_GPIO_OUTPUT_LOW(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

static void gpio_key_for_no_scan(const key_map_t *key)
{
  SET_ROW_GPIO_INPUT(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

static key_map_t* key_map(key_index_t key)
{
  if((key < gpio_first_key) || (key > gpio_last_key))
    return NULL;

  return gpio_key_map.map != NULL? (key_map_t*)&gpio_key_map.map[key]:NULL;
}

static inline bool read_col_gpio(u32 col)
{
  return gpio_read(col);
}

bool key_low_level_scan(const key_map_t *key)
{
  bool status;

  gpio_key_for_scan(key);

  status = read_col_gpio(key->col) ? 0:1;

  gpio_key_for_no_scan(key);

  return status;
}

void gpio_key_alloc(key_map_t *key_arry, u8 num)
{
  gpio_key_map.map = key_arry;
  gpio_key_map.num = num;
}

void gpio_key_init(u8 first_key, u8 last_key)
{
  if(gpio_key_map.map){
    for(u8 i = 0; i < gpio_key_map.num; i++){
      SET_ROW_GPIO_INPUT(gpio_key_map.map[i].row);
      SET_COL_GPIO_INPUT(gpio_key_map.map[i].col);
    }
  }

  gpio_first_key  = first_key;
  gpio_last_key   = last_key;
}

void gpio_key_sleep_setup()
{
  if(gpio_key_map.map){
    for(u8 i = 0; i < gpio_key_map.num; i++){
      if(gpio_key_map.map[i].is_wake_up_pin == IS_WAKE_UP){
        SET_COL_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].col);
        SET_ROW_GPIO_WITH_DEEPSLEEP_LOW_WAKEUP(gpio_key_map.map[i].row);
      }
    }
  }
}

void gpio_key_sleep_unset(u8 key)
{
  u32 col;

  if((key < gpio_first_key) || (key > gpio_last_key))
    return;

  if(gpio_key_map.map){
    gpio_key_map.map[key].is_wake_up_pin = NO_WAKE_UP;

    col = gpio_key_map.map[key].col;

    for(u8 i = 0; i < gpio_key_map.num; i++){
      if((gpio_key_map.map[i].col == col) && (i != key))
        gpio_key_map.map[i].is_wake_up_pin = NO_WAKE_UP;
    }
  }
}

void gpio_stuck_key_low_scan(key_status_t* key_s, key_index_t key)
{
  key_map_t *key_row_col;

  key_row_col = key_map(key);

  *key_s = (key_low_level_scan(key_row_col))? PRESSING:RELEASE;
}

void gpio_key_low_scan(key_status_t* key_s, key_index_t key)
{
  u32 time;
  u32 cur_time;
  key_map_t *key_row_col;

  time = debounce_time[key];

  key_row_col = key_map(key);

  if(!key_row_col)
    return;

  cur_time = clock_time();

  if(key_low_level_scan(key_row_col)){
    if(!time){
      debounce_time[key] = clock_time();
      *key_s = RELEASE;
    }else if(((u32)((int)cur_time - (int)time)) >= DEBOUNCE_TIME){
      *key_s = PRESSING;
    }else
      *key_s = RELEASE;
  }else{
    debounce_time[key] = 0;
    *key_s = RELEASE;
  }
}

#endif
