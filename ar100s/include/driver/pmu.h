/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                pmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pmu.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-8
* Descript: power management unit module public header.
* Update  : date                auther      ver     notes
*           2012-5-8 8:52:39    Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __PMU_H__
#define __PMU_H__

#include "include.h"

#define NMI_INT_TYPE_PMU (0)
#define NMI_INT_TYPE_RTC (1)
#define NMI_INT_TYPE_PMU_OFFSET (0x1 << NMI_INT_TYPE_PMU)
#define NMI_INT_TYPE_RTC_OFFSET (0x1 << NMI_INT_TYPE_RTC)

#define PMU_INV_ADDR            (0x00)

#define PMU_STEP_DELAY_US               (16)

/*PMU chip ID register*/
#define PMU_ID_REG        (0x03)
#define PMU_ID_REG_EXT        (0x0e)
#define PMU_VER_REG_EXT       (0x0f)


/* IC version */
#define AW2101_IC_VERSION       (0x17)
#define AW1669_IC_VERSION       (0x12)
#define AW1660_IC_VERSION       (0x11)
#define AW1736_IC_VERSION       (0x14)
#define AXP2202_IC_VERSION      (0x02)
#define AW1657_IC_VERSION       (0x10)

/* AW1660 registers list */
#define AW1660_PWR_SRC_STA      (0x00)
#define AW1660_PWRM_CHGR_STA    (0x01)
#define AW1660_PWR_REASON       (0x02)
#define AW1660_IC_NO_REG        (0x03)
#define AW1660_DATA_BUFF1       (0x04)
#define AW1660_DATA_BUFF2       (0x05)
#define AW1660_DATA_BUFF3       (0x06)
#define AW1660_DATA_BUFF4       (0x07)
#define AW1660_DATA_BUFF5       (0x08)
#define AW1660_DATA_BUFF6       (0x09)
#define AW1660_DATA_BUFF7       (0x0A)
#define AW1660_DATA_BUFF8       (0x0B)
#define AW1660_DATA_BUFF9       (0x0C)
#define AW1660_DATA_BUFFA       (0x0D)
#define AW1660_DATA_BUFFB       (0x0E)
#define AW1660_DATA_BUFFC       (0x0F)
#define AW1660_OUTPUT_PWR_CTRL0 (0x10)
#define AW1660_OUTPUT_PWR_CTRL1 (0x12)
#define AW1660_ALDO123_OP_MODE  (0x13)
#define AW1660_ON_OFF_SYN_CTRL  (0x14)
#define AW1660_DLDO1_VOL_CTRL   (0x15)
#define AW1660_DLDO2_VOL_CTRL   (0x16)
#define AW1660_DLDO3_VOL_CTRL   (0x17)
#define AW1660_DLDO4_VOL_CTRL   (0x18)
#define AW1660_ELDO1_VOL_CTRL   (0x19)
#define AW1660_ELDO2_VOL_CTRL   (0x1A)
#define AW1660_ELDO3_VOL_CTRL   (0x1B)
#define AW1660_FLDO1_VOL_CTRL   (0x1C)
#define AW1660_FLDO23_VOL_CTRL  (0x1D)
#define AW1660_DCDC1_VOL_CTRL   (0x20)
#define AW1660_DCDC2_VOL_CTRL   (0x21)
#define AW1660_DCDC3_VOL_CTRL   (0x22)
#define AW1660_DCDC4_VOL_CTRL   (0x23)
#define AW1660_DCDC5_VOL_CTRL   (0x24)
#define AW1660_DCDC6_VOL_CTRL   (0x25)
#define AW1660_DCDC7_VOL_CTRL   (0x26)
#define AW1660_DCDC_DVM_CTRL    (0x27)
#define AW1660_ALDO1_VOL_CTRL   (0x28)
#define AW1660_ALDO2_VOL_CTRL   (0x29)
#define AW1660_ALDO3_VOL_CTRL   (0x2a)
#define AW1660_BC_MOD_GLB_REG   (0x2c)
#define AW1660_BC_MOD_VBUS_REG  (0x2d)
#define AW1660_BC_USB_STA_REG   (0x2e)
#define AW1660_BC_DET_STA_REG   (0x2f)
#define AW1660_VBUS_CTRL        (0x30)
#define AW1660_PWR_WAKEUP_CTRL  (0x31)
#define AW1660_PWR_OFF_CTRL     (0x32)
#define AW1660_CHARGER_CTRL1    (0x33)
#define AW1660_CHARGER_CTRL2    (0x34)
#define AW1660_CHARGER_CTRL3    (0x35)
#define AW1660_POK_SET          (0x36)
#define AW1660_POK_PWR_OFF_SET  (0x37)
#define AW1660_VLTF_CHARGE_SET  (0x38)
#define AW1660_VHTF_CHARGE_SET  (0x39)
#define AW1660_ACIN_PATH_CTRL   (0x3a)
#define AW1660_DCDC_FREQ_SET    (0x3b)
#define AW1660_VLTF_WORK_SET    (0x3c)
#define AW1660_VHTF_WORK_SET    (0x3d)
#define AW1660_IF_MODE_SEL      (0x3e)
#define AW1660_IRQ_ENABLE1      (0x40)
#define AW1660_IRQ_ENABLE2      (0x41)
#define AW1660_IRQ_ENABLE3      (0x42)
#define AW1660_IRQ_ENABLE4      (0x43)
#define AW1660_IRQ_ENABLE5      (0x44)
#define AW1660_IRQ_ENABLE6      (0x45)
#define AW1660_IRQ_STATUS1      (0x48)
#define AW1660_IRQ_STATUS2      (0x49)
#define AW1660_IRQ_STATUS3      (0x4a)
#define AW1660_IRQ_STATUS4      (0x4b)
#define AW1660_IRQ_STATUS5      (0x4c)
#define AW1660_IRQ_STATUS6      (0x4d)
#define AW1660_IC_TEMP_HIG      (0x56)
#define AW1660_IC_TEMP_LOW      (0x57)
#define AW1660_CHAR_CURR_STA1   (0x7a)
#define AW1660_CHAR_CURR_STA2   (0x7b)
#define AW1660_GPIOLDO0_CTRL    (0x90)
#define AW1660_GPIOLDO0_VOL     (0x91)
#define AW1660_GPIOLDO1_CTRL    (0x92)
#define AW1660_GPIOLDO1_VOL     (0x93)
#define AW1660_BAT_QUANTITY     (0xb9)
#define AW1660_INVALID_ADDR     (0x100)

