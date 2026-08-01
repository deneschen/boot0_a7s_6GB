
/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                            super-standby module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : standby_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-14
* Descript: standby module internal header.
* Update  : date                auther      ver     notes
*           2012-5-14 8:56:44   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __STANDBY_I_H__
#define __STANDBY_I_H__

#include "include.h"

s32 standby_dram_crc_enable(void);
u32 standby_dram_crc(void);

typedef enum POWER_SCENE_FLAGS {
	TALKING_STANDBY_FLAG           = (1<<0x0),
	USB_STANDBY_FLAG               = (1<<0x1),
	MP3_STANDBY_FLAG               = (1<<0x2),
	SUPER_STANDBY_FLAG             = (1<<0x3),
	NORMAL_STANDBY_FLAG            = (1<<0x4),
	GPIO_STANDBY_FLAG              = (1<<0x5),
	MISC_STANDBY_FLAG              = (1<<0x6),
} power_scene_flags;

typedef enum {
	arisc_power_on = 0,
	arisc_power_retention = 1,
	arisc_power_off = 3,
} arisc_power_state_t;

typedef enum {
	arisc_system_shutdown = 0,
	arisc_system_reboot = 1,
	arisc_system_reset = 2,
	arisc_uboot_shutdown = 3,
} arisc_system_state_t;

enum start_mode {
	COLD_START = 0x0,
	FAKE_POWEROFF = 0x2,
	FAKE_POWEROFF_E = 0xe,	/* compatible with old platform, such as sun50iw9 */
	BOOT_NORMAL = 0xf
};

typedef struct {
	char name[8];
	uint32_t mux;
	uint32_t data;
	uint32_t drive;
	uint32_t pull;
	uint32_t eint;
} fake_poweroff_pinctrl_t;

extern u32 before_crc;
extern u32 after_crc;

#endif  /*__STANDBY_I_H__*/
