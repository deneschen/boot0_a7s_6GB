/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                watchdog  module
*
*                                    (c) Copyright 2012-2016, Superm Wu China
*                                             All Rights Reserved
*
* File    : Watchdog.c
* By      : Superm Wu
* Version : v1.0
* Date    : 2012-9-18
* Descript: watchdog controller public interfaces.
* Update  : date                auther      ver     notes
*           2012-9-18 19:08:23  Superm Wu   1.0     Create this file.
*********************************************************************************************************
*/
#include "watchdog_i.h"

/* watchdog base registers */
struct watchdog_regs *pwatchdog_regs;

_Static_assert(__builtin_offsetof(struct watchdog_regs, control) == 0x0c,
	       "watchdog control register offset mismatch");
_Static_assert(__builtin_offsetof(struct watchdog_regs, config) == 0x10,
	       "watchdog config register offset mismatch");
_Static_assert(__builtin_offsetof(struct watchdog_regs, mode) == 0x14,
	       "watchdog mode register offset mismatch");

/*
*********************************************************************************************************
*                                       INIT WATCHDOG
*
* Description:  initialize watchdog.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize watchdog succeeded, others if failed.
*********************************************************************************************************
*/
s32 watchdog_init(void)
{
	u32 value;

	/* initialize the pointer of watchdog registers */
	pwatchdog_regs = (struct watchdog_regs *)(WDOG0_REG_BASE);

	/* Keep watchdog running when DCXO is off and reset the whole system. */
	value = pwatchdog_regs->config & 0xffff;
	value &= ~((0x1 << 8) | (0x3 << 0));
	value |= WDOG0_WRITE_KEY | WDOG0_CLK_SRC_RTC32K | WDOG0_RST_SYS;
	pwatchdog_regs->config = value;

	/*
	 * set watchdog0 Interval Value: 128k cycles,
	 * about 4 s with the RTC_32K source (32.768 kHz).
	 */
	value = pwatchdog_regs->mode & 0xffff;
	value &= ~(0xf << 4);
	value |= WDOG0_WRITE_KEY | WDOG0_INTV_VALUE_04S;
	pwatchdog_regs->mode = value;

	return OK;
}

/*
*********************************************************************************************************
*                                       EXIT WATCHDOG
*
* Description:  exit watchdog.
*
* Arguments  :  none.
*
* Returns    :  OK if exit watchdog succeeded, others if failed.
*********************************************************************************************************
*/
s32 watchdog_exit(void)
{
	/* release regs addrress */
	pwatchdog_regs = NULL;

	return OK;
}

/*
*********************************************************************************************************
*                                       ENABLE WATCHDOG
*
* Description:  enable watchdog.
*
* Arguments  :  none.
*
* Returns    :  OK if enable watchdog succeeded, others if failed.
*********************************************************************************************************
*/
s32 watchdog_enable(void)
{
	u32 value;

	/* enable the watchdog0 */
	value = pwatchdog_regs->mode & 0xffff;
	value &= ~(0x1 << 0);
	value |= WDOG0_WRITE_KEY | WDOG0_EN;
	pwatchdog_regs->mode = value;

	return OK;
}

/*
*********************************************************************************************************
*                                       DISABLE WATCHDOG
*
* Description:  disable watchdog.
*
* Arguments  :  none.
*
* Returns    :  OK if disable watchdog succeeded, others if failed.
*********************************************************************************************************
*/
s32 watchdog_disable(void)
{
	u32 value;

	/* disable the watchdog0 */
	value = pwatchdog_regs->mode & 0xffff;
	value &= ~(0x1 << 0);
	value |= WDOG0_WRITE_KEY | WDOG0_DIS;
	pwatchdog_regs->mode = value;

	return OK;
}
