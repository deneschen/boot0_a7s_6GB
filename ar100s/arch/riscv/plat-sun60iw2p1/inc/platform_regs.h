/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 io module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : io.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-18
* Descript: io module public header.
* Update  : date                auther      ver     notes
*           2012-5-18 10:04:21  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __REG_BASE_H__
#define __REG_BASE_H__

#define CCU_REG_BASE		(0x02002000)
#define SYS_CFG_REG_BASE        (0x03000000)
#define CPUX_HWMSGBOX_REG_BASE	(0x03004000)
#define CPUS_HWMSGBOX_REG_BASE	(0x07094000)
#define RTC_REG_BASE		(0x07090000)
#define R_CPUCFG_REG_BASE	(0x07050000)
#define PCK600_REG_BASE		(0x07060000)
#define R_PRCM_REG_BASE		(0x07010000)
#define R_TMR01_REG_BASE	(0x07091000)
#define R_WDOG_REG_BASE		(0x07021000)
#define R_INTC_REG_BASE		(0x07024000)
#define R_PIO_REG_BASE		(0x07025000)
#define E902_CFG_BASE		(0x07032000)
#define R_CIR_REG_BASE		(0x07040000)
#define R_UART_REG_BASE		(0x07080000)
#define R_TWI_REG_BASE		(0x07083000)
#define CPUSUBSYS_REG_BASE	(0x08000000)
#define RISCV_CLIC_BASE		(0xE0800000)
#define TS_STAT_REG_BASE	(0x08010000) // ???
#define TS_CTRL_REG_BASE	(0x08020000) // ???
#define CPUCFG_REG_BASE		(0x08815000) // ????
#define CPU_PLL_REG_BASE	(0x08870000)
#define USB0_REG_BASE           (0x04101000)
#define USB1_REG_BASE           (0x04200000)

#define RV_CLK_16M		(0x82000000)

#define E902_AUTO_GATE_REG	(E902_CFG_BASE + 0x04)

/* rtc domain record reg */
#define RTC_LOSC_CTRL		(RTC_REG_BASE + 0x000)
#define RTC_LOSC_AUTO_STA	(RTC_REG_BASE + 0x004)
#define RTC_LOSC_OUT_GATING	(RTC_REG_BASE + 0x060)
#define RTC_RECORD_REG		(RTC_REG_BASE + 0x10c)
#define RTC_DTB_BASE_STORE_REG	(RTC_RECORD_REG)
#define RTC_XO_CTRL_REG		(RTC_REG_BASE + 0x160)

/* RTC_VDD_OFF_GATING_CTRL */
#define RTC_VCCIO_DEBONCE_EN	(0x100)
#define RTC_VCCIO_OUTPUT_EN	(0x01 << 7)
#define RTC_VCCIO_DETECT_EN	(0x01)

/* CPU CLK */
#define CPUA_CLK_REG		(CPU_PLL_REG_BASE + 0x101c)
#define CPUB_CLK_REG		(CPU_PLL_REG_BASE + 0x201c)
#define DSU_CLK_REG		(CPU_PLL_REG_BASE + 0x301c)

/* CPU PLL */
#define CPU_PLL_REG(n)			(CPU_PLL_REG_BASE + (n) * 0x1000)
#define CPU_PLL_EN			(1 << 31)
#define CPU_PLL_LDO_EN			(1 << 30)
#define CPU_PLL_LOCK_EN			(1 << 29)
#define CPU_PLL_LOCK_STATUS		(1 << 28)
#define CPU_PLL_OUTPUT			(1 << 27)
#define CPU_PLL_UPDATE			(1 << 26)
#define CPU_PLL_FACTOR			0x3FFF0F
#define CPU_FACTOR_P_MASK		(0x3 << 16)

/* CPU_CLK_REG */
#define CPU_CLK_SRC_SEL_SHIFT		(24)
#define CPU_CLK_SRC_SEL_MASK		(0x7 << CPU_CLK_SRC_SEL_SHIFT)
#define CPU_CLK_SRC_SEL(n)		((n) << CPU_CLK_SRC_SEL_SHIFT)

