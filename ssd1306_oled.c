#if defined(SSD1306_OLED)
// 液晶模块型号：12832，I2C接口
// 驱动IC是:SSD1306

#include "../../common.h"
#include "ssd1306_oled.h"
#include "main.h"
#include "board.h"

u8 const ascii_table_8x16[95][16];
u8 const ascii_table_5x8[95][5];

//写指令到OLED显示模块
static void lcd_command(u8 com)
{
  i2c_gpio_set(SSD1306_I2C_PORT);  	//SDA/CK : B6/D7

  i2c_master_init(LCD_IIC_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLOCK)) );

  i2c_write_series(0x00, 1, (unsigned char *)&com, 1);
}

//写数据到OLED显示模块
static void lcd_data(u8 data)
{
  i2c_gpio_set(SSD1306_I2C_PORT);  	//SDA/CK : B6/D7

  i2c_master_init(LCD_IIC_ADDRESS, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLOCK)) );

  i2c_write_series(0x40, 1, (unsigned char *)&data, 1);
}

static void lcd_address(u8 page, u8 column)
{
  column -=1;
  page   -=1;

  lcd_command(0xb0 + page);   				//设置页地址。每页是8行。一个画面的64行被分成8个页。我们平常所说的第1页，在LCD驱动IC里是第0页，所以在这里减去1*/
  lcd_command(((column >> 4) & 0x0f) + 0x10);	//设置列地址的高4位
  lcd_command(column & 0x0f);				//设置列地址的低4位
}

/*全屏清屏*/
void clear_screen()
{
  for(u8 i = 0; i < 8; i++){
    lcd_address(1 + i, 1);
    for(u8 j = 0; j < 128; j++)
      lcd_data(0x00);
  }
}

//===显示测试画面：例如全显示，隔行显示，隔列显示，雪花显示=====
void test_display(u8 data1, u8 data2)
{
  for(u8 j = 0; j < 4; j++){
    lcd_address(j + 1, 1);
    for(u8 i = 0; i < 128; i++){
      lcd_data(data1);
      lcd_data(data2);
    }
  }          
}

/*显示128x32点阵图像*/
void display_graphic_128x32(u8 page, u8 column, u8 *dp)
{
  for(u8 j = 0; j < 4; j++){		 //8
    lcd_address(page + j, column);
    for(u8 i = 0; i < 128; i++){		 //128
      lcd_data(*dp);
      dp++;
    }
  }          
}

void display_graphic_128x16(u8 page, u8 column, u8 *dp)
{
  for(u8 j = 0; j < 2; j++){		 //8
    lcd_address(page + j, column);
    for(u8 i = 0; i < 128; i++){		 //128
      lcd_data(*dp);
      dp++;
    }
  }
}

/*显示32x32点阵图像、汉字、生僻字或32x32点阵的其他图标*/
void display_graphic_32x32(u8 page, u8 column, u8 *dp)
{
  for(u8 j = 0; j < 4; j++){
    lcd_address(page + j, column);
    for (u8 i = 0; i < 31; i++){
      lcd_data(*dp);		/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
      dp++;	
    }
  }
}

/*显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标*/
void display_graphic_16x16_2(u8 reverse, u8 page, u8 column, u8 *dp)
{
  for(u8 j = 0; j < 2; j++){
    lcd_address(page + j, column);
    for (u8 i = 0; i < 16; i++){
      if(reverse == 1)
        lcd_data(*dp);		/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
      else
        lcd_data(~*dp);	/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
      dp++;
    }
  }
}

/*显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标*/
void display_graphic_16x16(u8 page, u8 column, u8 *dp)
{
  for(u8 j = 0; j < 2; j++){
    lcd_address(page + j, column);
    for (u8 i =0; i < 16; i++){
      lcd_data(*dp);		/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
      dp++;
    }
  }
}

/*显示8x16点阵图像、ASCII, 或8x16点阵的自造字符、其他图标*/
void display_graphic_8x16(u8 page, u8 column, u8 *dp)
{
  for(u8 j = 0; j < 2; j++){
    lcd_address(page + j, column);
    for (u8 i = 0; i < 8; i++){
      lcd_data(*dp);					/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
      dp++;
    }
  }
}

void display_string_8x16(u32 page, u32 column, u8 *text)
{
  u32 i = 0;
  u32 j;

  while(text[i] > 0x00){
    if((text[i] >= 0x20) && (text[i] <= 0x7e)){
      j = text[i] - 0x20;
      for(u8 n = 0; n < 2; n++){
        lcd_address(page + n, column);
        for(u8 k = 0; k < 8; k++)
          lcd_data(ascii_table_8x16[j][k+8*n]);/*显示5x7的ASCII字到LCD上，y为页地址，x为列地址，最后为数据*/
      }
      i++;
      column+=8;		
    }
    else
      i++;
  }
}

void display_string_5x8(u32 page, u32 column, u8 *text)
{
  u32 i = 0;
  u32 j;

  while(text[i] > 0x00){
    if((text[i] >= 0x20) && (text[i] < 0x7e)){
      j = text[i] - 0x20;
      lcd_address(page, column);
      for(u8 k = 0; k < 5; k++)
        lcd_data(ascii_table_5x8[j][k]);/*显示5x7的ASCII字到LCD上，y为页地址，x为列地址，最后为数据*/
      i++;
      column += 6;
    }
    else
      i++;
  }
}

//LCD模块初始化
void initial_lcd()
{
  //	LCD_RST =0;
  //	delay(30);
  //	LCD_RST =1;
  //	delay(100);	  //等待RC复位  大约10MS

  lcd_command(0xAE); //Display Off (0xAE/0xAF) 关显示

  lcd_command(0x40); //0x40  Set Display Start Line 起始行

  lcd_command(0x81); //Set Contrast Control,本指令的0x81不要改动，改下面的值
  lcd_command(0x8F); // 0xff 微调对比度的值，可设置范围0x00～0xff

  lcd_command(0xA1); //(0XA1/0XA0) Set Segment Re-Map  列扫描顺序：从左到右
  lcd_command(0xC8); //(0xC0/0xC8) Set COM Output Scan Direction  行扫描顺序：从上到下

  lcd_command(0xA6);	//Set Normal/Inverse Display

  lcd_command(0xA8); //Set Multiplex Ratio
  lcd_command(0x1F); // 1/64 Duty (0x0F~0x3F) duty=1/64	 3F

  lcd_command(0xD3);  //Set Display Offset 显示偏移
  lcd_command(0x00);	 //Shift Mapping RAM Counter (0x00~0x3F)

  lcd_command(0xD5);  //Set Display Clock Divide Ratio/Oscillator Frequency
  lcd_command(0x80);	 // Set Clock as 100 Frames/Sec

  lcd_command(0xD9); //Set Pre-Charge Period
  lcd_command(0x1F);	 //0xf1

  lcd_command(0xDA); //Set COM Pins Hardware Configuration
  lcd_command(0x00); 	  //12

  lcd_command(0xDB); //Set VCOMH Deselect Level
  lcd_command(0x40);	//0x40

  clear_screen();	  	//clear all dots
  //	delay(20);

  lcd_command(0x8D); //Set Charge Pump
  lcd_command(0x14);
  //lcd_command(0x10);

  lcd_command(0xaf);   //Set Display On
  //	delay(200);
}
#endif