/* AW1736 registers list */
#define AW1736_PWR_ON_SRC_STA   (0x00)
#define AW1736_PWR_OFF_SRC_STA  (0x01)
#define AW1736_IC_NO_REG        (0x03)
#define AW1736_DATA_BUFF1       (0x04)
#define AW1736_DATA_BUFF2       (0x05)
#define AW1736_DATA_BUFF3       (0x06)
#define AW1736_DATA_BUFF4       (0x07)
#define AW1736_OUTPUT_PWR_CTRL0 (0x10)
#define AW1736_OUTPUT_PWR_CTRL1 (0x11)
#define AW1736_OUTPUT_PWR_CTRL2 (0x12)
#define AW1736_DCDC1_VOL_CTRL   (0x13)
#define AW1736_DCDC2_VOL_CTRL   (0x14)
#define AW1736_DCDC3_VOL_CTRL   (0x15)
#define AW1736_DCDC4_VOL_CTRL   (0x16)
#define AW1736_DCDC5_VOL_CTRL   (0x17)
#define AW1736_DCDC6_VOL_CTRL   (0x18)
#define AW1736_ALDO1_VOL_CTRL   (0x19)
#define AW1736_DCDC_CTRL1       (0x1A)
#define AW1736_DCDC_CTRL2       (0x1B)
#define AW1736_DCDC_CTRL3       (0x1C)
#define AW1736_DCDC_FREQ_SET    (0x1D)
#define AW1736_OUT_MONITOR_CTL  (0x1E)
#define AW1736_IRQ_PWROK_VOFF   (0x1F)
#define AW1736_ALDO2_VOL_CTRL   (0x20)
#define AW1736_ALDO3_VOL_CTRL   (0x21)
#define AW1736_ALDO4_VOL_CTRL   (0x22)
#define AW1736_ALDO5_VOL_CTRL   (0x23)
#define AW1736_BLDO1_VOL_CTRL   (0x24)
#define AW1736_BLDO2_VOL_CTRL   (0x25)
#define AW1736_BLDO3_VOL_CTRL   (0x26)
#define AW1736_BLDO4_VOL_CTRL   (0x27)
#define AW1736_BLDO5_VOL_CTRL   (0x28)
#define AW1736_CLDO1_VOL_CTRL   (0x29)
#define AW1736_CLDO2_VOL_CTRL   (0x2A)
#define AW1736_CLDO3_VOL_CTRL   (0x2B)
#define AW1736_CLDO4_GPIO2_CTRL (0x2C)
#define AW1736_CLDO4_VOL_CTRL   (0x2D)
#define AW1736_CPUSLDO_VOL_CTRL (0x2E)
#define AW1736_WAKEUP_CTL_OCIRQ (0x31)
#define AW1736_PWR_DISABLE_DOWN (0x32)
#define AW1736_POK_SET          (0x36)
#define AW1736_INT_MODE_SELECT  (0x3E)
#define AW1736_IRQ_ENABLE1      (0x40)
#define AW1736_IRQ_ENABLE2      (0x41)
#define AW1736_IRQ_STATUS1      (0x48)
#define AW1736_IRQ_STATUS2      (0x49)
#define AW1736_INVALID_ADDR     (0x100)


