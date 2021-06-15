#ifndef __MAC_ID_H__
#define __MAC_ID_H__

extern void id_init();
extern void read_id(void *addr, u8 len);
extern void write_id(void *addr, u8 len);
extern void gen_random_id(u32 *data);

#endif
