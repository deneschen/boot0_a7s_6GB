/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <common.h>
#include <clock_diag_a733.h>
#include <arch/sun60iw2p1/cpu_autogen.h>
#include <arch/sun60iw2p1/clock_autogen.h>

#define A7S_BIT(n)			(1U << (n))
#define A7S_PLL_ENABLE			A7S_BIT(31)
#define A7S_PLL_LOCK_ENABLE		A7S_BIT(29)
#define A7S_PLL_LOCK			A7S_BIT(28)

#define A7S_CPU_PLL_BASE		SUNXI_CPU_PLL_CFG_BASE
#define A7S_CPU_BACK_PLL_CTRL		(A7S_CPU_PLL_BASE + 0x0000U)
#define A7S_CPU_A_PLL_CTRL		(A7S_CPU_PLL_BASE + 0x1000U)
#define A7S_CPU_A_CLK			(A7S_CPU_PLL_BASE + 0x101cU)
#define A7S_CPU_A_GATING		(A7S_CPU_PLL_BASE + 0x1020U)
#define A7S_CPU_B_PLL_CTRL		(A7S_CPU_PLL_BASE + 0x2000U)
#define A7S_CPU_B_CLK			(A7S_CPU_PLL_BASE + 0x201cU)
#define A7S_CPU_B_GATING		(A7S_CPU_PLL_BASE + 0x2020U)
#define A7S_DSU_PLL_CTRL		(A7S_CPU_PLL_BASE + 0x3000U)
#define A7S_DSU_CLK			(A7S_CPU_PLL_BASE + 0x301cU)
#define A7S_DSU_GATING			(A7S_CPU_PLL_BASE + 0x3020U)
#define A7S_CPU_SLOW_CLK_SEL		(A7S_CPU_PLL_BASE + 0x3034U)

#define A7S_RTC_XO_CTRL0		(SUNXI_RTC_BASE + 0x160U)
#define A7S_AUDIO1_PLL_CTRL		0x00000280U

static unsigned long a7s_ccu_reg(unsigned int offset)
{
	return SUNXI_CCMU_BASE + offset;
}

static unsigned int a7s_dcxo_rate(void)
{
	switch ((readl(A7S_RTC_XO_CTRL0) >> 14) & 3U) {
	case 1:
		return 19200U;
	case 2:
		return 26000U;
	default:
		return 24000U;
	}
}

static unsigned int a7s_div_rate(unsigned int parent, unsigned int divisor)
{
	return parent && divisor ? parent / divisor : 0;
}

static unsigned int a7s_main_pll_input(unsigned int reg,
					unsigned int dcxo,
					unsigned int ref)
{
	return reg & A7S_BIT(24) ? ref : dcxo;
}

static unsigned int a7s_main_pll_output(unsigned int reg,
					 unsigned int parent,
					 unsigned int p_shift,
					 unsigned int p_mask,
					 unsigned int gate_bit)
{
	unsigned int n;
	unsigned int m1;
	unsigned int p;

	if ((reg & (A7S_PLL_ENABLE | A7S_BIT(gate_bit))) !=
	    (A7S_PLL_ENABLE | A7S_BIT(gate_bit)))
		return 0;

	n = ((reg >> 8) & 0xffU) + 1U;
	m1 = ((reg >> 1) & 1U) + 1U;
	p = ((reg >> p_shift) & p_mask) + 1U;
	return a7s_div_rate(parent * n, m1 * p);
}

static unsigned int a7s_ref_pll_rate(unsigned int reg, unsigned int dcxo)
{
	unsigned int n;
	unsigned int m1;
	unsigned int p;

	if ((reg & (A7S_PLL_ENABLE | A7S_BIT(27))) !=
	    (A7S_PLL_ENABLE | A7S_BIT(27)))
		return 0;

	n = ((reg >> 8) & 0xffU) + 1U;
	m1 = ((reg >> 1) & 1U) + 1U;
	p = ((reg >> 16) & 0x7fU) + 1U;
	return a7s_div_rate(dcxo * n, m1 * p);
}

