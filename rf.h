#ifndef __RF_H__
#define __RF_H__

extern void send_rf_data(void *addr, u32 len);
extern u8 receive_rf_data(void *addr);
extern void rf_8359_set_tx(void);
extern void rf_8359_set_rx(void);
extern void decrease_rf_power_tx();
extern void send_rf_data_kemu(void *addr, u32 len);
extern void send_rf_data_ruierte(void *addr, u32 len);
extern void send_rf_data_yihe(void *addr, u32 len);
#endif
