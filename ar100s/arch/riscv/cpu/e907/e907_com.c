/*
 * c906.c
 *
 *  Created on: 2020-6-29
 *      Author: Administrator
 */

#include "e907_com.h"
//extern vector_table;
irqhandle vectors[256];

/* ###########################  Core Function Access  ########################### */
/** \ingroup  c906_Core_FunctionInterface
 \defgroup c906_Core_RegAccFunctions CSI Core Register Access Functions
 @{
 */
/**
 \brief   Enable IRQ Interrupts
 \details Enables IRQ interrupts by setting the IE-bit in the PSR.
 Can only be executed in Privileged modes.
 */
void __enable_irq(void)
{
	__asm volatile("csrs mstatus, 8");
	__asm volatile("li a0, 0x800");
	__asm volatile("csrs mie, a0");

}

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
 \brief   Get MXSTATUS
 \details Returns the content of the MXSTATUS Register.
 \return               MXSTATUS Register value
 */
u32 __get_MXSTATUS(void)
{
	u32 result;

	__asm volatile("csrr %0, 0x7c0" : "=r"(result)); /* mxstatus */
	return result;
}

/**
 \brief   Get MEXSTATUS
 \details Returns the content of the MEXSTATUS Register.
 \return               MEXSTATUS Register value
 */
u32 __get_MEXSTATUS(void)
{
	u32 result;

	__asm volatile("csrr %0, 0x7e1" : "=r"(result)); /* mexstatus */
	return result;
}

/**
 \brief   Set MEPC
 \details Writes the given value to the MEPC Register.
 \param [in]    mstatus  MEPC Register value to set
 */
void __set_MEPC(u32 mepc)
{
	__asm volatile("csrw mepc, %0" : : "r"(mepc));
}

/**
 \brief   Set MXSTATUS
 \details Writes the given value to the MXSTATUS Register.
 \param [in]    mxstatus  MXSTATUS Register value to set
 */
void __set_MXSTATUS(u32 mxstatus)
{
	__asm volatile("csrw 0x7c0, %0" : : "r"(mxstatus)); /* mxstatus */
}

/**
 \brief   Set MEXSTATUS
 \details Writes the given value to the MEXSTATUS Register.
 \param [in]    mexstatus  MEXSTATUS Register value to set
 */
