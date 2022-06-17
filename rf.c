/********************************************************************************************************
 * @file     rf.c
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
#if defined (RF)
#include "common.h"
#include "rf.h"

#include "../../esb_ll/esb_ll.h"
#include "interrupt.h"
#include "app.h"
#include "pkg.h"

/**
 * @brief      This function serves to configure system rf receive baseband
 * @param[in]  none
 * @return     none
 */
void rf_8359_set_rx ()
{
  ESB_SetDatarate (ESB_DR_2M);
  ESB_SetOutputPower (ESB_RF_POWER_10DBM);
  ESB_SetAddressWidth (ADDRESS_WIDTH_5BYTES);
  ESB_ClosePipe (ESB_PIPE_ALL);

  rf_acc_len_set (5);
  write_reg8 (0x408, 0x71);//0x71
  write_reg8 (0x409, 0x76);//0x76
  write_reg8 (0x40a, 0x51);//0x51
  write_reg8 (0x40b, 0x39);//0x39
  write_reg8 (0x40c, 0x95);//0x95

  write_reg8 (0x420, 38);

  ESB_OpenPipe (ESB_PIPE0, 1);

  ESB_ModeSet (ESB_MODE_PRX);
  ESB_SetNewRFChannel (140);

  ESB_TxSettleSet (149);
  ESB_RxSettleSet (80);

  irq_clr_src ();
  rf_irq_clr_src (FLD_RF_IRQ_ALL);
  //enable irq
  irq_enable_type (FLD_IRQ_ZB_RT_EN); //enable RF irq
  rf_irq_disable (FLD_RF_IRQ_ALL);
  rf_irq_enable (FLD_RF_IRQ_TX|FLD_RF_IRQ_TX_DS|FLD_RF_IRQ_RX_DR|FLD_RF_IRQ_RX);
  irq_enable (); //enable general irq

  ESB_PRXTrig ();
}

/**
 * @brief      This function serves to configure system rf transmit baseband
 * @param[in]  none
 * @return     none
 */
void rf_8359_set_tx ()
{
  ESB_SetDatarate (ESB_DR_2M);
#if defined (BIANJIEBAO_20D_LAOHUA)
  ESB_SetOutputPower (ESB_RF_POWER_M_20DBM);
#else
#if defined (NEW_REMOTE) || defined (NEW_RF_POWER)//2022.5.25，严工发邮件，功率统一由10dB降低为5dB
  ESB_SetOutputPower (ESB_RF_POWER_5DBM);
#else
  ESB_SetOutputPower (ESB_RF_POWER_10DBM);
#endif
#endif
  ESB_SetAddressWidth (ADDRESS_WIDTH_5BYTES);
  ESB_ClosePipe (ESB_PIPE_ALL);

  rf_acc_len_set (5);
  write_reg8 (0x408, 0x71);//0x71
  write_reg8 (0x409, 0x76);//0x76
  write_reg8 (0x40a, 0x51);//0x51
  write_reg8 (0x40b, 0x39);//0x39
  write_reg8 (0x40c, 0x95);//0x95

  write_reg8 (0x420, 38);

  ESB_OpenPipe (ESB_PIPE0, 1);
  ESB_SetTXPipe (ESB_PIPE0);

  ESB_OpenPipe (ESB_PIPE0, 1);
  ESB_SetTXPipe (ESB_PIPE0);

  ESB_ModeSet (ESB_MODE_PTX);
  ESB_SetNewRFChannel (140);
  ESB_SetAutoRetry (0,150);  //5,150
  ESB_RxTimeoutSet (500);
  ESB_RxSettleSet (80);
  ESB_TxSettleSet (149);

  WaitUs (150);
  //configure irq
  irq_clr_src ();
  rf_irq_clr_src (FLD_RF_IRQ_ALL);

  irq_enable_type (FLD_IRQ_ZB_RT_EN); //enable RF irq
  rf_irq_disable (FLD_RF_IRQ_ALL);
  // rf_irq_enable(FLD_RF_IRQ_TX|FLD_RF_IRQ_TX_DS|FLD_RF_IRQ_RETRY_HIT|FLD_RF_IRQ_RX_DR);
  irq_enable (); //enable general irq
}

