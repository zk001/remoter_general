/********************************************************************************************************
 * @file     ssd1306_oled.h
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
#ifndef __OLED_H
#define __OLED_H			  	 

#include "common.h"

#define SIZE        16
#define XLevelL		  0x02
#define XLevelH		  0x10
#define Max_Column	128
#define Max_Row		  64
#define	Brightness	0xFF
#define X_WIDTH 	  128
#define Y_WIDTH 	  64

// IIC_ADDRESS
#define	LCD_IIC_ADDRESS     0x78   //SSD1306  SLAVE ADDRESS
#define I2C_CLOCK           400000//KHZ

extern void initial_lcd ();
extern void initial_lcd_no_clr ();
extern void clear_screen ();
extern void display_graphic_128x32 (u8 page, u8 column, u8 *dp);
extern void display_graphic_128x32_2 (u8 reverse, u8 page, u8 column, u8* dp);
extern void test_display (u8 data1,u8 data2);
extern void display_string_8x16 (u32 page, u32 column, u8 *text);
extern void display_string_5x8 (u32 page, u32 column, u8 *text);
extern void display_graphic_16x16_2 (u8 reverse, u8 page, u8 column, u8 *dp);
extern void display_graphic_16x16 (u8 page, u8 column, u8 *dp);
extern void display_graphic_8x16 (u8 page, u8 column, u8 *dp);
extern void display_graphic_128x16 (u8 page, u8 column, u8 *dp);
extern void display_graphic_64x32 (u8 page, u8 column, u8* dp);
extern void display_graphic_32x32 (u8 page, u8 column, u8* dp);
extern void display_graphic_24x24 (u8 page, u8 column, u8* dp);
extern void display_graphic_20x16 (u8 page, u8 column, u8* dp);


extern void clr_graphic_8x16 (u8 page, u8 column);
extern void clr_graphic_16x16_2 (u8 reverse, u8 page, u8 column);
extern void clr_graphic_128x16 (u8 page, u8 column);
extern void display_graphic_1x16 (u8 page, u8 column, u8 *dp);
extern void clr_graphic_1x16 (u8 page, u8 column);
extern void clear_half_top_screen ();
extern void clear_half_bottom_screen ();
extern void display_graphic_1x8 (u8 page, u8 column, u8 *dp);

extern u8 const bmp_12832_0[];
extern u8 const zhuang1[];
extern u8 const shi1[];
extern u8 const yong1[];
extern u8 const tai1[];

#endif  




