/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                timer  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : timer-extend.c
* By      : AWA1442
* Version : v1.0
* Date    : 2020-11-16
* Descript: timer controller for new timer IP.
* Update  : date                auther      ver     notes
*           2020-11-16 14:12:17   AWA1442       1.0     Create this file.
*********************************************************************************************************
*/

#include "timer-extended.h"

volatile u32 timer_lock = 1;

/* delay timer handler */
struct timer *delay_timer;

/* the table of timers */
static struct timer timers[TIMERC_TIMERS_NUMBER] = {
	{ 0, TIMER_FREE, INTC_R_TIMER0_IRQ, 0, NULL, NULL },
	{ 1, TIMER_FREE, INTC_R_TIMER1_IRQ, 0, NULL, NULL },
};

static s32 timer_wait_clear(u32 reg, u32 mask)
{
	u32 timeout = EXT_TIMER_POLL_LIMIT;

	while (readl(reg) & mask) {
		if (!--timeout)
			return -ETIMEOUT;
	}
	return OK;
}

static s32 timer_ack_pending(struct timer *ptimer)
{
	u32 pending = 1U << ptimer->timer_no;
	u32 timeout = EXT_TIMER_POLL_LIMIT;

	writel(pending, EXT_TIMER_STA_REG);
	while (readl(EXT_TIMER_STA_REG) & pending) {
		if (!--timeout)
			return -ETIMEOUT;
	}
	return OK;
}


/*
*********************************************************************************************************
*                                       INIT TIMER
*
* Description:  initialize timer.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize timer succeeded, others if failed.
*********************************************************************************************************
*/
s32 timer_init(void)
{
	u32 index;
	u32 value;

	/* enbale timer clock gating. */
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_TIMER0_1, CCU_CLK_ON);

	/* set reset as de-assert state. */
	ccu_set_mclk_reset(CCU_MOD_CLK_R_TIMER0_1, CCU_CLK_NRESET);

	/* initialize timers */
	for (index = 0; index < TIMERC_TIMERS_NUMBER; index++) {
		writel(0x0, EXT_TIMER_CTRL_REG(index));

		/*
		 * timer tick time base on ms,
		 * source clock = pll-ref (fixed 24 MHz), pre-scale = 1.
		 * ms_ticks below assumes REFPLL is exactly 24.000 MHz.
		 */
		/* source clock = pll-ref */
		value = readl(EXT_TIMER_CLK_REG(index));
		value &= ~(0x7 << 4);
		value |= (0x4 << 4);
		writel(value, EXT_TIMER_CLK_REG(index));

		/* pre-scale = 1 */
		value = readl(EXT_TIMER_CLK_REG(index));
		value &= ~(0x7 << 1);
		value |= (0x0 << 1);
		value |= (0x1 << 0);
		writel(value, EXT_TIMER_CLK_REG(index));
#ifdef CFG_FPGA_PLATFORM
		timers[index].ms_ticks = 24 * 1000;	/* fix to 32k */
#else
		timers[index].ms_ticks = 24 * 1000;	/* 24M */
#endif
	}

	/*
	 * use timer[0] for system accurate delay service, tick base ms.
	 * single shot mode.
	 * by sunny at 2012-11-21 17:39:25.
	 */
	timers[0].status = TIMER_USED;
	delay_timer = &(timers[0]);

	/* set timer0 mode to single mode */
	value = readl(EXT_TIMER_CTRL_REG(0));
	value |= (0x1 << 7);
	writel(value, EXT_TIMER_CTRL_REG(0));

	/* register 24mhosc notifier call-back */
	ccu_24mhosc_reg_cb(timer_hosc_onoff_cb);
	timer_lock = 0;

	return OK;
}

/*
*********************************************************************************************************
*                                       EXIT TIMER
*
* Description:  exit timer.
*
* Arguments  :  none.
*
* Returns    :  OK if exit timer succeeded, others if failed.
*********************************************************************************************************
*/
s32 timer_exit(void)
{
	/* set reset as de-assert state. */
	ccu_set_mclk_reset(CCU_MOD_CLK_R_TIMER0_1, CCU_CLK_RESET);

	/* enbale timer clock gating. */
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_TIMER0_1, CCU_CLK_OFF);

	timer_lock = 1;

	return OK;
}

