/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : intc.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-3
* Descript: interrupt controller module.
* Update  : date                auther      ver     notes
*           2012-5-3 13:25:40   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "intc_i.h"

/* interrput controller registers offset */
#define CLIC_CFG		(RISCV_CLIC_BASE + 0x0)
#define CLIC_INFO		(RISCV_CLIC_BASE + 0x4)
#define CLIC_MINTTHRESH		(RISCV_CLIC_BASE + 0x8)
#define CLIC_INT_IP(n)		(RISCV_CLIC_BASE + 0x1000 + 0x4*n) // byte
#define CLIC_INT_IE(n)		(RISCV_CLIC_BASE + 0x1001 + 0x4*n) // byte
#define CLIC_INT_ATTR(n)	(RISCV_CLIC_BASE + 0x1002 + 0x4*n) // byte
#define CLIC_INT_CTL(n)		(RISCV_CLIC_BASE + 0x1003 + 0x4*n) // byte
#define CLIC_INT_REG(n)		(RISCV_CLIC_BASE + 0x1000 + 0x4*n) // word

#define CLINT_MSPI_REG		(RISCV_CLINT_BASE + 0x0000)
#define CLINT_MTIMECMPLO_REG	(RISCV_CLINT_BASE + 0x4000)
#define CLINT_MTIMECMPHI_REG	(RISCV_CLINT_BASE + 0x4004)
#define CLINT_MTIMELO_REG	(RISCV_CLINT_BASE + 0xBFF8)
#define CLINT_MTIMEHI_REG	(RISCV_CLINT_BASE + 0xBFFC)


/*
*********************************************************************************************************
*                                           INTERRUPT INIT
*
* Description:  initialize interrupt.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
s32 intc_init(void)
{
	return OK;
}

/*
*********************************************************************************************************
*                                         INTERRUPT EXIT
*
* Description:  exit interrupt.
*
* Arguments  :  none.
*
* Returns    :  OK if exit succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
s32 intc_exit(void)
{
	return OK;
}

/*
*********************************************************************************************************
*                                           ENABLE INTERRUPT
*
* Description:  enable a specific interrupt.
*
* Arguments  :  intno   : the source number of interrupt to which we want to enable.
*
* Returns    :  OK if enable interrupt succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
s32 intc_enable_interrupt(u32 intno)
{
	/*intno can't beyond then IRQ_SOURCE_MAX */
	ASSERT(intno < IRQ_SOUCE_MAX);

	writeb(readb(CLIC_INT_IE(intno)) | 0x1, CLIC_INT_IE(intno));

	INF("intno:%d interrupt enable\n", intno);

	return OK;
}

/*
*********************************************************************************************************
*                                           DISABLE INTERRUPT
*
* Description:  disable a specific interrupt.
*
* Arguments  :  intno  : the source number of interrupt which we want to disable.
*
* Returns    :  OK if disable interrupt succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
s32 intc_disable_interrupt(u32 intno)
{
	/*intno can't beyond then IRQ_SOURCE_MAX */
	ASSERT(intno < IRQ_SOUCE_MAX);

	writeb(readb(CLIC_INT_IE(intno)) & (~0x1), CLIC_INT_IE(intno));

	return OK;
}

s32 intc_interrupt_query_pending(u32 intno)
{
	/*intno can't beyond then IRQ_SOURCE_MAX */
	ASSERT(intno < IRQ_SOUCE_MAX);

	return (readb(CLIC_INT_IP(intno)) & 0x1);
}

s32 intc_interrupt_clear_pending(u32 intno)
{
	/*intno can't beyond then IRQ_SOURCE_MAX */
	ASSERT(intno < IRQ_SOUCE_MAX);

	writeb(readb(CLIC_INT_IP(intno)) | 0x1, CLIC_INT_IP(intno));

	return OK;
}

s32 intc_interrupt_is_enabled(u32 intno)
{
	/* intno can't beyond then IRQ_SOURCE_MAX */
	ASSERT(intno < IRQ_SOUCE_MAX);

	return (readb(CLIC_INT_IE(intno)) & 0x1);
}

/*
*********************************************************************************************************
*                                   GET CURRENT INTERRUPT
*
* Description: get the source number of current interrupt.
*
* Arguments  : none.
*
* Returns    : the source number of current interrupt.
*
* Note       :
*********************************************************************************************************
*/

extern u32 __get_MCAUSE(void);
u32 intc_get_current_interrupt(void)
{
	return (__get_MCAUSE() & 0xff);
}

s32 intc_set_mask(u32 intno, u32 mask)
{
	/* intno can't beyond then IRQ_SOURCE_MAX */
	ASSERT(intno < IRQ_SOUCE_MAX);

	writeb((readb(CLIC_INT_IE(intno)) & (~0x1)) | ((~mask) & 0x1), CLIC_INT_IE(intno));

	return OK;
}

s32 intc_set_group_config(u32 grp_irq_num, u32 mask)
{
	ASSERT(grp_irq_num < GRP_IRQ_MAX);

	u32 bit_os = grp_irq_num % 32;
	u32 reg_os = (grp_irq_num / 32) * 0x4;
	u32 reg = R_INTC_REG_BASE + 0xc0 + reg_os;

	writeb((readb(reg) & (~(0x1 << bit_os))) | ((mask & 0x1) << bit_os), reg);

	return OK;
}
