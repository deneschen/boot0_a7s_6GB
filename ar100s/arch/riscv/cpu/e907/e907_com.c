/*
 * c906.c
 *
 *  Created on: 2020-6-29
 *      Author: Administrator
 *
 * Trimmed to the functions actually used by this firmware:
 *   - trap handler (hadle_trap) entered from crt0.S
 *   - interrupt save/restore primitives used by cpu_c.c
 *   - wakeup-source interrupt mask used by service/standby
 * The T-Head CMSIS intrinsics and cache helpers previously in this file
 * were unreferenced and have been removed.
 */

#include "include.h"

/* ###########################  Core Function Access  ########################### */

/**
 \brief   Disable IRQ Interrupts
 \details Disables IRQ interrupts by clearing the IE-bit in the PSR.
 Can only be executed in Privileged modes.
 */
void __disable_irq(void)
{
	__asm volatile("csrc mstatus, 8");
	__asm volatile("li a0, 0x800");
	__asm volatile("csrc mie, a0");
}

/**
 \brief   Get MSTATUS
 \details Returns the content of the MSTATUS Register.
 \return               MSTATUS Register value
 */
u32 __get_MSTATUS(void)
{
	u32 result;

	__asm volatile("csrr %0, mstatus" : "=r"(result));
	return result;
}

/**
 \brief   Set MSTATUS
 \details Writes the given value to the MSTATUS Register.
 \param [in]    mstatus  MSTATUS Register value to set
 */
void __set_MSTATUS(u32 mstatus)
{
	__asm volatile("csrw mstatus, %0" : : "r"(mstatus));
}

/**
 \brief   Get MCAUSE
 \details Returns the content of the MCAUSE Register.
 \return               MCAUSE Register value
 */
u32 __get_MCAUSE(void)
{
	u32 result;

	__asm volatile("csrr %0, mcause" : "=r"(result));
	return result;
}

/**
 \details Writes the given value to the SP Register.
 \param [in]    sp  SP Register value to set
 */
__attribute__((naked)) void __set_SP(u32 sp)
{
	__asm volatile("mv sp, a0\n\t"
		       "ret");
}

/* ##################################    IRQ Functions  ############################################ */

/**
 \brief   Save the Irq context
 \details save the psr result before disable irq.
 */
u32 c906_irq_save(void)
{
	u32 result;
	result = __get_MSTATUS();
	__disable_irq();
	return result;
}

/**
 \brief   Restore the Irq context
 \details restore saved primask state.
 \param [in]      irq_state  psr irq state.
 */
void c906_irq_restore(u32 irq_state)
{
	__set_MSTATUS(irq_state);
}

extern s32 platform_nmi_handler(void *parg);
s32 __attribute__((weak)) platform_nmi_handler(void *parg)
{
	return OK;
}

#define INTC_R_NMI_EXCEPTION    (24)
static s32 exception_entry(void)
{
	if ((__get_MCAUSE() & 0xff) == INTC_R_NMI_EXCEPTION) {
		platform_nmi_handler(NULL);
	}

	return OK;
}

u32 hadle_trap(u32 mcause, u32 epc)
{
	if (mcause >> 31) {
		/* interrupt */
		interrupt_entry();
	} else {
		exception_entry();
	}

	return epc;
}

/* e902 wakeup source configuration */
void  interrput_arch_set_mask(s32 intno, bool state)
{
	if (state) {

		if (intno / (MASK1_START_INTERRUPT -1))
			writel(readl(E902_WAKEUP_MASK1_REG) | (0x01 << (intno - MASK1_START_INTERRUPT)), E902_WAKEUP_MASK1_REG);
		else
			writel(readl(E902_WAKEUP_MASK0_REG) | (0x01 << (intno - MASK0_START_INTERRUPT)), E902_WAKEUP_MASK0_REG);
	} else {

		if (intno / (MASK1_START_INTERRUPT -1))
			writel(readl(E902_WAKEUP_MASK1_REG) & ~(0x01 << (intno - MASK1_START_INTERRUPT)), E902_WAKEUP_MASK1_REG);
		else
			writel(readl(E902_WAKEUP_MASK0_REG) & ~(0x01 << (intno - MASK0_START_INTERRUPT)), E902_WAKEUP_MASK0_REG);
	}
};