void __set_MEXSTATUS(u32 mexstatus)
{
	__asm volatile("csrw 0x7e1, %0" : : "r"(mexstatus)); /* mexstatus */
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
 \brief   Get MHCR
 \details Returns the content of the MHCR Register.
 \return               MHCR Register value
 */
u32 __get_MHCR(void)
{
	u32 result;

	__asm volatile("csrr %0, 0x7c1" : "=r"(result)); /* mhcr */
	return result;
}

/**
 \brief   Set MHCR
 \details Writes the given value to the MHCR Register.
 \param [in]    mstatus  MHCR Register value to set
 */
void __set_MHCR(u32 mhcr)
{
	__asm volatile("csrw 0x7c1, %0" : : "r"(mhcr)); /* mhcr */
}

/**
 \brief   Get MISA Register
 \details Returns the content of the MISA Register.
 \return               MISA Register value
 */
u32 __get_MISA(void)
{
	u32 result;

	__asm volatile("csrr %0, misa" : "=r"(result));
	return result;
}

/**
 \brief   Set MISA
 \details Writes the given value to the MISA Register.
 \param [in]    misa  MISA Register value to set
 */
void __set_MISA(u32 misa)
{
	__asm volatile("csrw misa, %0" : : "r"(misa));
}

/**
 \brief   Get MIE Register
 \details Returns the content of the MIE Register.
 \return               MIE Register value
 */
u32 __get_MIE(void)
{
	u32 result;

	__asm volatile("csrr %0, mie" : "=r"(result));
	return result;
}

/**
 \brief   Set MIE
 \details Writes the given value to the MIE Register.
 \param [in]    mie  MIE Register value to set
 */
void __set_MIE(u32 mie)
{
	__asm volatile("csrw mie, %0" : : "r"(mie));
}

/**
 \brief   Get MTVEC Register
 \details Returns the content of the MTVEC Register.
 \return               MTVEC Register value
 */
u32 __get_MTVEC(void)
{
	u32 result;

	__asm volatile("csrr %0, mtvec" : "=r"(result));
	return result;
}

/**
 \brief   Set MTVEC
 \details Writes the given value to the MTVEC Register.
 \param [in]    mtvec  MTVEC Register value to set
 */
void __set_MTVEC(u32 mtvec)
{
	__asm volatile("csrw mtvec, %0" : : "r"(mtvec));
}

/**
 \brief   Set MTVT
 \details Writes the given value to the MTVT Register.
 \param [in]    mtvt  MTVT Register value to set
 */
void __set_MTVT(u32 mtvt)
{
	__asm volatile("csrw 0x307, %0" : : "r"(mtvt)); /* mtvt */
}

/**
 \brief   Get MTVT Register
 \details Returns the content of the MTVT Register.
 \return               MTVT Register value
 */
u32 __get_MTVT(void)
{
	u32 result;

	__asm volatile("csrr %0, 0x307" : "=r"(result)); /* mtvt */
	return result;
}

/**
 \brief   Get SP
 \details Returns the content of the SP Register.
 \return               SP Register value
 */
u32 __get_SP(void)
{
	u32 result;

	__asm volatile("mv %0, sp" : "=r"(result));
	return result;
}

/**
 \brief   Set SP
 \details Writes the given value to the SP Register.
 \param [in]    sp  SP Register value to set
 */
__attribute__((naked)) void __set_SP(u32 sp)
{
	__asm volatile("mv sp, a0\n\t"
		       "ret");
}

/**
 \brief   Get MSCRATCH Register
 \details Returns the content of the MSCRATCH Register.
 \return               MSCRATCH Register value
 */
u32 __get_MSCRATCH(void)
{
	u32 result;

	__asm volatile("csrr %0, mscratch" : "=r"(result));
	return result;
}

/**
 \brief   Set MSCRATCH
 \details Writes the given value to the MSCRATCH Register.
 \param [in]    mscratch  MSCRATCH Register value to set
 */
void __set_MSCRATCH(u32 mscratch)
{
	__asm volatile("csrw mscratch, %0" : : "r"(mscratch));
}

/**
 \brief   Get MCAUSE Register
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
 \brief   Get MNXTI Register
 \details Returns the content of the MNXTI Register.
 \return               MNXTI Register value
 */
u32 __get_MNXTI(void)
{
	u32 result;

	__asm volatile("csrr %0, 0x345" : "=r"(result)); /* mnxti */
	return result;
}

/**
 \brief   Set MNXTI
 \details Writes the given value to the MNXTI Register.
 \param [in]    mnxti  MNXTI Register value to set
 */
void __set_MNXTI(u32 mnxti)
{
	__asm volatile("csrw 0x345, %0" : : "r"(mnxti)); /* mnxti */
}

/**
 \brief   Get MINTSTATUS Register
 \details Returns the content of the MINTSTATUS Register.
 \return               MINTSTATUS Register value
 */
u32 __get_MINTSTATUS(void)
{
	u32 result;

	__asm volatile("csrr %0, 0x346" : "=r"(result)); /* mintstatus */
	return result;
}

/**
 \brief   Get MTVAL Register
 \details Returns the content of the MTVAL Register.
 \return               MTVAL Register value
 */
u32 __get_MTVAL(void)
{
	u32 result;

	__asm volatile("csrr %0, mtval" : "=r"(result));
	return result;
}

/**
 \brief   Get MIP Register
 \details Returns the content of the MIP Register.
 \return               MIP Register value
 */
u32 __get_MIP(void)
{
	u32 result;

	__asm volatile("csrr %0, mip" : "=r"(result));
	return result;
}

/**
 \brief   Set MIP
 \details Writes the given value to the MIP Register.
 \param [in]    mip  MIP Register value to set
 */
void __set_MIP(u32 mip)
{
	__asm volatile("csrw mip, %0" : : "r"(mip));
}

/**
 \brief   Get MCYCLEL Register
 \details Returns the content of the MCYCLEL Register.
 \return               MCYCLE Register value
 */
u32 __get_MCYCLE(void)
{
	u32 result;

	__asm volatile("csrr %0, mcycle" : "=r"(result));
	return result;
}

/**
 \brief   Get MCYCLEH Register
 \details Returns the content of the MCYCLEH Register.
 \return               MCYCLEH Register value
 */
u32 __get_MCYCLEH(void)
{
	u32 result;

	__asm volatile("csrr %0, mcycleh" : "=r"(result));
	return result;
}

/**
 \brief   Get MINSTRET Register
 \details Returns the content of the MINSTRET Register.
 \return               MINSTRET Register value
 */
u32 __get_MINSTRET(void)
{
	u32 result;

	__asm volatile("csrr %0, minstret" : "=r"(result));
	return result;
}

/**
 \brief   Get MINSTRETH Register
 \details Returns the content of the MINSTRETH Register.
 \return               MINSTRETH Register value
 */
u32 __get_MINSTRETH(void)
{
	u32 result;

	__asm volatile("csrr %0, minstreth" : "=r"(result));
	return result;
}

/**
 \brief   Get MVENDORID Register
 \details Returns the content of the MVENDROID Register.
 \return               MVENDORID Register value
 */
u32 __get_MVENDORID(void)
{
	u32 result;

	__asm volatile("csrr %0, mvendorid" : "=r"(result));
	return result;
}

/**
 \brief   Get MARCHID Register
 \details Returns the content of the MARCHID Register.
 \return               MARCHID Register value
 */
u32 __get_MARCHID(void)
{
	u32 result;

	__asm volatile("csrr %0, marchid" : "=r"(result));
	return result;
}

/**
 \brief   Get MIMPID Register
 \details Returns the content of the MIMPID Register.
 \return               MIMPID Register value
 */
u32 __get_MIMPID(void)
{
	u32 result;

	__asm volatile("csrr %0, mimpid" : "=r"(result));
	return result;
}

/**
 \brief   Get MHARTID Register
 \details Returns the content of the MHARTID Register.
 \return               MHARTID Register value
 */
u32 __get_MHARTID(void)
{
	u32 result;

	__asm volatile("csrr %0, mhartid" : "=r"(result));
	return result;
}

/**
 \brief   Get PMPCFGx Register
 \details Returns the content of the PMPCFGx Register.
 \return               PMPCFGx Register value
 */
u32 __get_PMPCFG0(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpcfg0" : "=r"(result));
	return result;
}

u32 __get_PMPCFG1(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpcfg1" : "=r"(result));
	return result;
}

u32 __get_PMPCFG2(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpcfg2" : "=r"(result));
	return result;
}

u32 __get_PMPCFG3(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpcfg3" : "=r"(result));
	return result;
}

/**
 \brief   Get PMPxCFG Register by index
 \details Returns the content of the PMPxCFG Register.
 \param [in]    idx    PMP region index
 \return               PMPxCFG Register value
 */
u8 __get_PMPxCFG(u32 idx)
{
	u32 pmpcfgx = 0;

	if (idx < 4) {
		pmpcfgx = __get_PMPCFG0();
	} else if (idx >= 4 && idx < 8) {
		idx -= 4;
		pmpcfgx = __get_PMPCFG1();
	} else if (idx >= 8 && idx < 12) {
		idx -= 8;
		pmpcfgx = __get_PMPCFG2();
	} else if (idx >= 12 && idx < 16) {
		idx -= 12;
		pmpcfgx = __get_PMPCFG3();
	} else {
		return 0;
	}

	return (u8)((pmpcfgx & (0xFF << (idx << 3))) >> (idx << 3));
}

/**
 \brief   Set PMPCFGx
 \details Writes the given value to the PMPCFGx Register.
 \param [in]    pmpcfg  PMPCFGx Register value to set
 */
void __set_PMPCFG0(u32 pmpcfg)
{
	__asm volatile("csrw pmpcfg0, %0" : : "r"(pmpcfg));
}

void __set_PMPCFG1(u32 pmpcfg)
{
	__asm volatile("csrw pmpcfg1, %0" : : "r"(pmpcfg));
}

void __set_PMPCFG2(u32 pmpcfg)
{
	__asm volatile("csrw pmpcfg2, %0" : : "r"(pmpcfg));
}

void __set_PMPCFG3(u32 pmpcfg)
{
	__asm volatile("csrw pmpcfg3, %0" : : "r"(pmpcfg));
}

/**
 \brief   Set PMPxCFG by index
 \details Writes the given value to the PMPxCFG Register.
 \param [in]    idx      PMPx region index
 \param [in]    pmpxcfg  PMPxCFG Register value to set
 */
void __set_PMPxCFG(u32 idx, u8 pmpxcfg)
{
	u32 pmpcfgx = 0;

	if (idx < 4) {
		pmpcfgx = __get_PMPCFG0();
		pmpcfgx = (pmpcfgx & ~(0xFF << (idx << 3))) | (pmpxcfg << (idx << 3));
		__set_PMPCFG0(pmpcfgx);
	} else if (idx >= 4 && idx < 8) {
		idx -= 4;
		pmpcfgx = __get_PMPCFG1();
		pmpcfgx = (pmpcfgx & ~(0xFF << (idx << 3))) | (pmpxcfg << (idx << 3));
		__set_PMPCFG1(pmpcfgx);
	} else if (idx >= 8 && idx < 12) {
		idx -= 8;
		pmpcfgx = __get_PMPCFG2();
		pmpcfgx = (pmpcfgx & ~(0xFF << (idx << 3))) | (pmpxcfg << (idx << 3));
		__set_PMPCFG2(pmpcfgx);
	} else if (idx >= 12 && idx < 16) {
		idx -= 12;
		pmpcfgx = __get_PMPCFG3();
		pmpcfgx = (pmpcfgx & ~(0xFF << (idx << 3))) | (pmpxcfg << (idx << 3));
		__set_PMPCFG3(pmpcfgx);
	} else {
		return;
	}
}

/**
 \brief   Get PMPADDRx Register
 \details Returns the content of the PMPADDRx Register.
 \return               PMPADDRx Register value
 */
u32 __get_PMPADDR0(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr0" : "=r"(result));
	return result;
}

u32 __get_PMPADDR1(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr1" : "=r"(result));
	return result;
}

u32 __get_PMPADDR2(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr2" : "=r"(result));
	return result;
}

u32 __get_PMPADDR3(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr3" : "=r"(result));
	return result;
}

u32 __get_PMPADDR4(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr4" : "=r"(result));
	return result;
}

u32 __get_PMPADDR5(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr5" : "=r"(result));
	return result;
}

u32 __get_PMPADDR6(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr6" : "=r"(result));
	return result;
}

u32 __get_PMPADDR7(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr7" : "=r"(result));
	return result;
}

u32 __get_PMPADDR8(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr8" : "=r"(result));
	return result;
}

u32 __get_PMPADDR9(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr9" : "=r"(result));
	return result;
}

u32 __get_PMPADDR10(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr10" : "=r"(result));
	return result;
}

u32 __get_PMPADDR11(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr11" : "=r"(result));
	return result;
}

u32 __get_PMPADDR12(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr12" : "=r"(result));
	return result;
}

u32 __get_PMPADDR13(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr13" : "=r"(result));
	return result;
}

u32 __get_PMPADDR14(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr14" : "=r"(result));
	return result;
}

u32 __get_PMPADDR15(void)
{
	u32 result;

	__asm volatile("csrr %0, pmpaddr15" : "=r"(result));
	return result;
}

/**
 \brief   Get PMPADDRx Register by index
 \details Returns the content of the PMPADDRx Register.
 \param [in]    idx    PMP region index
 \return               PMPADDRx Register value
 */
u32 __get_PMPADDRx(u32 idx)
{
	switch (idx) {
	case 0:
		return __get_PMPADDR0();
	case 1:
		return __get_PMPADDR1();
	case 2:
		return __get_PMPADDR2();
	case 3:
		return __get_PMPADDR3();
	case 4:
		return __get_PMPADDR4();
	case 5:
		return __get_PMPADDR5();
	case 6:
		return __get_PMPADDR6();
	case 7:
		return __get_PMPADDR7();
	case 8:
		return __get_PMPADDR8();
	case 9:
		return __get_PMPADDR9();
	case 10:
		return __get_PMPADDR10();
	case 11:
		return __get_PMPADDR11();
	case 12:
		return __get_PMPADDR12();
	case 13:
		return __get_PMPADDR13();
	case 14:
		return __get_PMPADDR14();
	case 15:
		return __get_PMPADDR15();
	default:
		return 0;
	}
}

/**
 \brief   Set PMPADDRx
 \details Writes the given value to the PMPADDRx Register.
 \param [in]    pmpaddr  PMPADDRx Register value to set
 */
void __set_PMPADDR0(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr0, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR1(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr1, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR2(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr2, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR3(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr3, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR4(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr4, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR5(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr5, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR6(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr6, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR7(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr7, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR8(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr8, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR9(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr9, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR10(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr10, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR11(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr11, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR12(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr12, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR13(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr13, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR14(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr14, %0" : : "r"(pmpaddr));
}

void __set_PMPADDR15(u32 pmpaddr)
{
	__asm volatile("csrw pmpaddr15, %0" : : "r"(pmpaddr));
}

/**
 \brief   Set PMPADDRx by index
 \details Writes the given value to the PMPADDRx Register.
 \param [in]    idx      PMP region index
 \param [in]    pmpaddr  PMPADDRx Register value to set
 */
void __set_PMPADDRx(u32 idx, u32 pmpaddr)
{
	switch (idx) {
	case 0:
		__set_PMPADDR0(pmpaddr);
		break;
	case 1:
		__set_PMPADDR1(pmpaddr);
		break;
	case 2:
		__set_PMPADDR2(pmpaddr);
		break;
	case 3:
		__set_PMPADDR3(pmpaddr);
		break;
	case 4:
		__set_PMPADDR4(pmpaddr);
		break;
	case 5:
		__set_PMPADDR5(pmpaddr);
		break;
	case 6:
		__set_PMPADDR6(pmpaddr);
		break;
	case 7:
		__set_PMPADDR7(pmpaddr);
		break;
	case 8:
		__set_PMPADDR8(pmpaddr);
		break;
	case 9:
		__set_PMPADDR9(pmpaddr);
		break;
	case 10:
		__set_PMPADDR10(pmpaddr);
		break;
	case 11:
		__set_PMPADDR11(pmpaddr);
		break;
	case 12:
		__set_PMPADDR12(pmpaddr);
		break;
	case 13:
		__set_PMPADDR13(pmpaddr);
		break;
	case 14:
		__set_PMPADDR14(pmpaddr);
		break;
	case 15:
		__set_PMPADDR15(pmpaddr);
		break;
	default:
		return;
	}
}

/**
 \brief   Enable interrupts and exceptions
 \details Enables interrupts and exceptions by setting the IE-bit and EE-bit in the PSR.
 Can only be executed in Privileged modes.
 */
void __enable_excp_irq(void)
{
	__enable_irq();
}

/**
 \brief   Disable interrupts and exceptions
 \details Disables interrupts and exceptions by clearing the IE-bit and EE-bit in the PSR.
 Can only be executed in Privileged modes.
 */
void __disable_excp_irq(void)
{
	__disable_irq();
}

#define __c906_GCC_OUT_REG(r) "=r" (r)
#define __c906_GCC_USE_REG(r) "r" (r)

/**
 \brief   No Operation
 \details No Operation does nothing. This instruction can be used for code alignment purposes.
 */
void __NOP(void)
{
	__asm volatile("nop");
}

/**
 \brief   return from M-MODE
 \details return from M-MODE.
 */
void __MRET(void)
{
	__asm volatile("mret");
}

/**
 \brief   Wait For Interrupt
 \details Wait For Interrupt is a hint instruction that suspends execution until one of a number of events occurs.
 */
void __WFI(void)
{
	__asm volatile("wfi");
}

/**
 \brief   Wait For Interrupt
 \details Wait For Interrupt is a hint instruction that suspends execution until one interrupt occurs.
 */
void __WAIT(void)
{
	__asm volatile("wfi");
}

/**
 \brief   Doze For Interrupt
 \details Doze For Interrupt is a hint instruction that suspends execution until one interrupt occurs.
 */
void __DOZE(void)
{
	__asm volatile("wfi");
}

/**
 \brief   Stop For Interrupt
 \details Stop For Interrupt is a hint instruction that suspends execution until one interrupt occurs.
 */
void __STOP(void)
{
	__asm volatile("wfi");
}

/**
 \brief   Instruction Synchronization Barrier
 \details Instruction Synchronization Barrier flushes the pipeline in the processor,
 so that all instructions following the ISB are fetched from cache or memory,
 after the instruction has been completed.
 */
void __ISB(void)
{
	__asm volatile("fence");
}

/**
 \brief   Data Synchronization Barrier
 \details Acts as a special kind of Data Memory Barrier.
 It completes when all explicit memory accesses before this instruction complete.
 */
void __DSB(void)
{
	__asm volatile("fence");
}

/**
 \brief   Invalid all icache
 \details invalid all icache.
 */
void __ICACHE_IALL(void)
{
//    __asm volatile("icache.iall");
	asm volatile(".word 0x0100000b":::"memory");
}

/**
 \brief   Invalid Icache by addr
 \details Invalid Icache by addr.
 \param [in] addr  operate addr
 */
//void __ICACHE_IPA(u32 addr)
//{
//    __asm volatile("icache.ipa %0" : : "r"(addr));
//}
/**
 \brief   Invalid all dcache
 \details invalid all dcache.
 */
void __DCACHE_IALL(void)
{
//    __asm volatile("dcache.iall");
	asm volatile(".word 0x0020000b":::"memory");
}

/**
 \brief   Clear all dcache
 \details clear all dcache.
 */
void __DCACHE_CALL(void)
{
//    __asm volatile("dcache.call");
	asm volatile(".word 0x0010000b":::"memory");
}

/**
 \brief   Clear&invalid all dcache
 \details clear & invalid all dcache.
 */
void __DCACHE_CIALL(void)
{
//    __asm volatile("dcache.ciall");
	asm volatile(".word 0x0030000b":::"memory");
}

//#if (__L2CACHE_PRESENT == 1U)
/**
 \brief   Invalid L2 cache
 \details invalid L2 cache.
 */
/*
 void __L2CACHE_IALL(void)
 {
 __asm volatile("l2cache.iall");
 }
 */

/**
 \brief   Clear L2cache
 \details clear L2cache.
 */
/*
 void __L2CACHE_CALL(void)
 {
 __asm volatile("l2cache.call");
 }
 */

/**
 \brief   Clear&invalid L2cache
 \details clear & invalid L2cache.
 */
/*
 void __L2CACHE_CIALL(void)
 {
 __asm volatile("l2cache.ciall");
 }
 */
//#endif
/**
 \brief   Invalid Dcache by addr
 \details Invalid Dcache by addr.
 \param [in] addr  operate addr
 */
/*
 void __DCACHE_IPA(u32 addr)
 {
 __asm volatile("dcache.ipa %0" : : "r"(addr));
 }
 */

/**
 \brief   Clear Dcache by addr
 \details Clear Dcache by addr.
 \param [in] addr  operate addr
 */
/*
 void __DCACHE_CPA(u32 addr)
 {
 __asm volatile("dcache.cpa %0" : : "r"(addr));
 }
 */

/**
 \brief   Clear & Invalid Dcache by addr
 \details Clear & Invalid Dcache by addr.
 \param [in] addr  operate addr
 */
/*
 void __DCACHE_CIPA(u32 addr)
 {
 __asm volatile("dcache.cipa %0" : : "r"(addr));
 }
 */

/**
 \brief   Data Memory Barrier
 \details Ensures the apparent order of the explicit memory operations before
 and after the instruction, without ensuring their completion.
 */
void __DMB(void)
{
	__asm volatile("fence");
}

/**
 \brief   Reverse byte order (32 bit)
 \details Reverses the byte order in integer value.
 \param [in]    value  Value to reverse
 \return               Reversed value
 */
/*
 u32 __REV(u32 value)
 {
 return __builtin_bswap32(value);
 }
 */

/**
 \brief   Reverse byte order (16 bit)
 \details Reverses the byte order in two unsigned short values.
 \param [in]    value  Value to reverse
 \return               Reversed value
 */
u32 __REV16(u32 value)
{
	u32 result;

	result = ((value & 0xFF000000) >> 8) | ((value & 0x00FF0000) << 8)
			| ((value & 0x0000FF00) >> 8) | ((value & 0x000000FF) << 8);

	return result;
}

/**
 \brief   Reverse byte order in signed short value
 \details Reverses the byte order in a signed short value with sign extension to integer.
 \param [in]    value  Value to reverse
 \return               Reversed value
 */
s32 __REVSH(s32 value)
{
	return (short) (((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8));
}

/**
 \brief   Rotate Right in unsigned value (32 bit)
 \details Rotate Right (immediate) provides the value of the contents of a register rotated by a variable number of bits.
 \param [in]    op1  Value to rotate
 \param [in]    op2  Number of Bits to rotate
 \return               Rotated value
 */
u32 __ROR(u32 op1, u32 op2)
{
	return (op1 >> op2) | (op1 << (32U - op2));
}

/**
 \brief   Breakpoint
 \details Causes the processor to enter Debug state
 Debug tools can use this to investigate system state when the instruction at a particular address is reached.
 */
void __BKPT(void)
{
	__asm volatile("ebreak");
}

/**
 \brief   Reverse bit order of value
 \details Reverses the bit order of the given value.
 \param [in]    value  Value to reverse
 \return               Reversed value
 */
u32 __RBIT(u32 value)
{
	u32 result;

	s32 s = 4 /*sizeof(v)*/* 8 - 1; /* extra shift needed at end */

	result = value; /* r will be reversed bits of v; first get LSB of v */

	for (value >>= 1U; value; value >>= 1U) {
		result <<= 1U;
		result |= value & 1U;
		s--;
	}

	result <<= s; /* shift when v's highest bits are zero */

	return result;
}

/**
 \brief   Count leading zeros
 \details Counts the number of leading zeros of a data value.
 \param [in]  value  Value to count the leading zeros
 \return             number of leading zeros in value
 */
#define __CLZ             __builtin_clz
/**
 \details This function saturates a signed value.
 \param [in]    x   Value to be saturated
 \param [in]    y   Bit position to saturate to [1..32]
 \return            Saturated value.
 */
s32 __SSAT(s32 x, u32 y)
{
	s32 posMax, negMin;
	u32 i;

	posMax = 1;

	for (i = 0; i < (y - 1); i++) {
		posMax = posMax * 2;
	}

	if (x > 0) {
		posMax = (posMax - 1);

		if (x > posMax) {
			x = posMax;
		}

//    x &= (posMax * 2 + 1);
	} else {
		negMin = -posMax;

		if (x < negMin) {
			x = negMin;
		}

//    x &= (posMax * 2 - 1);
	}

	return x;
}

/**
 \brief   Unsigned Saturate
 \details Saturates an unsigned value.
 \param [in]  value  Value to be saturated
 \param [in]    sat  Bit position to saturate to (0..31)
 \return             Saturated value
 */
u32 __USAT(u32 value, u32 sat)
{
	u32 result;

	if ((((0xFFFFFFFF >> sat) << sat) & value) != 0) {
		result = 0xFFFFFFFF >> (32 - sat);
	} else {
		result = value;
	}

	return result;
}

/**
 \brief   Unsigned Saturate for internal use
 \details Saturates an unsigned value, should not call directly.
 \param [in]  value  Value to be saturated
 \param [in]    sat  Bit position to saturate to (0..31)
 \return             Saturated value
 */
u32 __IUSAT(u32 value, u32 sat)
{
	u32 result;

	if (value & 0x80000000) { /* only overflow set bit-31 */
		result = 0;
	} else if ((((0xFFFFFFFF >> sat) << sat) & value) != 0) {
		result = 0xFFFFFFFF >> (32 - sat);
	} else {
		result = value;
	}

	return result;
}

/**
 \brief   Rotate Right with Extend
 \details This function moves each bit of a bitstring right by one bit.
 The carry input is shifted in at the left end of the bitstring.
 \note    carry input will always 0.
 \param [in]    op1  Value to rotate
 \return               Rotated value
 */
u32 __RRX(u32 op1)
{
	return 0;
}

/**
 \brief   LDRT Unprivileged (8 bit)
 \details Executes a Unprivileged LDRT instruction for 8 bit value.
 \param [in]    addr  Pointer to location
 \return             value of type u8 at (*ptr)
 */
u8 __LDRBT(volatile u8 *addr)
{
	u32 result;

	__asm volatile("lb %0, 0(%1)" : "=r"(result) : "r"(addr));

	return (u8) result; /* Add explicit type cast here */
}

/**
 \brief   LDRT Unprivileged (16 bit)
 \details Executes a Unprivileged LDRT instruction for 16 bit values.
 \param [in]    addr  Pointer to location
 \return        value of type u16 at (*ptr)
 */
u16 __LDRHT(volatile u16 *addr)
{
	u32 result;

	__asm volatile("lh %0, 0(%1)" : "=r"(result) : "r"(addr));

	return (u16) result; /* Add explicit type cast here */
}

/**
 \brief   LDRT Unprivileged (32 bit)
 \details Executes a Unprivileged LDRT instruction for 32 bit values.
 \param [in]    addr  Pointer to location
 \return        value of type u32 at (*ptr)
 */
u32 __LDRT(volatile u32 *addr)
{
	u32 result;

	__asm volatile("lw %0, 0(%1)" : "=r"(result) : "r"(addr));

	return result;
}

/**
 \brief   STRT Unprivileged (8 bit)
 \details Executes a Unprivileged STRT instruction for 8 bit values.
 \param [in]  value  Value to store
 \param [in]    addr  Pointer to location
 */
void __STRBT(u8 value, volatile u8 *addr)
{
	__asm volatile("sb %1, 0(%0)" :: "r"(addr), "r"((u32)value) : "memory");
}

/**
 \brief   STRT Unprivileged (16 bit)
 \details Executes a Unprivileged STRT instruction for 16 bit values.
 \param [in]  value  Value to store
 \param [in]    addr  Pointer to location
 */
void __STRHT(u16 value, volatile u16 *addr)
{
	__asm volatile("sh %1, 0(%0)" :: "r"(addr), "r"((u32)value) : "memory");
}

/**
 \brief   STRT Unprivileged (32 bit)
 \details Executes a Unprivileged STRT instruction for 32 bit values.
 \param [in]  value  Value to store
 \param [in]    addr  Pointer to location
 */
void __STRT(u32 value, volatile u32 *addr)
{
	__asm volatile("sw %1, 0(%0)" :: "r"(addr), "r"(value) : "memory");
}

/*@}*//* end of group c906_Core_InstructionInterface */

/* ###################  Compiler specific Intrinsics  ########################### */
/** \defgroup c906_SIMD_intrinsics CSI SIMD Intrinsics
 Access to dedicated SIMD instructions \n
 Single Instruction Multiple Data (SIMD) extensions are provided to simplify development of application software. SIMD extensions increase the processing capability without materially increasing the power consumption. The SIMD extensions are completely transparent to the operating system (OS), allowing existing OS ports to be used.

 @{
 */

/**
 \brief   Halfword packing instruction. Combines bits[15:0] of val1 with bits[31:16]
 of val2 levitated with the val3.
 \details Combine a halfword from one register with a halfword from another register.
 The second argument can be left-shifted before extraction of the halfword.
 \param [in]    val1   first 16-bit operands
 \param [in]    val2   second 16-bit operands
 \param [in]    val3   value for left-shifting val2. Value range [0..31].
 \return               the combination of halfwords.
 \remark
 res[15:0]  = val1[15:0]              \n
 res[31:16] = val2[31:16] << val3
 */
u32 __PKHBT(u32 val1, u32 val2, u32 val3)
{
	return ((((s32)(val1) << 0) & (s32) 0x0000FFFF)
			| (((s32)(val2) << val3) & (s32) 0xFFFF0000));
}

/**
 \brief   Halfword packing instruction. Combines bits[31:16] of val1 with bits[15:0]
 of val2 right-shifted with the val3.
 \details Combine a halfword from one register with a halfword from another register.
 The second argument can be right-shifted before extraction of the halfword.
 \param [in]    val1   first 16-bit operands
 \param [in]    val2   second 16-bit operands
 \param [in]    val3   value for right-shifting val2. Value range [1..32].
 \return               the combination of halfwords.
 \remark
 res[15:0]  = val2[15:0] >> val3        \n
 res[31:16] = val1[31:16]
 */
u32 __PKHTB(u32 val1, u32 val2, u32 val3)
{
	return ((((s32)(val1) << 0) & (s32) 0xFFFF0000)
			| (((s32)(val2) >> val3) & (s32) 0x0000FFFF));
}

/**
 \brief   Dual 16-bit signed saturate.
 \details This function saturates a signed value.
 \param [in]    x   two signed 16-bit values to be saturated.
 \param [in]    y   bit position for saturation, an integral constant expression in the range 1 to 16.
 \return        the sum of the absolute differences of the following bytes, added to the accumulation value:\n
 the signed saturation of the low halfword in val1, saturated to the bit position specified in
 val2 and returned in the low halfword of the return value.\n
 the signed saturation of the high halfword in val1, saturated to the bit position specified in
 val2 and returned in the high halfword of the return value.
 */
u32 __SSAT16(s32 x, const u32 y)
{
	s32 r = 0, s = 0;

	r = __SSAT((((s32) x << 16) >> 16), y) & (s32) 0x0000FFFF;
	s = __SSAT((((s32) x) >> 16), y) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned saturate.
 \details This function enables you to saturate two signed 16-bit values to a selected unsigned range.
 \param [in]    x   two signed 16-bit values to be saturated.
 \param [in]    y   bit position for saturation, an integral constant expression in the range 1 to 16.
 \return        the saturation of the two signed 16-bit values, as non-negative values:
 the saturation of the low halfword in val1, saturated to the bit position specified in
 val2 and returned in the low halfword of the return value.\n
 the saturation of the high halfword in val1, saturated to the bit position specified in
 val2 and returned in the high halfword of the return value.
 */
u32 __USAT16(u32 x, const u32 y)
{
	s32 r = 0, s = 0;

	r = __IUSAT(((x << 16) >> 16), y) & 0x0000FFFF;
	s = __IUSAT(((x) >> 16), y) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Quad 8-bit saturating addition.
 \details This function enables you to perform four 8-bit integer additions,
 saturating the results to the 8-bit signed integer range -2^7 <= x <= 2^7 - 1.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the saturated addition of the first byte of each operand in the first byte of the return value.\n
 the saturated addition of the second byte of each operand in the second byte of the return value.\n
 the saturated addition of the third byte of each operand in the third byte of the return value.\n
 the saturated addition of the fourth byte of each operand in the fourth byte of the return value.\n
 The returned results are saturated to the 8-bit signed integer range -2^7 <= x <= 2^7 - 1.
 \remark
 res[7:0]   = val1[7:0]   + val2[7:0]        \n
 res[15:8]  = val1[15:8]  + val2[15:8]       \n
 res[23:16] = val1[23:16] + val2[23:16]      \n
 res[31:24] = val1[31:24] + val2[31:24]
 */
u32 __QADD8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = __SSAT(((((s32) x << 24) >> 24) + (((s32) y << 24) >> 24)), 8)
			& (s32) 0x000000FF;
	s = __SSAT(((((s32) x << 16) >> 24) + (((s32) y << 16) >> 24)), 8)
			& (s32) 0x000000FF;
	t = __SSAT(((((s32) x << 8) >> 24) + (((s32) y << 8) >> 24)), 8)
			& (s32) 0x000000FF;
	u = __SSAT(((((s32) x) >> 24) + (((s32) y) >> 24)), 8) & (s32) 0x000000FF;

	return ((u32)((u << 24) | (t << 16) | (s << 8) | (r)));
}

/**
 \brief   Quad 8-bit unsigned saturating addition.
 \details This function enables you to perform four unsigned 8-bit integer additions,
 saturating the results to the 8-bit unsigned integer range 0 < x < 2^8 - 1.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the saturated addition of the first byte of each operand in the first byte of the return value.\n
 the saturated addition of the second byte of each operand in the second byte of the return value.\n
 the saturated addition of the third byte of each operand in the third byte of the return value.\n
 the saturated addition of the fourth byte of each operand in the fourth byte of the return value.\n
 The returned results are saturated to the 8-bit signed integer range 0 <= x <= 2^8 - 1.
 \remark
 res[7:0]   = val1[7:0]   + val2[7:0]        \n
 res[15:8]  = val1[15:8]  + val2[15:8]       \n
 res[23:16] = val1[23:16] + val2[23:16]      \n
 res[31:24] = val1[31:24] + val2[31:24]
 */
u32 __UQADD8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = __IUSAT((((x << 24) >> 24) + ((y << 24) >> 24)), 8) & 0x000000FF;
	s = __IUSAT((((x << 16) >> 24) + ((y << 16) >> 24)), 8) & 0x000000FF;
	t = __IUSAT((((x << 8) >> 24) + ((y << 8) >> 24)), 8) & 0x000000FF;
	u = __IUSAT((((x) >> 24) + ((y) >> 24)), 8) & 0x000000FF;

	return ((u << 24) | (t << 16) | (s << 8) | (r));
}

/**
 \brief   Quad 8-bit signed addition.
 \details This function performs four 8-bit signed integer additions.
 \param [in]    x  first four 8-bit summands.
 \param [in]    y  second four 8-bit summands.
 \return        the addition of the first bytes from each operand, in the first byte of the return value.\n
 the addition of the second bytes of each operand, in the second byte of the return value.\n
 the addition of the third bytes of each operand, in the third byte of the return value.\n
 the addition of the fourth bytes of each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = val1[7:0]   + val2[7:0]        \n
 res[15:8]  = val1[15:8]  + val2[15:8]       \n
 res[23:16] = val1[23:16] + val2[23:16]      \n
 res[31:24] = val1[31:24] + val2[31:24]
 */
u32 __SADD8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = ((((s32) x << 24) >> 24) + (((s32) y << 24) >> 24)) & (s32) 0x000000FF;
	s = ((((s32) x << 16) >> 24) + (((s32) y << 16) >> 24)) & (s32) 0x000000FF;
	t = ((((s32) x << 8) >> 24) + (((s32) y << 8) >> 24)) & (s32) 0x000000FF;
	u = ((((s32) x) >> 24) + (((s32) y) >> 24)) & (s32) 0x000000FF;

	return ((u32)((u << 24) | (t << 16) | (s << 8) | (r)));
}

/**
 \brief   Quad 8-bit unsigned addition.
 \details This function performs four unsigned 8-bit integer additions.
 \param [in]    x  first four 8-bit summands.
 \param [in]    y  second four 8-bit summands.
 \return        the addition of the first bytes from each operand, in the first byte of the return value.\n
 the addition of the second bytes of each operand, in the second byte of the return value.\n
 the addition of the third bytes of each operand, in the third byte of the return value.\n
 the addition of the fourth bytes of each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = val1[7:0]   + val2[7:0]        \n
 res[15:8]  = val1[15:8]  + val2[15:8]       \n
 res[23:16] = val1[23:16] + val2[23:16]      \n
 res[31:24] = val1[31:24] + val2[31:24]
 */
u32 __UADD8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = (((x << 24) >> 24) + ((y << 24) >> 24)) & 0x000000FF;
	s = (((x << 16) >> 24) + ((y << 16) >> 24)) & 0x000000FF;
	t = (((x << 8) >> 24) + ((y << 8) >> 24)) & 0x000000FF;
	u = (((x) >> 24) + ((y) >> 24)) & 0x000000FF;

	return ((u << 24) | (t << 16) | (s << 8) | (r));
}

/**
 \brief   Quad 8-bit saturating subtract.
 \details This function enables you to perform four 8-bit integer subtractions,
 saturating the results to the 8-bit signed integer range -2^7 <= x <= 2^7 - 1.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the subtraction of the first byte of each operand in the first byte of the return value.\n
 the subtraction of the second byte of each operand in the second byte of the return value.\n
 the subtraction of the third byte of each operand in the third byte of the return value.\n
 the subtraction of the fourth byte of each operand in the fourth byte of the return value.\n
 The returned results are saturated to the 8-bit signed integer range -2^7 <= x <= 2^7 - 1.
 \remark
 res[7:0]   = val1[7:0]   - val2[7:0]        \n
 res[15:8]  = val1[15:8]  - val2[15:8]       \n
 res[23:16] = val1[23:16] - val2[23:16]      \n
 res[31:24] = val1[31:24] - val2[31:24]
 */
u32 __QSUB8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = __SSAT(((((s32) x << 24) >> 24) - (((s32) y << 24) >> 24)), 8)
			& (s32) 0x000000FF;
	s = __SSAT(((((s32) x << 16) >> 24) - (((s32) y << 16) >> 24)), 8)
			& (s32) 0x000000FF;
	t = __SSAT(((((s32) x << 8) >> 24) - (((s32) y << 8) >> 24)), 8)
			& (s32) 0x000000FF;
	u = __SSAT(((((s32) x) >> 24) - (((s32) y) >> 24)), 8) & (s32) 0x000000FF;

	return ((u32)((u << 24) | (t << 16) | (s << 8) | (r)));
}

/**
 \brief   Quad 8-bit unsigned saturating subtraction.
 \details This function enables you to perform four unsigned 8-bit integer subtractions,
 saturating the results to the 8-bit unsigned integer range 0 < x < 2^8 - 1.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the subtraction of the first byte of each operand in the first byte of the return value.\n
 the subtraction of the second byte of each operand in the second byte of the return value.\n
 the subtraction of the third byte of each operand in the third byte of the return value.\n
 the subtraction of the fourth byte of each operand in the fourth byte of the return value.\n
 The returned results are saturated to the 8-bit unsigned integer range 0 <= x <= 2^8 - 1.
 \remark
 res[7:0]   = val1[7:0]   - val2[7:0]        \n
 res[15:8]  = val1[15:8]  - val2[15:8]       \n
 res[23:16] = val1[23:16] - val2[23:16]      \n
 res[31:24] = val1[31:24] - val2[31:24]
 */
u32 __UQSUB8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = __IUSAT((((x << 24) >> 24) - ((y << 24) >> 24)), 8) & 0x000000FF;
	s = __IUSAT((((x << 16) >> 24) - ((y << 16) >> 24)), 8) & 0x000000FF;
	t = __IUSAT((((x << 8) >> 24) - ((y << 8) >> 24)), 8) & 0x000000FF;
	u = __IUSAT((((x) >> 24) - ((y) >> 24)), 8) & 0x000000FF;

	return ((u << 24) | (t << 16) | (s << 8) | (r));
}

/**
 \brief   Quad 8-bit signed subtraction.
 \details This function enables you to perform four 8-bit signed integer subtractions.
 \param [in]    x  first four 8-bit operands of each subtraction.
 \param [in]    y  second four 8-bit operands of each subtraction.
 \return        the subtraction of the first bytes from each operand, in the first byte of the return value.\n
 the subtraction of the second bytes of each operand, in the second byte of the return value.\n
 the subtraction of the third bytes of each operand, in the third byte of the return value.\n
 the subtraction of the fourth bytes of each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = val1[7:0]   - val2[7:0]        \n
 res[15:8]  = val1[15:8]  - val2[15:8]       \n
 res[23:16] = val1[23:16] - val2[23:16]      \n
 res[31:24] = val1[31:24] - val2[31:24]
 */
u32 __SSUB8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = ((((s32) x << 24) >> 24) - (((s32) y << 24) >> 24)) & (s32) 0x000000FF;
	s = ((((s32) x << 16) >> 24) - (((s32) y << 16) >> 24)) & (s32) 0x000000FF;
	t = ((((s32) x << 8) >> 24) - (((s32) y << 8) >> 24)) & (s32) 0x000000FF;
	u = ((((s32) x) >> 24) - (((s32) y) >> 24)) & (s32) 0x000000FF;

	return ((u32)((u << 24) | (t << 16) | (s << 8) | (r)));
}

/**
 \brief   Quad 8-bit unsigned subtract.
 \details This function enables you to perform four 8-bit unsigned integer subtractions.
 \param [in]    x  first four 8-bit operands of each subtraction.
 \param [in]    y  second four 8-bit operands of each subtraction.
 \return        the subtraction of the first bytes from each operand, in the first byte of the return value.\n
 the subtraction of the second bytes of each operand, in the second byte of the return value.\n
 the subtraction of the third bytes of each operand, in the third byte of the return value.\n
 the subtraction of the fourth bytes of each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = val1[7:0]   - val2[7:0]        \n
 res[15:8]  = val1[15:8]  - val2[15:8]       \n
 res[23:16] = val1[23:16] - val2[23:16]      \n
 res[31:24] = val1[31:24] - val2[31:24]
 */
u32 __USUB8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = (((x << 24) >> 24) - ((y << 24) >> 24)) & 0x000000FF;
	s = (((x << 16) >> 24) - ((y << 16) >> 24)) & 0x000000FF;
	t = (((x << 8) >> 24) - ((y << 8) >> 24)) & 0x000000FF;
	u = (((x) >> 24) - ((y) >> 24)) & 0x000000FF;

	return ((u << 24) | (t << 16) | (s << 8) | (r));
}

/**
 \brief   Unsigned sum of quad 8-bit unsigned absolute difference.
 \details This function enables you to perform four unsigned 8-bit subtractions, and add the absolute values
 of the differences together, returning the result as a single unsigned integer.
 \param [in]    x  first four 8-bit operands of each subtraction.
 \param [in]    y  second four 8-bit operands of each subtraction.
 \return        the subtraction of the first bytes from each operand, in the first byte of the return value.\n
 the subtraction of the second bytes of each operand, in the second byte of the return value.\n
 the subtraction of the third bytes of each operand, in the third byte of the return value.\n
 the subtraction of the fourth bytes of each operand, in the fourth byte of the return value.\n
 The sum is returned as a single unsigned integer.
 \remark
 absdiff1   = val1[7:0]   - val2[7:0]        \n
 absdiff2   = val1[15:8]  - val2[15:8]       \n
 absdiff3   = val1[23:16] - val2[23:16]      \n
 absdiff4   = val1[31:24] - val2[31:24]      \n
 res[31:0]  = absdiff1 + absdiff2 + absdiff3 + absdiff4
 */
u32 __USAD8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = (((x << 24) >> 24) - ((y << 24) >> 24)) & 0x000000FF;
	s = (((x << 16) >> 24) - ((y << 16) >> 24)) & 0x000000FF;
	t = (((x << 8) >> 24) - ((y << 8) >> 24)) & 0x000000FF;
	u = (((x) >> 24) - ((y) >> 24)) & 0x000000FF;

	return (u + t + s + r);
}

/**
 \brief   Unsigned sum of quad 8-bit unsigned absolute difference with 32-bit accumulate.
 \details This function enables you to perform four unsigned 8-bit subtractions, and add the absolute values
 of the differences to a 32-bit accumulate operand.
 \param [in]    x  first four 8-bit operands of each subtraction.
 \param [in]    y  second four 8-bit operands of each subtraction.
 \param [in]  sum  accumulation value.
 \return        the sum of the absolute differences of the following bytes, added to the accumulation value:
 the subtraction of the first bytes from each operand, in the first byte of the return value.\n
 the subtraction of the second bytes of each operand, in the second byte of the return value.\n
 the subtraction of the third bytes of each operand, in the third byte of the return value.\n
 the subtraction of the fourth bytes of each operand, in the fourth byte of the return value.
 \remark
 absdiff1 = val1[7:0]   - val2[7:0]        \n
 absdiff2 = val1[15:8]  - val2[15:8]       \n
 absdiff3 = val1[23:16] - val2[23:16]      \n
 absdiff4 = val1[31:24] - val2[31:24]      \n
 sum = absdiff1 + absdiff2 + absdiff3 + absdiff4 \n
 res[31:0] = sum[31:0] + val3[31:0]
 */
/*
 u32 __USADA8(u32 x, u32 y, u32 sum)
 {
 s32 r, s, t, u;

 #ifdef __cplusplus
 r = (abs((long long)((x << 24) >> 24) - ((y << 24) >> 24))) & 0x000000FF;
 s = (abs((long long)((x << 16) >> 24) - ((y << 16) >> 24))) & 0x000000FF;
 t = (abs((long long)((x <<  8) >> 24) - ((y <<  8) >> 24))) & 0x000000FF;
 u = (abs((long long)((x) >> 24) - ((y) >> 24))) & 0x000000FF;
 #else
 r = (abs(((x << 24) >> 24) - ((y << 24) >> 24))) & 0x000000FF;
 s = (abs(((x << 16) >> 24) - ((y << 16) >> 24))) & 0x000000FF;
 t = (abs(((x <<  8) >> 24) - ((y <<  8) >> 24))) & 0x000000FF;
 u = (abs(((x) >> 24) - ((y) >> 24))) & 0x000000FF;
 #endif
 return (u + t + s + r + sum);
 }
 */

/**
 \brief   Dual 16-bit saturating addition.
 \details This function enables you to perform two 16-bit integer arithmetic additions in parallel,
 saturating the results to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the saturated addition of the low halfwords, in the low halfword of the return value.\n
 the saturated addition of the high halfwords, in the high halfword of the return value.\n
 The returned results are saturated to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \remark
 res[15:0]  = val1[15:0]  + val2[15:0]        \n
 res[31:16] = val1[31:16] + val2[31:16]
 */
u32 __QADD16(u32 x, u32 y)
{
	s32 r = 0, s = 0;

	r = __SSAT(((((s32) x << 16) >> 16) + (((s32) y << 16) >> 16)), 16)
			& (s32) 0x0000FFFF;
	s = __SSAT(((((s32) x) >> 16) + (((s32) y) >> 16)), 16) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned saturating addition.
 \details This function enables you to perform two unsigned 16-bit integer additions, saturating
 the results to the 16-bit unsigned integer range 0 < x < 2^16 - 1.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the saturated addition of the low halfwords, in the low halfword of the return value.\n
 the saturated addition of the high halfwords, in the high halfword of the return value.\n
 The results are saturated to the 16-bit unsigned integer range 0 < x < 2^16 - 1.
 \remark
 res[15:0]  = val1[15:0]  + val2[15:0]        \n
 res[31:16] = val1[31:16] + val2[31:16]
 */
u32 __UQADD16(u32 x, u32 y)
{
	s32 r = 0, s = 0;

	r = __IUSAT((((x << 16) >> 16) + ((y << 16) >> 16)), 16) & 0x0000FFFF;
	s = __IUSAT((((x) >> 16) + ((y) >> 16)), 16) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit signed addition.
 \details This function enables you to perform two 16-bit signed integer additions.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the addition of the low halfwords in the low halfword of the return value.\n
 the addition of the high halfwords in the high halfword of the return value.
 \remark
 res[15:0]  = val1[15:0]  + val2[15:0]        \n
 res[31:16] = val1[31:16] + val2[31:16]
 */
u32 __SADD16(u32 x, u32 y)
{
	s32 r = 0, s = 0;

	r = ((((s32) x << 16) >> 16) + (((s32) y << 16) >> 16)) & (s32) 0x0000FFFF;
	s = ((((s32) x) >> 16) + (((s32) y) >> 16)) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned addition
 \details This function enables you to perform two 16-bit unsigned integer additions.
 \param [in]    x   first two 16-bit summands for each addition.
 \param [in]    y   second two 16-bit summands for each addition.
 \return        the addition of the low halfwords in the low halfword of the return value.\n
 the addition of the high halfwords in the high halfword of the return value.
 \remark
 res[15:0]  = val1[15:0]  + val2[15:0]        \n
 res[31:16] = val1[31:16] + val2[31:16]
 */
u32 __UADD16(u32 x, u32 y)
{
	s32 r = 0, s = 0;

	r = (((x << 16) >> 16) + ((y << 16) >> 16)) & 0x0000FFFF;
	s = (((x) >> 16) + ((y) >> 16)) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit signed addition with halved results.
 \details This function enables you to perform two signed 16-bit integer additions, halving the results.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the halved addition of the low halfwords, in the low halfword of the return value.\n
 the halved addition of the high halfwords, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  + val2[15:0]) >> 1        \n
 res[31:16] = (val1[31:16] + val2[31:16]) >> 1
 */
u32 __SHADD16(u32 x, u32 y)
{
	s32 r, s;

	r = (((((s32) x << 16) >> 16) + (((s32) y << 16) >> 16)) >> 1)
			& (s32) 0x0000FFFF;
	s = (((((s32) x) >> 16) + (((s32) y) >> 16)) >> 1) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned addition with halved results.
 \details This function enables you to perform two unsigned 16-bit integer additions, halving the results.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the halved addition of the low halfwords, in the low halfword of the return value.\n
 the halved addition of the high halfwords, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  + val2[15:0]) >> 1        \n
 res[31:16] = (val1[31:16] + val2[31:16]) >> 1
 */
u32 __UHADD16(u32 x, u32 y)
{
	s32 r, s;

	r = ((((x << 16) >> 16) + ((y << 16) >> 16)) >> 1) & 0x0000FFFF;
	s = ((((x) >> 16) + ((y) >> 16)) >> 1) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Quad 8-bit signed addition with halved results.
 \details This function enables you to perform four signed 8-bit integer additions, halving the results.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the halved addition of the first bytes from each operand, in the first byte of the return value.\n
 the halved addition of the second bytes from each operand, in the second byte of the return value.\n
 the halved addition of the third bytes from each operand, in the third byte of the return value.\n
 the halved addition of the fourth bytes from each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = (val1[7:0]   + val2[7:0]  ) >> 1    \n
 res[15:8]  = (val1[15:8]  + val2[15:8] ) >> 1    \n
 res[23:16] = (val1[23:16] + val2[23:16]) >> 1    \n
 res[31:24] = (val1[31:24] + val2[31:24]) >> 1
 */
u32 __SHADD8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = (((((s32) x << 24) >> 24) + (((s32) y << 24) >> 24)) >> 1)
			& (s32) 0x000000FF;
	s = (((((s32) x << 16) >> 24) + (((s32) y << 16) >> 24)) >> 1)
			& (s32) 0x000000FF;
	t = (((((s32) x << 8) >> 24) + (((s32) y << 8) >> 24)) >> 1)
			& (s32) 0x000000FF;
	u = (((((s32) x) >> 24) + (((s32) y) >> 24)) >> 1) & (s32) 0x000000FF;

	return ((u32)((u << 24) | (t << 16) | (s << 8) | (r)));
}

/**
 \brief   Quad 8-bit unsigned addition with halved results.
 \details This function enables you to perform four unsigned 8-bit integer additions, halving the results.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the halved addition of the first bytes from each operand, in the first byte of the return value.\n
 the halved addition of the second bytes from each operand, in the second byte of the return value.\n
 the halved addition of the third bytes from each operand, in the third byte of the return value.\n
 the halved addition of the fourth bytes from each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = (val1[7:0]   + val2[7:0]  ) >> 1    \n
 res[15:8]  = (val1[15:8]  + val2[15:8] ) >> 1    \n
 res[23:16] = (val1[23:16] + val2[23:16]) >> 1    \n
 res[31:24] = (val1[31:24] + val2[31:24]) >> 1
 */
u32 __UHADD8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = ((((x << 24) >> 24) + ((y << 24) >> 24)) >> 1) & 0x000000FF;
	s = ((((x << 16) >> 24) + ((y << 16) >> 24)) >> 1) & 0x000000FF;
	t = ((((x << 8) >> 24) + ((y << 8) >> 24)) >> 1) & 0x000000FF;
	u = ((((x) >> 24) + ((y) >> 24)) >> 1) & 0x000000FF;

	return ((u << 24) | (t << 16) | (s << 8) | (r));
}

/**
 \brief   Dual 16-bit saturating subtract.
 \details This function enables you to perform two 16-bit integer subtractions in parallel,
 saturating the results to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the saturated subtraction of the low halfwords, in the low halfword of the return value.\n
 the saturated subtraction of the high halfwords, in the high halfword of the return value.\n
 The returned results are saturated to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \remark
 res[15:0]  = val1[15:0]  - val2[15:0]        \n
 res[31:16] = val1[31:16] - val2[31:16]
 */
u32 __QSUB16(u32 x, u32 y)
{
	s32 r, s;

	r = __SSAT(((((s32) x << 16) >> 16) - (((s32) y << 16) >> 16)), 16)
			& (s32) 0x0000FFFF;
	s = __SSAT(((((s32) x) >> 16) - (((s32) y) >> 16)), 16) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned saturating subtraction.
 \details This function enables you to perform two unsigned 16-bit integer subtractions,
 saturating the results to the 16-bit unsigned integer range 0 < x < 2^16 - 1.
 \param [in]    x   first two 16-bit operands for each subtraction.
 \param [in]    y   second two 16-bit operands for each subtraction.
 \return        the saturated subtraction of the low halfwords, in the low halfword of the return value.\n
 the saturated subtraction of the high halfwords, in the high halfword of the return value.\n
 The returned results are saturated to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \remark
 res[15:0]  = val1[15:0]  - val2[15:0]        \n
 res[31:16] = val1[31:16] - val2[31:16]
 */
u32 __UQSUB16(u32 x, u32 y)
{
	s32 r, s;

	r = __IUSAT((((x << 16) >> 16) - ((y << 16) >> 16)), 16) & 0x0000FFFF;
	s = __IUSAT((((x) >> 16) - ((y) >> 16)), 16) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit signed subtraction.
 \details This function enables you to perform two 16-bit signed integer subtractions.
 \param [in]    x   first two 16-bit operands of each subtraction.
 \param [in]    y   second two 16-bit operands of each subtraction.
 \return        the subtraction of the low halfword in the second operand from the low
 halfword in the first operand, in the low halfword of the return value. \n
 the subtraction of the high halfword in the second operand from the high
 halfword in the first operand, in the high halfword of the return value.
 \remark
 res[15:0]  = val1[15:0]  - val2[15:0]        \n
 res[31:16] = val1[31:16] - val2[31:16]
 */
u32 __SSUB16(u32 x, u32 y)
{
	s32 r, s;

	r = ((((s32) x << 16) >> 16) - (((s32) y << 16) >> 16)) & (s32) 0x0000FFFF;
	s = ((((s32) x) >> 16) - (((s32) y) >> 16)) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned subtract.
 \details This function enables you to perform two 16-bit unsigned integer subtractions.
 \param [in]    x   first two 16-bit operands of each subtraction.
 \param [in]    y   second two 16-bit operands of each subtraction.
 \return        the subtraction of the low halfword in the second operand from the low
 halfword in the first operand, in the low halfword of the return value. \n
 the subtraction of the high halfword in the second operand from the high
 halfword in the first operand, in the high halfword of the return value.
 \remark
 res[15:0]  = val1[15:0]  - val2[15:0]        \n
 res[31:16] = val1[31:16] - val2[31:16]
 */
u32 __USUB16(u32 x, u32 y)
{
	s32 r, s;

	r = (((x << 16) >> 16) - ((y << 16) >> 16)) & 0x0000FFFF;
	s = (((x) >> 16) - ((y) >> 16)) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit signed subtraction with halved results.
 \details This function enables you to perform two signed 16-bit integer subtractions, halving the results.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the halved subtraction of the low halfwords, in the low halfword of the return value.\n
 the halved subtraction of the high halfwords, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  - val2[15:0]) >> 1        \n
 res[31:16] = (val1[31:16] - val2[31:16]) >> 1
 */
u32 __SHSUB16(u32 x, u32 y)
{
	s32 r, s;

	r = (((((s32) x << 16) >> 16) - (((s32) y << 16) >> 16)) >> 1)
			& (s32) 0x0000FFFF;
	s = (((((s32) x) >> 16) - (((s32) y) >> 16)) >> 1) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned subtraction with halved results.
 \details This function enables you to perform two unsigned 16-bit integer subtractions, halving the results.
 \param [in]    x   first two 16-bit summands.
 \param [in]    y   second two 16-bit summands.
 \return        the halved subtraction of the low halfwords, in the low halfword of the return value.\n
 the halved subtraction of the high halfwords, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  - val2[15:0]) >> 1        \n
 res[31:16] = (val1[31:16] - val2[31:16]) >> 1
 */
u32 __UHSUB16(u32 x, u32 y)
{
	s32 r, s;

	r = ((((x << 16) >> 16) - ((y << 16) >> 16)) >> 1) & 0x0000FFFF;
	s = ((((x) >> 16) - ((y) >> 16)) >> 1) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Quad 8-bit signed addition with halved results.
 \details This function enables you to perform four signed 8-bit integer subtractions, halving the results.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the halved subtraction of the first bytes from each operand, in the first byte of the return value.\n
 the halved subtraction of the second bytes from each operand, in the second byte of the return value.\n
 the halved subtraction of the third bytes from each operand, in the third byte of the return value.\n
 the halved subtraction of the fourth bytes from each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = (val1[7:0]   - val2[7:0]  ) >> 1    \n
 res[15:8]  = (val1[15:8]  - val2[15:8] ) >> 1    \n
 res[23:16] = (val1[23:16] - val2[23:16]) >> 1    \n
 res[31:24] = (val1[31:24] - val2[31:24]) >> 1
 */
u32 __SHSUB8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = (((((s32) x << 24) >> 24) - (((s32) y << 24) >> 24)) >> 1)
			& (s32) 0x000000FF;
	s = (((((s32) x << 16) >> 24) - (((s32) y << 16) >> 24)) >> 1)
			& (s32) 0x000000FF;
	t = (((((s32) x << 8) >> 24) - (((s32) y << 8) >> 24)) >> 1)
			& (s32) 0x000000FF;
	u = (((((s32) x) >> 24) - (((s32) y) >> 24)) >> 1) & (s32) 0x000000FF;

	return ((u32)((u << 24) | (t << 16) | (s << 8) | (r)));
}

/**
 \brief   Quad 8-bit unsigned subtraction with halved results.
 \details This function enables you to perform four unsigned 8-bit integer subtractions, halving the results.
 \param [in]    x   first four 8-bit summands.
 \param [in]    y   second four 8-bit summands.
 \return        the halved subtraction of the first bytes from each operand, in the first byte of the return value.\n
 the halved subtraction of the second bytes from each operand, in the second byte of the return value.\n
 the halved subtraction of the third bytes from each operand, in the third byte of the return value.\n
 the halved subtraction of the fourth bytes from each operand, in the fourth byte of the return value.
 \remark
 res[7:0]   = (val1[7:0]   - val2[7:0]  ) >> 1    \n
 res[15:8]  = (val1[15:8]  - val2[15:8] ) >> 1    \n
 res[23:16] = (val1[23:16] - val2[23:16]) >> 1    \n
 res[31:24] = (val1[31:24] - val2[31:24]) >> 1
 */
u32 __UHSUB8(u32 x, u32 y)
{
	s32 r, s, t, u;

	r = ((((x << 24) >> 24) - ((y << 24) >> 24)) >> 1) & 0x000000FF;
	s = ((((x << 16) >> 24) - ((y << 16) >> 24)) >> 1) & 0x000000FF;
	t = ((((x << 8) >> 24) - ((y << 8) >> 24)) >> 1) & 0x000000FF;
	u = ((((x) >> 24) - ((y) >> 24)) >> 1) & 0x000000FF;

	return ((u << 24) | (t << 16) | (s << 8) | (r));
}

/**
 \brief   Dual 16-bit add and subtract with exchange.
 \details This function enables you to exchange the halfwords of the one operand,
 then add the high halfwords and subtract the low halfwords,
 saturating the results to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \param [in]    x   first operand for the subtraction in the low halfword,
 and the first operand for the addition in the high halfword.
 \param [in]    y   second operand for the subtraction in the high halfword,
 and the second operand for the addition in the low halfword.
 \return        the saturated subtraction of the high halfword in the second operand from the
 low halfword in the first operand, in the low halfword of the return value.\n
 the saturated addition of the high halfword in the first operand and the
 low halfword in the second operand, in the high halfword of the return value.\n
 The returned results are saturated to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \remark
 res[15:0]  = val1[15:0]  - val2[31:16]        \n
 res[31:16] = val1[31:16] + val2[15:0]
 */
u32 __QASX(u32 x, u32 y)
{
	s32 r, s;

	r = __SSAT(((((s32) x << 16) >> 16) - (((s32) y) >> 16)), 16)
			& (s32) 0x0000FFFF;
	s = __SSAT(((((s32) x) >> 16) + (((s32) y << 16) >> 16)), 16)
			& (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned saturating addition and subtraction with exchange.
 \details This function enables you to exchange the halfwords of the second operand and
 perform one unsigned 16-bit integer addition and one unsigned 16-bit subtraction,
 saturating the results to the 16-bit unsigned integer range 0 <= x <= 2^16 - 1.
 \param [in]    x   first operand for the subtraction in the low halfword,
 and the first operand for the addition in the high halfword.
 \param [in]    y   second operand for the subtraction in the high halfword,
 and the second operand for the addition in the low halfword.
 \return        the saturated subtraction of the high halfword in the second operand from the
 low halfword in the first operand, in the low halfword of the return value.\n
 the saturated addition of the high halfword in the first operand and the
 low halfword in the second operand, in the high halfword of the return value.\n
 The returned results are saturated to the 16-bit unsigned integer range 0 <= x <= 2^16 - 1.
 \remark
 res[15:0]  = val1[15:0]  - val2[31:16]        \n
 res[31:16] = val1[31:16] + val2[15:0]
 */
u32 __UQASX(u32 x, u32 y)
{
	s32 r, s;

	r = __IUSAT((((x << 16) >> 16) - ((y) >> 16)), 16) & 0x0000FFFF;
	s = __IUSAT((((x) >> 16) + ((y << 16) >> 16)), 16) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit addition and subtraction with exchange.
 \details It enables you to exchange the halfwords of the second operand, add the high halfwords
 and subtract the low halfwords.
 \param [in]    x   first operand for the subtraction in the low halfword,
 and the first operand for the addition in the high halfword.
 \param [in]    y   second operand for the subtraction in the high halfword,
 and the second operand for the addition in the low halfword.
 \return        the subtraction of the high halfword in the second operand from the
 low halfword in the first operand, in the low halfword of the return value.\n
 the addition of the high halfword in the first operand and the
 low halfword in the second operand, in the high halfword of the return value.
 \remark
 res[15:0]  = val1[15:0]  - val2[31:16]        \n
 res[31:16] = val1[31:16] + val2[15:0]
 */
u32 __SASX(u32 x, u32 y)
{
	s32 r, s;

	r = ((((s32) x << 16) >> 16) - (((s32) y) >> 16)) & (s32) 0x0000FFFF;
	s = ((((s32) x) >> 16) + (((s32) y << 16) >> 16)) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned addition and subtraction with exchange.
 \details This function enables you to exchange the two halfwords of the second operand,
 add the high halfwords and subtract the low halfwords.
 \param [in]    x   first operand for the subtraction in the low halfword,
 and the first operand for the addition in the high halfword.
 \param [in]    y   second operand for the subtraction in the high halfword,
 and the second operand for the addition in the low halfword.
 \return        the subtraction of the high halfword in the second operand from the
 low halfword in the first operand, in the low halfword of the return value.\n
 the addition of the high halfword in the first operand and the
 low halfword in the second operand, in the high halfword of the return value.
 \remark
 res[15:0]  = val1[15:0]  - val2[31:16]        \n
 res[31:16] = val1[31:16] + val2[15:0]
 */
u32 __UASX(u32 x, u32 y)
{
	s32 r, s;

	r = (((x << 16) >> 16) - ((y) >> 16)) & 0x0000FFFF;
	s = (((x) >> 16) + ((y << 16) >> 16)) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit signed addition and subtraction with halved results.
 \details This function enables you to exchange the two halfwords of one operand, perform one
 signed 16-bit integer addition and one signed 16-bit subtraction, and halve the results.
 \param [in]    x   first 16-bit operands.
 \param [in]    y   second 16-bit operands.
 \return        the halved subtraction of the high halfword in the second operand from the
 low halfword in the first operand, in the low halfword of the return value.\n
 the halved addition of the low halfword in the second operand from the high
 halfword in the first operand, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  - val2[31:16]) >> 1        \n
 res[31:16] = (val1[31:16] + val2[15:0]) >> 1
 */
u32 __SHASX(u32 x, u32 y)
{
	s32 r, s;

	r = (((((s32) x << 16) >> 16) - (((s32) y) >> 16)) >> 1) & (s32) 0x0000FFFF;
	s = (((((s32) x) >> 16) + (((s32) y << 16) >> 16)) >> 1) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned addition and subtraction with halved results and exchange.
 \details This function enables you to exchange the halfwords of the second operand,
 add the high halfwords and subtract the low halfwords, halving the results.
 \param [in]    x   first operand for the subtraction in the low halfword, and
 the first operand for the addition in the high halfword.
 \param [in]    y   second operand for the subtraction in the high halfword, and
 the second operand for the addition in the low halfword.
 \return        the halved subtraction of the high halfword in the second operand from the
 low halfword in the first operand, in the low halfword of the return value.\n
 the halved addition of the low halfword in the second operand from the high
 halfword in the first operand, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  - val2[31:16]) >> 1        \n
 res[31:16] = (val1[31:16] + val2[15:0]) >> 1
 */
u32 __UHASX(u32 x, u32 y)
{
	s32 r, s;

	r = ((((x << 16) >> 16) - ((y) >> 16)) >> 1) & 0x0000FFFF;
	s = ((((x) >> 16) + ((y << 16) >> 16)) >> 1) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit subtract and add with exchange.
 \details This function enables you to exchange the halfwords of one operand,
 then subtract the high halfwords and add the low halfwords,
 saturating the results to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \param [in]    x   first operand for the addition in the low halfword,
 and the first operand for the subtraction in the high halfword.
 \param [in]    y   second operand for the addition in the high halfword,
 and the second operand for the subtraction in the low halfword.
 \return        the saturated addition of the low halfword of the first operand and the high
 halfword of the second operand, in the low halfword of the return value.\n
 the saturated subtraction of the low halfword of the second operand from the
 high halfword of the first operand, in the high halfword of the return value.\n
 The returned results are saturated to the 16-bit signed integer range -2^15 <= x <= 2^15 - 1.
 \remark
 res[15:0]  = val1[15:0]  + val2[31:16]        \n
 res[31:16] = val1[31:16] - val2[15:0]
 */
u32 __QSAX(u32 x, u32 y)
{
	s32 r, s;

	r = __SSAT(((((s32) x << 16) >> 16) + (((s32) y) >> 16)), 16)
			& (s32) 0x0000FFFF;
	s = __SSAT(((((s32) x) >> 16) - (((s32) y << 16) >> 16)), 16)
			& (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned saturating subtraction and addition with exchange.
 \details This function enables you to exchange the halfwords of the second operand and perform
 one unsigned 16-bit integer subtraction and one unsigned 16-bit addition, saturating
 the results to the 16-bit unsigned integer range 0 <= x <= 2^16 - 1.
 \param [in]    x   first operand for the addition in the low halfword,
 and the first operand for the subtraction in the high halfword.
 \param [in]    y   second operand for the addition in the high halfword,
 and the second operand for the subtraction in the low halfword.
 \return        the saturated addition of the low halfword of the first operand and the high
 halfword of the second operand, in the low halfword of the return value.\n
 the saturated subtraction of the low halfword of the second operand from the
 high halfword of the first operand, in the high halfword of the return value.\n
 The returned results are saturated to the 16-bit unsigned integer range 0 <= x <= 2^16 - 1.
 \remark
 res[15:0]  = val1[15:0]  + val2[31:16]        \n
 res[31:16] = val1[31:16] - val2[15:0]
 */
u32 __UQSAX(u32 x, u32 y)
{
	s32 r, s;

	r = __IUSAT((((x << 16) >> 16) + ((y) >> 16)), 16) & 0x0000FFFF;
	s = __IUSAT((((x) >> 16) - ((y << 16) >> 16)), 16) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit unsigned subtract and add with exchange.
 \details This function enables you to exchange the halfwords of the second operand,
 subtract the high halfwords and add the low halfwords.
 \param [in]    x   first operand for the addition in the low halfword,
 and the first operand for the subtraction in the high halfword.
 \param [in]    y   second operand for the addition in the high halfword,
 and the second operand for the subtraction in the low halfword.
 \return        the addition of the low halfword of the first operand and the high
 halfword of the second operand, in the low halfword of the return value.\n
 the subtraction of the low halfword of the second operand from the
 high halfword of the first operand, in the high halfword of the return value.\n
 \remark
 res[15:0]  = val1[15:0]  + val2[31:16]        \n
 res[31:16] = val1[31:16] - val2[15:0]
 */
u32 __USAX(u32 x, u32 y)
{
	s32 r, s;

	r = (((x << 16) >> 16) + ((y) >> 16)) & 0x0000FFFF;
	s = (((x) >> 16) - ((y << 16) >> 16)) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit signed subtraction and addition with exchange.
 \details This function enables you to exchange the two halfwords of one operand and perform one
 16-bit integer subtraction and one 16-bit addition.
 \param [in]    x   first operand for the addition in the low halfword, and the first operand
 for the subtraction in the high halfword.
 \param [in]    y   second operand for the addition in the high halfword, and the second
 operand for the subtraction in the low halfword.
 \return        the addition of the low halfword of the first operand and the high
 halfword of the second operand, in the low halfword of the return value.\n
 the subtraction of the low halfword of the second operand from the
 high halfword of the first operand, in the high halfword of the return value.\n
 \remark
 res[15:0]  = val1[15:0]  + val2[31:16]        \n
 res[31:16] = val1[31:16] - val2[15:0]
 */
u32 __SSAX(u32 x, u32 y)
{
	s32 r, s;

	r = ((((s32) x << 16) >> 16) + (((s32) y) >> 16)) & (s32) 0x0000FFFF;
	s = ((((s32) x) >> 16) - (((s32) y << 16) >> 16)) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit signed subtraction and addition with halved results.
 \details This function enables you to exchange the two halfwords of one operand, perform one signed
 16-bit integer subtraction and one signed 16-bit addition, and halve the results.
 \param [in]    x   first 16-bit operands.
 \param [in]    y   second 16-bit operands.
 \return        the halved addition of the low halfword in the first operand and the
 high halfword in the second operand, in the low halfword of the return value.\n
 the halved subtraction of the low halfword in the second operand from the
 high halfword in the first operand, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  + val2[31:16]) >> 1        \n
 res[31:16] = (val1[31:16] - val2[15:0]) >> 1
 */
u32 __SHSAX(u32 x, u32 y)
{
	s32 r, s;

	r = (((((s32) x << 16) >> 16) + (((s32) y) >> 16)) >> 1) & (s32) 0x0000FFFF;
	s = (((((s32) x) >> 16) - (((s32) y << 16) >> 16)) >> 1) & (s32) 0x0000FFFF;

	return ((u32)((s << 16) | (r)));
}

/**
 \brief   Dual 16-bit unsigned subtraction and addition with halved results and exchange.
 \details This function enables you to exchange the halfwords of the second operand,
 subtract the high halfwords and add the low halfwords, halving the results.
 \param [in]    x   first operand for the addition in the low halfword, and
 the first operand for the subtraction in the high halfword.
 \param [in]    y   second operand for the addition in the high halfword, and
 the second operand for the subtraction in the low halfword.
 \return        the halved addition of the low halfword in the first operand and the
 high halfword in the second operand, in the low halfword of the return value.\n
 the halved subtraction of the low halfword in the second operand from the
 high halfword in the first operand, in the high halfword of the return value.
 \remark
 res[15:0]  = (val1[15:0]  + val2[31:16]) >> 1        \n
 res[31:16] = (val1[31:16] - val2[15:0]) >> 1
 */
u32 __UHSAX(u32 x, u32 y)
{
	s32 r, s;

	r = ((((x << 16) >> 16) + ((y) >> 16)) >> 1) & 0x0000FFFF;
	s = ((((x) >> 16) - ((y << 16) >> 16)) >> 1) & 0x0000FFFF;

	return ((s << 16) | (r));
}

/**
 \brief   Dual 16-bit signed multiply with exchange returning difference.
 \details This function enables you to perform two 16-bit signed multiplications, subtracting
 one of the products from the other. The halfwords of the second operand are exchanged
 before performing the arithmetic. This produces top * bottom and bottom * top multiplication.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \return        the difference of the products of the two 16-bit signed multiplications.
 \remark
 p1 = val1[15:0]  * val2[31:16]       \n
 p2 = val1[31:16] * val2[15:0]        \n
 res[31:0] = p1 - p2
 */
u32 __SMUSDX(u32 x, u32 y)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y) >> 16))
					- ((((s32) x) >> 16) * (((s32) y << 16) >> 16))));
}

/**
 \brief   Sum of dual 16-bit signed multiply with exchange.
 \details This function enables you to perform two 16-bit signed multiplications with exchanged
 halfwords of the second operand, adding the products together.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \return        the sum of the products of the two 16-bit signed multiplications with exchanged halfwords of the second operand.
 \remark
 p1 = val1[15:0]  * val2[31:16]       \n
 p2 = val1[31:16] * val2[15:0]        \n
 res[31:0] = p1 + p2
 */
u32 __SMUADX(u32 x, u32 y)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y) >> 16))
					+ ((((s32) x) >> 16) * (((s32) y << 16) >> 16))));
}

/**
 \brief   Saturating add.
 \details This function enables you to obtain the saturating add of two integers.
 \param [in]    x   first summand of the saturating add operation.
 \param [in]    y   second summand of the saturating add operation.
 \return        the saturating addition of val1 and val2.
 \remark
 res[31:0] = SAT(val1 + SAT(val2))
 */
s32 __QADD(s32 x, s32 y)
{
	s32 result;

	if (y >= 0) {
		if ((s32)((u32) x + (u32) y) >= x) {
			result = x + y;
		} else {
			result = 0x7FFFFFFF;
		}
	} else {
		if ((s32)((u32) x + (u32) y) < x) {
			result = x + y;
		} else {
			result = 0x80000000;
		}
	}

	return result;
}

/**
 \brief   Saturating subtract.
 \details This function enables you to obtain the saturating add of two integers.
 \param [in]    x   first summand of the saturating add operation.
 \param [in]    y   second summand of the saturating add operation.
 \return        the saturating addition of val1 and val2.
 \remark
 res[31:0] = SAT(val1 - SAT(val2))
 */
s32 __QSUB(s32 x, s32 y)
{
	s32 tmp;
	s32 result;

	tmp = (s32) x - (s32) y;

	if (tmp > 0x7fffffff) {
		tmp = 0x7fffffff;
	} else if (tmp < (-2147483647 - 1)) {
		tmp = -2147483647 - 1;
	}

	result = tmp;
	return result;
}

/**
 \brief   Dual 16-bit signed multiply with single 32-bit accumulator.
 \details This function enables you to perform two signed 16-bit multiplications,
 adding both results to a 32-bit accumulate operand.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the product of each multiplication added to the accumulate value, as a 32-bit integer.
 \remark
 p1 = val1[15:0]  * val2[15:0]      \n
 p2 = val1[31:16] * val2[31:16]     \n
 res[31:0] = p1 + p2 + val3[31:0]
 */
u32 __SMLAD(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y << 16) >> 16))
					+ ((((s32) x) >> 16) * (((s32) y) >> 16)) + (((s32) sum))));
}

/**
 \brief   Pre-exchanged dual 16-bit signed multiply with single 32-bit accumulator.
 \details This function enables you to perform two signed 16-bit multiplications with exchanged
 halfwords of the second operand, adding both results to a 32-bit accumulate operand.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the product of each multiplication with exchanged halfwords of the second
 operand added to the accumulate value, as a 32-bit integer.
 \remark
 p1 = val1[15:0]  * val2[31:16]     \n
 p2 = val1[31:16] * val2[15:0]      \n
 res[31:0] = p1 + p2 + val3[31:0]
 */
u32 __SMLADX(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y) >> 16))
					+ ((((s32) x) >> 16) * (((s32) y << 16) >> 16))
					+ (((s32) sum))));
}

/**
 \brief   Dual 16-bit signed multiply with exchange subtract with 32-bit accumulate.
 \details This function enables you to perform two 16-bit signed multiplications, take the
 difference of the products, subtracting the high halfword product from the low
 halfword product, and add the difference to a 32-bit accumulate operand.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the difference of the product of each multiplication, added to the accumulate value.
 \remark
 p1 = val1[15:0]  * val2[15:0]       \n
 p2 = val1[31:16] * val2[31:16]      \n
 res[31:0] = p1 - p2 + val3[31:0]
 */
u32 __SMLSD(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y << 16) >> 16))
					- ((((s32) x) >> 16) * (((s32) y) >> 16)) + (((s32) sum))));
}

