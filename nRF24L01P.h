/**@file  	    nRF24L01P.h
* @brief            nRF24L01+ low level operations and configurations.
* @author           hyh
* @date             2021.6.9
* @version          1.0
* @copyright        Chengdu Ebyte Electronic Technology Co.Ltd
**********************************************************************************
*/
#if defined(NRF24L01)
#ifndef nRF24L01P_H
#define nRF24L01P_H

//#include "bsp.h"
//#include "MyTypeDef.h"
#include "common.h"
#include "nRF24L01P_REG.h"
#include "board.h"

#define st(x)      do { x } while (__LINE__ == -1)

INT8U BSP_SPI_ExchangeByte(u8 cmd, u8 cmd_len, u8 data_len);

/*----------------------------------------------------*/

/*------------------------------------------------*/


#define IRQ_ALL ((1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT))
/*Data Rate selection*/
typedef enum {DRATE_250K,DRATE_1M,DRATE_2M}L01_DRATE;
/*Power selection*/
typedef enum {POWER_N_0,POWER_N_6,POWER_N_12,POWER_N_18}L01_PWR;
/*Mode selection*/
typedef enum {TX_MODE,RX_MODE}L01_MODE;
/*CE pin level selection*/
typedef enum {CE_LOW,CE_HIGH}CE_STAUS;
/*
================================================================================
============================Configurations and Options==========================
================================================================================
*/
#define DYNAMIC_PACKET      1 //1:DYNAMIC packet length, 0:fixed
#define FIXED_PACKET_LEN    32//Packet size in fixed size mode
#define INIT_ADDR           1,2,3,4,5
/*
================================================================================
==========================List of externally provided functions ================
================================================================================
*/
#define L01_CE_LOW()       st(gpio_set_func (PIN_L01_CE, AS_GPIO); gpio_set_output_en (PIN_L01_CE, 1); gpio_set_input_en (PIN_L01_CE, 0);gpio_write (PIN_L01_CE, 0);)//GPIO_ResetBits(PORT_L01_CE, PIN_L01_CE)//Set CE low level
#define L01_CE_HIGH()      st(gpio_set_func (PIN_L01_CE, AS_GPIO); gpio_set_output_en (PIN_L01_CE, 1); gpio_set_input_en (PIN_L01_CE, 0);gpio_write (PIN_L01_CE, 1);)//GPIO_SetBits(PORT_L01_CE, PIN_L01_CE)//Set CE high level
#define GET_L01_IRQ()      ({gpio_set_func (PIN_L01_IRQ, AS_GPIO); gpio_set_output_en (PIN_L01_IRQ, 0); gpio_set_input_en (PIN_L01_IRQ, 1);gpio_setup_up_down_resistor (PIN_L01_IRQ, PM_PIN_PULLUP_1M);gpio_read (PIN_L01_IRQ) != 0 ? true:false;})//GPIO_ReadInputDataBit(PORT_L01_IRQ, PIN_L01_IRQ)//Get the IRQ pin status

/*
================================================================================
-------------------------------------Exported APIs------------------------------
================================================================================
*/
/*Set the level status of the CE pin low or high*/
void L01_SetCE(CE_STAUS status);
/*Read the value from the specified register */
INT8U L01_ReadSingleReg(INT8U addr);
/*Read the values of the specified registers and store them in buffer*/
void L01_ReadMultiReg(INT8U start_addr,INT8U *buffer,INT8U size);
/*Write a value to the specified register*/
void L01_WriteSingleReg(INT8U addr,INT8U value);
/*Write buffer to the specified registers */
void L01_WriteMultiReg(INT8U start_addr,INT8U *buffer,INT8U size);
/*Set the nRF24L01 into PowerDown mode */
void L01_SetPowerDown(void);
/*Set the nRF24L01 into PowerUp mode*/
void L01_SetPowerUp(void);
/*Flush the TX buffer*/
void L01_FlushTX(void);
/*Flush the RX buffer*/
void L01_FlushRX(void);
/*Reuse the last transmitted payload*/
void L01_ReuseTXPayload(void);
/*Read the status register of the nRF24L01*/
INT8U L01_ReadStatusReg(void);
/*Clear the IRQ caused by the nRF24L01+*/
void L01_ClearIRQ(INT8U irqMask);
/*Read the IRQ status of the nRF24L01+*/
INT8U L01_ReadIRQSource(void);
/*Read the payload width of the top buffer of the FIFO */
INT8U L01_ReadTopFIFOWidth(void);
/*Read the RX payload from the FIFO and store them in buffer*/
INT8U L01_ReadRXPayload(INT8U *buffer);
/*Write TX Payload to a data pipe,and PRX will return ACK back*/
void L01_WriteTXPayload_Ack(INT8U *buffer,INT8U size);
/*Write TX Payload to a data pipe,and PRX won't return ACK back*/
void L01_WriteTXPayload_NoAck(INT8U *buffer,INT8U size);
/*Write TX Payload to a data pipe when RX mode*/
void L01_WriteRXPayload_InAck(INT8U *buffer,INT8U size);
/*Write Transmit address into TX_ADDR register */
void L01_SetTXAddr(INT8U *Addrbuffer,INT8U Addr_size);
/*Write address for the RX pipe*/
void L01_SetRXAddr(INT8U pipeNum,INT8U *addrBuffer,INT8U addr_size);
/*Set the data rate of the nRF24L01+ */
void L01_SetDataRate(L01_DRATE drate);
/*Set the power of the nRF24L01+ */
void L01_SetPower(L01_PWR power);
/*Set the frequency of the nRF24L01+*/
void L01_WriteHoppingPoint(INT8U freq);
/*Set the nRF24L01+ as TX/RX mode*/
void L01_SetTRMode(L01_MODE mode);
/*Initialize the nRF24L01+ */
void L01_Init(void);
void APP_SwitchToRx(void);
void APP_SwitchToTx(void);
void APP_TransmitHandler(INT8U* buffer, INT8U len);
INT8U APP_RecieveHandler(INT8U* buffer);
#endif
#endif