/* AXP2202 registers list */
#define AXP2202_COMM_STAT0      (0x00)
#define AXP2202_COMM_STAT1      (0x01)
#define AXP2202_CHIP_ID         (0x03)
#define AXP2202_CHIP_VER        (0x04)
#define AXP2202_BC_DECT         (0x05)
#define AXP2202_ILIM_TYPE       (0x06)
#define AXP2202_COMM_FAULT      (0x08)
#define AXP2202_ICO_CFG         (0X0A)
#define AXP2202_CLK_EN          (0X0B)
#define AXP2202_COMM_CFG        (0X10)
#define AXP2202_BATFET_CTRL     (0X12)
#define AXP2202_RBFET_CTRL      (0X13)
#define AXP2202_DIE_TEMP_CFG    (0X14)
#define AXP2202_VSYS_MIN        (0x15)
#define AXP2202_VINDPM_CFG      (0x16)
#define AXP2202_IIN_LIM         (0x17)
#define AXP2202_RESET_CFG       (0x18)
#define AXP2202_MODULE_EN       (0x19)
#define AXP2202_WATCHDOG_CFG    (0x1A)
#define AXP2202_GAUGE_THLD      (0x1B)
#define AXP2202_GPIO_CTRL       (0x1C)
#define AXP2202_LOW_POWER_CFG   (0x1D)
#define AXP2202_BST_CFG0        (0x1E)
#define AXP2202_BST_CFG1        (0x1F)
#define AXP2202_PWRON_STAT      (0x20)
#define AXP2202_PWROFF_STAT     (0x21)
#define AXP2202_PWROFF_EN       (0x22)
#define AXP2202_DCDC_PWROFF_EN  (0x23)
#define AXP2202_PWR_TIME_CTRL   (0x24)
#define AXP2202_SLEEP_CFG       (0x25)
#define AXP2202_PONLEVEL        (0x26)
#define AXP2202_SOFT_PWROFF     (0x27)
#define AXP2202_AUTO_SLP_MAP0   (0x28)
#define AXP2202_AUTO_SLP_MAP1   (0x29)
#define AXP2202_AUTO_SLP_MAP2   (0x2A)
#define AXP2202_FAST_PWRON_CFG0 (0x2B)
#define AXP2202_FAST_PWRON_CFG1 (0x2C)
#define AXP2202_FAST_PWRON_CFG2 (0x2D)
#define AXP2202_FAST_PWRON_CFG3 (0x2E)
#define AXP2202_FAST_PWRON_CFG4 (0x2F)
#define AXP2202_I2C_CFG         (0x30)
#define AXP2202_BUS_MODE_SEL    (0x3E)
#define AXP2202_INTEN1          (0x40)
#define AXP2202_INTEN2          (0x41)
#define AXP2202_INTEN3          (0x42)
#define AXP2202_INTEN4          (0x43)
#define AXP2202_INTEN5          (0x44)
#define AXP2202_INTSTS1         (0x48)
#define AXP2202_INTSTS2         (0x49)
#define AXP2202_INTSTS3         (0x4A)
#define AXP2202_INTSTS4         (0x4B)
#define AXP2202_INTSTS5         (0x4C)
#define AXP2202_TS_CFG          (0x50)
#define AXP2202_TS_HYSHL2H      (0x52)
#define AXP2202_TS_HYSH2L       (0x53)
#define AXP2202_VLTF_CHG        (0x54)
#define AXP2202_VHTF_CHG        (0x55)
#define AXP2202_VLTF_WORK       (0x56)
#define AXP2202_VHTF_WORK       (0x57)
#define AXP2202_JEITA_CFG       (0x58)
#define AXP2202_JEITA_CV_CFG    (0x59)
#define AXP2202_JEITA_COOL      (0x5A)
#define AXP2202_JEITA_WARM      (0x5B)
#define AXP2202_TS_CFG_DATA_H   (0x5C)
#define AXP2202_TS_CFG_DATA_L   (0x5D)
#define AXP2202_RECHG_CFG       (0x60)
#define AXP2202_IPRECHG_CFG     (0x61)
#define AXP2202_ICC_CFG         (0x62)
#define AXP2202_ITERM_CFG       (0x63)
#define AXP2202_VTERM_CFG       (0x64)
#define AXP2202_TREGU_THLD      (0x65)
#define AXP2202_CHG_FREQ        (0x66)
#define AXP2202_CHG_TMR_CFG     (0x67)
#define AXP2202_BAT_DET         (0x68)
#define AXP2202_IR_COMP         (0x69)
#define AXP2202_BTN_CHG_CFG     (0x6A)
#define AXP2202_SW_TEST_CFG     (0x6B)
#define AXP2202_CHGLED_CFG      (0x70)
#define AXP2202_LOW_NUM         (0x72)
#define AXP2202_HIGH_NUM        (0x73)
#define AXP2202_TRANS_NUM       (0x74)
#define AXP2202_DUTY_STEP       (0x76)
#define AXP2202_DUTY_MIN        (0x77)
#define AXP2202_PWN_PERIOD      (0x78)
#define AXP2202_DCDC_CFG0       (0x80)
#define AXP2202_DCDC_CFG1       (0x81)
#define AXP2202_DCDC_CFG2       (0x82)
#define AXP2202_DCDC1_CFG       (0x83)
#define AXP2202_DCDC2_CFG       (0x84)
#define AXP2202_DCDC3_CFG       (0x85)
#define AXP2202_DCDC4_CFG       (0x86)
#define AXP2202_DVM_STAT        (0x87)
#define AXP2202_DCDC_OC_CFG     (0x88)
#define AXP2202_DCDC_VDSDT_ADJ  (0x89)
#define AXP2202_LDO_EN_CFG0     (0x90)
#define AXP2202_LDO_EN_CFG1     (0x91)
#define AXP2202_ALDO1_CFG       (0x93)
#define AXP2202_ALDO2_CFG       (0x94)
#define AXP2202_ALDO3_CFG       (0x95)
#define AXP2202_ALDO4_CFG       (0x96)
#define AXP2202_BLDO1_CFG       (0x97)
#define AXP2202_BLDO2_CFG       (0x98)
#define AXP2202_BLDO3_CFG       (0x99)
#define AXP2202_BLDO4_CFG       (0x9A)
#define AXP2202_CLDO1_CFG       (0x9B)
#define AXP2202_CLDO2_CFG       (0x9C)
#define AXP2202_CLDO3_CFG       (0x9D)
#define AXP2202_CLDO4_CFG       (0x9E)
#define AXP2202_CPUSLDO_CFG      (0x9F)
#define AXP2202_IP_VER          (0xA0)
#define AXP2202_GAUGE_BROM      (0xA1)
#define AXP2202_GAUGE_CONFIG    (0xA2)
#define AXP2202_GAUGE_TEMP      (0xA3)
#define AXP2202_GAUGE_SOC       (0xA4)
#define AXP2202_TIME2EMPTY_H    (0xA6)
#define AXP2202_TIME2EMPTY_L    (0xA7)
#define AXP2202_TIME2FULL_H     (0xA8)
#define AXP2202_TIME2FULL_L     (0xA9)
#define AXP2202_FW_VERSION      (0xAB)
#define AXP2202_INT0_FLAG       (0xAC)
#define AXP2202_COUTER_PERIOD   (0xAD)
#define AXP2202_FG_ADDR         (0xB0)
#define AXP2202_FG_DATA_H       (0xB2)
#define AXP2202_FG_DATA_L       (0xB3)
#define AXP2202_RAM_MBIST       (0xB4)
#define AXP2202_ROM_TEST        (0xB5)
#define AXP2202_ROM_TEST_RT0    (0xB6)
#define AXP2202_ROM_TEST_RT1    (0xB7)
#define AXP2202_ROM_TEST_RT2    (0xB8)
#define AXP2202_ROM_TEST_RT3    (0xB9)
#define AXP2202_WD_CLR_DIS      (0xBA)
#define AXP2202_ADC_CH_EN0      (0xC0)
#define AXP2202_ADC_CH_EN1      (0xC1)
#define AXP2202_ADC_CH_EN2      (0xC2)
#define AXP2202_ADC_CH_EN3      (0xC3)
#define AXP2202_VBAT_H          (0xC4)
#define AXP2202_VBAT_L          (0xC5)
#define AXP2202_VBUS_H          (0xC6)
#define AXP2202_VBUS_L          (0xC7)
#define AXP2202_VSYS_H          (0xC8)
#define AXP2202_VSYS_L          (0xC9)
#define AXP2202_ICHG_H          (0xCA)
#define AXP2202_ICHG_L          (0xCB)
#define AXP2202_CHG_DBG_SEL     (0xCC)
#define AXP2202_ADC_DATA_SEL    (0xCD)
#define AXP2202_ADC_DATA_H      (0xCE)
#define AXP2202_ADC_DATA_L      (0xCF)
#define AXP2202_BC_CFG0         (0xD0)
#define AXP2202_BC_CFG1         (0xD1)
#define AXP2202_BC_CFG2         (0xD2)
#define AXP2202_BC_CFG3         (0xD3)
#define AXP2202_CC_VERSION      (0xE0)
#define AXP2202_CC_GLB_CTRL     (0xE1)
#define AXP2202_CC_LP_CTRL      (0xE2)
#define AXP2202_CC_MODE_CTRL    (0xE3)
#define AXP2202_CC_TGL_CTRL     (0xE4)
#define AXP2202_CC_FORCE_STAT   (0xE5)
#define AXP2202_CC_ANA_CTRL     (0xE6)
#define AXP2202_CC_STAT0        (0xE7)
#define AXP2202_CC_STAT1        (0xE8)
#define AXP2202_CC_STAT2        (0xE9)
#define AXP2202_CC_STAT3        (0xEA)
#define AXP2202_CC_STAT4        (0xEB)
#define AXP2202_CC_ANA_STAT0    (0xEC)
#define AXP2202_CC_ANA_STAT1    (0xED)
#define AXP2202_CC_ANA_STAT2    (0xEE)
#define AXP2202_BUFFER0         (0xF0)
#define AXP2202_BUFFERC         (0xFF)
#define AXP2202_INVALID_ADDR    (0x100)


