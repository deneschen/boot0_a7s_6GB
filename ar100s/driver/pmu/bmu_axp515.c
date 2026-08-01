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

#include "bmu_i.h"

/**
 * only called by bmu common function.
 */

static void bmu_axp515_clear_irq(void)
{
	u8 devaddr = RSB_RTSADDR_AXP515;
	u8 data;
	u8 regaddr;

	save_state_flag(REC_SHUTDOWN | 0x400);

	data = 0x00;
	for (regaddr = AXP515_INTEN1; regaddr <= AXP515_INTEN6; regaddr++) {
		pmu_reg_write(&devaddr, &regaddr, &data, 1);
	}

	data = 0xFF;

	for (regaddr = AXP515_INTSTS1; regaddr <= AXP515_INTSTS6; regaddr++) {
		pmu_reg_write(&devaddr, &regaddr, &data, 1);
	}

	regaddr = AXP515_INTEN3;
	data = 0xc0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	regaddr = AXP515_INTEN4;
	data = 0xf0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	LOG("clear axp515 irq\n");
}

static void bmu_axp515_reset(void)
{
	save_state_flag(REC_SHUTDOWN | 0x401);

	bmu_axp515_clear_irq();

	LOG("reset axp515\n");
}

static void bmu_axp515_shutdown(void)
{
	u8 devaddr = RSB_RTSADDR_AXP515;
	u8 regaddr;
	u8 data;

	save_state_flag(REC_SHUTDOWN | 0x402);

	bmu_axp515_clear_irq();

	regaddr = AXP515_CC_GLOBAL_CTRL;
	data = 0x2;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	time_mdelay(100);
	data = 0x9;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	regaddr = AXP515_BATFET_DLY;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= ~(0x30);
	data |= 0x10;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	regaddr = AXP515_ILIMIT;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data |= 0x80;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	LOG("close axp515 batfet\n");
}

static s32 bmu_axp515_charging_vbus_det(void)
{
	u8 devaddr = RSB_RTSADDR_AXP515;
	u8 regaddr = AXP515_STATUS0;
	u8 val;

	save_state_flag(REC_SHUTDOWN | 0x501);

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* vbus presence */
	if ((val & 0x02) == 0x02) {
		return OK;
	}
	return FAIL;
}

static s32 bmu_axp515_is_exist(void)
{
	u8 devaddr = RSB_RTSADDR_AXP515;
	u8 regaddr = AXP515_IC_TYPE;
	u8 val = 0;

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* axp515 presence */
	val &= 0XCF;
	if (val == 0x46 || val == 0x49) {
		LOG("axp515 exist\n");
		return OK;
	}
	return FAIL;
}

bmu_ops_t bmu_axp515_ops = {
	.bmu_is_exist = bmu_axp515_is_exist,
	.bmu_shutdown = bmu_axp515_shutdown,
	.bmu_reset = bmu_axp515_reset,
	.bmu_charging_vbus_det = bmu_axp515_charging_vbus_det,
};