/**
 \brief   Dual 16-bit signed multiply with exchange subtract with 32-bit accumulate.
 \details This function enables you to exchange the halfwords in the second operand, then perform two 16-bit
 signed multiplications. The difference of the products is added to a 32-bit accumulate operand.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the difference of the product of each multiplication, added to the accumulate value.
 \remark
 p1 = val1[15:0]  * val2[31:16]     \n
 p2 = val1[31:16] * val2[15:0]      \n
 res[31:0] = p1 - p2 + val3[31:0]
 */
u32 __SMLSDX(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y) >> 16))
					- ((((s32) x) >> 16) * (((s32) y << 16) >> 16))
					+ (((s32) sum))));
}

/**
 \brief   Dual 16-bit signed multiply with single 64-bit accumulator.
 \details This function enables you to perform two signed 16-bit multiplications, adding both results
 to a 64-bit accumulate operand. Overflow is only possible as a result of the 64-bit addition.
 This overflow is not detected if it occurs. Instead, the result wraps around modulo2^64.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the product of each multiplication added to the accumulate value.
 \remark
 p1 = val1[15:0]  * val2[15:0]      \n
 p2 = val1[31:16] * val2[31:16]     \n
 sum = p1 + p2 + val3[63:32][31:0]  \n
 res[63:32] = sum[63:32]            \n
 res[31:0]  = sum[31:0]
 */