/* AW1657 registers list */
#define AW1657_PWR_SRC_STA      (0x00)
#define AW1657_IC_NO_REG        (0x03)
#define AW1657_DATA_BUFF1       (0x04)
#define AW1657_DATA_BUFF2       (0x05)
#define AW1657_DATA_BUFF3       (0x06)
#define AW1657_DATA_BUFF4       (0x07)
#define AW1657_OUTPUT_PWR_CTRL1 (0x10)
#define AW1657_OUTPUT_PWR_CTRL2 (0x11)
#define AW1657_DCDCA_VOL_CTRL   (0x12)
#define AW1657_DCDCB_VOL_CTRL   (0x13)
#define AW1657_DCDCC_VOL_CTRL   (0x14)
#define AW1657_DCDCD_VOL_CTRL   (0x15)
#define AW1657_DCDCE_VOL_CTRL   (0x16)
#define AW1657_ALDO1_VOL_CTRL   (0x17)
#define AW1657_ALDO2_VOL_CTRL   (0x18)
#define AW1657_ALDO3_VOL_CTRL   (0x19)
#define AW1657_DCDC_MODE_CTRL1  (0x1A)
#define AW1657_DCDC_MODE_CTRL2  (0x1B)
#define AW1657_DCDC_FRE_SET     (0x1C)
#define AW1657_OUTPUT_MONI_CTRL (0x1D)
#define AW1657_IRQ_POK_SET      (0x1F)
#define AW1657_BLDO1_VOL_CTRL   (0x20)
#define AW1657_BLDO2_VOL_CTRL   (0x21)
#define AW1657_BLDO3_VOL_CTRL   (0x22)
#define AW1657_BLDO4_VOL_CTRL   (0x23)
#define AW1657_CLDO1_VOL_CTRL   (0x24)
#define AW1657_CLDO2_VOL_CTRL   (0x25)
#define AW1657_CLDO3_VOL_CTRL   (0x26)
#define AW1657_PWR_WAKEUP_CTRL  (0x31)
#define AW1657_PWR_DOWN_SEQ     (0x32)
#define AW1657_WAKEUP_FUNC_SET  (0x35)
#define AW1657_POK_SET          (0x36)
#define AW1657_IF_MODE_SEL      (0x3E)
#define AW1657_SP_CTRL_REG      (0x3F)
#define AW1657_IRQ_ENABLE1      (0x40)
#define AW1657_IRQ_ENABLE2      (0x41)
#define AW1657_IRQ_STATUS1      (0x48)
#define AW1657_IRQ_STATUS2      (0x49)
#define AW1657_INVALID_ADDR     (0x100)

/* List of registers for eta6973 */
#define ETA6973_REG_00		(0x00)
#define ETA6973_REG_01		(0x01)
#define ETA6973_REG_02		(0x02)
#define ETA6973_REG_03		(0x03)
#define ETA6973_REG_04		(0x04)
#define ETA6973_REG_05		(0x05)
#define ETA6973_REG_06		(0x06)
#define ETA6973_REG_07		(0x07)
#define ETA6973_REG_08		(0x08)
#define ETA6973_REG_09		(0x09)
#define ETA6973_REG_0A		(0x0A)
#define ETA6973_REG_0B		(0x0B)

/* List of registers for axp519 */
#define AXP519_CHIP_ID					0x01
#define AXP519_IRQ_EN0					0x02
#define AXP519_IRQ_EN1					0x03
#define AXP519_IRQ0						0x04
#define AXP519_IRQ1						0x05
#define AXP519_SYS_STATUS				0x06
#define AXP519_WORK_CFG					0x0d
#define AXP519_ADC_CFG					0x10
#define AXP519_ADC_H					0x11
#define AXP519_ADC_L					0x12
#define AXP519_DISCHG_SET1				0x20
#define AXP519_VBUSOUT_VOL_SET_H			0x23
#define AXP519_CHG_SET1					0x30
#define AXP519_CHG_SET3					0x32
#define AXP519_VTERM_CFG_H				0x34
#define AXP519_OVP_SET					0x38
#define AXP519_LINLIM_SET				0x39
#define AXP519_VBATLIM_SET				0x3a
#define AXP519_NTC_SET1					0x43
#define AXP519_NTC_SET2					0x44
#define AXP519_END						0xff

/* List of registers for tcs4838 */
#define TCS4838_VSEL0		0x00
#define TCS4838_VSEL1		0x01
#define TCS4838_CTRL		0x02
#define TCS4838_ID1		0x03
#define TCS4838_ID2		0x04
#define TCS4838_TCS_PGOOD	0x05

/* List of registers for sy8827g */
#define SY8827G_VSEL0		0x00
#define SY8827G_VSEL1		0x01
#define SY8827G_CTRL		0x02
#define SY8827G_ID1		0x03
#define SY8827G_ID2		0x04
#define SY8827G_PGOOD		0x05

/* define AXP1530 REGISTER */
#define AXP1530_POWER_ON_SOURCE_INDIVATION			(0x00)
#define AXP1530_POWER_OFF_SOURCE_INDIVATION			(0x01)
#define AXP1530_VERSION						(0x03)
#define AXP1530_OUTPUT_POWER_ON_OFF_CTL				(0x10)
#define AXP1530_DCDC_DVM_PWM_CTL				(0x12)
#define AXP1530_DC1OUT_VOL					(0x13)
#define AXP1530_DC2OUT_VOL					(0x14)
#define AXP1530_DC3OUT_VOL					(0x15)
#define AXP1530_ALDO1OUT_VOL					(0x16)
#define AXP1530_DLDO1OUT_VOL					(0x17)
#define AXP1530_POWER_DOMN_SEQUENCE				(0x1A)
#define AXP1530_PWROK_VOFF_SERT					(0x1B)
#define AXP1530_POWER_WAKEUP_CTL				(0x1C)
#define AXP1530_OUTPUT_MONITOR_CONTROL				(0x1D)
#define AXP1530_POK_SET						(0x1E)
#define AXP1530_IRQ_ENABLE					(0x20)
#define AXP1530_IRQ_STATUS					(0x21)
#define AXP1530_WRITE_LOCK					(0x70)
#define AXP1530_ERROR_MANAGEMENT				(0x71)
#define AXP1530_DCDC1_2_POWER_ON_DEFAULT_SET			(0x80)
#define AXP1530_DCDC3_ALDO1_POWER_ON_DEFAULT_SET		(0x81)

/* define AXP8191 REGISTER */
#define   AXP8191_IC_TYPE                       (0x03)
#define   AXP8191_BUFFER0                       (0x04)
#define   AXP8191_CHIP_ID                       (0x0E)
#define   AXP8191_CHIP_VER                      (0x0F)
#define   AXP8191_DCDC_CTL1                     (0x10)
#define   AXP8191_DCDC_CTL2                     (0x11)

