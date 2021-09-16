#if (defined LOW_POWER_DETECT)
#include "../drivers.h"
#include "low_power.h"
#include "board.h"
#include "app.h"
#include "n_timer.h"

u32 sample_result[32];
u32 low_bat_process_time;
u32 low_bat_start_time;
low_bat_finish* low_bat_finish_cb;
bool  low_bat_warnning;
u32  power_threshold;

bool is_low_power(u32 threshold)
{
  u32 sum = 0;
  u32 avg;
  u8 len = sizeof(sample_result)/sizeof(sample_result[0]);

  adc_init();
  adc_vbat_init(ADC_INPUT_PIN);
  adc_power_on_sar_adc(1);

  for(u8 i = 0; i < len; i++)
    sample_result[i] = adc_sample_and_get_result();

  for(u8 i = 0; i < len; i++)
    sum += sample_result[i];

  avg = sum/len;

  return (avg < threshold);
}

bool low_bat_chk(u32 threshold, low_bat_warn* cb_warn, low_bat_finish* cb_finsish, u32 low_bat_time)
{
  if(is_low_power(threshold)){
    if(cb_warn)
      cb_warn();
    if(cb_finsish)
      low_bat_finish_cb = cb_finsish;
    low_bat_warnning = 1;
    power_threshold = threshold;
    low_bat_process_time = low_bat_time;
    low_bat_start_time = clock_time();
    return 1;
  }
  return 0;
}

void low_bat_update()
{
  if(low_bat_start_time){
    if(n_clock_time_exceed(low_bat_start_time, low_bat_process_time)){
      low_bat_start_time = 0;
      low_bat_warnning = 0;
      if(low_bat_finish_cb)
        low_bat_finish_cb();
    }
  }
}

bool is_bat_warn()
{
  return low_bat_warnning;
}

void clr_bat_warn()
{
  low_bat_warnning = 0;
}

u32 low_power_threshold()
{
  return power_threshold;
}
#endif