/*
*********************************************************************************************************
*                                       REQUEST TIMER
*
* Description:  request a hardware timer.
*
* Arguments  :  phdle   : the callback when the requested timer tick reached.
*               parg    : the argument for the callback.
*
* Returns    :  the handler if request hardware timer succeeded, NULL if failed.
*
* Note       :  the callback execute entironment : CPU disable interrupt response.
*********************************************************************************************************
*/
HANDLE timer_request(__pCBK_t phdle, void *parg)
{
	u32 cpsr;
	u32 index;
	struct timer *ptimer = NULL;

	if (timer_lock)
		return NULL;

	cpsr = cpu_disable_int();
	for (index = 0; index < TIMERC_TIMERS_NUMBER; index++) {
		if (timers[index].status == TIMER_FREE) {
			/* allocate this timer */
			ptimer = &(timers[index]);
			ptimer->status = TIMER_USED;
			ptimer->phandler = phdle;
			ptimer->parg = parg;
			break;
		}
	}
	cpu_enable_int(cpsr);

	if (ptimer == NULL) {
		/* no freed timer now */
		WRN("no free timer now\n");
		return NULL;
	}

	/* install timer isr */
	if (install_isr(ptimer->irq_no, timer_isr, (void *)ptimer) != OK) {
		cpsr = cpu_disable_int();
		ptimer->status = TIMER_FREE;
		ptimer->phandler = NULL;
		ptimer->parg = NULL;
		cpu_enable_int(cpsr);
		return NULL;
	}

	return (HANDLE) ptimer;
}

/*
*********************************************************************************************************
*                                       RELEASE TIMER
*
* Description:  release a hardware timer.
*
* Arguments  :  htimer  : the handler of the released timer.
*
* Returns    :  OK if release hardware timer succeeded, others if failed.
*********************************************************************************************************
*/
s32 timer_release(HANDLE htimer)
{
	struct timer *ptimer = (struct timer *)htimer;
	u32 cpsr;
	s32 ret;

	ASSERT(ptimer != NULL);
	if (!ptimer || ptimer == delay_timer || ptimer->status != TIMER_USED)
		return -EINVAL;

	ret = timer_stop(htimer);
	if (ret != OK)
		return ret;
	ret = uninstall_isr(ptimer->irq_no, timer_isr);
	if (ret != OK)
		return ret;

	cpsr = cpu_disable_int();
	ptimer->status = TIMER_FREE;
	ptimer->phandler = NULL;
	ptimer->parg = NULL;
	cpu_enable_int(cpsr);

	return OK;
}

/*
*********************************************************************************************************
*                                       START TIMER
*
* Description:  start a hardware timer.
*
* Arguments  :  htimer  : the timer handler which we want to start.
*               period  : the period of the timer trigger, base on ms.
*               mode    : the mode the timer trigger, details please
*                         refer to timer trigger mode.
*
* Returns    :  OK if start hardware timer succeeded, others if failed.
*********************************************************************************************************
*/
s32 timer_start(HANDLE htimer, u32 period, u32 mode)
{
	u64 ticks;
	u32 value;
	s32 ret;

	struct timer *ptimer = (struct timer *)htimer;

	ASSERT(ptimer != NULL);

	if (timer_lock || ptimer->status != TIMER_USED)
		return -EACCES;
	if (!period || mode > TIMER_MODE_ONE_SHOOT)
		return -EINVAL;
	ticks = (u64)ptimer->ms_ticks * period;
	if (ticks > 0xffffffffULL)
		return -EINVAL;

	/* set timer period */
	writel((u32)ticks, EXT_TIMER_IVL_REG(ptimer->timer_no));

	/* reload interval value to current value */
	value = readl(EXT_TIMER_CTRL_REG(ptimer->timer_no));
	value |= EXT_TIMER_RELOAD;
	writel(value, EXT_TIMER_CTRL_REG(ptimer->timer_no));
	ret = timer_wait_clear(EXT_TIMER_CTRL_REG(ptimer->timer_no),
			       EXT_TIMER_RELOAD);
	if (ret != OK)
		return ret;

	/* set timer mode */
	value = readl(EXT_TIMER_CTRL_REG(ptimer->timer_no));
	value &= ~EXT_TIMER_MODE;
	value |= (mode << 7);
	writel(value, EXT_TIMER_CTRL_REG(ptimer->timer_no));

	/* clear timer pending */
	ret = timer_ack_pending(ptimer);
	if (ret != OK)
		return ret;

	/* enable timer interrupt */
	value = readl(EXT_TIMER_IRQ_REG);
	value |= (1 << ptimer->timer_no);
	writel(value, EXT_TIMER_IRQ_REG);
	ret = interrupt_enable(ptimer->irq_no);
	if (ret != OK)
		return ret;

	/* enable timer */
	value = readl(EXT_TIMER_CTRL_REG(ptimer->timer_no));
	value |= EXT_TIMER_ENABLE;
	writel(value, EXT_TIMER_CTRL_REG(ptimer->timer_no));

	return OK;
}