#define   AXP8191_DC1OUT_VOL                    (0x12)
#define   AXP8191_DC2OUT_VOL                    (0x13)
#define   AXP8191_DC3OUT_VOL                    (0x14)
#define   AXP8191_DC4OUT_VOL                    (0x15)
#define   AXP8191_DC5OUT_VOL                    (0x16)
#define   AXP8191_DC6OUT_VOL                    (0x17)
#define   AXP8191_DC7OUT_VOL                    (0x18)
#define   AXP8191_DC8OUT_VOL                    (0x19)
#define   AXP8191_DC9OUT_VOL                    (0x1A)

#define   AXP8191_DCDC_MODE_CTL1                (0x1B)
#define   AXP8191_DCDC_MODE_CTL2                (0x1C)
#define   AXP8191_DCDC_MODE_CTL3                (0x1D)
#define   AXP8191_DCDC_MODE_CTL4                (0x1E)

#define   AXP8191_DC8SET_STATUS                 (0x1F)

#define   AXP8191_LDO_POWER_ON_OFF_CTL1         (0x20)
#define   AXP8191_LDO_POWER_ON_OFF_CTL2         (0x21)
#define   AXP8191_LDO_POWER_ON_OFF_CTL3         (0x22)
#define   AXP8191_LDO_POWER_ON_OFF_CTL4         (0x23)

#define   AXP8191_ALDO1OUT_VOL                  (0x24)
#define   AXP8191_ALDO2OUT_VOL                  (0x25)
#define   AXP8191_ALDO3OUT_VOL                  (0x26)
#define   AXP8191_ALDO4OUT_VOL                  (0x27)
#define   AXP8191_ALDO5OUT_VOL                  (0x28)
#define   AXP8191_ALDO6OUT_VOL                  (0x29)

#define   AXP8191_BLDO1OUT_VOL                  (0x2A)
#define   AXP8191_BLDO2OUT_VOL                  (0x2B)
#define   AXP8191_BLDO3OUT_VOL                  (0x2C)
#define   AXP8191_BLDO4OUT_VOL                  (0x2D)
#define   AXP8191_BLDO5OUT_VOL                  (0x2E)

#define   AXP8191_CLDO1OUT_VOL                  (0x2F)
#define   AXP8191_CLDO2OUT_VOL                  (0x30)
#define   AXP8191_CLDO3OUT_VOL                  (0x31)
#define   AXP8191_CLDO4OUT_VOL                  (0x32)
#define   AXP8191_CLDO5OUT_VOL                  (0x33)

#define   AXP8191_DLDO1OUT_VOL                  (0x34)
#define   AXP8191_DLDO2OUT_VOL                  (0x35)
#define   AXP8191_DLDO3OUT_VOL                  (0x36)
#define   AXP8191_DLDO4OUT_VOL                  (0x37)
#define   AXP8191_DLDO5OUT_VOL                  (0x38)
#define   AXP8191_DLDO6OUT_VOL                  (0x39)

#define   AXP8191_ELDO1OUT_VOL                  (0x3A)
#define   AXP8191_ELDO2OUT_VOL                  (0x3B)
#define   AXP8191_ELDO3OUT_VOL                  (0x3C)
#define   AXP8191_ELDO4OUT_VOL                  (0x3D)
#define   AXP8191_ELDO5OUT_VOL                  (0x3E)
#define   AXP8191_ELDO6OUT_VOL                  (0x3F)

#define   AXP8191_IRQ_ENABLE1                   (0x40)
#define   AXP8191_IRQ_ENABLE2                   (0x41)
#define   AXP8191_IRQ_ENABLE3                   (0x42)
#define   AXP8191_IRQ_ENABLE4                   (0x43)

#define   AXP8191_IRQ_STATUS1                   (0x48)
#define   AXP8191_IRQ_STATUS2                   (0x49)
#define   AXP8191_IRQ_STATUS3                   (0x4A)
#define   AXP8191_IRQ_STATUS4                   (0x4B)

#define   AXP8191_POWER_ON_OFF_SOURCE_INDIVATION        (0x50)
#define   AXP8191_POWEROFF_SOURCE_INDIVATION            (0x51)
#define   AXP8191_POWEROFF_SOURCE_EN1                   (0x52)
#define   AXP8191_POWEROFF_SOURCE_EN2                   (0x53)
#define   AXP8191_OVP_DISCHARGE_TEMPERATURE_CFG         (0x54)
#define   AXP8191_POWER_DISABLE_POWER_DOWN_SEQUENCE     (0x55)
#define   AXP8191_WAKEUP_CTRL_VOFF_SET                  (0x56)
#define   AXP8191_PONLEVEL_SET                          (0x57)
#define   AXP8191_AUTO_SLEEP_CFG1                       (0x58)
#define   AXP8191_AUTO_SLEEP_CFG2                       (0x59)
#define   AXP8191_AUTO_SLEEP_CFG3                       (0x5A)
#define   AXP8191_AUTO_SLEEP_CFG4                       (0x5B)
#define   AXP8191_AUTO_SLEEP_CFG5                       (0x5C)
#define   AXP8191_AUTO_SLEEP_CFG6                       (0x5D)
#define   AXP8191_TS_CTRL                               (0x60)
#define   AXP8191_TS_HYSL2H                             (0x61)
#define   AXP8191_TS_HYSH2L                             (0x62)
#define   AXP8191_VLTF_WORK                             (0x63)
#define   AXP8191_VHTF_WORK                             (0x64)
#define   AXP8191_TS_ADC_EN_DATA_H                      (0x65)
#define   AXP8191_TS_ADC_DATA_L                         (0x66)
#define   AXP8191_TDIE_ADC_EN_DATA_H                    (0x67)
#define   AXP8191_TDIE_ADC_DATA_L                       (0x68)
#define   AXP8191_GPADC_EN_DATA_H                       (0x69)
#define   AXP8191_GPADC_DATA_L                          (0x6A)
#define   AXP8191_PS_ADC_EN_DATA_H                      (0x6B)
#define   AXP8191_PS_ADC_DATA_L                         (0x6C)

#define   AXP8191_CHANNEL_DEBUG_ADC_SEL                 (0x6D)

