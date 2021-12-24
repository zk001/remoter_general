/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, CMOSTEK SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * Copyright (C) CMOSTEK SZ.
 */

/*!
 * @file    cmt2300a.h
 * @brief   CMT2300A transceiver RF chip driver
 *
 * @version 1.3
 * @date    Jul 17 2017
 * @author  CMOSTEK R@D
 */

#ifndef __CMT2300A_H
#define __CMT2300A_H

#include "common.h"

#ifdef __cplusplus 
extern "C" { 
#endif

#include "board.h"
#define st(x)      do { x } while (__LINE__ == -1)

#define SET_GPIO_OUT(x)          st(gpio_set_func (x##_PIN, AS_GPIO); gpio_set_output_en (x##_PIN, 1); gpio_set_input_en (x##_PIN, 0);)
#define SET_GPIO_IN(x)           st(gpio_set_func (x##_PIN, AS_GPIO); gpio_set_output_en (x##_PIN, 0); gpio_set_input_en (x##_PIN, 1);gpio_setup_up_down_resistor (x##_PIN, PM_PIN_PULLUP_1M);)

#define SET_GPIO_H(x)           st(gpio_write (x##_PIN, 1);)
#define SET_GPIO_L(x)           st(gpio_write (x##_PIN, 0);)
#define READ_GPIO_PIN(x)        (gpio_read (x##_PIN) != 0 ? 1:0)

/* ************************************************************************
*  The following need to be modified by user
*  ************************************************************************ */
#define CMT2300A_SetGpio1In()           SET_GPIO_IN(CMT_GPIO1_GPIO)
#define CMT2300A_SetGpio2In()           SET_GPIO_IN(CMT_GPIO2_GPIO)
#define CMT2300A_SetGpio3In()           SET_GPIO_IN(CMT_GPIO3_GPIO)
#define CMT2300A_ReadGpio1()            READ_GPIO_PIN(CMT_GPIO1_GPIO)
#define CMT2300A_ReadGpio2()            READ_GPIO_PIN(CMT_GPIO2_GPIO)
#define CMT2300A_ReadGpio3()            READ_GPIO_PIN(CMT_GPIO3_GPIO)
#define CMT2300A_DelayMs(ms)            WaitMs(ms)
#define CMT2300A_DelayUs(us)            WaitUs(us)
	
void CMT2300A_InitGpio(void);

u8 CMT2300A_ReadReg(u8 addr);
void CMT2300A_WriteReg(u8 addr, u8 dat);

void CMT2300A_ReadFifo(u8 buf[], u16 len);
void CMT2300A_WriteFifo(const u8 buf[], u16 len);	
/* ************************************************************************
   The following are for chip status controls.
*  ************************************************************************ */
void CMT2300A_SoftReset(void);
u8 CMT2300A_GetChipStatus(void);
BOOL CMT2300A_AutoSwitchStatus(u8 nGoCmd);
BOOL CMT2300A_GoSleep(void);
BOOL CMT2300A_GoStby(void);
BOOL CMT2300A_GoTFS(void);
BOOL CMT2300A_GoRFS(void);
BOOL CMT2300A_GoTx(void);
BOOL CMT2300A_GoRx(void);


/* ************************************************************************
*  The following are for chip interrupts, GPIO, FIFO operations.
*  ************************************************************************ */
void CMT2300A_ConfigGpio(u8 nGpioSel);
void CMT2300A_ConfigInterrupt(u8 nInt1Sel, u8 nInt2Sel);
void CMT2300A_SetInterruptPolar(BOOL bActiveHigh);
void CMT2300A_SetFifoThreshold(u8 nFifoThreshold);
void CMT2300A_EnableAntennaSwitch(u8 nMode);
void CMT2300A_EnableInterrupt(u8 nEnable);
void CMT2300A_EnableRxFifoAutoClear(BOOL bEnable);
void CMT2300A_EnableFifoMerge(BOOL bEnable);
void CMT2300A_EnableReadFifo(void);
void CMT2300A_EnableWriteFifo(void);
void CMT2300A_RestoreFifo(void);
u8 CMT2300A_ClearTxFifo(void);
u8 CMT2300A_ClearRxFifo(void);
u8 CMT2300A_ClearInterruptFlags(void);


/* ************************************************************************
*  The following are for Tx DIN operations in direct mode.
*  ************************************************************************ */
void CMT2300A_ConfigTxDin(u8 nDinSel);
void CMT2300A_EnableTxDin(BOOL bEnable);
void CMT2300A_EnableTxDinInvert(BOOL bEnable);


/* ************************************************************************
*  The following are general operations.
*  ************************************************************************ */
BOOL CMT2300A_IsExist(void);
u8 CMT2300A_GetRssiCode(void);
int CMT2300A_GetRssiDBm(void);
void CMT2300A_SetFrequencyChannel(u8 nChann);
void CMT2300A_SetFrequencyStep(u8 nOffset);
void CMT2300A_SetPayloadLength(u16 nLength);
void CMT2300A_EnableLfosc(BOOL bEnable);
void CMT2300A_EnableLfoscOutput(BOOL bEnable);
void CMT2300A_EnableAfc(BOOL bEnable);
void CMT2300A_SetAfcOvfTh(u8 afcOvfTh);


/* ************************************************************************
*  The following are for chip initializes.
*  ************************************************************************ */
void CMT2300A_Init(void);
BOOL CMT2300A_ConfigRegBank(u8 base_addr, const u8 bank[], u8 len);

void RF_Init(void);
void RF_Config(void);

#ifdef __cplusplus
} 
#endif

#endif
