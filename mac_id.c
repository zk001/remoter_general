/********************************************************************************************************
 * @file     mac_id.c
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
#include "../../drivers.h"
#include "../common.h"
#include "mac_id.h"
#include "app.h"

/**
 * @brief      This function serves to checkout if the id is set or not
 * @param[in]  none
 * @return 1 if it is not set, otherwise it is set
 */
static bool is_original_id()
{
  u8 data[8] = {0,0,0,0,0x12,0x34,0x56,0x78};
  u8 raw_data[8] = {0};

  flash_read_page(ID_Flash_Addr, sizeof(raw_data), (u8*)raw_data);

  return (!memcmp((const void*)&data[4], (const void*)&raw_data[4], 4))? 0:1;
}

/**
 * @brief      This function serves to read id
 * @param[in]  adrr - the address of the flash, where to read id
 * @param[in]  len  - the id length
 * @return     none
 */
void read_id(void* addr, u8 len)
{
  flash_read_page(ID_Flash_Addr, len, (u8*)addr);
}

/**
 * @brief      This function serves to write id
 * @param[in]  adrr - the address of the flash, where to write id
 * @param[in]  len  - the id length
 * @return     none
 */
void write_id(void* addr, u8 len)
{
  u8 data[8] = {0,0,0,0,0x12,0x34,0x56,0x78};

  memcpy(data, addr, len);

  flash_erase_sector(ID_Flash_Addr);

  flash_write_page(ID_Flash_Addr, sizeof(data), (u8*)data);
}

/**
 * @brief      This function serves to generate random id
 * @param[out] data - the generated id
 * @return     none
 */
void gen_random_id(u32* data)
{
  random_generator_init();
  generateRandomNum(4, (u8*)data);
}

/**
 * @brief      This function serves to init id
 * @param[in]  none
 * @return     none
 */
void id_init()
{
  u8 data[8] = {0,0,0,0,0x12,0x34,0x56,0x78};
  if(is_original_id()){
    gen_random_id((u32*)data);
    write_id(data, sizeof(data));
  }
}