static unsigned int a7s_cpu_back_pll_rate(unsigned int reg,
					   unsigned int dcxo)
{
	return a7s_main_pll_output(reg, dcxo, 20, 7U, 27);
}

static unsigned int a7s_cpu_linear_pll_rate(unsigned int reg,
					     unsigned int dcxo)
{
	unsigned int n;
	unsigned int p;
	unsigned int m0;
	unsigned int m1;

	if ((reg & (A7S_PLL_ENABLE | A7S_BIT(27))) !=
	    (A7S_PLL_ENABLE | A7S_BIT(27)))
		return 0;

	/* Unlike the main CCU PLLs, the linear CPU PLL stores N directly. */
	n = (reg >> 8) & 0xffU;
	p = ((reg >> 16) & 0xfU) + 1U;
	m0 = ((reg >> 20) & 3U) + 1U;
	m1 = (reg & 0xfU) + 1U;
	return a7s_div_rate(dcxo * n, p * m0 * m1);
}

static unsigned int a7s_cpu_slow_rate(unsigned int selector,
					unsigned int dcxo)
{
	switch (selector & 3U) {
	case 0:
		return dcxo;
	case 1:
		return 32U;
	case 2:
		return 16000U;
	default:
		return 0;
	}
}

static unsigned int a7s_cpu_cluster_rate(unsigned int clk_reg,
					  unsigned int slow,
					  unsigned int cluster_pll,
					  unsigned int peri0_2x,
					  unsigned int cpu_back)
{
	unsigned int source = (clk_reg >> 24) & 7U;

	switch (source) {
	case 0:
	case 1:
	case 2:
		return slow;
	case 3:
		return a7s_div_rate(cluster_pll,
					 1U << ((clk_reg >> 16) & 3U));
	case 4:
		return a7s_div_rate(peri0_2x, 2U);
	case 5:
		return cpu_back;
	default:
		return 0;
	}
}

static unsigned int a7s_dsu_rate(unsigned int clk_reg,
				  unsigned int slow,
				  unsigned int dsu_pll,
				  unsigned int cpu_a,
				  unsigned int peri0_2x)
{
	unsigned int source = (clk_reg >> 24) & 7U;
	unsigned int rate;

	switch (source) {
	case 0:
	case 1:
	case 2:
		rate = slow;
		break;
	case 3:
		rate = a7s_div_rate(dsu_pll,
					1U << ((clk_reg >> 16) & 3U));
		break;
	case 4:
		rate = cpu_a;
		break;
	case 5:
		rate = a7s_div_rate(peri0_2x, 2U);
		break;
	default:
		rate = 0;
		break;
	}

	return rate;
}

static unsigned int a7s_standard_bus_rate(unsigned int reg,
					   unsigned int dcxo,
					   unsigned int peri0_2x)
{
	unsigned int parent;

	switch ((reg >> 24) & 3U) {
	case 0:
		parent = dcxo;
		break;
	case 1:
		parent = 32U;
		break;
	case 2:
		parent = 16000U;
		break;
	default:
		parent = a7s_div_rate(peri0_2x, 2U);
		break;
	}

	return a7s_div_rate(parent, (reg & 0x1fU) + 1U);
}

static unsigned int a7s_uart_rate(unsigned int reg,
				   unsigned int dcxo,
				   unsigned int peri0_2x,
				   unsigned int peri0_480)
{
	unsigned int parent;

	switch ((reg >> 24) & 7U) {
	case 0:
		parent = dcxo;
		break;
	case 1:
		parent = 32U;
		break;
	case 2:
		parent = 16000U;
		break;
	case 3:
		parent = a7s_div_rate(peri0_2x, 2U);
		break;
	case 4:
		parent = peri0_480;
		break;
	default:
		parent = 0;
		break;
	}

	return a7s_div_rate(parent, (reg & 0x1fU) + 1U);
}