/* DSU_CLK_REG */
#define DSU_CLK_SRC_SEL_SHIFT		(24)
#define DSU_CLK_SRC_SEL_MASK		(0x7 << DSU_CLK_SRC_SEL_SHIFT)
#define DSU_FACTOR_MASK			(0x3 << 16)
#define DSU_CLK_SRC_SEL(n)		((n) << DSU_CLK_SRC_SEL_SHIFT)
#define DSU_FACTOR_P(n)			((n) << DSU_FACTOR_P_SHIFT)

#define CPU_PLL_FACTOR_M1_MASK		(0xf << 0)
#define CPU_PLL_FACTOR_N_MASK		(0xff << 8)
#define CPU_PLL_FACTOR_P_MASK		(0xf << 16)
#define CPU_PLL_FACTOR_M0_MASK		(0x3 << 20)
#define CPU_PLL_FACTOR_MASK		(CPU_PLL_FACTOR_M1_MASK | CPU_PLL_FACTOR_N_MASK | CPU_PLL_FACTOR_P_MASK | CPU_PLL_FACTOR_M0_MASK)

//name by pll function
#define CCU_PLL_REF_REG			(CCU_REG_BASE + 0x00)
#define CCU_PLL_DDR_REG			(CCU_REG_BASE + 0x20)
#define CCU_PLL_PERI0_REG		(CCU_REG_BASE + 0xA0)
#define CCU_PLL_PERI1_REG		(CCU_REG_BASE + 0xC0)
#define CCU_PLL_CPU0_REG		(CCU_REG_BASE + 0xE0)
#define CCU_PLL_VIDEO0_REG              (CCU_REG_BASE + 0x120)
#define CCU_PLL_VIDEO1_REG		(CCU_REG_BASE + 0x140)
#define CCU_PLL_VIDEO2_REG		(CCU_REG_BASE + 0x160)
#define CCU_PLL_VE0_REG			(CCU_REG_BASE + 0x220)
#define CCU_PLL_VE1_REG			(CCU_REG_BASE + 0x240)
#define CCU_PLL_AUDIO0_REG		(CCU_REG_BASE + 0x260)
#define CCU_PLL_AUDIO1_REG		(CCU_REG_BASE + 0x280)
#define CCU_PLL_NPU_REG			(CCU_REG_BASE + 0x2A0)
#define CCU_PLL_DE_REG			(CCU_REG_BASE + 0x2E0)

#define CCU_AHB_CFG_REG             (CCU_REG_BASE + 0x500)
#define CCU_APB0_CFG_REG            (CCU_REG_BASE + 0x510)
#define CCU_APB1_CFG_REG            (CCU_REG_BASE + 0x518)
#define CCU_APB_UART_CFG_REG        (CCU_REG_BASE + 0x538)
#define CCU_TRACE_CFG_REG           (CCU_REG_BASE + 0x540)
#define CCU_GIC_CFG_REG             (CCU_REG_BASE + 0x560)
#define CCU_NSI_CFG_REG             (CCU_REG_BASE + 0x580)
#define CCU_MBUS_CLK_REG            (CCU_REG_BASE + 0x588)
#define CCU_MSGBOX_BGR_REG          (CCU_REG_BASE + 0x744)
#define CCU_SPINLOCK_BGR_REG        (CCU_REG_BASE + 0x724)
#define CCU_USB0_CLK_REG            (CCU_REG_BASE + 0x1300)
#define CCU_USB0_BGR_REG            (CCU_REG_BASE + 0x1304)
#define CCU_USB1_CLK_REG            (CCU_REG_BASE + 0x1308)
#define CCU_USB1_BGR_REG            (CCU_REG_BASE + 0x130C)
#define CCU_USB0_USB1_REF_CLK_REG   (CCU_REG_BASE + 0x1340)

