/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _BOOT0_CLOCK_DIAG_A733_H_
#define _BOOT0_CLOCK_DIAG_A733_H_

/* All rates are expressed in kHz to keep early 32-bit calculations bounded. */
struct a7s_clock_rates {
	unsigned int dcxo;
	unsigned int ref;
	unsigned int cpu_back_pll;
	unsigned int cpu_a_pll;
	unsigned int cpu_b_pll;
	unsigned int dsu_pll;
	unsigned int cpu_a;
	unsigned int cpu_b;
	unsigned int dsu;
	unsigned int dsu_axi;
	unsigned int dsu_apb;
	unsigned int dsu_gic;
	unsigned int ddr;
	unsigned int peri0_2x;
	unsigned int peri0_800;
	unsigned int peri0_480;
	unsigned int peri1_2x;
	unsigned int peri1_800;
	unsigned int peri1_480;
	unsigned int npu;
	unsigned int de_3x;
	unsigned int cci;
	unsigned int ahb;
	unsigned int apb0;
	unsigned int apb1;
	unsigned int uart;
	unsigned int gic;
	unsigned int nsi;
	unsigned int mbus;
};

/* Read the current hardware configuration without changing any register. */
void a7s_clock_read_rates(struct a7s_clock_rates *rates);

/* Print current PLL, CPU/DSU and system-bus rates from a read-only snapshot. */
void a7s_clock_dump(void);

#endif
