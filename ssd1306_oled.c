/********************************************************************************************************
 * @file     ssd1306_oled.c
 *
 * @brief    This is the source file for TLSR8258
 *
 * @author	 Driver Group
 * @date     Sep 22, 2021
 *
 * @par      Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 * @par      History:
 * 			 1.initial release(DEC. 26 2018)
 *
 * @version  A001
 *
 *******************************************************************************************************/
#if defined(SSD1306_OLED)
// Һ��ģ���ͺţ�12832��I2C�ӿ�
// ����IC��:SSD1306

#include "../../common.h"
#include "ssd1306_oled.h"
#include "main.h"
#include "board.h"

u8 const ascii_table_8x16 [95][16];
u8 const ascii_table_5x8 [95][5];

/**
 * @brief      This function serves to write instruction to oled
 * @param[in]  com - the instruction will be write to oled
 * @return     none
 */
static void lcd_command (u8 com)
{
  i2c_write_series (0x00, 1, (u8*)&com, 1);
}

/**
 * @brief      This function serves to write data to oled
 * @param[in]  com - the data will be write to oled
 * @return     none
 */
static void lcd_data (u8 data)
{
  i2c_write_series (0x40, 1, (u8*)&data, 1);
}

/**
 * @brief      This function serves to write address to oled
 * @param[in]  page   - the page will be write to oled
 * @param[in]  column - the column will be write to oled
 * @return     none
 */
static void lcd_address (u8 page, u8 column)
{
  column -=1;
  page   -=1;

  lcd_command (0xb0 + page);   				//����ҳ��ַ��ÿҳ��8�С�һ�������64�б��ֳ�8��ҳ������ƽ����˵�ĵ�1ҳ����LCD����IC���ǵ�0ҳ�������������ȥ1*/
  lcd_command (((column >> 4) & 0x0f) + 0x10);	//�����е�ַ�ĸ�4λ
  lcd_command (column & 0x0f);				//�����е�ַ�ĵ�4λ
}

/**
 * @brief      This function serves to clear oled half top screen
 * @param[in]  none
 * @return     none
 */
void clear_half_top_screen ()
{
  for (u8 i = 0; i < 2; i++) {
	lcd_address (3 + i, 1);
	for (u8 j = 0; j < 128; j++)
	  lcd_data (0x00);
  }
}

/**
 * @brief      This function serves to clear oled half bottom screen
 * @param[in]  none
 * @return     none
 */
void clear_half_bottom_screen ()
{
  for (u8 i = 0; i < 2; i++) {
	lcd_address (1 + i, 1);
	for (u8 j = 0; j < 128; j++)
	  lcd_data (0x00);
  }
}

/**
 * @brief      This function serves to clear all oled screen
 * @param[in]  none
 * @return     none
 */
void clear_screen ()
{
  for (u8 i = 0; i < 4; i++) {
    lcd_address (1 + i, 1);
    for (u8 j = 0; j < 128; j++)
      lcd_data (0x00);
  }
}

//===��ʾ���Ի��棺����ȫ��ʾ��������ʾ��������ʾ��ѩ����ʾ=====
/**
 * @brief      This function serves to test oled
 * @param[in]  none
 * @return     none
 */
void test_display (u8 data1, u8 data2)
{
  for (u8 j = 0; j < 4; j++) {
    lcd_address (j + 1, 1);
    for (u8 i = 0; i < 128; i++) {
      lcd_data (data1);
      lcd_data (data2);
    }
  }          
}

/*��ʾ128x32����ͼ��*/
/**
 * @brief      This function serves to display 128*32
 * @param[in]  page   - the page will be write to oled
 * @param[in]  column - the column will be write to oled
 * @param[in]  dp     - the data will be write to oled
 * @return     none
 */
void display_graphic_128x32 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 4; j++) {		 //8
    lcd_address (page + j, column);
    for (u8 i = 0; i < 128; i++) {		 //128
      lcd_data (*dp);
      dp++;
    }
  }          
}
/*64*32*/
void display_graphic_64x32 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 4; j++) {		 //8
    lcd_address (page + j, column);
    for (u8 i = 0; i < 64; i++) {		 //128
      lcd_data (*dp);
      dp++;
    }
  }
}

