#ifndef __MAC_ID_H__
#define __MAC_ID_H__

#define CHECK_FLAG 0x12345678
#define CHECK_FLAG_LEN 4
#define CHECK_FLAG_ADDR ID_Flash_Addr + 4

extern void id_init();
extern void read_id(void *addr, u8 len);
extern void write_id(void *addr, u8 len);
extern void gen_random_id(u32 *data);

#endif
