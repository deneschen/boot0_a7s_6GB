/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                timer  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : timer_delay-extend.c
* By      : AWA1442
* Version : v1.0
* Date    : 2020-11-16
* Descript: timer delay service.
* Update  : date                auther      ver     notes
*           2020-11-16 14:12:17   AWA1442       1.0     Create this file.
*********************************************************************************************************
*/

#include "timer-extended.h"

extern volatile u32 timer_lock;

static s32 delay_wait_clear(u32 reg, u32 mask)
{
	u32 timeout = EXT_TIMER_POLL_LIMIT;

	while (readl(reg) & mask) {
		if (!--timeout)
			return -ETIMEOUT;
	}
	return OK;
}

static s32 delay_wait_pending(u32 timer_no, u64 deadline)
{
	u32 pending = 1U << timer_no;

	while (!(readl(EXT_TIMER_STA_REG) & pending)) {
		if (cpucfg_counter_read() >= deadline)
			return -ETIMEOUT;
	}
	return OK;
}

void time_mdelay(u32 ms)
{
	u64 ticks;
	u64 deadline;
	u32 value;

	if (timer_lock)
		return;

	/*
	 * check delay time too long
	 * ...
	 */
	ticks = (u64)delay_timer->ms_ticks * ms;
	if (!ms || ticks > 0xffffffffULL) {
		/* no delay */
		return;
	}

	/* config timer internal value */
	writel((u32)ticks, EXT_TIMER_IVL_REG(delay_timer->timer_no));

	/*  reload interval value to current value */
	value = readl(EXT_TIMER_CTRL_REG(delay_timer->timer_no));
	value |= EXT_TIMER_RELOAD;
	writel(value, EXT_TIMER_CTRL_REG(delay_timer->timer_no));
	if (delay_wait_clear(EXT_TIMER_CTRL_REG(delay_timer->timer_no),
			     EXT_TIMER_RELOAD) != OK)
		return;

	/* clear timer pending */
	writel((1 << delay_timer->timer_no), EXT_TIMER_STA_REG);

	/* start timer */
	value = readl(EXT_TIMER_CTRL_REG(delay_timer->timer_no));
	value |= EXT_TIMER_ENABLE;
	writel(value, EXT_TIMER_CTRL_REG(delay_timer->timer_no));

	/* The always-on counter is 24 MHz; allow one extra millisecond. */
	deadline = cpucfg_counter_read() + (u64)(ms + 1U) * 24000U;
	if (delay_wait_pending(delay_timer->timer_no, deadline) != OK)
		ERR("timer delay timeout: %d ms\n", ms);

	/* stop timer */
	value = readl(EXT_TIMER_CTRL_REG(delay_timer->timer_no));
	value &= ~EXT_TIMER_ENABLE;
	writel(value, EXT_TIMER_CTRL_REG(delay_timer->timer_no));

	/* clear timer pending */
	writel((1 << delay_timer->timer_no), EXT_TIMER_STA_REG);
}

void cnt64_udelay(u32 us)
{
	u64 expire = 0;

	if (us == 0) {
		/* no delay */
		return;
	}

	/* calc expire time */
	expire = (u64)us * 24U + cpucfg_counter_read();
	while (expire > cpucfg_counter_read()) {
		/* wait busy */
		;
	}
}

void time_udelay(u32 us)
{
	u32 cycles;
	u32 cpus_freq;

	if (us == 0) {
		/* no delay */
		return;
	}

	if (is_hosc_lock() || is_suspend_lock()) {
		cpus_freq = ccu_get_sclk_freq(CCU_SYS_CLK_CPUS);
		if (cpus_freq / 1000000) {
			cycles = (cpus_freq / 1000000) * us;
		 } else {
			cycles = (cpus_freq * us + (1000000 - 1)) / 1000000;
		 }
		time_cdelay(cycles);
	} else {
		cnt64_udelay(us);
	}

}
