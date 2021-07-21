#ifndef __OLED_H
#define __OLED_H			  	 

#include "drivers.h"
#include "../../common.h"

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
extern void initial_lcd();
extern void clear_screen();
extern void display_graphic_128x32(u8 page, u8 column, u8 *dp);
extern void test_display(u8 data1,u8 data2);
extern void display_string_8x16(u32 page, u32 column, u8 *text);
extern void display_string_5x8(u32 page, u32 column, u8 *text);
extern void display_graphic_16x16_2(u8 reverse, u8 page, u8 column, u8 *dp);
extern void display_graphic_16x16(u8 page, u8 column, u8 *dp);
extern void display_graphic_8x16(u8 page, u8 column, u8 *dp);
extern void display_graphic_128x16(u8 page, u8 column, u8 *dp);

extern u8 const bmp_12832_0[];
extern u8 const zhuang1[];
extern u8 const shi1[];
extern u8 const yong1[];
extern u8 const tai1[];

#endif  
	 



