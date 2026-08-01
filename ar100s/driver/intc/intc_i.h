/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : intc_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-3
* Descript: interrupt controller internal header.
* Update  : date                auther      ver     notes
*           2012-5-3 13:27:40   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __INTC_I_H__
#define __INTC_I_H__

#include "include.h"

struct int_isr_node {
	__pISR_t pisr;		/*ISR process handler */
	void *parg;		/*argument for isr process */
};

/*local functions*/
s32 intc_init(void);
s32 intc_exit(void);
s32 intc_set_fiq_triggermode(u32 triggermode);
s32 intc_enable_interrupt(u32 intno);
s32 intc_disable_interrupt(u32 intno);
u32 intc_get_current_interrupt(void);
s32 intc_interrupt_is_enabled(u32 intno);
s32 intc_set_mask(u32 intno, u32 mask);
s32 intc_set_group_config(u32 grp_irq_num, u32 mask);
s32 intc_interrupt_query_pending(u32 intno);
s32 intc_interrupt_clear_pending(u32 intno);

s32 isr_default(void *arg);

#endif /*__INTC_I_H__*/