static unsigned int a7s_gic_rate(unsigned int reg,
				  const struct a7s_clock_rates *rates)
{
	unsigned int parent;

	if (!(reg & A7S_BIT(31)))
		return 0;
	switch ((reg >> 24) & 3U) {
	case 0:
		parent = rates->dcxo;
		break;
	case 1:
		parent = 32U;
		break;
	case 2:
		parent = a7s_div_rate(rates->peri0_2x, 2U);
		break;
	default:
		parent = rates->peri0_480;
		break;
	}
	return a7s_div_rate(parent, (reg & 0x1fU) + 1U);
}

static unsigned int a7s_nsi_rate(unsigned int reg,
				  const struct a7s_clock_rates *rates)
{
	unsigned int parent;

	if (!(reg & A7S_BIT(31)))
		return 0;
	switch ((reg >> 24) & 7U) {
	case 0:
		parent = rates->ddr;
		break;
	case 1:
		parent = rates->peri0_800;
		break;
	case 2:
		parent = a7s_div_rate(rates->peri0_2x, 2U);
		break;
	case 3:
		parent = rates->cci;
		break;
	case 4:
		parent = rates->de_3x;
		break;
	case 5:
		parent = rates->npu;
		break;
	default:
		parent = 0;
		break;
	}
	return a7s_div_rate(parent, (reg & 0x1fU) + 1U);
}

static unsigned int a7s_mbus_rate(unsigned int reg,
				   const struct a7s_clock_rates *rates)
{
	unsigned int parent;

	if (!(reg & A7S_BIT(31)))
		return 0;
	switch ((reg >> 24) & 7U) {
	case 0:
		parent = a7s_div_rate(rates->peri0_2x, 2U);
		break;
	case 1:
		parent = rates->ddr;
		break;
	case 2:
		parent = rates->peri0_480;
		break;
	case 3:
		parent = a7s_div_rate(rates->peri0_2x, 3U);
		break;
	case 4:
		parent = rates->cci;
		break;
	case 5:
		parent = rates->npu;
		break;
	default:
		parent = 0;
		break;
	}
	return a7s_div_rate(parent, (reg & 0x1fU) + 1U);
}

