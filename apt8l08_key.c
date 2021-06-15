#if defined(APT8_KEY)
#include "led.h"
#include "apt8l08_key.h"
#include "main.h"
#include "../drivers.h"
#include "gpio_key.h"
#include "n_timer.h"

//static const u8 APTT8L16ArrySensing[8]= {0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04};
//static const u8 APTTouchRegAdd[14]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d};
//static const u8 APTTouchRegDat[14]={0x04,0x51,0x20,0x00,0x00,0x00,0x08,0x02,0x02,0x10,0xff,0x04,0x00,0x08};
static const u8 APTT8L16ArrySensing[8]= {0x03,0x04,0x04,0x03,0x03,0x04,0x04,0x04};
static const u8 APTTouchRegAdd[14]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d};
static const u8 APTTouchRegDat[14]={0x03,0x50,0x20,0x00,0x00,0x00,0x08,0x02,0x02,0x10,0x10,0x04,0x00,0x00};

_attribute_data_retention_ static u8 apt8_first_key;
_attribute_data_retention_ static u8 apt8_last_key;

//write one byte data to apt8 register
static void apt8_set_reg(u8 addr, u8 data)
{
  u8 rd_data; 
  u8 cnt = 10;

  do{
    i2c_write_series(addr, 1, (unsigned char *)&data, 1);
    WaitUs(5);
    i2c_read_series(addr, 1, (unsigned char *)&rd_data, 1);
    if(rd_data != data){
      WaitMs(20);
      cnt--;
    }
  }while((rd_data != data) && cnt);
}

//apt8 enter config mode
static void apt8_set_cfg()
{
  apt8_set_reg(SYS_CON, 0x5a);
}

//apt8 enter active mode
static void apt_set_active()
{
  apt8_set_reg(SYS_CON, 0);
}

static key_index_t touch_key_map(key_index_t key)
{
  if(key < apt8_first_key)
    return apt8_first_key;

  if(key > apt8_last_key)
    return apt8_last_key;

  return key - apt8_first_key;
}

void apt_enter_sleep()
{
  i2c_gpio_set(I2C_GPIO_GROUP_B6D7);  	//SDA/CK : B6/D7

  i2c_master_init(APT8_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*200000)) );

  apt8_set_cfg();
}

void apt_exit_sleep()
{
  i2c_gpio_set(I2C_GPIO_GROUP_B6D7);  	//SDA/CK : B6/D7

  i2c_master_init(APT8_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*200000)) );

  apt_set_active();
}

void apt8_init(u8 first_key, u8 last_key)
{
  i2c_gpio_set(I2C_GPIO_GROUP_B6D7);  	//SDA/CK : B6/D7

  i2c_master_init(APT8_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*200000)) );

  WaitMs(5);

  apt8_set_cfg();

  WaitMs(5);

  apt_set_active();

  WaitMs(5);

  apt8_set_cfg();

  for(u8 i = 0; i < 14; i++)
    apt8_set_reg(APTTouchRegAdd[i], APTTouchRegDat[i]);

  for(u8 i = 0; i < 8; i++)
    apt8_set_reg(i, APTT8L16ArrySensing[i]);

  apt_set_active();

  WaitMs(300);

  apt8_first_key = first_key;
  apt8_last_key  = last_key;
}

void apt8_reset()
{
  i2c_gpio_set(I2C_GPIO_GROUP_B6D7);  	//SDA/CK : B6/D7

  i2c_master_init(APT8_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*200000)) );

  apt8_set_cfg();

  WaitMs(5);

  apt_set_active();

  WaitMs(5);
}

//read key value
void apt8_read(key_status_t* key_s, key_index_t key)
{
  u8 rd_data;
  u8 touch_key;

  i2c_gpio_set(I2C_GPIO_GROUP_B6D7);  	//SDA/CK : B6/D7

  i2c_master_init(APT8_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*200000)) );

  i2c_read_series(KVR0, 1, (unsigned char *)&rd_data, 1);

  touch_key = touch_key_map(key);

  *key_s = (rd_data & (1 << touch_key)) ? PRESSING:RELEASE;
}

void touch_key_sleep_unset(u8 key)
{

}

#endif