u32 __SMLALD(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y << 16) >> 16))
					+ ((((s32) x) >> 16) * (((s32) y) >> 16)) + (((u32) sum))));
}

/**
 \brief   Dual 16-bit signed multiply with exchange with single 64-bit accumulator.
 \details This function enables you to exchange the halfwords of the second operand, and perform two
 signed 16-bit multiplications, adding both results to a 64-bit accumulate operand. Overflow
 is only possible as a result of the 64-bit addition. This overflow is not detected if it occurs.
 Instead, the result wraps around modulo2^64.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the product of each multiplication added to the accumulate value.
 \remark
 p1 = val1[15:0]  * val2[31:16]     \n
 p2 = val1[31:16] * val2[15:0]      \n
 sum = p1 + p2 + val3[63:32][31:0]  \n
 res[63:32] = sum[63:32]            \n
 res[31:0]  = sum[31:0]
 */
u32 __SMLALDX(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y) >> 16))
					+ ((((s32) x) >> 16) * (((s32) y << 16) >> 16))
					+ (((u32) sum))));
}

/**
 \brief   dual 16-bit signed multiply subtract with 64-bit accumulate.
 \details This function It enables you to perform two 16-bit signed multiplications, take the difference
 of the products, subtracting the high halfword product from the low halfword product, and add the
 difference to a 64-bit accumulate operand. Overflow cannot occur during the multiplications or the
 subtraction. Overflow can occur as a result of the 64-bit addition, and this overflow is not
 detected. Instead, the result wraps round to modulo2^64.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the difference of the product of each multiplication, added to the accumulate value.
 \remark
 p1 = val1[15:0]  * val2[15:0]      \n
 p2 = val1[31:16] * val2[31:16]     \n
 res[63:32][31:0] = p1 - p2 + val3[63:32][31:0]
 */
