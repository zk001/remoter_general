#include "common.h"
#include "gear.h"

_attribute_data_retention_ gear_t* base_value;
_attribute_data_retention_ u8 num_base_value;
_attribute_data_retention_ u8 num_region;

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
	local_name = base_value[user_id * num_region + i].name;
    if (strcmp ((char*)name, (char*)local_name) == 0)
      value = base_value[user_id * num_region + i].min_value;
  }
  return value;
}

void register_gear (gear_t* user_value, u8 region, u8 num)
{
  if (user_value && num) {
    base_value = user_value;
    num_base_value = num;
    num_region = region;
  }
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
    return;
  }

  if (cur_user_value->method == BACK) {
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

u8 get_gear (u8 user_id, char* name)
{
  gear_t* user_value = NULL;

  user_value = get_user_data (user_id, name);

  if (user_value)
    return user_value->value;
  else
	return 0;
}
