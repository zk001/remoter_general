#if (defined LOW_POWER_DETECT)
#include "../drivers.h"
#include "low_power.h"
#include "board.h"

u32 sample_result[32];

void vbat_init()
{
  adc_init();
  adc_vbat_init(ADC_INPUT_PIN);
  adc_power_on_sar_adc(1);
}

inline u32 vbat_get_result()
{
  return adc_sample_and_get_result();
}

bool is_low_bat()
{
  u32 sum = 0;
  u32 avg;
  u32 len = sizeof(sample_result)/sizeof(sample_result[0]);

  for(u8 i = 0; i < len; i++){
    sample_result[i] = vbat_get_result();
  }

  for(u8 i = 0; i < len; i++){
    sum += sample_result[i];
  }

  avg = sum / len;

  return (avg < THRESHOLD);
}
#endif
