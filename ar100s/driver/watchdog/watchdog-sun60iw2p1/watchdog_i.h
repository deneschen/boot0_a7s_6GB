/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                watchdog  module
*
*                                    (c) Copyright 2012-2016, Superm Wu China
*                                             All Rights Reserved
*
* File    : Watchdog_i.h
* By      : Superm Wu
* Version : v1.0
* Date    : 2012-9-18
* Descript: watchdog controller public interfaces.
* Update  : date                auther      ver     notes
*           2012-9-18 19:08:23  Superm Wu   1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __WATCHDOG_I_H__
#define __WATCHDOG_I_H__

#include "include.h"

/* the base address of watchdog0 register */
#define WDOG0_REG_BASE (0x07021000)

/* watchdog0 IRQ Enable Register Options */
#define WDOG0_IRG_EN     (1 << 0)
#define WDOG0_IRG_DIS    (0 << 0)

/* watchdog0 Status Register Options */
#define WDOG0_IRQ_PEND   (1 << 0)

/* watchdog0 Control Register Options */
#define WDOG0_KEY_FIELD  (0xa57 << 1)
#define WDOG0_RESTART    (0x1 << 0)

/* watchdog0 Configuration Register Options */
#define WDOG0_WRITE_KEY       (0x16aaU << 16)
#define WDOG0_CLK_SRC_RTC32K  (0 << 8)
#define WDOG0_CLK_SRC_DCXO750 (1 << 8)
#define WDOG0_RST_CPUS     (0 << 0)
#define WDOG0_RST_SYS      (1 << 0)
#define WDOG0_RST_INT      (2 << 0)

/*
 * watchdog0 Mode Register Options
 * watchdog0 Interval Value
 * The selected RTC_32K clock is nominally 32.768 kHz.
 */
#define WDOG0_INTV_VALUE_500MS (0 << 4)  /* 0.5 sec, 16000  */
#define WDOG0_INTV_VALUE_01S   (1 << 4)  /* 1.0 sec, 32000  */
#define WDOG0_INTV_VALUE_02S   (2 << 4)  /* 2.0 sec, 64000  */
#define WDOG0_INTV_VALUE_03S   (3 << 4)  /* 3.0 sec, 96000  */
#define WDOG0_INTV_VALUE_04S   (4 << 4)  /* 4.0 sec, 128000 */
#define WDOG0_INTV_VALUE_05S   (5 << 4)  /* 5.0 sec, 160000 */
#define WDOG0_INTV_VALUE_06S   (6 << 4)  /* 6.0 sec, 192000 */
#define WDOG0_INTV_VALUE_08S   (7 << 4)  /* 8.0 sec, 256000 */
#define WDOG0_INTV_VALUE_10S   (8 << 4)  /* 10.0 sec, 320000 */
#define WDOG0_INTV_VALUE_12S   (9 << 4)  /* 12.0 sec, 384000 */
#define WDOG0_INTV_VALUE_14S   (10 << 4) /* 14.0 sec, 448000 */
#define WDOG0_INTV_VALUE_16S   (11 << 4) /* 16.0 sec, 512000 */

/* watchdog0 enable and disable */
#define WDOG0_EN  (1<<0)
#define WDOG0_DIS (0<<0)


/* hardware watchdog registers */
typedef struct watchdog_regs {
	volatile u32    irq_enable;
	volatile u32    irq_status;
	volatile u32    software_reset;
	volatile u32    control;
	volatile u32    config;
	volatile u32    mode;
	volatile u32    output_config;
} watchdog_regs_t;

/* Declare Some Internal Function */
s32 begin_feed_watchdog(void);
s32 finish_feed_watchdog(void);
s32 watchdog_restart_server(void *parg);

#endif /*__WATCHDOG_I_H__ */