u32 __SMLSLD(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y << 16) >> 16))
					- ((((s32) x) >> 16) * (((s32) y) >> 16)) + (((u32) sum))));
}

/**
 \brief   Dual 16-bit signed multiply with exchange subtract with 64-bit accumulate.
 \details This function enables you to exchange the halfwords of the second operand, perform two 16-bit multiplications,
 adding the difference of the products to a 64-bit accumulate operand. Overflow cannot occur during the
 multiplications or the subtraction. Overflow can occur as a result of the 64-bit addition, and this overflow
 is not detected. Instead, the result wraps round to modulo2^64.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \param [in]  sum   accumulate value.
 \return        the difference of the product of each multiplication, added to the accumulate value.
 \remark
 p1 = val1[15:0]  * val2[31:16]      \n
 p2 = val1[31:16] * val2[15:0]       \n
 res[63:32][31:0] = p1 - p2 + val3[63:32][31:0]
 */
u32 __SMLSLDX(u32 x, u32 y, u32 sum)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y) >> 16))
					- ((((s32) x) >> 16) * (((s32) y << 16) >> 16))
					+ (((u32) sum))));
}

/**
 \brief   32-bit signed multiply with 32-bit truncated accumulator.
 \details This function enables you to perform a signed 32-bit multiplications, adding the most
 significant 32 bits of the 64-bit result to a 32-bit accumulate operand.
 \param [in]    x   first operand for multiplication.
 \param [in]    y   second operand for multiplication.
 \param [in]  sum   accumulate value.
 \return        the product of multiplication (most significant 32 bits) is added to the accumulate value, as a 32-bit integer.
 \remark
 p = val1 * val2      \n
 res[31:0] = p[63:32] + val3[31:0]
 */
