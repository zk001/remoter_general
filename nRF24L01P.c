/**@file  	    nRF24L01P.c
* @brief            nRF24L01+ low level operations and configurations.
* @author           hyh
* @date             2021.9.17
* @version          1.0
* @copyright        Chengdu Ebyte Electronic Technology Co.Ltd
**********************************************************************************
*/
#if defined(NRF24L01)
#include "nRF24L01P.h"
#include "main.h"

/*the CE pin level status*/
static INT8U CE_Status = 0;
/*!
================================================================================
------------------------------------Functions-----------------------------------
================================================================================
*/
/*!
 *  @brief          Get the level status of the CE pin
 *  @param          None     
 *  @return         CE pin level status: 0:low; 1:high
 *  @note          
*/
INT8U L01_GetCEStatus(void)
{
    return CE_Status;
}
/*!
 *  @brief          Set the level status of the CE pin low or high
 *  @param          status:CE pin level status    
 *  @return         None
 *  @note          
*/
void L01_SetCE(CE_STAUS status)
{
    CE_Status = status;
    if (status == CE_LOW)    { L01_CE_LOW(); }
    else                     { L01_CE_HIGH(); }
}

/*!
 *  @brief        Read the value from the specified register   
 *  @param        addr:the address of the register
 *  @return       value:the value read from the register  
 *  @note          
*/
INT8U L01_ReadSingleReg(INT8U addr)
{
    INT8U value;
    INT8U w_data;

    w_data = R_REGISTER | addr;
    spi_read (&w_data, 1, &value, 1, TELINK_CS_PIN);

    return value;
}
/*!
 *  @brief        Read the values of the specified registers and store them in buffer
 *  @param        start_addr:the start address of the registers
 *  @param        buffer:the buffer stores the read values
*  @param         size:the size to be read
 *  @return       None  
 *  @note          
*/
//void L01_ReadMultiReg(INT8U start_addr,INT8U *buffer,INT8U size)
//{
//    INT8U data_change_order;
//
//    data_change_order = byte_change (R_REGISTER | start_addr);
//
//    spi_read (&data_change_order, 1, buffer, size, TELINK_CS_PIN);
//}
/*!
 *  @brief        Write a value to the specified register   
 *  @param        addr:the address of the register
 *  @param        value:the value to be written  
 *  @return       None
 *  @note          
*/
void L01_WriteSingleReg(INT8U addr,INT8U value)
{
    INT8U w_data;

    w_data = W_REGISTER | addr;

    spi_write (&w_data, 1, &value, 1, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Write buffer to the specified registers  
 *  @param        start_addr:the start address of the registers
 *  @param        buffer:the buffer to be written
 *  @param        size:the size to be written  
 *  @return       None
 *  @note          
*/
void L01_WriteMultiReg(INT8U start_addr,INT8U *buffer,INT8U size)
{

    INT8U w_data;

    w_data = W_REGISTER | start_addr;

    spi_write (&w_data, 1, buffer, size, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Set the nRF24L01 into PowerDown mode          
 *  @param        None
 *  @return       None  
 *  @note          
*/
void L01_SetPowerDown(void)
{
    INT8U controlreg = L01_ReadSingleReg(L01REG_CONFIG);
    L01_WriteSingleReg(L01REG_CONFIG,controlreg & (~(1 << PWR_UP)));
}
/*!
 *  @brief        Set the nRF24L01 into PowerUp mode       
 *  @param        None
 *  @return       None  
 *  @note          
*/
void L01_SetPowerUp(void)
{
    INT8U controlreg = L01_ReadSingleReg(L01REG_CONFIG);
    L01_WriteSingleReg(L01REG_CONFIG,controlreg | (1 << PWR_UP));
}
/*!
 *  @brief        Flush the TX buffer             
 *  @param        None 
 *  @return       None  
 *  @note          
*/
void L01_FlushTX(void)
{
    INT8U w_data;

    w_data = FLUSH_TX;

    spi_write (&w_data, 1, NULL, 0, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Flush the RX buffer           
 *  @param        None
 *  @return       None  
 *  @note          
*/
void L01_FlushRX(void)
{
    INT8U w_data;

    w_data = FLUSH_RX;

    spi_write (&w_data, 1, NULL, 0, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Reuse the last transmitted payload           
 *  @param        None
 *  @return       None  
 *  @note          
*/
void L01_ReuseTXPayload(void)
{
    INT8U w_data;

    w_data = REUSE_TX_PL;

    spi_write (&w_data, 1, NULL, 0, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        NOP operation about the nRF24L01+           
 *  @param        None
 *  @return       None  
 *  @note          
*/
void L01_Nop(void)
{
    INT8U w_data;

    w_data = NOP;

	spi_write (&w_data, 1, NULL, 0, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Read the status register of the nRF24L01+           
 *  @param        None
 *  @return       the value of the status register  
 *  @note          
*/
INT8U L01_ReadStatusReg(void)
{
    INT8U status;
    INT8U w_data;

    w_data = R_REGISTER + L01REG_STATUS;

    spi_read (&w_data, 1, &status, 1, TELINK_CS_PIN);

    return status;
}
/*!
 *  @brief        Clear the IRQ caused by the nRF24L01+           
 *  @param        irqMask:RX_DR(bit[6]),TX_DS(bit[5]),MAX_RT(bit[4])
 *  @return       None  
 *  @note          
*/
void L01_ClearIRQ(INT8U irqMask)
{
    INT8U status = 0;
    irqMask &= IRQ_ALL;
    status = L01_ReadStatusReg();
    L01_WriteSingleReg(L01REG_STATUS,irqMask | status);
    L01_ReadStatusReg();
}
/*!
 *  @brief        Read the IRQ status of the nRF24L01+           
 *  @param        None
 *  @return       irqMask:RX_DR(bit[6]),TX_DS(bit[5]),MAX_RT(bit[4]) 
 *  @note          
*/
INT8U L01_ReadIRQSource(void)
{
    INT8U status = 0;
    status = L01_ReadStatusReg();
    return (status & IRQ_ALL);
}
/*!
 *  @brief        Read the payload width of the top buffer of the FIFO           
 *  @param        None
 *  @return       width:the width of the pipe buffer  
 *  @note          
*/
INT8U L01_ReadTopFIFOWidth(void)
{
    INT8U width;
    INT8U w_data;

    w_data = R_RX_PL_WID;

    spi_read (&w_data, 1, &width, 1, TELINK_CS_PIN);

    return width;
}
/*!
 *  @brief        Read the RX payload from the FIFO and store them in buffer            
 *  @param        buffer:the buffer to store the data
 *  @return       the length to be read
 *  @note          
*/
INT8U L01_ReadRXPayload(INT8U *buffer)
{
    INT8U w_data;
    INT8U width;
    width = L01_ReadTopFIFOWidth();
    if(width > 32)
    {
        L01_FlushRX();
        return 0;
    }

    w_data = R_RX_PAYLOAD;

    spi_read (&w_data, 1, buffer, width, TELINK_CS_PIN);
    L01_FlushRX();
    return width;
}
/*!
 *  @brief        Write TX Payload to a data pipe,and PRX will return ACK back         
 *  @param        buffer:the buffer stores the data
 *  @param        size:the size to be written  
 *  @return       None  
 *  @note          
*/
void L01_WriteTXPayload_Ack(INT8U *buffer,INT8U size)
{
    INT8U w_data;
    INT8U w_size = (size > 32) ? 32 : size;
    L01_FlushTX();
    w_data = W_TX_PAYLOAD;

	spi_write (&w_data, 1, buffer, w_size, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Write TX Payload to a data pipe,and PRX won't return ACK back         
 *  @param        buffer:the buffer stores the data
 *  @param        size:the size to be written  
 *  @return       None  
 *  @note          
*/
void L01_WriteTXPayload_NoAck(INT8U *buffer,INT8U size)
{
    INT8U w_data;

    if (size > 32 || size == 0)
    {
        return;
    }

    w_data = W_TX_PAYLOAD_NOACK;
	spi_write (&w_data, 1, buffer, size, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Write TX Payload to a data pipe when RX mode         
 *  @param        buffer:the buffer stores the data
 *  @param        size:the size to be written  
 *  @return       None  
 *  @note          
*/
void L01_WriteRXPayload_InAck(INT8U *buffer,INT8U size)
{
    INT8U w_size = (size > 32) ? 32 : size;
    INT8U w_data;

    w_data = W_TX_PAYLOAD_NOACK;

	spi_write (&w_data, 1, buffer, w_size, TELINK_CS_PIN);// pls refer to the datasheet for the write and read format of spi.
}
/*!
 *  @brief        Write Transmit address into TX_ADDR register          
 *  @param        addrBuffer:the buffer stores the address
 *  @param        addr_size:the address byte num
 *  @return       None  
 *  @note         Used for a PTX device only 
*/
void L01_SetTXAddr(INT8U *addrBuffer,INT8U addr_size)
{
    INT8U size = (addr_size > 5) ? 5 : addr_size;
    L01_WriteMultiReg(L01REG_TX_ADDR,addrBuffer,size);
}
/*!
 *  @brief        Write address for the RX pipe 
 *  @param        pipeNum:the number of the data pipe         
 *  @param        addrBuffer:the buffer stores the address
 *  @param        addr_size:the address byte num
 *  @return       None  
 *  @note          
*/
void L01_SetRXAddr(INT8U pipeNum,INT8U *addrBuffer,INT8U addr_size)
{
    INT8U size = (addr_size > 5) ? 5 : addr_size;
    INT8U num = (pipeNum > 5) ? 5 : pipeNum;
    L01_WriteMultiReg(L01REG_RX_ADDR_P0 + num,addrBuffer,size);
}
/*!
 *  @brief        Set the data rate of the nRF24L01+          
 *  @param        drate:250K,1M,2M
 *  @return       None  
 *  @note          
*/
void L01_SetDataRate(L01_DRATE drate)
{
//    INT8U mask = L01_ReadSingleReg(L01REG_RF_SETUP);
//    mask &= ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));
//    if(drate == DRATE_250K)
//    {
//        mask |= (1 << RF_DR_LOW);
//    }
//    else if(drate == DRATE_1M)
//    {
//        mask |= ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));
//    }
//    else if(drate == DRATE_2M)
//    {
//        mask |= (1 << RF_DR_HIGH);
//    }
    L01_WriteSingleReg(L01REG_RF_SETUP,0x26);
}
/*!
 *  @brief        Set the power of the nRF24L01+          
 *  @param        power:18dB,12dB,6dB,0dB
 *  @return       None  
 *  @note          
*/
void L01_SetPower(L01_PWR power)
{
//    INT8U mask = L01_ReadSingleReg(L01REG_RF_SETUP) & ~0x07;
//    switch (power)
//    {
//    case POWER_N_18:
//        mask |= PWR_N_18DB;
//        break;
//    case POWER_N_12:
//        mask |= PWR_N_12DB;
//        break;
//    case POWER_N_6:
//        mask |= PWR_N_6DB;
//        break;
//    case POWER_N_0:
//        mask |= PWR_N_0DB;
//        break;
//    default:
//        break;
//    }
    L01_WriteSingleReg(L01REG_RF_SETUP,0x01);
}
/*!
 *  @brief        Set the frequency of the nRF24L01+          
 *  @param        freq:the hopping frequency point,range:0-125,2400Mhz-2525Mhz
 *  @return       None  
 *  @note          
*/
void L01_WriteHoppingPoint(INT8U freq)
{
    L01_WriteSingleReg(L01REG_RF_CH,freq <= 125 ? freq : 125);
}
/*!
 *  @brief        Set the nRF24L01+ as TX/RX mode         
 *  @param        mode:TX/RX
 *  @return       None  
 *  @note          
*/
void L01_SetTRMode(L01_MODE mode)
{
    INT8U mask = L01_ReadSingleReg(L01REG_CONFIG);
    if (mode == TX_MODE)
    {
        mask &= ~(1 << PRIM_RX);
    }
    else if (mode == RX_MODE)
    {
        mask |= (1 << PRIM_RX);
    }
    L01_WriteSingleReg(L01REG_CONFIG,mask);
}
/*!
 *  @brief        Initialize the nRF24L01+         
 *  @param        None
 *  @return       None  
 *  @note          
*/
void L01_Init(void)
{
    INT8U addr[5] = {INIT_ADDR};
//  INT8U status = L01_GetCEStatus();

	//spi clock 500K, only master need set i2c clock
    spi_master_init((unsigned char)(CLOCK_SYS_CLOCK_HZ/(2*2000000)-1), SPI_MODE0);          //div_clock. spi_clk = sys_clk/((div_clk+1)*2),mode select
    spi_master_gpio_set(NRF_SPI_PORT);    //master mode £ºspi pin set

    L01_SetCE(CE_LOW);
    L01_ClearIRQ(IRQ_ALL);

#if DYNAMIC_PACKET == 1
    //dynamic payload length
    L01_WriteSingleReg(L01REG_DYNPD,(1 << DPL_P0));//Enable pipe 0 dynamic payload length
    L01_WriteSingleReg(L01REG_FEATURE,(1 << EN_DPL)|(1 << EN_ACK_PAY));
    L01_ReadSingleReg(L01REG_DYNPD);
    L01_ReadSingleReg(L01REG_FEATURE);
#elif DYNAMIC_PACKET == 0
    //fixed payload length
    L01_WriteSingleReg(L01REG_RX_PW_P0,FIXED_PACKET_LEN);
#endif
    L01_WriteSingleReg(L01REG_CONFIG,0x0a);//Enable CRC,2 bytes
    L01_WriteSingleReg(L01REG_ENAA,(1 << ENAA_P0));//Auto Ack in pipe0
    L01_WriteSingleReg(L01REG_EN_RXADDR,(1 << ERX_P0));//Enable RX pipe 0
    L01_WriteSingleReg(L01REG_SETUP_AW,AW_5BYTES);//Address width:5bytes
    L01_WriteSingleReg(L01REG_SETUP_RETR,0xf0);//ARD:4000us,repeat time:15
    L01_WriteSingleReg(L01REG_RF_CH,0x3c);
    L01_SetDataRate(DRATE_250K);
//    L01_WriteHoppingPoint(0);
//    L01_SetPower(POWER_N_0);
    L01_SetTXAddr(addr,5);//Set TX address
    L01_SetRXAddr(0,addr,5);//Set RX address
    /*********/
    L01_WriteSingleReg(L01REG_RF_CH,0x4d);
    L01_ReadSingleReg(L01REG_RF_SETUP);
    L01_WriteSingleReg(L01REG_RF_SETUP,0x26);
    L01_FlushRX();
    L01_FlushTX();
    L01_ReadStatusReg();
    L01_WriteSingleReg(L01REG_STATUS,0x7e);
    L01_ReadStatusReg();
}

void APP_SwitchToRx(void)
{
    L01_SetCE(CE_LOW);
    L01_SetPowerUp();
    L01_SetTRMode(RX_MODE);
    L01_FlushRX();
    L01_FlushTX();
    L01_ClearIRQ(IRQ_ALL);
    L01_SetCE(CE_HIGH);
}

void APP_SwitchToTx(void)
{
    L01_SetCE(CE_LOW);
    L01_SetPowerUp();
    L01_SetTRMode(TX_MODE);
    L01_FlushRX();
    L01_FlushTX();
    L01_ClearIRQ(IRQ_ALL);
}

void APP_TransmitHandler(INT8U* buffer, INT8U len)
{
	APP_SwitchToTx();
	L01_WriteTXPayload_NoAck(buffer, len);
	L01_SetCE(CE_HIGH);
//	while(GET_L01_IRQ() != 0);
	WaitMs (15);
	L01_FlushTX();
	L01_ClearIRQ(IRQ_ALL);
}

INT8U APP_RecieveHandler(INT8U* buffer)
{
    INT8U len = 0;
    if(GET_L01_IRQ() == 0)
    {
        if(L01_ReadIRQSource() & (1 << RX_DR))//detect RF module recieve interrupt
        {
            if((len = L01_ReadRXPayload(buffer)) != 0)
            {

            }
        }
        L01_FlushRX();
        L01_ClearIRQ(IRQ_ALL);
    }

    return len;
}
#endif
