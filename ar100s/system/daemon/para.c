/*
*********************************************************************************************************
*                                                AR200 SYSTEM
*                                     AR200 Software System Develop Kits
*                                                startup module
*
*                                    (c) Copyright 2012-2016, superm China
*                                             All Rights Reserved
*
* File    : para.c
* By      : superm
* Version : v1.0
* Date    : 2012-5-13
* Descript: startup module.
* Update  : date                auther      ver     notes
*           2013-5-23 9:32:20   superm      1.0     Create this file.
*********************************************************************************************************
*/

#include "daemon_i.h"
#include "para.h"
#include <libfdt.h>

uint32_t *platform_dram_para;

/* if no define dram init */
uint32_t  __attribute__((weak)) *dram_dts_parse(void)
{
	return NULL;
};

struct arisc_para arisc_para __attribute__ ((section("dts_paras"))) = {
	.para_info      = 3,
	.para_magic     = 0x73555043,
};

void arisc_para_init(void)
{
}

void set_paras(void)
{
}

s32 platform_dts_parse_late(void)
{
	platform_dram_para = dram_dts_parse();
	return platform_dram_para ? OK : -EFAIL;
}