#define   AXP8191_ADC_CTL                               (0x6E)
#define   AXP8191_ADC_CTL                               (0x6E)
#define   AXP8191_GPIO_FUNC_CTL                         (0x70)
#define   AXP8191_GPIO_INPUT_CTL                        (0x71)
#define   AXP8191_GPIO_OUPUT_CTL                        (0x72)
#define   AXP8191_PWM_CTL1                              (0x73)
#define   AXP8191_PWM_CTL2                              (0x74)
#define   AXP8191_PWM_CTL3                              (0x75)
#define   AXP8191_BACKUP_BAT_CHARGE_CTL                 (0x76)
#define   AXP8191_WATCHDOG_CFG                          (0x77)
#define   AXP8191_WRITE_LOCK_F1                         (0xF0)
#define   AXP8191_EFUSE_CTL                             (0xF1)
#define   AXP8191_VREF                                  (0xF2)
#define   AXP8191_SCL_SDA_CFG                           (0xF3)
#define   AXP8191_REG_ADD_EXT                           (0xFF)

#define   AXP8191_COMMOM_CFG1                           (0x100)
#define   AXP8191_COMMOM_CFG2                           (0x101)
#define   AXP8191_COMMOM_CFG3                           (0x102)
#define   AXP8191_COMMOM_CFG4                           (0x103)

#define   AXP8191_DCDC1VOL_DEFAULT_SET                  (0x104)
#define   AXP8191_DCDC2VOL_DEFAULT_SET                  (0x105)
#define   AXP8191_DCDC3VOL_DEFAULT_SET                  (0x106)
#define   AXP8191_DCDC4VOL_DEFAULT_SET                  (0x107)
#define   AXP8191_DCDC5VOL_DEFAULT_SET                  (0x108)
#define   AXP8191_DCDC6VOL_DEFAULT_SET                  (0x109)
#define   AXP8191_DCDC7VOL_DEFAULT_SET                  (0x10A)
#define   AXP8191_DCDC8VOL_DEFAULT_SET                  (0x10B)
#define   AXP8191_DCDC9VOL_DEFAULT_SET                  (0x10C)

#define   AXP8191_ALDO1_ALDO2_VOL_DEFAULT_SET           (0x10D)
#define   AXP8191_ALDO3_ALDO4_VOL_DEFAULT_SET           (0x10E)
#define   AXP8191_ALDO5_ALDO6_VOL_DEFAULT_SET           (0x10F)

#define   AXP8191_BLDO1_BLDO2_VOL_DEFAULT_SET           (0x110)
#define   AXP8191_BLDO3_BLDO4_VOL_DEFAULT_SET           (0x111)
#define   AXP8191_BLDO5_CLDO1_VOL_DEFAULT_SET           (0x112)
#define   AXP8191_CLDO2_CLDO3_VOL_DEFAULT_SET           (0x113)
#define   AXP8191_CLDO4_CLDO5_VOL_DEFAULT_SET           (0x114)

#define   AXP8191_DLDO1_CLDO2_VOL_DEFAULT_SET           (0x115)
#define   AXP8191_DLDO3_CLDO4_VOL_DEFAULT_SET           (0x116)
#define   AXP8191_DLDO5_CLDO6_VOL_DEFAULT_SET           (0x117)

#define   AXP8191_ELDO1_ELDO2_VOL_DEFAULT_SET           (0x118)
#define   AXP8191_ELDO3_ELDO4_VOL_DEFAULT_SET           (0x119)
#define   AXP8191_ELDO5_ELDO6_VOL_DEFAULT_SET           (0x11A)

#define   AXP8191_DC8_OUTPUT_SET                        (0x12A)
#define   AXP8191_RTC_DCXO_CTL_VOFF                     (0x12C)
#define   AXP8191_DCDC_OC                               (0x12D)
#define   AXP8191_FANOUT_SEQ_TRIM                       (0x12E)
#define   AXP8191_VREF_VRPN_TUNING                      (0x12F)
#define   AXP8191_VREF_VOLTAGE_TUNING                   (0x130)
#define   AXP8191_BIAS_TUNING                           (0x131)
#define   AXP8191_FREQUENCY_TUNING                      (0x132)
#define   AXP8191_ADC_TUNING                            (0x133)
#define   AXP8191_DCDC_TRIM1                            (0x135)
#define   AXP8191_DCDC_TRIM2                            (0x136)

#define   AXP8191_PAGE_SELECT                           (0x137)
#define   AXP8191_TEST_MODE_CTL1                        (0x180)
#define   AXP8191_TEST_MODE_CTL2                        (0x181)
#define   AXP8191_TEST_MODE_CTL3                        (0x188)
#define   AXP8191_DCDC_DEBUG1                           (0x18A)
#define   AXP8191_DCDC_DEBUG2                           (0x18B)
#define   AXP8191_DCDC_DEBUG3                           (0x18C)
#define   AXP8191_DCDC_DEBUG4                           (0x18D)

/* define AXP515 REGISTER */
#define AXP515_STATUS0             (0x00)
#define AXP515_STATUS1             (0x02)
#define AXP515_IC_TYPE             (0x03)
#define AXP515_COMM_FAULT          (0x04)
#define AXP515_ILIMIT              (0x10)
#define AXP515_RBFET_SET           (0x11)
#define AXP515_MODULE_EN           (0x12)
#define AXP515_POK_SET             (0x15)
#define AXP515_BATFET_DLY          (0x16)
#define AXP515_PWRON_CTL           (0x17)
#define AXP515_GPIO1_CTL           (0x18)
#define AXP515_GPIO2_CTL           (0x19)
#define AXP515_GPIO1_SIGNAL        (0x1A)
#define AXP515_CC_EN               (0x22)
#define AXP515_ADC_EN              (0x24)
#define AXP515_OFF_CTL             (0x28)
#define AXP515_CC_GLOBAL_CTRL      (0x31)
#define AXP515_CC_LOW_POWER_CTRL   (0x32)
#define AXP515_CC_MODE_CTRL        (0x33)
#define AXP515_CC_STATUS0          (0x37)
#define AXP515_CC_STATUS4          (0x3B)
#define AXP515_INTEN1              (0x40)
#define AXP515_INTEN2              (0x41)
#define AXP515_INTEN3              (0x42)
#define AXP515_INTEN4              (0x43)
#define AXP515_INTEN5              (0x44)
#define AXP515_INTEN6              (0x45)
#define AXP515_INTSTS1             (0x48)
#define AXP515_INTSTS2             (0x49)
#define AXP515_INTSTS3             (0x4A)
#define AXP515_INTSTS4             (0x4B)
#define AXP515_INTSTS5             (0x4C)
#define AXP515_INTSTS6             (0x4D)
#define AXP515_DIE_TEMP            (0x56)
#define AXP515_VTS_RES             (0x58)
#define AXP515_VBATH_REG           (0x78)
#define AXP515_IBATH_REG           (0x7A)
#define AXP515_DISIBATH_REG        (0x7c)
#define AXP515_ADC_CONTROL         (0x80)
#define AXP515_TS_PIN_CONTROL      (0x81)
#define AXP515_VLTF_CHARGE         (0x84)
#define AXP515_VHTF_CHARGE         (0x85)
#define AXP515_VLTF_WORK           (0x86)
#define AXP515_VHTF_WORK           (0x87)
#define AXP515_IPRECHG_CFG         (0x8A)
#define AXP515_ICC_CFG             (0x8B)
#define AXP515_CHARGE_CONTROL2     (0x8C)
#define AXP515_ITERM_CFG           (0x8D)
#define AXP515_TIMER2_SET          (0x8E)
#define AXP515_CHGLED_CFG          (0x90)
#define AXP515_INS_VBAT_REG        (0xA0)
#define AXP515_COULOMB_CTL         (0xB8)
#define AXP515_CAP                 (0xB9)
#define AXP515_RDC0                (0xBA)
#define AXP515_RDC1                (0xBB)
#define AXP515_OCV0                (0xBC)
#define AXP515_OCV1                (0xBD)
#define AXP515_OCV_CURV0           (0xC0)
#define AXP515_BATCAP0             (0xE0)
#define AXP515_BATCAP1             (0xE1)
#define AXP515_OCV_PERCENT         (0xE4)
#define AXP515_CLUMB_PERCENT       (0xE5)
#define AXP515_WARNING_LEVEL       (0xE6)
#define AXP515_ADJUST_PARA         (0xE8)
#define AXP515_ADJUST_PARA1        (0xE9)
#define AXP515_ADDR_EXTENSION      (0xFF)

