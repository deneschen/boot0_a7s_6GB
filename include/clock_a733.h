/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _BOOT0_CLOCK_A733_H_
#define _BOOT0_CLOCK_A733_H_

enum a7s_clock_status {
	A7S_CLOCK_OK = 0,
	A7S_CLOCK_ERR_REF_PLL = -1,
	A7S_CLOCK_ERR_PERI0_PLL = -2,
	A7S_CLOCK_ERR_PERI1_PLL = -3,
	A7S_CLOCK_ERR_BUS = -4,
};

/*
 * Establish the boot-critical A733 clock roots and system buses.
 * CPU/DSU, DDR, NSI/MBUS, GPU/NPU and media clocks keep their stage owner.
 */
int a7s_clock_init(void);

#endif
