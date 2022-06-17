#ifndef __I2C_PROTOCOL_H__
#define __I2C_PROTOCOL_H__

//sample in scl high period, change in scl low period
#include "board.h"
//#include "header.h"
#if (defined(SIM_I2C))
#if (defined(APT8L08_KEY))
#define SDA_LOW(SDA)   st(gpio_set_output_en(SDA, 1);)
#define SDA_HIGH(SDA)  st(gpio_set_output_en(SDA, 0);)
#define SCL_HIGH(SCL)  st(gpio_set_output_en(SCL, 0);)
#define SCL_LOW(SCL)   st(gpio_set_output_en(SCL, 1);)
#elif (defined(AW9523_LED))
#define SDA_LOW(SDA)   st(gpio_write(SDA, 0);)
#define SDA_HIGH(SDA)  st(gpio_write(SDA, 1);)
#define SCL_HIGH(SCL)  st(gpio_write(SCL, 1);)
#define SCL_LOW(SCL)   st(gpio_write(SCL, 0);)
#endif

#define SDA_LOW(SDA)   st(gpio_write(SDA, 0);)
#define SDA_HIGH(SDA)  st(gpio_write(SDA, 1);)
#define SCL_HIGH(SCL)  st(gpio_write(SCL, 1);)
#define SCL_LOW(SCL)   st(gpio_write(SCL, 0);)

#define SET_SDA_AS_GPIO(SDA) st(gpio_set_func(SDA, AS_GPIO);)
#define SET_SCL_AS_GPIO(SCL) st(gpio_set_func(SCL, AS_GPIO);)
#define SDA_PIN_INPUT(SDA)  st(gpio_set_output_en(SDA, 0);gpio_set_input_en(SDA, 1);)
#define SDA_PIN_OUTPUT(SDA) st(gpio_set_output_en(SDA, 1);gpio_set_input_en(SDA, 0);)
#define SCL_PIN_OUTPUT(SCL) st(gpio_set_output_en(SCL, 1);gpio_set_input_en(SCL, 0);)
#define READ_SDA_STATUS(SDA) (gpio_read(SDA))

u32 SDA;
u32 SCL;

static void i2c_port_init (u32 sda, u32 scl)
{
  SDA = sda;
  SCL = scl;

  SET_SDA_AS_GPIO(SDA);
  SET_SCL_AS_GPIO(SCL);

  SDA_PIN_OUTPUT(SDA);
  SCL_PIN_OUTPUT(SCL);

  SDA_HIGH(SDA);
  SCL_HIGH(SCL);
}//SDA HIGH SCL HIGH IDLE

static void start (void)//THE STATUS OF SDA IS HIGH AND SCL IS HIGH WHEN ENTER THIS FUN
{
  WaitUs (5);
  SDA_HIGH(SDA);
  WaitUs (5);
  SCL_HIGH(SCL);
  WaitUs (5);
  SDA_LOW(SDA);
  WaitUs (5);
  SCL_LOW(SCL);
  WaitUs (5);
}//SDA LOW SCL LOW

static void stop (void)//THE STATUS OF SCL IS LOW WHEN ENTER THIS FUN
{
  WaitUs (5);
  SDA_LOW(SDA);
  WaitUs (5);
  SCL_HIGH(SCL);
  WaitUs (5);
  SDA_HIGH(SDA);
  WaitUs (5);
}//SDA HIGH SCL HIGH

static void wait_ack (void)//THE STATUS OF SCL IS LOW WHEN ENTER THIS FUN
{
  u8 cnt = 10;

  SDA_PIN_INPUT(SDA);
  WaitUs (5);
  SCL_HIGH(SCL);

  do {
    WaitUs (5);
    cnt--;
  } while ((READ_SDA_STATUS(SDA)) && cnt);//O FOR ACK

  WaitUs (5);
  SCL_LOW(SCL);
  SDA_PIN_OUTPUT(SDA);
}//SDA ? SCL LOW

static void ack (void)//THE STATUS OF SCL IS LOW WHEN ENTER THIS FUN
{
  WaitUs (5);
  SDA_LOW(SDA);
  WaitUs (5);
  SCL_HIGH(SCL);
  WaitUs (5);
  SCL_LOW(SCL);
  WaitUs (5);
}//SDA LOW SCL LOW

static void no_ack (void)
{
  WaitUs (5);
  SDA_HIGH(SDA);
  WaitUs (5);
  SCL_HIGH(SCL);
  WaitUs (5);
  SCL_LOW(SCL);
  WaitUs (5);
}//SDA HIGH SCL LOW

static void write_data (u8 data)//THE STATUS OF SCL IS LOW WHEN ENTER THIS FUN
{
  WaitUs (5);

  for (u8 i = 0; i < 8; i++) {
    if (data & 0x80)
      SDA_HIGH(SDA);
    else
      SDA_LOW(SDA);
    WaitUs (5);
    SCL_HIGH(SCL);
    WaitUs (5);
    SCL_LOW(SCL);
    data <<= 1;
  }
}//SDA ? SCL LOW

static u8 read_data (void)//THE STATUS OF SCL IS LOW WHEN ENTER THIS FUN
{
  u8 data = 0;
  SDA_PIN_INPUT(SDA);

  WaitUs (5);

  for (u8 i = 0; i < 8; i++) {
    data <<= 1;
    SCL_HIGH(SCL);
    WaitUs (5);
    if (READ_SDA_STATUS(SDA))
      data |= 0x01;
    SCL_LOW(SCL);
    WaitUs (5);
  }

  SDA_PIN_OUTPUT(SDA);

  return data;
}//SDA ? SCL LOW

static void write_one_data (u8 address, u8 reg, u8 data)
{
  star t();
  write_data (address);
  wait_ack ();
  write_data (reg);
  wait_ack ();
  write_data (data);
  wait_ack ();
  stop ();
}

static u8 read_one_data (u8 address, u8 reg)
{
  u8 data;

  start ();
  write_data (address);
  wait_ack ();
  write_data (reg);
  wait_ack ();
  start ();
  write_data (address|0x01);
  wait_ack ();
  data = read_data();
  no_ack ();
  stop ();
  return data;
}
#endif
#endif