/* CCU_CFG_REG */
#define CCU_CLK_SRC_SEL_SHIFT		(24)
#define CCU_FACTOR_M_SHIFT		(0)
#define CCU_CLK_SRC_SEL_MASK		(0x7 << CCU_CLK_SRC_SEL_SHIFT)
#define CCU_FACTOR_M_MASK		(0x3ff << CCU_FACTOR_M_SHIFT)
#define CCU_CLK_SRC_SEL(n)		((n) << CCU_CLK_SRC_SEL_SHIFT)
#define CCU_FACTOR_M(n)			((n) << CCU_FACTOR_M_SHIFT)
#define CCU_CLK_CTL_MASK		(1 << 31)

/* CCU_PLL_REG */
#define PLL_OUTPUT_ENABLE_SHIFT		(27)
#define PLL_LOCK_STATUS_SHIFT		(28)
#define PLL_LOCK_ENABLE_SHIFT		(29)
#define PLL_LDO_ENABLE_SHIFT		(30)
#define PLL_ENABLE_SHIFT		(31)

#define PLL_OUTPUT_ENABLE_MASK		(0x1 << PLL_OUTPUT_ENABLE_SHIFT)
#define PLL_LOCK_STATUS_MASK		(0x1 << PLL_LOCK_STATUS_SHIFT)
#define PLL_LOCK_ENABLE_MASK		(0x1 << PLL_LOCK_ENABLE_SHIFT)
#define PLL_LDO_ENABLE_MASK		(0x1 << PLL_LDO_ENABLE_SHIFT)
#define PLL_ENABLE_MASK			(0x1 << PLL_ENABLE_SHIFT)


#define PLL_FACTOR_M0_MASK		(0x1 << 0)
#define PLL_FACTOR_M1_MASK		(0x1 << 1)
#define PLL_FACTOR_P2_MASK		(0x1f << 2)
#define PLL_FACTOR_N_MASK		(0xff << 8)
#define PLL_FACTOR_P0_MASK		(0x1f << 16)
#define PLL_FACTOR_P1_MASK		(0x1f << 20)
#define PLL_FACKOR_MASK			(PLL_FACTOR_M0_MASK | PLL_FACTOR_M1_MASK | PLL_FACTOR_P2_MASK |\
					PLL_FACTOR_N_MASK | PLL_FACTOR_P0_MASK | PLL_FACTOR_P1_MASK)

/* CCU_USB0_CLK_REG */
#define USB0_CLKEN_MASK			(0x1 << 31)
#define USB0_PHY_RSTN_MASK		(0x1 << 30)

/* CCU_USB0_BGR_REG */
#define USB0_EHCI_RST_MASK		(0x1 << 20)
#define USB0_OHCI_RST_MASK		(0x1 << 16)
#define USB0_EHCI_GATING_MASK		(0x1 << 4)
#define USB0_OHCI_GATING_MASK		(0x1 << 0)

/* CCU_USB1_CLK_REG */
#define USB1_CLKEN_MASK			(0x1 << 31)
#define USB1_PHY_RSTN_MASK		(0x1 << 30)

/* CCU_USB1_BGR_REG */
#define USB1_EHCI_RST_MASK		(0x1 << 20)
#define USB1_OHCI_RST_MASK		(0x1 << 16)
#define USB1_EHCI_GATING_MASK		(0x1 << 4)
#define USB1_OHCI_GATING_MASK		(0x1 << 0)

/* CCU_USB0_USB1_REF_CLK_REG */
#define USB0_USB1_REF_CLK_GATING_MASK	(0x1 << 31)