/*
*********************************************************************************************************
*                                       STOP TIMER
*
* Description:  stop a hardware timer.
*
* Arguments  :  htimer  : the timer handler which we want to stop.
*
* Returns    :  OK if stop hardware timer succeeded, others if failed.
*********************************************************************************************************
*/
s32 timer_stop(HANDLE htimer)
{
	struct timer *ptimer = (struct timer *)htimer;

	u32 value;

	ASSERT(ptimer != NULL);

	if (timer_lock || ptimer->status != TIMER_USED)
		return -EACCES;

	/* disable timer */
	value = readl(EXT_TIMER_CTRL_REG(ptimer->timer_no));
	value &= ~EXT_TIMER_ENABLE;
	writel(value, EXT_TIMER_CTRL_REG(ptimer->timer_no));

	/* disable timer irq */
	value = readl(EXT_TIMER_IRQ_REG);
	value &= ~(1 << ptimer->timer_no);
	writel(value, EXT_TIMER_IRQ_REG);
	if (interrupt_disable(ptimer->irq_no) != OK)
		return -EFAIL;

	/* clear timer pending */
	return timer_ack_pending(ptimer);
}

/*
*********************************************************************************************************
*                                       TIMER ISR
*
* Description:  the isr for timer interrupt.
*
* Arguments  :  parg    : the argument for timer isr.
*
* Returns    :  TRUE if process timer interrupt succeeded, others if failed.
*********************************************************************************************************
*/
s32 timer_isr(void *parg)
{
	struct timer *ptimer = (struct timer *)parg;

	if (timer_lock)
		return -EACCES;

	/* check pending status valid or not */
	if (readl(EXT_TIMER_STA_REG) & (1 << ptimer->timer_no)) {
		/* TIMER_STA is W1C; acknowledge even if release raced the IRQ. */
		if (timer_ack_pending(ptimer) != OK)
			return -ETIMEOUT;

		/* process the timer handler */
		if (ptimer->phandler == NULL) {
			WRN("timer irq handler not install\n");
			return FALSE;
		}

		/* handler timer irq */
		ptimer->phandler(ptimer->parg);

		return TRUE;
	}
	return FALSE;
}

s32 timer_hosc_onoff_cb(u32 message, u32 aux)
{
#ifdef CFG_FPGA_PLATFORM
	return OK;
#else

	u32 index;
	u32 value;

	if (timer_lock)
		return -EACCES;

	switch (message) {
	case CCU_HOSC_WILL_OFF_NOTIFY:
		{
			/*
			 * 24m hosc will power-off,
			 * timer switch clock source to 32k losc.
			 */
			INF("24m hosc will power-off notify\n");
			for (index = 0; index < TIMERC_TIMERS_NUMBER; index++) {
				/* set source clock to 32k */
				value = readl(EXT_TIMER_CLK_REG(index));
				value &= ~(0x7 << 4);
				value |= (0x1 << 4);
				writel(value, EXT_TIMER_CLK_REG(index));

				timers[index].ms_ticks = 32;	/* 32 */
				value = readl(EXT_TIMER_IVL_REG(index));
				value = value / (24000 / 32);
				writel(value, EXT_TIMER_IVL_REG(index));
			}
			return OK;
		}
	case CCU_HOSC_ON_READY_NOTIFY:
		{
			/*
			 * 24m hosc power-on already,
			 * timer swith source clock to 24m hosc.
			 */
			INF("24m hosc power-on ready notify\n");
			for (index = 0; index < TIMERC_TIMERS_NUMBER; index++) {
				value = readl(EXT_TIMER_IVL_REG(index));
				value = value * (24000 / 32);
				writel(value, EXT_TIMER_IVL_REG(index));

				/* set source clock to pll-ref (24M) */
				value = readl(EXT_TIMER_CLK_REG(index));
				value &= ~(0x7 << 4);
				value |= (0x4 << 4);
				writel(value, EXT_TIMER_CLK_REG(index));
				timers[index].ms_ticks = 24 * 1000;	/* 24k */
			}
			return OK;
		}
	default:
		{
			break;
		}
	}
	return -ESRCH;
#endif
}
