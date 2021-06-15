#include "../../drivers.h"
#include "../common.h"
#include "mac_id.h"
#include "app.h"

static bool is_original_id()
{
  u8 data[8] = {0,0,0,0,0x12,0x34,0x56,0x78};
  u8 raw_data[8] = {0};

  flash_read_page(ID_Flash_Addr, sizeof(raw_data), (unsigned char *)raw_data);

  return (!memcmp((const void*)&data[4], (const void*)&raw_data[4], 4))? 0:1;
}

void read_id(void *addr, u8 len)
{
  flash_read_page(ID_Flash_Addr, len, (unsigned char *)addr);
}

void write_id(void *addr, u8 len)
{
  u8 data[8] = {0,0,0,0,0x12,0x34,0x56,0x78};

  memcpy(data, addr, len);

  flash_erase_sector(ID_Flash_Addr);

  flash_write_page(ID_Flash_Addr, sizeof(data), (unsigned char *)data);
}

void gen_random_id(u32 *data)
{
  random_generator_init();
  generateRandomNum(4, (unsigned char*)data);
}

void id_init()
{
  u8 data[8] = {0,0,0,0,0x12,0x34,0x56,0x78};
  if(is_original_id()){
    gen_random_id((u32*)data);
    write_id(data, sizeof(data));
  }
}
