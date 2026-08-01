/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                bmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : bmu.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: power management unit.
* Update  : date                auther      ver     notes
*           2012-5-22 13:33:03  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "include.h"

#if defined CFG_AXP515_USED
extern bmu_ops_t bmu_axp515_ops;
static bmu_ops_t *bmu_ops_p = &bmu_axp515_ops;
#else
#error "sun60iw2p1 not support no bmu"
#endif

static u32 bmu_exist = FALSE;

void bmu_shutdown(void)
{
	if (is_bmu_exist() == FALSE)
		return;

	if (bmu_ops_p->bmu_shutdown)
		bmu_ops_p->bmu_shutdown();
}

void bmu_reset(void)
{
	if (is_bmu_exist() == FALSE)
		return;

	if (bmu_ops_p->bmu_reset)
		bmu_ops_p->bmu_reset();
}

s32 bmu_charging_vbus_det(void)
{
	if (is_bmu_exist() == FALSE)
		return -1;

	if (bmu_ops_p->bmu_charging_vbus_det)
		return bmu_ops_p->bmu_charging_vbus_det();

	return -1;
}

s32 bmu_charging_reset(void)
{
	if (is_bmu_exist() == FALSE)
		return -1;

	if (bmu_ops_p->bmu_charging_reset)
		return bmu_ops_p->bmu_charging_reset();

	return -1;
}

s32 bmu_init(void)
{
	/* power_mode may parse from dts */
	if (bmu_ops_p->bmu_is_exist() < 0) {
		bmu_exist = FALSE;
		LOG("bmu is not exist\n");
	} else {
		bmu_exist = TRUE;
		LOG("bmu is exist\n");
	}
	return OK;
}

u32 is_bmu_exist(void)
{
	return bmu_exist;
}
