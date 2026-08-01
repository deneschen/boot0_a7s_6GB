#include <config.h>

	.syntax unified
	.thumb

	.section .text.fip_handoff, "ax", %progbits
	.align 2
	.global fip_handoff_start
	.global fip_handoff_end
	.type fip_handoff_start, %function
	.thumb_func
fip_handoff_start:
	mov	r6, r0
	mov	r7, r1
	mov	r8, r2			/* validated HW_CONFIG address */

	/* Hold AR100S in reset before replacing its SRAM image. */
	ldr	r3, =SUNXI_RCPUCFG_BASE
	ldr	r4, [r3]
	bic	r4, r4, #1
	dmb	sy
	str	r4, [r3]
	dsb	sy

	/* Enable the documented E902 clock and configuration bus. */
	ldr	r3, =SUNXI_RISCV_24M_CLK_REG
	ldr	r4, [r3]
	bic	r4, r4, #RISCV_CLK_SOURCE_MASK
	orr	r4, r4, #RISCV_CLK_GATING
	str	r4, [r3]
	dsb	sy

	/* Keep configuration reset asserted while opening both bus gates. */
	ldr	r3, =SUNXI_RISCV_BGR_REG
	ldr	r4, [r3]
	bic	r4, r4, #RISCV_CFG_RESET
	orr	r4, r4, #(RISCV_CFG_GATING | RISCV_BUS_GATING)
	str	r4, [r3]
	dsb	sy
	orr	r4, r4, #RISCV_CFG_RESET
	str	r4, [r3]
	dsb	sy

	ldr	r3, =SCP_SRAM_BASE
	movs	r4, #0
	ldr	r5, =SCP_SRAM_SIZE
1:
	str	r4, [r3], #4
	subs	r5, r5, #4
	bne	1b

	ldr	r3, =SCP_SRAM_BASE
2:
	cmp	r7, #0
	beq	3f
	ldrb	r4, [r6], #1
	strb	r4, [r3], #1
	subs	r7, r7, #1
	b	2b

	3:
	/* Publish immutable startup inputs before releasing the E902 reset. */
	ldr	r3, =SUNXI_RTC_DTB_BASE_STORE_REG
	str	r8, [r3]
	ldr	r3, =SUNXI_E902_RST_START_ADDR_REG
	ldr	r4, =SUNXI_E902_RESET_VECTOR
	str	r4, [r3]

	dsb	sy
	isb	sy
	ldr	r3, =SUNXI_RCPUCFG_BASE
	ldr	r4, [r3]
	orr	r4, r4, #1
	dmb	sy
	str	r4, [r3]
	dsb	sy
	isb	sy

	/* Identity mapping lets this DRAM stub continue after disabling the MMU. */
	mrc	p15, 0, r4, c1, c0, 0
	bic	r4, r4, #0x1000
	bic	r4, r4, #0x7
	mcr	p15, 0, r4, c1, c0, 0
	isb	sy
	movs	r4, #0
	mcr	p15, 0, r4, c7, c5, 0
	mcr	p15, 0, r4, c7, c5, 6
	dsb	sy
	isb	sy

	/* Match boot0_jmp_monitor(): publish BL31 and wake the application CPU. */
	ldr	r3, =RVBARADDR0_L
	ldr	r4, =CONFIG_MONITOR_BASE
	dmb	sy
	str	r4, [r3]
	dmb	sy
	movs	r2, #0
	str	r2, [r3, #4]
	mrc	p15, 0, r2, c12, c0, 2
	orr	r2, r2, #3
	dsb	sy
	mcr	p15, 0, r2, c12, c0, 2
	isb	sy
	4:
	wfi
	b	4b

	.ltorg
fip_handoff_end:
	.size fip_handoff_start, fip_handoff_end - fip_handoff_start

	.section .note.GNU-stack, "", %progbits
