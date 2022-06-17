#include "common.h"
#include "gear.h"
#include "flash_lock.h"

#define IS_STORE_FLASH_ADDR 0x025000

_attribute_data_retention_ gear_t* base_value;
_attribute_data_retention_ u8 num_base_value;
_attribute_data_retention_ u8 num_region;
_attribute_data_retention_ u8* gear_arry;
_attribute_data_retention_ u32 gear_flash_addr;
u32 is_store;

static gear_t* get_user_data (u8 user_id, char* name)
{
  gear_t* value = NULL;
  char* local_name;

  if (user_id > num_region - 1 || !name)
    return NULL;

  for (u8 i = 0; i < num_base_value; i++) {
	local_name = base_value[user_id * num_base_value + i].name;
    if (strcmp ((char*)name, (char*)local_name) == 0)
      value = &base_value[user_id * num_base_value + i];
  }
  return value;
}

static u8 get_max_gear (u8 user_id, char* name)
{
  u8 value = 0;
  char* local_name;

  if (user_id > num_region - 1 || !name)
    return 0;

  for (u8 i = 0; i < num_base_value; i++) {
	local_name = base_value[user_id * num_base_value + i].name;
    if (strcmp ((char*)name, (char*)local_name) == 0)
      value = base_value[user_id * num_base_value + i].max_value;
  }
  return value;
}

static u8 get_min_gear (u8 user_id, char* name)
{
  u8 value = 0;
  char* local_name;

  if (user_id > num_region - 1 || !name)
    return 0;

  for (u8 i = 0; i < num_base_value; i++) {
	local_name = base_value[user_id * num_base_value + i].name;
    if (strcmp ((char*)name, (char*)local_name) == 0)
      value = base_value[user_id * num_base_value + i].min_value;
  }
  return value;
}

static void store_gear2flash ()
{
  if (num_base_value && num_region && gear_arry && gear_flash_addr) {
    for (u8 i = 0; i < num_region; i++) {
	  for (u8 j = 0; j < num_base_value; j++)
	    gear_arry[i*num_base_value + j] = base_value[i*num_base_value + j].value;
	}

    general_flash_unlock ();

    flash_erase_sector(gear_flash_addr);

    flash_write_page(gear_flash_addr, num_base_value*num_region, (u8 *)gear_arry);

    general_flash_lock ();
  }
}

static void check_flash_data_valid ()
{
  for (u8 i = 0; i < num_region; i++) {
	for (u8 j = 0; j < num_base_value; j++) {
	  if (gear_arry[i*num_base_value + j] > base_value[i*num_base_value + j].max_value)
	    gear_arry[i*num_base_value + j] = base_value[i*num_base_value + j].max_value;
	  if (gear_arry[i*num_base_value + j] < base_value[i*num_base_value + j].min_value)
	    gear_arry[i*num_base_value + j] = base_value[i*num_base_value + j].min_value;
	  base_value[i*num_base_value + j].value = gear_arry[i*num_base_value + j];
	}
  }
}

void check_sram_data_valid (u8 user_id, char* name, u8 max_value, u8 min_value)
{
  gear_t* user_value = NULL;

  user_value = get_user_data (user_id, name);

  if (!user_value)
    return;

  if (user_value->max_value > max_value)
    user_value->max_value = max_value;
  if (user_value->min_value < min_value)
    user_value->min_value = min_value;

  if (user_value->value > max_value)
    user_value->value = max_value;
  if (user_value->value < min_value)
    user_value->value = min_value;
}

void register_gear (unsigned long flash_addr, u8* flash_buf, gear_t* user_value, u8 region, u8 num)
{
  if (user_value && num) {
    base_value = user_value;
	num_base_value = num;
	num_region = region;
  }

  general_flash_unlock ();

  if (flash_addr && flash_buf) {
	gear_arry = flash_buf;
	gear_flash_addr = flash_addr;

    flash_read_page (IS_STORE_FLASH_ADDR, 4, (u8*)&is_store);

    if (is_store == 0xffffffff) {
	  flash_write_page (gear_flash_addr, num_region*num_base_value, gear_arry);

	  is_store = 0x12345678;
	  flash_write_page (IS_STORE_FLASH_ADDR, 4, (u8*)&is_store);
    } else {
	  flash_read_page (gear_flash_addr, num_region*num_base_value, (u8*)gear_arry);

	  check_flash_data_valid ();
    }
  }

  general_flash_lock ();
}

void update_gear (u8 user_id, char* name, up_or_down_t direction)
{
  gear_t* cur_user_value = NULL;
  u8 max_gear;
  u8 min_gear;

  max_gear = get_max_gear (user_id, name);
  min_gear = get_min_gear (user_id, name);

  cur_user_value = get_user_data (user_id, name);

  if (!cur_user_value)
    return;

  if (direction == GEAR_UP || direction == GEAR_DOWN) {
    if (direction == GEAR_UP && cur_user_value->value != max_gear)
      cur_user_value->value++;
    else if (direction == GEAR_DOWN && cur_user_value->value != min_gear)
      cur_user_value->value--;
  } else {
    if (cur_user_value->method == BACK) {
	  if (max_gear == cur_user_value->value)
	    cur_user_value->direction = GEAR_DOWN;
	  else if (min_gear == cur_user_value->value)
	    cur_user_value->direction = GEAR_UP;

      if (cur_user_value->direction == GEAR_UP)
        cur_user_value->value++;
      else
        cur_user_value->value--;

	  if (max_gear == cur_user_value->value)
	    cur_user_value->direction = GEAR_DOWN;
	  else if (min_gear == cur_user_value->value)
	    cur_user_value->direction = GEAR_UP;
    } else if (cur_user_value->method == RESET) {
      if (cur_user_value->value == max_gear)
        cur_user_value->value = min_gear;
	  else
	    cur_user_value->value++;
    }
  }
  store_gear2flash ();
}

u8 get_gear (u8 user_id, char* name)
{
  gear_t* user_value = NULL;

  user_value = get_user_data (user_id, name);

  if (user_value)
    return user_value->value;
  else
	return 0;
}

u8 get_gear_direction (u8 user_id, char* name)
{
  gear_t* user_value = NULL;

  user_value = get_user_data (user_id, name);

  if (!user_value)
	return 0;

  return user_value->direction;
}

void cpy_user_data (u8 is_store_flash, u8 user_id_dst, u8 user_id_src, char* name)
{
  gear_t* user_value_dst = NULL;
  gear_t* user_value_src = NULL;

  user_value_dst = get_user_data (user_id_dst, name);
  user_value_src = get_user_data (user_id_src, name);

  if (!user_value_dst && !user_value_src)
	return;

  user_value_dst->value = user_value_src->value;

  if (is_store_flash)
    store_gear2flash ();
}

void cpy_user_data_with_direction (u8 is_store_flash, u8 user_id_dst, u8 user_id_src, char* name)
{
  gear_t* user_value_dst = NULL;
  gear_t* user_value_src = NULL;

  user_value_dst = get_user_data (user_id_dst, name);
  user_value_src = get_user_data (user_id_src, name);

  if (!user_value_dst && !user_value_src)
	return;

  user_value_dst->value     = user_value_src->value;
  user_value_dst->direction = user_value_src->direction;

  if (is_store_flash)
	store_gear2flash ();
}
