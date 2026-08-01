/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                pmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pmu.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: power management unit.
* Update  : date                auther      ver     notes
*           2012-5-22 13:33:03  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "pmu_i.h"

#define SUNXI_CHARGING_FLAG_AXP8191 (0x08)
#define SUNXI_REBOOT_FLAG_AXP8191   (0x01)

/**
 * axp8191 voltages info table,
 * the index of table is voltage type.
 */
pmu_onoff_reg_bitmap_t axp8191_onoff_reg_bitmap[] = {
	/* reg_addr         offset */
	{AXP8191_DCDC_CTL1,      0},		/* AXP8191_POWER_DCDC1 */
	{AXP8191_DCDC_CTL1,      1},		/* AXP8191_POWER_DCDC2 */
	{AXP8191_DCDC_CTL1,      2},		/* AXP8191_POWER_DCDC3 */
	{AXP8191_DCDC_CTL1,      3},		/* AXP8191_POWER_DCDC4 */
	{AXP8191_DCDC_CTL1,      4},		/* AXP8191_POWER_DCDC5 */
	{AXP8191_DCDC_CTL1,      5},		/* AXP8191_POWER_DCDC6 */
	{AXP8191_DCDC_CTL1,      6},		/* AXP8191_POWER_DCDC7 */
	{AXP8191_DCDC_CTL1,      7},		/* AXP8191_POWER_DCDC8 */
	{AXP8191_DCDC_CTL2,      0},		/* AXP8191_POWER_DCDC9 */
	{AXP8191_DCDC_CTL2,      3},		/* AXP8191_POWER_DC1SW1 */
	{AXP8191_DCDC_CTL2,      4},		/* AXP8191_POWER_DC1SW2 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      0},	/* AXP8191_POWER_ALDO1 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      1},	/* AXP8191_POWER_ALDO2 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      2},	/* AXP8191_POWER_ALDO3 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      3},	/* AXP8191_POWER_ALDO4 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      4},	/* AXP8191_POWER_ALDO5 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      5},	/* AXP8191_POWER_ALDO6 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      6},	/* AXP8191_POWER_BLDO1 */
	{AXP8191_LDO_POWER_ON_OFF_CTL1,      7},	/* AXP8191_POWER_BLDO2 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      0},	/* AXP8191_POWER_BLDO3 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      1},	/* AXP8191_POWER_BLDO4 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      2},	/* AXP8191_POWER_BLDO5 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      3},	/* AXP8191_POWER_CLDO1 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      4},	/* AXP8191_POWER_CLDO2 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      5},	/* AXP8191_POWER_CLDO3 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      6},	/* AXP8191_POWER_CLDO4 */
	{AXP8191_LDO_POWER_ON_OFF_CTL2,      7},	/* AXP8191_POWER_CLDO5 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      0},	/* AXP8191_POWER_DLDO1 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      1},	/* AXP8191_POWER_DLDO2 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      2},	/* AXP8191_POWER_DLDO3 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      3},	/* AXP8191_POWER_DLDO4 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      4},	/* AXP8191_POWER_DLDO5 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      5},	/* AXP8191_POWER_DLDO6 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      6},	/* AXP8191_POWER_ELDO1 */
	{AXP8191_LDO_POWER_ON_OFF_CTL3,      7},	/* AXP8191_POWER_ELDO2 */
	{AXP8191_LDO_POWER_ON_OFF_CTL4,      0},	/* AXP8191_POWER_ELDO3 */
	{AXP8191_LDO_POWER_ON_OFF_CTL4,      1},	/* AXP8191_POWER_ELDO4 */
	{AXP8191_LDO_POWER_ON_OFF_CTL4,      2},	/* AXP8191_POWER_ELDO5 */
	{AXP8191_LDO_POWER_ON_OFF_CTL4,      3},	/* AXP8191_POWER_ELDO6 */
	{0x00,                               0},	/* AXP8191_POWER_RTCLDO */
	{0x00,                               0},	/* AXP8191_POWER_MAX */
};

/**
 * axp8191 specific function,
 * only called by pmu common function.
 */
 static u8 _axp8191_vbus_check(void)
{
	/* battery & vbus presence */
	if (is_bmu_exist() == TRUE) {
		if (bmu_charging_vbus_det() == OK) {
			LOG("%s:%d bmu_charging_vbus_det\n", __func__, __LINE__);
			return 1;
		}
	}
	return 0;
}

static void _axp8191_pmu_softset(u8 val)
{
	u8 devaddr = RSB_RTSADDR_AXP8191;
	u8 regaddr = AXP8191_POWER_DISABLE_POWER_DOWN_SEQUENCE;
	u8 data;

	save_state_flag(REC_SHUTDOWN | 0x300 | val);

	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data |= 1 << (7 - val);
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	if (val)
		LOG("reset system\n");
	else
		LOG("poweroff system\n");

	while (1)
		;
}


static void axp8191_pmu_shutdown(void)
{
	save_state_flag(REC_SHUTDOWN | 0x201);

	bmu_shutdown();
	_axp8191_pmu_softset(0);
}

static void axp8191_pmu_reset(void)
{
	u8 devaddr = RSB_RTSADDR_AXP8191;
	u8 regaddr = AXP8191_BUFFER0;
	u8 data = SUNXI_REBOOT_FLAG_AXP8191;

	save_state_flag(REC_SHUTDOWN | 0x202);

	bmu_reset();
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	_axp8191_pmu_softset(1);
}

static void axp8191_pmu_charging_reset(void)
{
	u8 devaddr = RSB_RTSADDR_AXP8191;
	u8 regaddr = AXP8191_BUFFER0;
	u8 val;

	save_state_flag(REC_SHUTDOWN | 0x203);
	val = _axp8191_vbus_check();
	if (val) {
		val = SUNXI_CHARGING_FLAG_AXP8191;
		pmu_reg_write(&devaddr, &regaddr, &val, 1);
		bmu_reset();
		_axp8191_pmu_softset(1);
	}
}

static s32 axp8191_pmu_set_voltage_state(u32 type, u32 state)
{
	u8 devaddr = RSB_RTSADDR_AXP8191;
	u8 regaddr;
	u8 data;
	u32 offset;

	regaddr = axp8191_onoff_reg_bitmap[type].regaddr;
	offset  = axp8191_onoff_reg_bitmap[type].offset;

	//read-modify-write
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= (~(1 << offset));
	data |= (state << offset);
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	if (state == POWER_VOL_ON) {
		//delay 1ms for open PMU output
		time_mdelay(1);
	}

	return OK;
}

pmu_ops_t pmu_axp8191_ops = {
	.pmu_shutdown = axp8191_pmu_shutdown,
	.pmu_reset = axp8191_pmu_reset,
	.pmu_charging_reset = axp8191_pmu_charging_reset,
	.pmu_set_voltage_state = axp8191_pmu_set_voltage_state,
};

