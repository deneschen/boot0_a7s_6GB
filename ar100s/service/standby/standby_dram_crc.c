/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                dram crc module
*
*                                    (c) Copyright 2012-2016, Superm Wu China
*                                             All Rights Reserved
*
* File    : dram_crc.c
* By      : Superm Wu
* Version : v1.0
* Date    : 2012-9-18
* Descript: set dram crc paras.
* Update  : date                auther      ver     notes
*           2012-9-18 19:08:23  Superm Wu   1.0     Create this file.
*********************************************************************************************************
*/
#include "include.h"

#define DRAM_CRC_MAX_LEN	(16U * 1024U * 1024U)

u32 dram_crc_enable  = 0;
u32 dram_crc_src     = 0x40000000;
u32 dram_crc_len     = (1024 * 1024);

s32 standby_set_dram_crc_paras(u32 enable, u32 src, u32 len)
{
	if (enable > 1)
		return -EINVAL;
	if (!enable) {
		dram_crc_enable = 0;
		return OK;
	}

	if ((src & (sizeof(u32) - 1)) || (len & (sizeof(u32) - 1)) ||
	    !len || len > DRAM_CRC_MAX_LEN || src < ARISC_DRAM_BASE ||
	    src >= ARISC_DRAM_END || len > ARISC_DRAM_END - src) {
		ERR("invalid dram CRC range: src=%x len=%x\n", src, len);
		return -EINVAL;
	}

	dram_crc_enable = enable;
	dram_crc_src    = src;
	dram_crc_len    = len;

	return OK;
}

s32 standby_dram_crc_enable(void)
{
	return dram_crc_enable;
}

u32 standby_dram_crc(void)
{
	u32 *pdata = (u32 *)(dram_crc_src);
	u32 *end = (u32 *)(dram_crc_src + dram_crc_len);
	u32 crc = 0xffffffffU;
	u32 value;
	u32 bit;

	INF("crc begin src:%x len:%x\n", dram_crc_src, dram_crc_len);
	while (pdata < end) {
		value = *pdata++;
		for (bit = 0; bit < 32; bit++) {
			u32 mask = 0U - ((crc ^ value) & 1U);

			crc = (crc >> 1) ^ (0xedb88320U & mask);
			value >>= 1;
		}
	}
	INF("crc finish...\n");

	return ~crc;
}