/* rtc domain record reg */
#define RTC_YMD				(RTC_REG_BASE	+ 0x10)
#define RTC_HMS				(RTC_REG_BASE	+ 0x14)
#define RTC_GPR0				(RTC_REG_BASE	+ 0x100 + 0 * 0x04)
#define RTC_GPR1				(RTC_REG_BASE	+ 0x100 + 1 * 0x04)
#define RTC_AUTO_CALI_CTRL			(RTC_REG_BASE + 0x00C)
#define RTC_GP_DATA5_REG			(RTC_REG_BASE + 0x114)
#define RTC_XO_WRT_PROTECT			(RTC_REG_BASE + 0x15c)
#define RTC_VDD_OFF_GATING_CTRL		(RTC_REG_BASE + 0x1f4)
#define RTC_IR_CODE_STORE_REG		(RTC_REG_BASE + 0x120)
#define RTC_CFG_REG					(RTC_REG_BASE + 0x310)
#define RTC_YMD_MASK			(0xFFFF)
#define RTC_HMS_H_SHIFT		(16)
#define RTC_HMS_H_MASK		(0x1F << RTC_HMS_H_SHIFT)
#define RTC_HMS_M_SHIFT		(8)
#define RTC_HMS_M_MASK		(0x3F << RTC_HMS_M_SHIFT)
#define RTC_HMS_S_SHIFT		(0)
#define RTC_HMS_S_MASK		(0x3F << RTC_HMS_S_SHIFT)

/* USB */
#define USB0_USB_CTRL_REG           (USB0_REG_BASE + 0x800)
#define USB1_USB_CTRL_REG           (USB1_REG_BASE + 0x800)
#define USB_STANDBY_CLOCK_SEL       (0x1 << 31)
#define USB_CTRL_NORMAL_MODE        (0x0 << 31)
#define USB_CTRL_STANDBY_MODE       (0x1 << 31) /* usb clock switch to RC 16M clock */

/* SMC */
#define SUNXI_SMC_PBASE			(0x0A000000)
#define SUNXI_SMC1_PBASE		(SUNXI_SMC_PBASE + 0x10000)
#define SMC_ACTION_REG			(SUNXI_SMC_PBASE + 0x0004)
#define SMC_FUNCTION_BYPASS		(1UL<<6)
#define SMC_ITLV_CTRL_REG		(SUNXI_SMC_PBASE + 0x0018)
#define SMC_MST0_BYP_REG		(SUNXI_SMC_PBASE + 0x0070)
#define SMC_MST1_BYP_REG		(SUNXI_SMC_PBASE + 0x0074)
#define SMC_MST0_SEC_REG		(SUNXI_SMC_PBASE + 0x0080)
#define SMC_MST1_SEC_REG		(SUNXI_SMC_PBASE + 0x0084)
#define SMC_REGION_COUNT (8)	/* total 16, not all used, save some space */
#define SMC_REGION_SETUP_LOW_REG(x)	(SUNXI_SMC_PBASE + 0x100 + 0x10*(x))
#define SMC_REGION_SETUP_HIGH_REG(x)	(SUNXI_SMC_PBASE + 0x104 + 0x10*(x))
#define SMC_REGION_ATTRIBUTE_REG(x)	(SUNXI_SMC_PBASE + 0x108 + 0x10*(x))
#define CFG_SUNXI_SMC_GLITCH_WORKAROUND	/* Do not operate 'SMC_FUNCTION_BYPASS' on this SoC */
#define SUNXI_SMC_HAS_ITLV		/* We have at least two SMCs for interleaving */

/* SID */
#define SUNXI_SID_PBASE			(0x03006000)
#define SID_SEC_MODE_STA		(SUNXI_SID_PBASE + 0xA0)
#define SID_SEC_MODE_MASK		(0x1)

/* timerstamp regs */
#define SUNXI_TIMESTAMP_STA_CNT_LOW_REG		(TS_STAT_REG_BASE)
#define SUNXI_TIMESTAMP_STA_CNT_HI_REG		(TS_STAT_REG_BASE + 0x04)
#define SUNXI_TIMESTAMP_CTRL_CNT_LOW_REG	(TS_CTRL_REG_BASE + 0x08)
#define SUNXI_TIMESTAMP_CTRL_CNT_HI_REG		(TS_CTRL_REG_BASE + 0x0c)
#define SUNXI_TIMESTAMP_CTRL_CNT_FREQID_REG	(TS_CTRL_REG_BASE + 0x20)