/* AXP1530 voltage type */
enum {
	AXP1530_POWER_DCDC1 = 0x0,
	AXP1530_POWER_DCDC2,
	AXP1530_POWER_DCDC3,
	AXP1530_POWER_MAX,
};

/* aw1660 voltage type */
enum {
	AXP1660_POWER_DCDC1 = 0x0,
	AXP1660_POWER_DCDC2,
	AXP1660_POWER_DCDC3,
	AXP1660_POWER_DCDC4,
	AXP1660_POWER_DCDC5,
	AXP1660_POWER_DCDC6,
	AXP1660_POWER_DCDC7,
	AXP1660_POWER_RTCLDO,
	AXP1660_POWER_ALDO1,
	AXP1660_POWER_ALDO2,
	AXP1660_POWER_ALDO3,
	AXP1660_POWER_DLDO1,
	AXP1660_POWER_DLDO2,
	AXP1660_POWER_DLDO3,
	AXP1660_POWER_DLDO4,
	AXP1660_POWER_ELDO1,
	AXP1660_POWER_ELDO2,
	AXP1660_POWER_ELDO3,
	AXP1660_POWER_FLDO1,
	AXP1660_POWER_FLDO2,
	AXP1660_POWER_LDOIO0,
	AXP1660_POWER_LDOIO1,
	AXP1660_POWER_DC1SW,
	AXP1660_POWER_MAX,
};

/* aw1736 voltage types */
enum {
	AW1736_POWER_DCDC1 = 0x0,
	AW1736_POWER_DCDC2,
	AW1736_POWER_DCDC3,
	AW1736_POWER_DCDC4,
	AW1736_POWER_DCDC5,
	AW1736_POWER_DCDC6,
	AW1736_POWER_ALDO1,
	AW1736_POWER_ALDO2,
	AW1736_POWER_ALDO3,
	AW1736_POWER_ALDO4,
	AW1736_POWER_ALDO5,
	AW1736_POWER_BLDO1,
	AW1736_POWER_BLDO2,
	AW1736_POWER_BLDO3,
	AW1736_POWER_BLDO4,
	AW1736_POWER_BLDO5,
	AW1736_POWER_CLDO1,
	AW1736_POWER_CLDO2,
	AW1736_POWER_CLDO3,
	AW1736_POWER_CLDO4,
	AW1736_POWER_CPUSLDO,
	AW1736_POWER_DC1SW,
	AW1736_POWER_RTC,
	AW1736_POWER_MAX,
};


/* axp2202 voltage type */
enum {
	AXP2202_POWER_DCDC1 = 0x0,
	AXP2202_POWER_DCDC2,
	AXP2202_POWER_DCDC3,
	AXP2202_POWER_DCDC4,
	AXP2202_POWER_ALDO1,
	AXP2202_POWER_ALDO2,
	AXP2202_POWER_ALDO3,
	AXP2202_POWER_ALDO4,
	AXP2202_POWER_BLDO1,
	AXP2202_POWER_BLDO2,
	AXP2202_POWER_BLDO3,
	AXP2202_POWER_BLDO4,
	AXP2202_POWER_CLDO1,
	AXP2202_POWER_CLDO2,
	AXP2202_POWER_CLDO3,
	AXP2202_POWER_CLDO4,
	AXP2202_POWER_CPUSLDO,
	AXP2202_POWER_RTC,
	AXP2202_POWER_MAX,
};


/* aw1657 voltage type */
enum {
	AW1657_POWER_DCDCA = 0x0,
	AW1657_POWER_DCDCB,
	AW1657_POWER_DCDCC,
	AW1657_POWER_DCDCD,
	AW1657_POWER_DCDCE,
	AW1657_POWER_ALDO1,
	AW1657_POWER_ALDO2,
	AW1657_POWER_ALDO3,
	AW1657_POWER_BLDO1,
	AW1657_POWER_BLDO2,
	AW1657_POWER_BLDO3,
	AW1657_POWER_BLDO4,
	AW1657_POWER_CLDO1,
	AW1657_POWER_CLDO2,
	AW1657_POWER_CLDO3,
	AW1657_POWER_DCESW,
	AW1657_POWER_MAX,
};

enum {
	AXP8191_POWER_DCDC1 = 0,
	AXP8191_POWER_DCDC2,
	AXP8191_POWER_DCDC3,
	AXP8191_POWER_DCDC4,
	AXP8191_POWER_DCDC5,
	AXP8191_POWER_DCDC6,
	AXP8191_POWER_DCDC7,
	AXP8191_POWER_DCDC8,
	AXP8191_POWER_DCDC9,
	AXP8191_POWER_DC1SW1,
	AXP8191_POWER_DC1SW2,
	AXP8191_POWER_ALDO1,
	AXP8191_POWER_ALDO2,
	AXP8191_POWER_ALDO3,
	AXP8191_POWER_ALDO4,
	AXP8191_POWER_ALDO5,
	AXP8191_POWER_ALDO6,
	AXP8191_POWER_BLDO1,
	AXP8191_POWER_BLDO2,
	AXP8191_POWER_BLDO3,
	AXP8191_POWER_BLDO4,
	AXP8191_POWER_BLDO5,
	AXP8191_POWER_CLDO1,
	AXP8191_POWER_CLDO2,
	AXP8191_POWER_CLDO3,
	AXP8191_POWER_CLDO4,
	AXP8191_POWER_CLDO5,
	AXP8191_POWER_DLDO1,
	AXP8191_POWER_DLDO2,
	AXP8191_POWER_DLDO3,
	AXP8191_POWER_DLDO4,
	AXP8191_POWER_DLDO5,
	AXP8191_POWER_DLDO6,
	AXP8191_POWER_ELDO1,
	AXP8191_POWER_ELDO2,
	AXP8191_POWER_ELDO3,
	AXP8191_POWER_ELDO4,
	AXP8191_POWER_ELDO5,
	AXP8191_POWER_ELDO6,
	AXP8191_POWER_RTCLDO,
	AXP8191_POWER_MAX,
};

