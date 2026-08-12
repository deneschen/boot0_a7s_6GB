#include <config.h>

	.syntax unified
	.thumb

	/*
	 * Relocated to CONFIG_FIP_HANDOFF_BASE (DRAM) before use: SCP_SRAM
	 * (0x44000..0x6c000) overlaps boot0 @ 0x47000, so this code cannot
	 * run from SRAM while staging SCP_BL2.
	 *
	 * Skip SCTLR MMU/cache disable: it hangs on this A733 path, and RMR
	 * restarts the PE with a clean MMU state at CONFIG_MONITOR_ENTRY.
	 */
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
	ldr	r3, =SUNXI_E902_RESET_CTRL_REG
	ldr	r4, [r3]
	bic	r4, r4, #E902_RESET_DEASSERT
	dmb	sy
	str	r4, [r3]
	dsb	sy

	/* Word clear then word copy (byte loop was needlessly slow). */
	ldr	r3, =SCP_SRAM_BASE
	movs	r4, #0
	ldr	r5, =SCP_SRAM_SIZE
1:
	str	r4, [r3], #4
	subs	r5, r5, #4
	bne	1b

	/* r7 may not be word-aligned; copy remaining bytes after words. */
	ldr	r3, =SCP_SRAM_BASE
	mov	r5, r7
	lsrs	r5, r5, #2
	beq	2f
1:
	ldr	r4, [r6], #4
	str	r4, [r3], #4
	subs	r5, r5, #1
	bne	1b
2:
	ands	r7, r7, #3
	beq	4f
3:
	ldrb	r4, [r6], #1
	strb	r4, [r3], #1
	subs	r7, r7, #1
	bne	3b

4:
	/*
	 * The MMU and D-cache are still enabled. Publish the SRAM clear/copy
	 * before releasing E902; the C-side clean only covers the FIP source.
	 */
	ldr	r3, =SCP_SRAM_BASE
	ldr	r5, =SCP_SRAM_SIZE
	add	r5, r3, r5
	bic	r3, r3, #31
6:
	mcr	p15, 0, r3, c7, c10, 1
	add	r3, r3, #32
	cmp	r3, r5
	blo	6b
	dsb	sy

	/* Publish startup inputs before releasing E902. */
	ldr	r3, =SUNXI_RTC_DTB_BASE_STORE_REG
	str	r8, [r3]
	ldr	r3, =SUNXI_AR100S_BOOT0_TRACE_REG
	ldr	r4, =SUNXI_AR100S_BOOT0_TRACE_MAGIC
	str	r4, [r3]
	ldr	r3, =SUNXI_AR100S_E902_TRACE_REG
	movs	r4, #0
	str	r4, [r3]

	/* Reproduce the A733 vendor BL31 E902 release sequence exactly. */
	ldr	r3, =SUNXI_RISCV_BGR_REG
	ldr	r4, =(RISCV_CFG_RESET | RISCV_CFG_GATING | RISCV_BUS_GATING)
	str	r4, [r3]
	dsb	sy

	ldr	r3, =SUNXI_E902_RST_START_ADDR_REG
	ldr	r4, =SUNXI_E902_RESET_VECTOR
	str	r4, [r3]
	dsb	sy

	ldr	r3, =SUNXI_E902_RESET_CTRL_REG
	ldr	r4, [r3]
	orr	r4, r4, #E902_RESET_DEASSERT
	dmb	sy
	str	r4, [r3]
	dsb	sy

	ldr	r3, =SUNXI_RISCV_24M_CLK_REG
	ldr	r4, [r3]
	orr	r4, r4, #RISCV_CLK_GATING
	str	r4, [r3]
	dsb	sy
	isb	sy

	/* Preserve the real secure-state E902 reset readback for diagnosis. */
	ldr	r3, =SUNXI_E902_RESET_CTRL_REG
	ldr	r4, [r3]
	and	r4, r4, #E902_RESET_DEASSERT
	ldr	r5, =SUNXI_AR100S_BOOT0_TRACE_MAGIC
	orr	r4, r4, r5
	ldr	r3, =SUNXI_AR100S_BOOT0_TRACE_REG
	str	r4, [r3]
	dsb	sy

	/*
	 * Program INITARCH + both RVBAR aliases to the real AArch64 BL31
	 * entry (monitor header + 0x1000), then request RMR AA64+RR.
	 */
	ldr	r3, =SUNXI_INITARCH_REG
	ldr	r4, [r3]
	orr	r4, r4, #1
	dmb	sy
	str	r4, [r3]
	dsb	sy

	ldr	r3, =SUNXI_ALT_RVBAR_LO
	ldr	r4, =CONFIG_MONITOR_ENTRY
	dmb	sy
	str	r4, [r3]
	dmb	sy
	movs	r2, #0
	str	r2, [r3, #4]

	ldr	r3, =RVBARADDR0_L
	ldr	r4, =CONFIG_MONITOR_ENTRY
	dmb	sy
	str	r4, [r3]
	dmb	sy
	movs	r2, #0
	str	r2, [r3, #4]
	dsb	sy

	mrc	p15, 0, r2, c12, c0, 2
	orr	r2, r2, #3
	dsb	sy
	mcr	p15, 0, r2, c12, c0, 2
	isb	sy
5:
	wfi
	b	5b

	.ltorg
fip_handoff_end:
	.size fip_handoff_start, fip_handoff_end - fip_handoff_start

	.section .note.GNU-stack, "", %progbits