/*
 u32 __SMMLA(s32 x, s32 y, s32 sum)
 {
 return (u32)((s32)((s32)((s32)x * (s32)y) >> 32) + sum);
 }
 */

/**
 \brief   Sum of dual 16-bit signed multiply.
 \details This function enables you to perform two 16-bit signed multiplications, adding the products together.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \return        the sum of the products of the two 16-bit signed multiplications.
 \remark
 p1 = val1[15:0]  * val2[15:0]      \n
 p2 = val1[31:16] * val2[31:16]     \n
 res[31:0] = p1 + p2
 */
u32 __SMUAD(u32 x, u32 y)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y << 16) >> 16))
					+ ((((s32) x) >> 16) * (((s32) y) >> 16))));
}

/**
 \brief   Dual 16-bit signed multiply returning difference.
 \details This function enables you to perform two 16-bit signed multiplications, taking the difference
 of the products by subtracting the high halfword product from the low halfword product.
 \param [in]    x   first 16-bit operands for each multiplication.
 \param [in]    y   second 16-bit operands for each multiplication.
 \return        the difference of the products of the two 16-bit signed multiplications.
 \remark
 p1 = val1[15:0]  * val2[15:0]      \n
 p2 = val1[31:16] * val2[31:16]     \n
 res[31:0] = p1 - p2
 */