/* prcm regs */
#define CPUS_CFG_REG			(R_PRCM_REG_BASE + 0x000)
#define AHBS_CFG_REG			(CPUS_CFG_REG)
#define APBS0_CFG_REG			(R_PRCM_REG_BASE + 0x00c)
#define APBS1_CFG_REG			(R_PRCM_REG_BASE + 0x010)
#define R_TIMER_BUS_GATE_RST_REG	(R_PRCM_REG_BASE + 0x11c)
#define R_PWM_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x13c)
#define R_MSGBOX_GATE_RST_REG		(R_PRCM_REG_BASE + 0x17c)
#define R_UART_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x18c)
#define R_TWI_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x19c)
#define R_IR_RX_CLOCK_REG		(R_PRCM_REG_BASE + 0x1c0)
#define R_IR_RX_BUS_GATE_RST_REG	(R_PRCM_REG_BASE + 0x1cc)
#define RTC_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x20c)
#define RV_24M_CLK_REG			(R_PRCM_REG_BASE + 0x210)
#define PLL_CTRL_REG1			(R_PRCM_REG_BASE + 0x244)
#define VDD_SYS_PWROFF_GATING_REG	(R_PRCM_REG_BASE + 0x250)
#define ANA_PWR_RST_REG			(R_PRCM_REG_BASE + 0x254)
#define VDD_SYS_PWR_RST_REG		(R_PRCM_REG_BASE + 0x260)
#define NMI_INT_EN_REG			(R_PRCM_REG_BASE + 0x324)
#define DEV_BUS_CTL_REG			(R_PRCM_REG_BASE + 0x338)
#define BUS_AUTO_CLK_REG		(R_PRCM_REG_BASE + 0x33c)
#define MSRA_CTL_REG			(R_PRCM_REG_BASE + 0x360)
#define R_TIMER0_CLK_REG		(R_PRCM_REG_BASE + 0x100)

/* APBS0_CFG_REG */
#define APBS0_CLK_SRC_SEL_SHIFT		(24)
#define APBS0_FACTOR_M_SHIFT		(0)

#define APBS0_CLK_SRC_SEL_MASK		(0x7 << APBS0_CLK_SRC_SEL_SHIFT)
#define APBS0_FACTOR_M_MASK		(0x1f << APBS0_FACTOR_M_SHIFT)

#define APBS0_CLK_SRC_SEL(n)		((n) << APBS0_CLK_SRC_SEL_SHIFT)
#define APBS0_FACTOR_M(n)		((n) << APBS0_FACTOR_M_SHIFT)

/* APBS1_CFG_REG */
#define APBS1_CLK_SRC_SEL_SHIFT		(24)
#define APBS1_FACTOR_M_SHIFT		(0)

#define APBS1_CLK_SRC_SEL_MASK		(0x7 << APBS1_CLK_SRC_SEL_SHIFT)
#define APBS1_FACTOR_M_MASK		(0x1f << APBS1_FACTOR_M_SHIFT)

#define APBS1_CLK_SRC_SEL(n)		((n) << APBS1_CLK_SRC_SEL_SHIFT)
#define APBS1_FACTOR_M(n)		((n) << APBS1_FACTOR_M_SHIFT)

/* CPUS_CFG_REG */
#define CPUS_CLK_SRC_SEL_SHIFT		(24)
#define CPUS_FACTOR_M_SHIFT		(0)

#define CPUS_CLK_SRC_SEL_MASK		(0x7 << CPUS_CLK_SRC_SEL_SHIFT)
#define CPUS_FACTOR_M_MASK		(0x1f << CPUS_FACTOR_M_SHIFT)