void a7s_clock_read_rates(struct a7s_clock_rates *rates)
{
	unsigned int ref_reg = readl(a7s_ccu_reg(PLL_REF_CTRL_REG));
	unsigned int ddr_reg = readl(a7s_ccu_reg(PLL_DDR_CTRL_REG));
	unsigned int peri0_reg = readl(a7s_ccu_reg(PLL_PERI0_CTRL_REG));
	unsigned int peri1_reg = readl(a7s_ccu_reg(PLL_PERI1_CTRL_REG));
	unsigned int npu_reg = readl(a7s_ccu_reg(PLL_NPU_CTRL_REG));
	unsigned int de_reg = readl(a7s_ccu_reg(PLL_DE_CTRL_REG));
	unsigned int cci_reg = readl(a7s_ccu_reg(PLL_CCI_CTRL_REG));
	unsigned int cpu_back_reg = readl(A7S_CPU_BACK_PLL_CTRL);
	unsigned int cpu_a_pll_reg = readl(A7S_CPU_A_PLL_CTRL);
	unsigned int cpu_b_pll_reg = readl(A7S_CPU_B_PLL_CTRL);
	unsigned int dsu_pll_reg = readl(A7S_DSU_PLL_CTRL);
	unsigned int cpu_a_clk_reg = readl(A7S_CPU_A_CLK);
	unsigned int cpu_b_clk_reg = readl(A7S_CPU_B_CLK);
	unsigned int dsu_clk_reg = readl(A7S_DSU_CLK);
	unsigned int slow;
	unsigned int input;

	rates->dcxo = a7s_dcxo_rate();
	rates->ref = a7s_ref_pll_rate(ref_reg, rates->dcxo);

	input = a7s_main_pll_input(peri0_reg, rates->dcxo, rates->ref);
	rates->peri0_2x = a7s_main_pll_output(peri0_reg, input, 20, 7U, 27);
	rates->peri0_800 = a7s_main_pll_output(peri0_reg, input, 16, 7U, 26);
	rates->peri0_480 = a7s_main_pll_output(peri0_reg, input, 2, 7U, 25);

	input = a7s_main_pll_input(peri1_reg, rates->dcxo, rates->ref);
	rates->peri1_2x = a7s_main_pll_output(peri1_reg, input, 20, 7U, 27);
	rates->peri1_800 = a7s_main_pll_output(peri1_reg, input, 16, 7U, 26);
	rates->peri1_480 = a7s_main_pll_output(peri1_reg, input, 2, 7U, 25);

	input = a7s_main_pll_input(ddr_reg, rates->dcxo, rates->ref);
	rates->ddr = a7s_main_pll_output(ddr_reg, input, 20, 7U, 27);
	input = a7s_main_pll_input(npu_reg, rates->dcxo, rates->ref);
	rates->npu = a7s_main_pll_output(npu_reg, input, 20, 7U, 27);
	input = a7s_main_pll_input(de_reg, rates->dcxo, rates->ref);
	rates->de_3x = a7s_main_pll_output(de_reg, input, 16, 7U, 26);
	input = a7s_main_pll_input(cci_reg, rates->dcxo, rates->ref);
	rates->cci = a7s_main_pll_output(cci_reg, input, 20, 7U, 27);

	rates->cpu_back_pll = a7s_cpu_back_pll_rate(cpu_back_reg,
						       rates->dcxo);
	rates->cpu_a_pll = a7s_cpu_linear_pll_rate(cpu_a_pll_reg,
						      rates->dcxo);
	rates->cpu_b_pll = a7s_cpu_linear_pll_rate(cpu_b_pll_reg,
						      rates->dcxo);
	rates->dsu_pll = a7s_cpu_linear_pll_rate(dsu_pll_reg, rates->dcxo);
	slow = a7s_cpu_slow_rate(readl(A7S_CPU_SLOW_CLK_SEL), rates->dcxo);
	rates->cpu_a = a7s_cpu_cluster_rate(cpu_a_clk_reg, slow,
						rates->cpu_a_pll,
						rates->peri0_2x,
						rates->cpu_back_pll);
	if (!(readl(A7S_CPU_A_GATING) & 1U))
		rates->cpu_a = 0;
	rates->cpu_b = a7s_cpu_cluster_rate(cpu_b_clk_reg, slow,
						rates->cpu_b_pll,
						rates->peri0_2x,
						rates->cpu_back_pll);
	if (!(readl(A7S_CPU_B_GATING) & 1U))
		rates->cpu_b = 0;
	rates->dsu = a7s_dsu_rate(dsu_clk_reg, slow, rates->dsu_pll,
				      rates->cpu_a, rates->peri0_2x);
	if (!(readl(A7S_DSU_GATING) & 1U))
		rates->dsu = 0;
	rates->dsu_axi = a7s_div_rate(rates->dsu, (dsu_clk_reg & 3U) + 1U);
	rates->dsu_gic = a7s_div_rate(rates->dsu,
					  ((dsu_clk_reg >> 2) & 3U) + 1U);
	rates->dsu_apb = a7s_div_rate(rates->dsu,
					  ((dsu_clk_reg >> 8) & 3U) + 1U);

	rates->ahb = a7s_standard_bus_rate(readl(a7s_ccu_reg(AHB_CLK_REG)),
					       rates->dcxo,
					       rates->peri0_2x);
	rates->apb0 = a7s_standard_bus_rate(readl(a7s_ccu_reg(APB0_CLK_REG)),
						rates->dcxo,
						rates->peri0_2x);
	rates->apb1 = a7s_standard_bus_rate(readl(a7s_ccu_reg(APB1_CLK_REG)),
						rates->dcxo,
						rates->peri0_2x);
	rates->uart = a7s_uart_rate(readl(a7s_ccu_reg(APB_UART_CLK_REG)),
					rates->dcxo, rates->peri0_2x,
					rates->peri0_480);
	rates->gic = a7s_gic_rate(readl(a7s_ccu_reg(GIC_CLK_REG)), rates);
	rates->nsi = a7s_nsi_rate(readl(a7s_ccu_reg(NSI_CLK_REG)), rates);
	rates->mbus = a7s_mbus_rate(readl(a7s_ccu_reg(MBUS_CLK_REG)), rates);
}