u32 __SMUSD(u32 x, u32 y)
{
	return ((u32)(
			((((s32) x << 16) >> 16) * (((s32) y << 16) >> 16))
					- ((((s32) x) >> 16) * (((s32) y) >> 16))));
}

/**
 \brief   Dual extracted 8-bit to 16-bit signed addition.
 \details This function enables you to extract two 8-bit values from the second operand (at bit positions
 [7:0] and [23:16]), sign-extend them to 16-bits each, and add the results to the first operand.
 \param [in]    x   values added to the sign-extended to 16-bit values.
 \param [in]    y   two 8-bit values to be extracted and sign-extended.
 \return        the addition of val1 and val2, where the 8-bit values in val2[7:0] and
 val2[23:16] have been extracted and sign-extended prior to the addition.
 \remark
 res[15:0]  = val1[15:0] + SignExtended(val2[7:0])      \n
 res[31:16] = val1[31:16] + SignExtended(val2[23:16])
 */
u32 __SXTAB16(u32 x, u32 y)
{
	return ((u32)(
			(((((s32) y << 24) >> 24) + (((s32) x << 16) >> 16))
					& (s32) 0x0000FFFF)
					| (((((s32) y << 8) >> 8) + (((s32) x >> 16) << 16))
							& (s32) 0xFFFF0000)));
}

/**
 \brief   Extracted 16-bit to 32-bit unsigned addition.
 \details This function enables you to extract two 8-bit values from one operand, zero-extend
 them to 16 bits each, and add the results to two 16-bit values from another operand.
 \param [in]    x   values added to the zero-extended to 16-bit values.
 \param [in]    y   two 8-bit values to be extracted and zero-extended.
 \return        the addition of val1 and val2, where the 8-bit values in val2[7:0] and
 val2[23:16] have been extracted and zero-extended prior to the addition.
 \remark
 res[15:0]  = ZeroExt(val2[7:0]   to 16 bits) + val1[15:0]      \n
 res[31:16] = ZeroExt(val2[31:16] to 16 bits) + val1[31:16]
 */
u32 __UXTAB16(u32 x, u32 y)
{
	return ((u32)(
			((((y << 24) >> 24) + ((x << 16) >> 16)) & 0x0000FFFF)
					| ((((y << 8) >> 8) + ((x >> 16) << 16)) & 0xFFFF0000)));
}

/**
 \brief   Dual extract 8-bits and sign extend each to 16-bits.
 \details This function enables you to extract two 8-bit values from an operand and sign-extend them to 16 bits each.
 \param [in]    x   two 8-bit values in val[7:0] and val[23:16] to be sign-extended.
 \return        the 8-bit values sign-extended to 16-bit values.\n
 sign-extended value of val[7:0] in the low halfword of the return value.\n
 sign-extended value of val[23:16] in the high halfword of the return value.
 \remark
 res[15:0]  = SignExtended(val[7:0])       \n
 res[31:16] = SignExtended(val[23:16])
 */
u32 __SXTB16(u32 x)
{
	return ((u32)(
			((((s32) x << 24) >> 24) & (s32) 0x0000FFFF)
					| ((((s32) x << 8) >> 8) & (s32) 0xFFFF0000)));
}

/**
 \brief   Dual extract 8-bits and zero-extend to 16-bits.
 \details This function enables you to extract two 8-bit values from an operand and zero-extend them to 16 bits each.
 \param [in]    x   two 8-bit values in val[7:0] and val[23:16] to be zero-extended.
 \return        the 8-bit values sign-extended to 16-bit values.\n
 sign-extended value of val[7:0] in the low halfword of the return value.\n
 sign-extended value of val[23:16] in the high halfword of the return value.
 \remark
 res[15:0]  = SignExtended(val[7:0])       \n
 res[31:16] = SignExtended(val[23:16])
 */
u32 __UXTB16(u32 x)
{
	return ((u32)(
			(((x << 24) >> 24) & 0x0000FFFF) | (((x << 8) >> 8) & 0xFFFF0000)));
}

//#define CORET_BASE          (PLIC_BASE + 0x4000000UL)                            /*!< CORET Base Address */
//#define PLIC_BASE           (0x4000000000UL)                          /*!< PLIC Base Address */

//#define CORET               ((CORET_Type   *)     CORET_BASE  )       /*!< SysTick configuration struct */
#define CLINT               ((CLINT_Type    *)    CLINT_BASE)       /*!< CLINT configuration struct */
//#define PLIC                ((PLIC_Type    *)     PLIC_BASE   )       /*!< PLIC configuration struct */

/*@} */

/*******************************************************************************
 *                Hardware Abstraction Layer
 Core Function Interface contains:
 - Core VIC Functions
 - Core CORET Functions
 - Core Register Access Functions
 ******************************************************************************/
/**
 \defgroup c906_Core_FunctionInterface Functions and Instructions Reference
 */

/* ##########################   VIC functions  #################################### */
/**
 \ingroup  c906_Core_FunctionInterface
 \defgroup c906_Core_VICFunctions VIC Functions
 \brief    Functions that manage interrupts and exceptions via the VIC.
 @{
 */

/* The following MACROS handle generation of the register offset and byte masks */
#define _BIT_SHIFT(IRQn)         (((((u32)(s32)(IRQn))) & 0x03UL) * 8UL)
#define _IP_IDX(IRQn)            ((((u32)(s32)(IRQn)) >> 5UL))
#define _IP2_IDX(IRQn)           ((((u32)(s32)(IRQn)) >> 2UL))

/**
 \brief   Enable External Interrupt
 \details Enable a device-specific interrupt in the VIC interrupt controller.
 \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
// void c906_vic_enable_irq(s32 IRQn)
//{
////    CLINT->CLINTINT[IRQn].IE |= CLINT_INTIE_IE_Msk;
//
//	PLIC->PLIC_H0_MIE[IRQn/32] = PLIC->PLIC_H0_MIE[IRQn/32] | (0x1 << (IRQn%32));
//}
/**
 \brief   Disable External Interrupt
 \details Disable a device-specific interrupt in the VIC interrupt controller.
 \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
// void c906_vic_disable_irq(s32 IRQn)
//{
//    //CLINT->CLINTINT[IRQn].IE &= ~CLINT_INTIE_IE_Msk;
//    PLIC->PLIC_H0_MIE[IRQn/32] = PLIC->PLIC_H0_MIE[IRQn/32] & (~(0x1 << (IRQn%32)));
//}
/**
 \brief   Enable External Secure Interrupt
 \details Enable a secure device-specific interrupt in the VIC interrupt controller.
 \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
/*
 void c906_vic_enable_sirq(s32 IRQn)
 {
 //CLINT->CLINTINT[IRQn].IE |= (CLINT_INTIE_IE_Msk | CLINT_INTIE_T_Msk);
 c906_vic_enable_irq(IRQn);
 }
 */
/**
 \brief   Disable External Secure Interrupt
 \details Disable a secure device-specific interrupt in the VIC interrupt controller.
 \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
/*
 void c906_vic_disable_sirq(s32 IRQn)
 {
 //CLINT->CLINTINT[IRQn].IE &= ~(CLINT_INTIE_IE_Msk | CLINT_INTIE_T_Msk);
 c906_vic_disable_irq(IRQn);
 }
 */
/**
 \brief   Check Interrupt is Enabled or not
 \details Read the enabled register in the VIC and returns the pending bit for the specified interrupt.
 \param [in]      IRQn  Interrupt number.
 \return             0  Interrupt status is not enabled.
 \return             1  Interrupt status is enabled.
 */
// u32 c906_vic_get_enabled_irq(s32 IRQn)
//{
////    return (u32)(CLINT->CLINTINT[IRQn].IE & CLINT_INTIE_IE_Msk);
//    return (u32)((PLIC->PLIC_H0_MIE[IRQn/32] >> IRQn%32) & 0x1);
//}
/**
 \brief   Check Interrupt is Pending or not
 \details Read the pending register in the VIC and returns the pending bit for the specified interrupt.
 \param [in]      IRQn  Interrupt number.
 \return             0  Interrupt status is not pending.
 \return             1  Interrupt status is pending.
 */
// u32 c906_vic_get_pending_irq(s32 IRQn)
//{
////    return (u32)(CLINT->CLINTINT[IRQn].IP & CLINT_INTIP_IP_Msk);
//
//	return (u32)((PLIC->PLIC_IP[IRQn/32] >> IRQn%32) & 0x1);
//}
/**
 \brief   Set Pending Interrupt
 \details Set the pending bit of an external interrupt.
 \param [in]      IRQn  Interrupt number. Value cannot be negative.
 */
// void c906_vic_set_pending_irq(s32 IRQn)
//{
////    CLINT->CLINTINT[IRQn].IP |= CLINT_INTIP_IP_Msk;
//	PLIC->PLIC_IP[IRQn/32] = PLIC->PLIC_IP[IRQn/32] | (0x1 << (IRQn%32));
//}
/**
 \brief   Clear Pending Interrupt
 \details Clear the pending bit of an external interrupt.
 \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
// void c906_vic_clear_pending_irq(s32 IRQn)
//{
////    CLINT->CLINTINT[IRQn].IP &= ~CLINT_INTIP_IP_Msk;
//	PLIC->PLIC_H0_SCLAIM = IRQn;
//}
/**
 \brief   Set Interrupt Priority
 \details Set the priority of an interrupt.
 \note    The priority cannot be set for every core interrupt.
 \param [in]      IRQn  Interrupt number.
 \param [in]  priority  Priority to set.
 */
// void c906_vic_set_prio(s32 IRQn, u32 priority)
//{
////    u8 nlbits = (CLINT->CLINTINFO & CLINT_INFO_CLINTINTCTLBITS_Msk) >> CLINT_INFO_CLINTINTCTLBITS_Pos;
////    CLINT->CLINTINT[IRQn].CTL = (CLINT->CLINTINT[IRQn].CTL & (~CLINT_INTCFG_PRIO_Msk)) | (priority << (8 - nlbits));
//
//    PLIC->PLIC_PRIO[IRQn] = priority;
//}
/**
 \brief   Get Interrupt Priority
 \details Read the priority of an interrupt.
 The interrupt number can be positive to specify an external (device specific) interrupt,
 or negative to specify an internal (core) interrupt.
 \param [in]   IRQn  Interrupt number.
 \return             Interrupt Priority.
 Value is aligned automatically to the implemented priority bits of the microcontroller.
 */
// u32 c906_vic_get_prio(s32 IRQn)
//{
////    u8 nlbits = (CLINT->CLINTINFO & CLINT_INFO_CLINTINTCTLBITS_Msk) >> CLINT_INFO_CLINTINTCTLBITS_Pos;
////    return CLINT->CLINTINT[IRQn].CTL >> (8 - nlbits);
//
//    u32 prio = PLIC->PLIC_PRIO[IRQn];
//    return prio;
//}
/**
 \brief   Set interrupt handler
 \details Set the interrupt handler according to the interrupt num, the handler will be filled in irq vectors.
 \param [in]      IRQn  Interrupt number.
 \param [in]   handler  Interrupt handler.
 */