/**
 * @brief      This function serves to decrease system rf power
 * @param[in]  none
 * @return     none
 */
void decrease_rf_power_tx ()
{
  ESB_SetDatarate (ESB_DR_2M);
  ESB_SetOutputPower (ESB_RF_POWER_M_16DBM);
  ESB_SetAddressWidth (ADDRESS_WIDTH_5BYTES);
  ESB_ClosePipe (ESB_PIPE_ALL);

  rf_acc_len_set (5);
  write_reg8 (0x408, 0x71);
  write_reg8 (0x409, 0x76);
  write_reg8 (0x40a, 0x51);
  write_reg8 (0x40b, 0x39);
  write_reg8 (0x40c, 0x95);

  write_reg8 (0x420, 38);

  ESB_OpenPipe (ESB_PIPE0, 1);
  ESB_SetTXPipe (ESB_PIPE0);

  ESB_OpenPipe (ESB_PIPE0, 1);
  ESB_SetTXPipe (ESB_PIPE0);

  ESB_ModeSet (ESB_MODE_PTX);
  ESB_SetNewRFChannel (140);
  ESB_SetAutoRetry (0,150);  //5,150
  ESB_RxTimeoutSet (500);
  ESB_RxSettleSet (80);
  ESB_TxSettleSet (149);

  WaitUs (150);
  //configure irq
  irq_clr_src ();
  rf_irq_clr_src (FLD_RF_IRQ_ALL);

  irq_enable_type (FLD_IRQ_ZB_RT_EN); //enable RF irq
  rf_irq_disable (FLD_RF_IRQ_ALL);
  // rf_irq_enable(FLD_RF_IRQ_TX|FLD_RF_IRQ_TX_DS|FLD_RF_IRQ_RETRY_HIT|FLD_RF_IRQ_RX_DR);
  irq_enable (); //enable general irq
}

/**
 * @brief      This function serves to send yihe rf data
 * @param[in]  addr - the data address
 * @param[in]  len  - the length of the data
 * @return     none
 */
void send_rf_data_yihe (void* addr, u32 len)
{
  volatile u8 tmp;

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (250);
}

/**
 * @brief      This function serves to send yihe peidui rf data
 * @param[in]  addr - the data address
 * @param[in]  len  - the length of the data
 * @return     none
 */
void send_rf_data_yihe_peidui (void* addr, u32 len)
{
  volatile u8 tmp;

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }
}
/**
 * @brief      This function serves to send ruierte rf data
 * @param[in]  addr - the data address
 * @param[in]  len  - the length of the data
 * @return     none
 */
void send_rf_data_ruierte (void* addr, u32 len)
{
  volatile u8 tmp;

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }
  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }
  WaitMs (90);
}

/**
 * @brief      This function serves to send kemu rf data
 * @param[in]  addr - the data address
 * @param[in]  len  - the length of the data
 * @return     none
 */
void send_rf_data_kemu (void* addr, u32 len)
{
  volatile u8 tmp;

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (250);
}

/**
 * @brief      This function serves to send kemu rf data
 * @param[in]  addr - the data address
 * @param[in]  len  - the length of the data
 * @return     none
 */
void send_rf_data_kemu_peidui (void* addr, u32 len)
{
  volatile u8 tmp;

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }

  WaitMs (10);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for (u8 j = 0; j < 4; j++) {//四个通道发送
      ESB_ModeSet (ESB_MODE_PTX);
      ESB_SetNewRFChannel (rf_channel_select[j]);
      WaitUs (100);
      tmp = ESB_WriteTxPayload (0, addr, len);
      if (tmp)
        ESB_PTXTrig ();
      WaitUs (400);
    }
  }
}

