#if defined(AW9523_LED)
#include "aw9523_led.h"
#include "main.h"
#include "led.h"

//led
//should we set gpio direction???
//shuld we set gcr register???
//P1 ports is output mode as default when power on and P0 ports is high-z

static const u8 aw9523_led_table[] = {
  PORT0_BIT_0,PORT0_BIT_1,PORT0_BIT_2,PORT1_BIT_2,PORT1_BIT_3,
  PORT0_BIT_3,PORT0_BIT_4,PORT1_BIT_1,PORT0_BIT_5,PORT1_BIT_7,
  PORT1_BIT_5,PORT1_BIT_6,PORT1_BIT_0,PORT1_BIT_4,PORT0_BIT_6,
  PORT0_BIT_7
};

static void aw9523_set_conf(u8 conf)
{
  i2c_write_byte(GCR, 1, conf);
}

static void aw9523_set_led_mode(u8 aw_port)
{
  u8 port;

  port = aw_port ? P1_MOD_SWITCH:P0_MOD_SWITCH;

  i2c_write_byte(port, 1, 0);
}

void aw9523_set_gpio_mode(u8 aw_port)
{
  u8 port;

  port = aw_port ? P1_MOD_SWITCH:P0_MOD_SWITCH;

  i2c_write_byte(port, 1, 0xff);
}

void aw9523_init()
{
  //  u8 id;

  i2c_gpio_set(AW9523_I2C_PORT);  	//SDA/CK : C0/C1
  i2c_master_init(AW9523_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*200000)) );

  RSTN_OUT_LOW();

  WaitUs(100);

  RSTN_OUT_HIGH(); 

  WaitMs(5);

  i2c_write_byte(SOFT_RESET, 1, 0x00);

  WaitMs(1);

  //  id = i2c_read_byte(0x03, 1);

  aw9523_set_conf(0x13);//P0 push_pull mode,led current = max/4

  aw9523_set_led_mode(0);//port0 as led mode

  aw9523_set_led_mode(1);//port1 as led mode
}

static void aw9523_led_on(u8 port_bit)
{
  aw9523_set_led_dim(port_bit, LED_BRIGHT_LEVEL);
}

static void aw9523_other_led_on(u8 port_bit)
{
  aw9523_set_led_dim(port_bit, 20);
  //	  aw9523_set_led_dim(port_bit, LED_BRIGHT_LEVEL);

}

static void aw9523_led_off(u8 port_bit)
{
  aw9523_set_led_dim(port_bit, 0);
}

void aw9523_set_led_dim(u8 port_bit, u8 level)
{
  u8 port;

  switch(port_bit){
    case PORT0_BIT_0: port = STEP_DIM_SET4;break;
    case PORT0_BIT_1: port = STEP_DIM_SET5;break;
    case PORT0_BIT_2: port = STEP_DIM_SET6;break;
    case PORT0_BIT_3: port = STEP_DIM_SET7;break;
    case PORT0_BIT_4: port = STEP_DIM_SET8;break;
    case PORT0_BIT_5: port = STEP_DIM_SET9;break;
    case PORT0_BIT_6: port = STEP_DIM_SET10;break;
    case PORT0_BIT_7: port = STEP_DIM_SET11;break;
    case PORT1_BIT_0: port = STEP_DIM_SET0;break;
    case PORT1_BIT_1: port = STEP_DIM_SET1;break;
    case PORT1_BIT_2: port = STEP_DIM_SET2;break;
    case PORT1_BIT_3: port = STEP_DIM_SET3;break;
    case PORT1_BIT_4: port = STEP_DIM_SET12;break;
    case PORT1_BIT_5: port = STEP_DIM_SET13;break;
    case PORT1_BIT_6: port = STEP_DIM_SET14;break;
    case PORT1_BIT_7: port = STEP_DIM_SET15;break;
    default:port = 0;break;
  }

  i2c_gpio_set(AW9523_I2C_PORT);  	//SDA/CK : C0/C1

  i2c_master_init(AW9523_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*200000)) );

  i2c_write_byte(port, 1, level);
}

void aw9523_led_on_off(u32 leds, u8 mode)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while(leds){
    if(leds & led){
      if((leds == HAL_LED_10) || (leds == HAL_LED_11) || (leds == HAL_LED_12)){
        if(mode == HAL_LED_MODE_ON)
          aw9523_other_led_on(aw9523_led_table[num]);
        else
          aw9523_led_off(aw9523_led_table[num]);
        leds ^= led;
      }else{
        if(mode == HAL_LED_MODE_ON)
          aw9523_led_on(aw9523_led_table[num]);
        else
          aw9523_led_off(aw9523_led_table[num]);
        leds ^= led;
      }
      //      if(mode == HAL_LED_MODE_ON)
      //        aw9523_led_on(aw9523_led_table[num]);
      //      else
      //        aw9523_led_off(aw9523_led_table[num]);
      //      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}

void aw9523_led_breath(u32 leds, u8 tim)
{
  u8  num = 0;
  u32 led = HAL_LED_1;

  while(leds){
    if(leds & led){
      aw9523_set_led_dim(aw9523_led_table[num], tim);
      leds ^= led;
    }
    num++;
    led <<= 1;
  }
}

#endif