static void a7s_print_rate(unsigned int rate)
{
	if (!rate)
		printf("off");
	else
		printf("%u.%03uM", rate / 1000U, rate % 1000U);
}

static const char *a7s_pll_lock_state(unsigned int reg)
{
	if (!(reg & A7S_PLL_LOCK_ENABLE))
		return "unchecked";
	return reg & A7S_PLL_LOCK ? "locked" : "unlocked";
}

static void a7s_print_single_pll(const char *name, unsigned int offset,
				  unsigned int rate)
{
	unsigned int reg = readl(a7s_ccu_reg(offset));

	printf("  PLL %s=", name);
	a7s_print_rate(rate);
	printf(" %s reg=%08x\n", a7s_pll_lock_state(reg), reg);
}

static void a7s_print_cpu_pll(const char *name, unsigned long address,
			      unsigned int rate)
{
	unsigned int reg = readl(address);

	printf("  CPU PLL %s=", name);
	a7s_print_rate(rate);
	printf(" %s reg=%08x\n", a7s_pll_lock_state(reg), reg);
}

static void a7s_print_main_single(const char *name, unsigned int offset,
				   unsigned int dcxo, unsigned int ref,
				   unsigned int p_shift, unsigned int p_mask)
{
	unsigned int reg = readl(a7s_ccu_reg(offset));
	unsigned int input = a7s_main_pll_input(reg, dcxo, ref);
	unsigned int rate = a7s_main_pll_output(reg, input, p_shift,
						p_mask, 27);

	a7s_print_single_pll(name, offset, rate);
}

static void a7s_print_main_dual(const char *name, unsigned int offset,
				 unsigned int dcxo, unsigned int ref)
{
	unsigned int reg = readl(a7s_ccu_reg(offset));
	unsigned int input = a7s_main_pll_input(reg, dcxo, ref);
	unsigned int output0 = a7s_main_pll_output(reg, input, 20, 7U, 27);
	unsigned int output1 = a7s_main_pll_output(reg, input, 16, 7U, 26);

	printf("  PLL %s=", name);
	a7s_print_rate(output0);
	printf("/3x=");
	a7s_print_rate(output1);
	printf(" %s reg=%08x\n", a7s_pll_lock_state(reg), reg);
}

static void a7s_print_audio_plls(unsigned int ref)
{
	unsigned int reg = readl(a7s_ccu_reg(PLL_AUDIO0_CTRL_REG));
	unsigned int rate = a7s_main_pll_output(reg, ref, 16, 0x7fU, 27);

	printf("  PLL AUDIO0=");
	a7s_print_rate(rate);
	printf(" %s reg=%08x\n", a7s_pll_lock_state(reg), reg);

	reg = readl(a7s_ccu_reg(A7S_AUDIO1_PLL_CTRL));
	printf("  PLL AUDIO1=");
	a7s_print_rate(a7s_main_pll_output(reg, ref, 20, 7U, 27));
	printf("/div5=");
	a7s_print_rate(a7s_main_pll_output(reg, ref, 16, 7U, 27));
	printf(" %s reg=%08x\n", a7s_pll_lock_state(reg), reg);
}