/* TCS4838 voltage type */
enum {
	TCS4838_POWER_DCDC0 = 0x0,
	TCS4838_POWER_DCDC1,
	TCS4838_POWER_MAX,
};

enum {
	SY8827G_POWER_DCDC0 = 0x0,
	SY8827G_POWER_DCDC1,
	SY8827G_POWER_MAX,
};

typedef enum power_key_type {
	POWER_KEY_SHORT = 1,
	POWER_KEY_LONG  = 2,
	POWER_LOW_POWER = 3,
} power_key_type_e;

typedef enum power_voltage_state {
	POWER_VOL_OFF = 0x0,
	POWER_VOL_ON  = 0x1,
} power_voltage_state_e;

typedef enum power_mode {
	POWER_MODE_AXP = 0,
	POWER_MODE_DUMMY = 1,
	POWER_MODE_PWM = 2,
} power_mode_e;

/*
 * keep the struct word align
 * by superm at 2014-2-13 15:34:09
 */
typedef struct pmu_onoff_reg_bitmap {
	u16 regaddr;
	u16 offset;
	u8  state;
	u8  dvm_st;
} pmu_onoff_reg_bitmap_t;
extern pmu_onoff_reg_bitmap_t pmu_onoff_reg_bitmap[];

typedef struct pmu_ops {
	s32 (*pmu_shutdown)(void);
	s32 (*pmu_reset)(void);
	s32 (*pmu_charging_reset)(void);
	s32 (*pmu_set_voltage_state)(u32 type, u32 state);
	s32 (*pmu_clear_pendings)(void);
} pmu_ops_t;

typedef struct bmu_ops {
	s32 (*bmu_is_exist)(void);
	s32 (*bmu_charging_reset)(void);
	s32 (*bmu_charging_vbus_det)(void);
	void (*bmu_reset)(void);
	void (*bmu_shutdown)(void);
} bmu_ops_t;

s32 pmu_set_voltage(u32 type, u32 voltage);
s32 pmu_get_voltage(u32 type);
s32 pmu_set_voltage_state(u32 type, u32 state);

/* register read and write */
s32 pmu_reg_read(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);
s32 pmu_reg_write(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);

s32 pmu_get_chip_id(struct message *pmessage);

/* pmu standby process */
s32 pmu_query_event(u32 *event);
int get_nmi_int_type(void);
u32 pmu_output_is_stable(u32 type);
s32 pmu_get_voltage_state(u32 type);
s32 pmu_set_paras(struct message *pmessage);
s32 set_pwr_tree(struct message *pmessage);

s32 pmu_get_batconsum(void);
u32 pmu_get_powerstate(u32 power_reg);

void watchdog_reset(void);

typedef struct pmu_paras {
	u8 *devaddr;
	u8 *regaddr;
	u8 *data;
	u32 len;
} pmu_paras_t;

typedef struct box_start_os_cfg {
	u32 used;
	u32 start_type;
	u32 irkey_used;
	u32 pmukey_used;
	u32 pmukey_num;
	u32 led_power;
	u32 led_state;
} box_start_os_cfg_t;

extern void pmu_sysconfig_cfg(void);
extern void pmu_set_lowpcons(void);
extern void pmu_sys_lowpcons(void);
extern s32 pmu_set_pok_time(int time);
extern void pmu_poweroff_system(void);
extern void pmu_reset_system(void);
extern int pmu_set_gpio(unsigned int id, unsigned int state);
extern s32 pmu_clear_pendings(void);
extern bool pmu_pin_detect(void);

/*gpio_power_key for box_standby*/
extern bool gpio_power_key_detect(void);
extern void gpio_power_key_cfg(void);


/*bt_hostwake for box_standby*/
extern void bt_hostwake_cfg(void);
extern bool bt_hostwake_detect(void);

#ifdef CFG_BMU_USED
extern s32 bmu_init(void);
extern u32 is_bmu_exist(void);
extern s32 bmu_charging_reset(void);
extern s32 bmu_charging_vbus_det(void);
extern void bmu_shutdown(void);
extern void bmu_reset(void);
#else
static inline s32 bmu_init(void) { return -1; }
static inline s32 is_bmu_exist(void) { return -1; }
static inline s32 bmu_charging_reset(void) { return -1; }
static inline s32 bmu_charging_vbus_det(void) { return -1; }
static inline void bmu_shutdown(void) { return; }
static inline void bmu_reset(void) { return; }
#endif

#ifdef CFG_PMU_USED
extern s32 pmu_init(void);
extern s32 pmu_exit(void);
extern u32 is_pmu_exist(void);
extern s32 pmu_reg_write_para(pmu_paras_t *para);
extern s32 pmu_reg_read_para(pmu_paras_t *para);
extern s32 pmu_shutdown(void);
extern s32 pmu_reset(void);
extern s32 pmu_charging_reset(void);
extern s32 pmu_standby_init(void);
extern s32 pmu_standby_exit(void);
#else
static inline s32 pmu_init(void) { return -1; }
static inline s32 pmu_exit(void) { return -1; }
static inline s32 is_pmu_exist(void) { return -1; }
static inline s32 pmu_reg_write_para(pmu_paras_t *para) { return -1; }
static inline s32 pmu_reg_read_para(pmu_paras_t *para) { return -1; }
static inline s32 pmu_shutdown(void) { return -ENODEV; }
static inline s32 pmu_reset(void) { return -ENODEV; }
static inline s32 pmu_charging_reset(void) { return -ENODEV; }
static inline s32 pmu_standby_init(void) { return -1; }
static inline s32 pmu_standby_exit(void) { return -1; }
#endif

#if defined CFG_PMU_EXT_USED
extern s32 pmu_ext_is_exist(void);
extern s32 pmu_ext_set_voltage_state(u32 type, u32 state);
#else
static inline s32 pmu_ext_is_exist(void) { return FALSE; }
static inline s32 pmu_ext_set_voltage_state(u32 type, u32 state) { return -1; }
#endif

#endif  /* __PMU_H__ */
