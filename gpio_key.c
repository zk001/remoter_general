#if defined(GPIO_KEY)
#include "../../common.h"
#include "key.h"
#include "board.h"
#include "gpio_key.h"
#include "led.h"

static u32 debounce_time[MAX_GPIO_KEY];
static u8 gpio_first_key;
static u8 gpio_last_key;

_attribute_data_retention_ static struct {
  const key_map_t *map;
  u8 num;
}gpio_key_map;

//init
//row set output low
//col set input 1m pull_pu resistor low_level wakeup

//sleep
//row set output low 
//col set input 1m pull_up resistor low_level wakeup 

//scan
//set scan row output low other row key input
// if col is low_level ,the key is pressing 

void key_alloc(const key_map_t *key_arry, u8 num)
{
  gpio_key_map.map = key_arry;
  gpio_key_map.num = num;
}

void key_gpio_init(u8 first_key, u8 last_key)
{//init status,col input with deep sleep and row input
  if(gpio_key_map.map){
    for(u8 i = 0; i < gpio_key_map.num; i++){
      SET_ROW_GPIO_INPUT(gpio_key_map.map[i].row);
      SET_COL_GPIO_INPUT(gpio_key_map.map[i].col);
    }
  }

  gpio_first_key  = first_key;
  gpio_last_key   = last_key;
}

void key_gpio_sleep_init()
{
  if(gpio_key_map.map){
    for(u8 i = 0; i < gpio_key_map.num; i++){
      if(gpio_key_map.map[i].is_wake_up_pin == IS_WAKE_UP){
        SET_COL_GPIO_WITH_DEEPSLEEP(gpio_key_map.map[i].col);
        SET_ROW_GPIO_WITH_DEEPSLEEP(gpio_key_map.map[i].row);
      }
    }
  }
}

void key_gpio_for_scan(const key_map_t *key)
{//set row output low and col iput for scan
  SET_ROW_GPIO_OUTPUT_LOW(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

void key_gpio_for_no_scan(const key_map_t *key)
{//set row input and col input with deep sleep
  SET_ROW_GPIO_INPUT(key->row);
  SET_COL_GPIO_INPUT(key->col);
}

key_map_t* key_map(key_index_t key)
{
  if(key < gpio_first_key)
    return NULL;

  if(key > gpio_last_key)
    return NULL;

  return gpio_key_map.map != NULL? (key_map_t*)&gpio_key_map.map[key]:NULL;
}

bool read_col_gpio(u32 col)
{
  return gpio_read(col);
}

bool key_low_level_scan(const key_map_t *key)
{
  bool status;

  key_gpio_for_scan(key);

  status = read_col_gpio(key->col) ? 0:1;

  key_gpio_for_no_scan(key);

  return status;
}

void low_key_scan(key_status_t* key_s, key_index_t key)
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
    }else if((cur_time - time) >= DEBOUNCE_TIME){
      *key_s = PRESSING;
    }else
      *key_s = RELEASE;
  }else{
    debounce_time[key] = 0;
    *key_s = RELEASE;
  }
}
#endif