void send_rf_data_common (void* addr, u32 len)
{
  volatile u8 tmp;

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
	for (u8 j = 0; j < 4; j++) {//四个通道发送
	  ESB_ModeSet (ESB_MODE_PTX);
	  ESB_SetNewRFChannel (rf_channel_select[j]);
	  WaitUs (100);
	  tmp = ESB_WriteTxPayload (0, addr, len);
	  if (tmp)
		ESB_PTXTrig ();
	  WaitUs (400);
	}
  }

  WaitMs (5);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
	for (u8 j = 0; j < 4; j++) {//四个通道发送
	  ESB_ModeSet (ESB_MODE_PTX);
	  ESB_SetNewRFChannel (rf_channel_select[j]);
	  WaitUs (100);
	  tmp = ESB_WriteTxPayload (0, addr, len);
	  if (tmp)
		ESB_PTXTrig ();
	  WaitUs (400);
	}
  }

  WaitMs (5);

  for (u8 i = 0; i < 3; i++) {//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
	for (u8 j = 0; j < 4; j++) {//四个通道发送
	  ESB_ModeSet (ESB_MODE_PTX);
	  ESB_SetNewRFChannel (rf_channel_select[j]);
	  WaitUs (100);
	  tmp = ESB_WriteTxPayload (0, addr, len);
	  if (tmp)
		ESB_PTXTrig ();
	  WaitUs (400);
	}
  }

//  WaitMs (250);
}


/**
 * @brief      This function serves to receive rf data
 * @param[in]  addr - the data address
 * @return the data length
 */
u8 receive_rf_data (void* addr)
{
  u8 len;
  if (rx_flag) {
    rx_flag = 0;
    len =  ESB_ReadRxPayload (addr) & 0xff;
    ESB_PRXTrig ();
    return len;
  }else
    return 0;
}

/**
 * @brief      This function serves to receive peidui information from peer device
 * @param[in]  uid - the id of the local device
 * @return true if peidui ok
 */
#if defined (RUIERTE_NEW_RF)//瑞尔特触摸按键遥控器用的瑞尔特自己的协议，不是迪富的协议
bool receive_from_peer (u32 uid)
{
  u8 chksum;
  rf_package_t rx_buf;

  if (receive_rf_data (&rx_buf)) {
	if (rx_buf.send_data.len == 0x0d &&\
		rx_buf.send_data.addr == 0x19 &&\
		rx_buf.send_data.info == 0xa0 &&\
		rx_buf.send_data.id_h == ((uid & 0xff0000) >> 16) &&\
		rx_buf.send_data.id_m == ((uid & 0xff00) >> 8) &&\
		rx_buf.send_data.id_l == (uid & 0xff)) {

	  chksum = *((u8*)&rx_buf.send_data.addr) +\
			  *((u8*)&rx_buf.send_data.info) +\
			   *((u8*)&rx_buf.send_data.id_h) +\
			   *((u8*)&rx_buf.send_data.id_m) +\
			   *((u8*)&rx_buf.send_data.id_l) +\
			   *((u8*)&rx_buf.send_data.fun) +\
			  *((u8*)&rx_buf.send_data.para0) +\
			  *((u8*)&rx_buf.send_data.para1) +\
			  *((u8*)&rx_buf.send_data.para2) +\
			  *((u8*)&rx_buf.send_data.para3) +\
			  *((u8*)&rx_buf.send_data.para4) +\
			  *((u8*)&rx_buf.send_data.para5);

	  if (rx_buf.send_data.chksum == chksum && rx_buf.send_data.fun == 0xaa)
		return true;
	}
  }
  return false;
}
#else
bool receive_from_peer (u32 uid)
{
  rf_package_t rx_buf;

  if (receive_rf_data ((u8*)&rx_buf)) {
    if ((rx_buf.pid == uid || rx_buf.pid == 0) && rx_buf.control_key == 0x88)
      return true;
  }
  return false;
}
#endif
#endif