void c906_vic_set_vector(u32 IRQn, irqhandle handler)
{
	if (IRQn >= 0 && IRQn < 1024) {
		//u32 *vectors = (u32 *)&vector_table;
		vectors[IRQn] = handler;
	}
}

/*@} end of c906_Core_VICFunctions */

/* ##########################   PMP functions  #################################### */
/**
 \ingroup  c906_Core_FunctionInterface
 \defgroup c906_Core_PMPFunctions PMP Functions
 \brief    Functions that manage interrupts and exceptions via the VIC.
 @{
 */

/**
 \brief  configure memory protected region.
 \details
 \param [in]  idx        memory protected region (0, 1, 2, ..., 15).
 \param [in]  base_addr  base address must be aligned with page size.
 \param [in]  size       \ref region_size_e. memory protected region size.
 \param [in]  attr       \ref region_size_t. memory protected region attribute.
 \param [in]  enable     enable or disable memory protected region.
 */
void c906_mpu_config_region(u32 idx, u32 base_addr, region_size_e size,
		mpu_region_attr_t attr, u32 enable)
{
	u8 pmpxcfg = 0;
	u32 addr = 0;

	if (idx > 15) {
		return;
	}

	if (!enable) {
		attr.a = 0;
	}

	if (attr.a == ADDRESS_MATCHING_TOR) {
		addr = base_addr >> 2;
	} else {
		if (size == REGION_SIZE_4B) {
			addr = base_addr >> 2;
			attr.a = 2;
		} else {
			addr = ((base_addr >> 2) & (0xFFFFFFFFU - ((1 << (size + 1)) - 1)))
					| ((1 << size) - 1);
		}
	}

	__set_PMPADDRx(idx, addr);

	pmpxcfg |= (attr.r << PMP_PMPCFG_R_Pos) | (attr.w << PMP_PMPCFG_W_Pos)
			| (attr.x << PMP_PMPCFG_X_Pos) | (attr.a << PMP_PMPCFG_A_Pos)
			| (attr.l << PMP_PMPCFG_L_Pos);

	__set_PMPxCFG(idx, pmpxcfg);
}

/**
 \brief  disable mpu region by idx.
 \details
 \param [in]  idx        memory protected region (0, 1, 2, ..., 15).
 */
void c906_mpu_disable_region(u32 idx)
{
	__set_PMPxCFG(idx, __get_PMPxCFG(idx) & (~PMP_PMPCFG_A_Msk));
}

/*@} end of c906_Core_PMPFunctions */

/* ##################################    SysTick function  ############################################ */
/**
 \ingroup  c906_Core_FunctionInterface
 \defgroup c906_Core_SysTickFunctions SysTick Functions
 \brief    Functions that configure the System.
 @{
 */

/**
 \brief   CORE timer Configuration
 \details Initializes the System Timer and its interrupt, and starts the System Tick Timer.
 Counter is in free running mode to generate periodic interrupts.
 \param [in]  ticks  Number of ticks between two interrupts.
 \param [in]  IRQn   core timer Interrupt number.
 \return          0  Function succeeded.
 \return          1  Function failed.
 \note    When the variable <b>__Vendor_SysTickConfig</b> is set to 1, then the
 function <b>SysTick_Config</b> is not included. In this case, the file <b><i>device</i>.h</b>
 must contain a vendor-specific implementation of this function.
 */
// u32 c906_coret_config(u32 ticks, s32 IRQn)
//{
//    u32 value = (((u32)CORET->MTIMECMPH0) << 32) + (u32)CORET->MTIMECMPL0;
//
//#if 0
//    if(value){
//        value = value + (u32)ticks;
//        CORET->MTIMECMPH0 = (u32)(value >> 32);
//        CORET->MTIMECMPL0 = (u32)value;
//    }else
//    {
//        u32 result;
//	__ASM volatile("csrr %0, 0xc01" : "=r"(result));
//
//        value = result + (u32)ticks;
//        CORET->MTIMECMPH0 = (u32)(value >> 32);
//        CORET->MTIMECMPL0 = (u32)value;
//    }
//#else
//        value = value + (u32)ticks;
//        CORET->MTIMECMPH0 = (u32)(value >> 32);
//        CORET->MTIMECMPL0 = (u32)value;
//#endif
//
//
//    return (0UL);
//}
/**
 \brief   get CORE timer reload value
 \return          CORE timer counter value.
 */
// u32 c906_coret_get_load(void)
//{
//    u32 value = (((u32)CORET->MTIMECMPH0) << 32) + (u32)CORET->MTIMECMPL0;
//    return value;
//}
/**
 \brief   get CORE timer reload high value
 \return          CORE timer counter value.
 */
// u32 c906_coret_get_loadh(void)
//{
//    u32 value = (((u32)CORET->MTIMECMPH0) << 32) + (u32)CORET->MTIMECMPL0;
//    return (value >> 32) & 0xFFFFFFFF;
//}
/**
 \brief   get CORE timer counter value
 \return          CORE timer counter value.
 */
u32 c906_coret_get_value(void)
{
	u32 result;
	__asm volatile("csrr %0, 0xc01" : "=r"(result));
	return result;
}

/**
 \brief   get CORE timer counter high value
 \return          CORE timer counter value.
 */
/*
 u32 c906_coret_get_valueh(void)
 {
 u32 result;
 __asm volatile("csrr %0, time" : "=r"(result));
 return (result >> 32) & 0xFFFFFFFF;
 }
 */

u32 c906_coret_get_valuel(void)
{
	u32 result;
	__asm volatile("csrr %0, time" : "=r"(result));
	return (result) & 0xFFFFFFFF;
}

/*@} end of c906_core_DebugFunctions */

/* ##########################  Cache functions  #################################### */
/**
 \ingroup  c906_Core_FunctionInterface
 \defgroup c906_Core_CacheFunctions Cache Functions
 \brief    Functions that configure Instruction and Data cache.
 @{
 */

/**
 \brief   Enable I-Cache
 \details Turns on I-Cache
 */
void c906_icache_enable(void)
{
#if (__ICACHE_PRESENT == 1U)
	u32 cache;
	__DSB();
	__ISB();
	__ICACHE_IALL();
	cache = __get_MHCR();
	cache |= CACHE_MHCR_IE_Msk;
	__set_MHCR(cache);
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Disable I-Cache
 \details Turns off I-Cache
 */
void c906_icache_disable(void)
{
#if (__ICACHE_PRESENT == 1U)
	u32 cache;
	__DSB();
	__ISB();
	cache = __get_MHCR();
	cache &= ~CACHE_MHCR_IE_Msk; /* disable icache */
	__set_MHCR(cache);
	__ICACHE_IALL(); /* invalidate all icache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Invalidate I-Cache
 \details Invalidates I-Cache
 */
void c906_icache_invalid(void)
{
#if (__ICACHE_PRESENT == 1U)
	__DSB();
	__ISB();
	__ICACHE_IALL(); /* invalidate all icache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Enable D-Cache
 \details Turns on D-Cache
 \note    I-Cache also turns on.
 */
void c906_dcache_enable(void)
{
#if (__DCACHE_PRESENT == 1U)
	u32 cache;
	__DSB();
	__ISB();
	__DCACHE_IALL();                        /* invalidate all dcache */
	cache = __get_MHCR();
	cache |= (CACHE_MHCR_DE_Msk);      /* enable all Cache */
	__set_MHCR(cache);

	__DSB();
	__ISB();
#endif
}

/**
 \brief   Disable D-Cache
 \details Turns off D-Cache
 \note    I-Cache also turns off.
 */
void c906_dcache_disable(void)
{
#if (__DCACHE_PRESENT == 1U)
	u32 cache;
	__DSB();
	__ISB();
	cache = __get_MHCR();
	cache &= ~(u32)CACHE_MHCR_DE_Msk; /* disable all Cache */
	__set_MHCR(cache);
	__DCACHE_IALL();                             /* invalidate all Cache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Invalidate D-Cache
 \details Invalidates D-Cache
 \note    I-Cache also invalid
 */
void c906_dcache_invalid(void)
{
#if (__DCACHE_PRESENT == 1U)
	__DSB();
	__ISB();
	__DCACHE_IALL();                            /* invalidate all Cache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Clean D-Cache
 \details Cleans D-Cache
 \note    I-Cache also cleans
 */
void c906_dcache_clean(void)
{
#if (__DCACHE_PRESENT == 1U)
	__DSB();
	__ISB();
	__DCACHE_CALL();                                     /* clean all Cache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Clean & Invalidate D-Cache
 \details Cleans and Invalidates D-Cache
 \note    I-Cache also flush.
 */
void c906_dcache_clean_invalid(void)
{
#if (__DCACHE_PRESENT == 1U)
	__DSB();
	__ISB();
	__DCACHE_CIALL();                                   /* clean and inv all Cache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Invalidate L2-Cache
 \details Invalidates L2-Cache
 \note
 */
void c906_l2cache_invalid(void)
{
#if (__L2CACHE_PRESENT == 1U)
	__DSB();
	__ISB();
	__L2CACHE_IALL();                            /* invalidate l2 Cache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Clean L2-Cache
 \details Cleans L2-Cache
 \note
 */
void c906_l2cache_clean(void)
{
#if (__L2CACHE_PRESENT == 1U)
	__DSB();
	__ISB();
	__L2CACHE_CALL();                                     /* clean l2 Cache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   Clean & Invalidate L2-Cache
 \details Cleans and Invalidates L2-Cache
 \note
 */
void c906_l2cache_clean_invalid(void)
{
#if (__L2CACHE_PRESENT == 1U)
	__DSB();
	__ISB();
	__L2CACHE_CIALL();                                   /* clean and inv l2 Cache */
	__DSB();
	__ISB();
#endif
}

/**
 \brief   D-Cache Invalidate by address
 \details Invalidates D-Cache for the given address
 \param[in]   addr    address (aligned to 32-byte boundary)
 \param[in]   dsize   size of memory block (in number of bytes)
 */
void c906_dcache_invalid_range(u32 *addr, s32 dsize)
{
#if (__DCACHE_PRESENT == 1U)
	s32 op_size = dsize + (u32)addr % 32;
	u32 op_addr = (u32)addr;
	s32 linesize = 32;

	__DSB();

	while (op_size > 0) {
		__DCACHE_IPA(op_addr);
		op_addr += linesize;
		op_size -= linesize;
	}

	__DSB();
	__ISB();
#endif
}

/**
 \brief   D-Cache Clean by address
 \details Cleans D-Cache for the given address
 \param[in]   addr    address (aligned to 32-byte boundary)
 \param[in]   dsize   size of memory block (in number of bytes)
 */
void c906_dcache_clean_range(u32 *addr, s32 dsize)
{

#if (__DCACHE_PRESENT == 1)
	volatile s32 op_size = dsize + (u32)addr % 32;
	volatile u32 op_addr = (u32) addr & CACHE_INV_ADDR_Msk;
	s32 linesize = 32;

	__DSB();

	while (op_size > 0) {
		__DCACHE_CPA(op_addr);
		op_addr += linesize;
		op_size -= linesize;
	}

	__DSB();
	__ISB();
#endif

}

/**
 \brief   D-Cache Clean and Invalidate by address
 \details Cleans and invalidates D_Cache for the given address
 \param[in]   addr    address (aligned to 16-byte boundary)
 \param[in]   dsize   size of memory block (aligned to 16-byte boundary)
 */
void c906_dcache_clean_invalid_range(u32 *addr, s32 dsize)
{
#if (__DCACHE_PRESENT == 1U)
	volatile s32 op_size = dsize + (u32)addr % 32;
	volatile u32 op_addr = (u32) addr;
	s32 linesize = 32;

	__DSB();

	while (op_size > 0) {
		__DCACHE_CIPA(op_addr);
		op_addr += linesize;
		op_size -= linesize;
	}

	__DSB();
	__ISB();
#endif
}

/**
 \brief   setup cacheable range Cache
 \details setup Cache range
 */
void c906_cache_set_range(u32 index, u32 baseAddr, u32 size, u32 enable)
{
	;
}

/**
 \brief   Enable cache profile
 \details Turns on Cache profile
 */
void c906_cache_enable_profile(void)
{
	;
}

/**
 \brief   Disable cache profile
 \details Turns off Cache profile
 */
void c906_cache_disable_profile(void)
{
	;
}

/**
 \brief   Reset cache profile
 \details Reset Cache profile
 */
void c906_cache_reset_profile(void)
{
	;
}

/**
 \brief   cache access times
 \details Cache access times
 \note    every 256 access add 1.
 \return          cache access times, actual times should be multiplied by 256
 */
u32 c906_cache_get_access_time(void)
{
	return 0;
}

/**
 \brief   cache miss times
 \details Cache miss times
 \note    every 256 miss add 1.
 \return          cache miss times, actual times should be multiplied by 256
 */
u32 c906_cache_get_miss_time(void)
{
	return 0;
}

/*@} end of c906_Core_CacheFunctions */

/*@} end of c906_core_DebugFunctions */

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

/*@} end of IRQ Functions */

static void null_plic_hdle(void)
{
	printk("No irq registered handler for this calling !!\n");
}

static void null_clic_hdle(void)
{
	printk("No irq registered handler for this calling !!\n");
}

void init_plic_handle(void)
{
	u32 i;
	for (i = 0; i < 256; i++)
		vectors[i] = null_plic_hdle;
}

void init_clic_handle(void)
{
	u32 i;
	for (i = 0; i < 256; i++)
		vectors[i] = null_clic_hdle;
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
//	u32 intnum;
//	pattern_goto(0x1234);
//	printk("%s: mcause: 0x%x\n", __func__, mcause);
//	printk("%s: epc: 0x%x\n", __func__, epc);
	if (mcause >> 31) {
		//interrupt
//		intnum = mcause & 0xFF;
//		msg("hadle_trap: int%d coming!\n",intnum);
//		vectors[intnum]();
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
