#include "../common.h"
#include "wakeup.h"
#include "app.h"

_attribute_data_retention_ static bool wake_up;

bool is_wakeup_from_sleep()
{
  return wake_up;
}

void clr_wakeup_flag()
{
  wake_up = 0;
}

void set_wakeup_flag()
{
  wake_up = 1;
}