/**
 * @brief      This function serves to display 1*8
 * @param[in]  page   - the page will be write to oled
 * @param[in]  column - the column will be write to oled
 * @param[in]  dp     - the data will be write to oled
 * @return     none
 */
void display_graphic_1x8 (u8 page, u8 column, u8* dp)
{
  for(u8 j = 0; j < 1; j++) {		 //8
	lcd_address (page + j, column);
	for (u8 i = 0; i < 1; i++) {		 //128
	  lcd_data (*dp);
	}
  }
}

/**
 * @brief      This function serves to clear 1*16
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @return     none
 */
void clr_graphic_1x16 (u8 page, u8 column)
{
  for (u8 j = 0; j < 2; j++) {		 //8
	lcd_address (page + j, column);
	for (u8 i = 0; i < 1; i++) {		 //128
	  lcd_data (0);
	}
  }
}

/**
 * @brief      This function serves to display 1*16
 * @param[in]  page   - the page will be write to oled
 * @param[in]  column - the column will be write to oled
 * @param[in]  dp     - the data will be write to oled
 * @return     none
 */
void display_graphic_1x16 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 2; j++) {		 //8
	lcd_address (page + j, column);
	for (u8 i = 0; i < 1; i++) {		 //128
	  lcd_data (*dp);
	  dp++;
	}
  }
}

/**
 * @brief      This function serves to clear 128*16
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @return     none
 */
void clr_graphic_128x16 (u8 page, u8 column)
{
  for (u8 j = 0; j < 2; j++) {		 //8
	lcd_address (page + j, column);
	for (u8 i = 0; i < 128; i++)	 //128
	  lcd_data (0);
  }
}

/**
 * @brief      This function serves to display 128*16
 * @param[in]  page   - the page will be write to oled
 * @param[in]  column - the column will be write to oled
 * @param[in]  dp     - the data will be write to oled
 * @return     none
 */
void display_graphic_128x16 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 2; j++) {		 //8
    lcd_address (page + j, column);
    for (u8 i = 0; i < 128; i++) {		 //128
      lcd_data (*dp);
      dp++;
    }
  }
}

/*��ʾ32x32����ͼ�񡢺��֡���Ƨ�ֻ�32x32���������ͼ��*/
/**
 * @brief      This function serves to display 32*32
 * @param[in]  page   - the page will be write to oled
 * @param[in]  column - the column will be write to oled
 * @param[in]  dp     - the data will be write to oled
 * @return     none
 */
void display_graphic_32x32 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 4; j++) {
    lcd_address (page + j, column);
    for (u8 i = 0; i < 32; i++) {
      lcd_data (*dp);		/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
      dp++;	
    }
  }
}

/**
 * @brief      This function serves to clear 16*16
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @return     none
 */
void clr_graphic_16x16_2 (u8 reverse, u8 page, u8 column)
{
  for (u8 j = 0; j < 2; j++) {
	lcd_address (page + j, column);
	for (u8 i = 0; i < 16; i++) {
	  if (reverse == 1)
	    lcd_data (0);		/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
	  else
		lcd_data (0xff);
	}
  }
}

/*��ʾ16x16����ͼ�񡢺��֡���Ƨ�ֻ�16x16���������ͼ��*/
/**
 * @brief      This function serves to display 16*16
 * @param[in]  reverse - reverse the display
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @param[in]  dp      - the data will be write to oled
 * @return     none
 */
void display_graphic_16x16_2 (u8 reverse, u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 2; j++) {
    lcd_address (page + j, column);
    for (u8 i = 0; i < 16; i++) {
      if (reverse == 1)
        lcd_data (*dp);		/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
      else
        lcd_data (~*dp);	/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
      dp++;
    }
  }
}

/*��ʾ16x16����ͼ�񡢺��֡���Ƨ�ֻ�16x16���������ͼ��*/
/**
 * @brief      This function serves to display 16*16
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @param[in]  dp      - the data will be write to oled
 * @return     none
 */