#define CPUS_CLK_SRC_SEL(n)		((n) << CPUS_CLK_SRC_SEL_SHIFT)
#define CPUS_FACTOR_M(n)		((n) << CPUS_FACTOR_M_SHIFT)

/* VDD_SYS_PWROFF_GATING_REG */
#define VDD_USB2CPUS_GATING_SHIFT	(8)
#define VDD_SYS2USB_GATING_SHIFT	(3)

#define VDD_USB2CPUS_GATING_MASK	(0x1 << VDD_USB2CPUS_GATING_SHIFT)
#define VDD_SYS2USB_GATING_MASK		(0x1 << VDD_SYS2USB_GATING_SHIFT)

#define VDD_USB2CPUS_GATING(n)		((n) << VDD_USB2CPUS_GATING_SHIFT)
#define VDD_SYS2USB_GATING(n)		((n) << VDD_SYS2USB_GATING_SHIFT)

//name by pll function
#define CCU_PLL_DDR0_REG		(CCU_REG_BASE + 0X020)
#define CCU_PLL_PERIPH0_REG		(CCU_REG_BASE + 0x0A0)
#define CCU_PLL_PERIPH1_REG		(CCU_REG_BASE + 0x0C0)
#define CCU_AHB_CLK_REG			(CCU_REG_BASE + 0x500)
#define CCU_APB0_CFG_REG		(CCU_REG_BASE + 0x510)
#define CCU_APB1_CFG_REG		(CCU_REG_BASE + 0x518)
#define CCU_MBUS_CLK_REG		(CCU_REG_BASE + 0x588)
#define CCU_MSGBOX_BGR_REG		(CCU_REG_BASE + 0x744)
#define CCU_SPINLOCK_BGR_REG		(CCU_REG_BASE + 0x724)

/* R_PIO_REG*/
#define PIN_REG_CFG(n, i)		((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + ((i) >> 3) * 0x4 + 0x00))
#define PIN_REG_DLEVEL(n, i)		((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + ((i) >> 3) * 0x4 + 0x14))
#define PIN_REG_PULL(n, i)		((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + ((i) >> 4) * 0x4 + 0x24))
#define PIN_REG_DATA(n)			((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + 0x10))

#define PIN_REG_CFG_VALUE(n, i)		readl(PIN_REG_CFG(n, i))
#define PIN_REG_DLEVEL_VALUE(n, i)	readl(PIN_REG_DLEVEL(n, i))
#define PIN_REG_PULL_VALUE(n, i)	readl(PIN_REG_PULL(n, i))
#define PIN_REG_DATA_VALUE(n)		readl(PIN_REG_DATA(n))

#define PIN_REG_INT_CFG(n, i)		((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + ((i) >> 3) * 0x4 + 0x200))
#define PIN_REG_INT_CTL(n)		((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + 0x210))
#define PIN_REG_INT_STAT(n)		((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + 0x214))
#define PIN_REG_INT_DEBOUNCE(n)		((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + 0x218))
#define PIN_NUM_INT_CFG_OFFSET(i)	((i % 8) * 0x4)

#define CCU_DCXO_FREQ			(26000000)	//26M external crystal
#define CCU_SYS_CLK24M_FREQ		(24000000)	//fixed 24M system clock
#define CCU_LOSC_FREQ			(31250)	//31250
#define CCU_CPUS_PLL0_FREQ		(200000000)
#define CCU_APBS2_PLL0_FREQ		(200000000)
#define CCU_IOSC_FREQ			(16000000)	//16M
#define CCU_CPUS_POST_DIV		(100000000)	//cpus post div source clock freq
#define CCU_PERIPH0_FREQ		(600000000)	//600M

#define E902_WAKEUP_MASK0_REG (E902_CFG_BASE + 0x64)
#define E902_WAKEUP_MASK1_REG (E902_CFG_BASE + 0x68)

#define MASK0_START_INTERRUPT 16
#define MASK1_START_INTERRUPT 48

#endif