void a7s_clock_dump(void)
{
	struct a7s_clock_rates rates;
	unsigned int ref_reg;
	unsigned int peri_reg;
	unsigned int cpu_a_clk;
	unsigned int cpu_b_clk;
	unsigned int dsu_clk;

	a7s_clock_read_rates(&rates);
	ref_reg = readl(a7s_ccu_reg(PLL_REF_CTRL_REG));
	cpu_a_clk = readl(A7S_CPU_A_CLK);
	cpu_b_clk = readl(A7S_CPU_B_CLK);
	dsu_clk = readl(A7S_DSU_CLK);

	printf("A7S clock dump: read-only nominal rates\n");
	printf("  OSC DCXO=");
	a7s_print_rate(rates.dcxo);
	printf(" REF=");
	a7s_print_rate(rates.ref);
	printf(" %s reg=%08x\n", a7s_pll_lock_state(ref_reg), ref_reg);

	a7s_print_cpu_pll("BACK", A7S_CPU_BACK_PLL_CTRL,
			  rates.cpu_back_pll);
	a7s_print_cpu_pll("A", A7S_CPU_A_PLL_CTRL, rates.cpu_a_pll);
	a7s_print_cpu_pll("B", A7S_CPU_B_PLL_CTRL, rates.cpu_b_pll);
	a7s_print_cpu_pll("DSU", A7S_DSU_PLL_CTRL, rates.dsu_pll);
	printf("  CPU CLK A=");
	a7s_print_rate(rates.cpu_a);
	printf("(src=%u reg=%08x) B=", (cpu_a_clk >> 24) & 7U, cpu_a_clk);
	a7s_print_rate(rates.cpu_b);
	printf("(src=%u reg=%08x)\n", (cpu_b_clk >> 24) & 7U, cpu_b_clk);
	printf("  DSU CLK core=");
	a7s_print_rate(rates.dsu);
	printf(" AXI=");
	a7s_print_rate(rates.dsu_axi);
	printf(" APB=");
	a7s_print_rate(rates.dsu_apb);
	printf(" GIC=");
	a7s_print_rate(rates.dsu_gic);
	printf(" src=%u reg=%08x\n", (dsu_clk >> 24) & 7U, dsu_clk);

	a7s_print_single_pll("DDR", PLL_DDR_CTRL_REG, rates.ddr);
	peri_reg = readl(a7s_ccu_reg(PLL_PERI0_CTRL_REG));
	printf("  PLL PERI0 2x=");
	a7s_print_rate(rates.peri0_2x);
	printf(" 800=");
	a7s_print_rate(rates.peri0_800);
	printf(" 480=");
	a7s_print_rate(rates.peri0_480);
	printf(" %s reg=%08x\n", a7s_pll_lock_state(peri_reg), peri_reg);
	peri_reg = readl(a7s_ccu_reg(PLL_PERI1_CTRL_REG));
	printf("  PLL PERI1 2x=");
	a7s_print_rate(rates.peri1_2x);
	printf(" 800=");
	a7s_print_rate(rates.peri1_800);
	printf(" 480=");
	a7s_print_rate(rates.peri1_480);
	printf(" %s reg=%08x\n", a7s_pll_lock_state(peri_reg), peri_reg);

	a7s_print_main_single("GPU0", PLL_GPU0_CTRL_REG, rates.dcxo,
			      rates.ref, 20, 7U);
	a7s_print_main_dual("VIDEO0", PLL_VIDEO0_CTRL_REG, rates.dcxo,
			    rates.ref);
	a7s_print_main_dual("VIDEO1", PLL_VIDEO1_CTRL_REG, rates.dcxo,
			    rates.ref);
	a7s_print_main_dual("VIDEO2", PLL_VIDEO2_CTRL_REG, rates.dcxo,
			    rates.ref);
	a7s_print_main_single("VE0", PLL_VE0_CTRL_REG, rates.dcxo,
			      rates.ref, 20, 7U);
	a7s_print_main_single("VE1", PLL_VE1_CTRL_REG, rates.dcxo,
			      rates.ref, 20, 7U);
	a7s_print_audio_plls(rates.ref);
	a7s_print_single_pll("NPU", PLL_NPU_CTRL_REG, rates.npu);
	a7s_print_main_dual("DE", PLL_DE_CTRL_REG, rates.dcxo, rates.ref);
	a7s_print_single_pll("CCI", PLL_CCI_CTRL_REG, rates.cci);

	printf("  BUS AHB=");
	a7s_print_rate(rates.ahb);
	printf(" APB0=");
	a7s_print_rate(rates.apb0);
	printf(" APB1=");
	a7s_print_rate(rates.apb1);
	printf(" UART=");
	a7s_print_rate(rates.uart);
	printf("\n  FABRIC GIC=");
	a7s_print_rate(rates.gic);
	printf(" NSI=");
	a7s_print_rate(rates.nsi);
	printf(" MBUS=");
	a7s_print_rate(rates.mbus);
	printf("\n");
}
