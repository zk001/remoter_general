#include "../../drivers.h"
#include "../../esb_ll/esb_ll.h"
#include "rf.h"
#include "interrupt.h"

void rf_8359_set_rx()
{
  ESB_SetDatarate(ESB_DR_2M);
  ESB_SetOutputPower(ESB_RF_POWER_10DBM);
  ESB_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
  ESB_ClosePipe(ESB_PIPE_ALL);

  rf_acc_len_set(5);
  write_reg8(0x408, 0x71);
  write_reg8(0x409, 0x76);
  write_reg8(0x40a, 0x51);
  write_reg8(0x40b, 0x39);
  write_reg8(0x40c, 0x95);

  write_reg8(0x420, 38);

  ESB_OpenPipe(ESB_PIPE0, 1);

  ESB_ModeSet(ESB_MODE_PRX);
  ESB_SetNewRFChannel(140);

  ESB_TxSettleSet(149);
  ESB_RxSettleSet(80);

  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);
  //enable irq
  irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
  rf_irq_disable(FLD_RF_IRQ_ALL);
  rf_irq_enable(FLD_RF_IRQ_TX|FLD_RF_IRQ_TX_DS|FLD_RF_IRQ_RX_DR|FLD_RF_IRQ_RX);
  irq_enable(); //enable general irq

  ESB_PRXTrig();
}

void rf_8359_set_tx()
{
  ESB_SetDatarate(ESB_DR_2M);
  ESB_SetOutputPower(ESB_RF_POWER_10DBM);
  ESB_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
  ESB_ClosePipe(ESB_PIPE_ALL);

  rf_acc_len_set(5);
  write_reg8(0x408, 0x71);
  write_reg8(0x409, 0x76);
  write_reg8(0x40a, 0x51);
  write_reg8(0x40b, 0x39);
  write_reg8(0x40c, 0x95);

  write_reg8(0x420, 38);

  ESB_OpenPipe(ESB_PIPE0, 1);
  ESB_SetTXPipe(ESB_PIPE0);

  ESB_OpenPipe(ESB_PIPE0, 1);
  ESB_SetTXPipe(ESB_PIPE0);

  ESB_ModeSet(ESB_MODE_PTX);
  ESB_SetNewRFChannel(140);
  ESB_SetAutoRetry(0,150);  //5,150
  ESB_RxTimeoutSet(500);
  ESB_RxSettleSet(80);
  ESB_TxSettleSet(149);

  WaitUs(150);
  //configure irq
  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);

  irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
  rf_irq_disable(FLD_RF_IRQ_ALL);
  // rf_irq_enable(FLD_RF_IRQ_TX|FLD_RF_IRQ_TX_DS|FLD_RF_IRQ_RETRY_HIT|FLD_RF_IRQ_RX_DR);
  irq_enable(); //enable general irq
}

void decrease_rf_power_tx()
{
  ESB_SetDatarate(ESB_DR_2M);
  ESB_SetOutputPower(ESB_RF_POWER_M_20DBM);
  ESB_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
  ESB_ClosePipe(ESB_PIPE_ALL);

  rf_acc_len_set(5);
  write_reg8(0x408, 0x71);
  write_reg8(0x409, 0x76);
  write_reg8(0x40a, 0x51);
  write_reg8(0x40b, 0x39);
  write_reg8(0x40c, 0x95);

  write_reg8(0x420, 38);

  ESB_OpenPipe(ESB_PIPE0, 1);
  ESB_SetTXPipe(ESB_PIPE0);

  ESB_OpenPipe(ESB_PIPE0, 1);
  ESB_SetTXPipe(ESB_PIPE0);

  ESB_ModeSet(ESB_MODE_PTX);
  ESB_SetNewRFChannel(140);
  ESB_SetAutoRetry(0,150);  //5,150
  ESB_RxTimeoutSet(500);
  ESB_RxSettleSet(80);
  ESB_TxSettleSet(149);

  WaitUs(150);
  //configure irq
  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);

  irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
  rf_irq_disable(FLD_RF_IRQ_ALL);
  // rf_irq_enable(FLD_RF_IRQ_TX|FLD_RF_IRQ_TX_DS|FLD_RF_IRQ_RETRY_HIT|FLD_RF_IRQ_RX_DR);
  irq_enable(); //enable general irq
}

void send_rf_data_yihe(void *addr, u32 len)
{
  volatile unsigned char tmp;

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }

  WaitMs(10);

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }

  WaitMs(10);

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }

  WaitMs(250);
}

void send_rf_data_ruierte(void *addr, u32 len)
{
  volatile unsigned char tmp;

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }

  WaitMs(10);

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }
  WaitMs(100);
}

void send_rf_data_kemu(void *addr, u32 len)
{
  volatile unsigned char tmp;

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }

  WaitMs(10);

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }

  WaitMs(10);

  for(u8 i = 0; i < 3; i++){//一共发送3次  2.4*3=7.2ms发送一个循环，接收端8ms才切换信道，所以每次切换信道理论上都能接收端
    for(u8 j = 0; j < 4; j++){//四个通道发送
      ESB_ModeSet(ESB_MODE_PTX);
      ESB_SetNewRFChannel(rf_channel_select[j]);
      WaitUs(100);
      tmp = ESB_WriteTxPayload(0, addr, len);
      if(tmp)
        ESB_PTXTrig();
      WaitUs(400);
    }
  }

  WaitMs(250);
}

u8 receive_rf_data(void *addr)
{
  u8 len;
  if(rx_flag) {
    rx_flag = 0;
    len =  ESB_ReadRxPayload(addr) & 0xff;
    ESB_PRXTrig();
    return len;
  }else
    return 0;
}