void display_graphic_16x16 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 2; j++) {
    lcd_address (page + j, column);
    for (u8 i =0; i < 16; i++) {
      lcd_data (*dp);		/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
      dp++;
    }
  }
}

/**
 * @brief      This function serves to clear 8*16
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @return     none
 */
void clr_graphic_8x16 (u8 page, u8 column)
{
  for (u8 j = 0; j < 2; j++) {
	lcd_address (page + j, column);
	for (u8 i = 0; i < 8; i++)
	  lcd_data (0);					/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
  }
}

/*��ʾ8x16����ͼ��ASCII, ��8x16����������ַ�������ͼ��*/
/**
 * @brief      This function serves to display 8*16
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @param[in]  dp      - the data will be write to oled
 * @return     none
 */
void display_graphic_8x16 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 2; j++) {
    lcd_address (page + j, column);
    for (u8 i = 0; i < 8; i++) {
      lcd_data (*dp);					/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
      dp++;
    }
  }
}

void display_graphic_20x16 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 2; j++) {
    lcd_address (page + j, column);
    for (u8 i = 0; i < 20; i++) {
      lcd_data (*dp);					/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
      dp++;
    }
  }
}

void display_graphic_24x16 (u8 page, u8 column, u8* dp)
{
  for (u8 j = 0; j < 2; j++) {
    lcd_address (page + j, column);
    for (u8 i = 0; i < 24; i++) {
      lcd_data (*dp);					/*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
      dp++;
    }
  }
}
/**
 * @brief      This function serves to display 8*16
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @param[in]  text    - the data will be write to oled
 * @return     none
 */
void display_string_8x16 (u32 page, u32 column, u8* text)
{
  u32 i = 0;
  u32 j;

  while (text[i] > 0x00) {
    if ((text[i] >= 0x20) && (text[i] <= 0x7e)) {
      j = text[i] - 0x20;
      for (u8 n = 0; n < 2; n++) {
        lcd_address (page + n, column);
        for (u8 k = 0; k < 8; k++)
          lcd_data (ascii_table_8x16[j][k+8*n]);/*��ʾ5x7��ASCII�ֵ�LCD�ϣ�yΪҳ��ַ��xΪ�е�ַ�����Ϊ����*/
      }
      i++;
      column += 8;
    } else
      i++;
  }
}

/**
 * @brief      This function serves to display 5*8
 * @param[in]  page    - the page will be write to oled
 * @param[in]  column  - the column will be write to oled
 * @param[in]  text    - the data will be write to oled
 * @return     none
 */
void display_string_5x8 (u32 page, u32 column, u8* text)
{
  u32 i = 0;
  u32 j;

  while (text[i] > 0x00) {
    if ((text[i] >= 0x20) && (text[i] < 0x7e)) {
      j = text[i] - 0x20;
      lcd_address (page, column);
      for (u8 k = 0; k < 5; k++)
        lcd_data (ascii_table_5x8[j][k]);/*��ʾ5x7��ASCII�ֵ�LCD�ϣ�yΪҳ��ַ��xΪ�е�ַ�����Ϊ����*/
      i++;
      column += 6;
    }else
      i++;
  }
}

//LCDģ���ʼ��
/**
 * @brief      This function serves to init the oled
 * @param[in]  none
 * @return     none
 */
void initial_lcd()
{
  //	LCD_RST =0;
  //	delay(30);
  //	LCD_RST =1;
  //	delay(100);	  //�ȴ�RC��λ  ��Լ10MS

  i2c_gpio_set (SSD1306_I2C_PORT);  	//SDA/CK : B6/D7
  i2c_master_init (LCD_IIC_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLOCK)) );

  lcd_command (0xAE); //Display Off (0xAE/0xAF) ����ʾ

  lcd_command (0x40); //0x40  Set Display Start Line ��ʼ��

  lcd_command (0x81); //Set Contrast Control,��ָ���0x81��Ҫ�Ķ����������ֵ
  lcd_command (0x8F); // 0xff ΢���Աȶȵ�ֵ�������÷�Χ0x00��0xff

  lcd_command (0xA1); //(0XA1/0XA0) Set Segment Re-Map  ��ɨ��˳�򣺴�����
  lcd_command (0xC8); //(0xC0/0xC8) Set COM Output Scan Direction  ��ɨ��˳�򣺴��ϵ���

  lcd_command (0xA6);	//Set Normal/Inverse Display

  lcd_command (0xA8); //Set Multiplex Ratio
  lcd_command (0x1F); // 1/64 Duty (0x0F~0x3F) duty=1/64	 3F

  lcd_command (0xD3);  //Set Display Offset ��ʾƫ��
  lcd_command (0x00);	 //Shift Mapping RAM Counter (0x00~0x3F)

  lcd_command (0xD5);  //Set Display Clock Divide Ratio/Oscillator Frequency
  lcd_command (0x80);	 // Set Clock as 100 Frames/Sec

  lcd_command (0xD9); //Set Pre-Charge Period
  lcd_command (0x1F);	 //0xf1

  lcd_command (0xDA); //Set COM Pins Hardware Configuration
  lcd_command (0x00); 	  //12

  lcd_command (0xDB); //Set VCOMH Deselect Level
  lcd_command (0x40);	//0x40

  clear_screen ();	  	//clear all dots
  //	delay(20);

  lcd_command (0x8D); //Set Charge Pump
  lcd_command (0x14);
  //lcd_command(0x10);

  lcd_command (0xaf);   //Set Display On
  //	delay(200);
}

/**
 * @brief      This function serves to init the oled without clear screen
 * @param[in]  none
 * @return     none
 */
void initial_lcd_no_clr ()
{
  //	LCD_RST =0;
  //	delay(30);
  //	LCD_RST =1;
  //	delay(100);	  //�ȴ�RC��λ  ��Լ10MS

  //  lcd_command(0xAE); //Display Off (0xAE/0xAF) ����ʾ
  i2c_gpio_set (SSD1306_I2C_PORT);  	//SDA/CK : B6/D7
  i2c_master_init (LCD_IIC_ADDRESS, (u8)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLOCK)) );

  lcd_command (0x40); //0x40  Set Display Start Line ��ʼ��

  lcd_command (0x81); //Set Contrast Control,��ָ���0x81��Ҫ�Ķ����������ֵ
  lcd_command (0x8F); // 0xff ΢���Աȶȵ�ֵ�������÷�Χ0x00��0xff

  lcd_command (0xA1); //(0XA1/0XA0) Set Segment Re-Map  ��ɨ��˳�򣺴�����
  lcd_command (0xC8); //(0xC0/0xC8) Set COM Output Scan Direction  ��ɨ��˳�򣺴��ϵ���

  lcd_command (0xA6);	//Set Normal/Inverse Display

  lcd_command (0xA8); //Set Multiplex Ratio
  lcd_command (0x1F); // 1/64 Duty (0x0F~0x3F) duty=1/64	 3F

  lcd_command (0xD3);  //Set Display Offset ��ʾƫ��
  lcd_command (0x00);	 //Shift Mapping RAM Counter (0x00~0x3F)

  lcd_command (0xD5);  //Set Display Clock Divide Ratio/Oscillator Frequency
  lcd_command (0x80);	 // Set Clock as 100 Frames/Sec

  lcd_command (0xD9); //Set Pre-Charge Period
  lcd_command (0x1F);	 //0xf1

  lcd_command(0xDA); //Set COM Pins Hardware Configuration
  lcd_command (0x00); 	  //12

  lcd_command (0xDB); //Set VCOMH Deselect Level
  lcd_command (0x40);	//0x40

  //  clear_screen();	  	//clear all dots
  //	delay(20);

  lcd_command (0x8D); //Set Charge Pump
  lcd_command (0x14);
  //lcd_command(0x10);

  lcd_command (0xaf);   //Set Display On
  //	delay(200);
}
#endif
