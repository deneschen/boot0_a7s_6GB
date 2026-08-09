/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <common.h>
#include <clock_a733.h>
#include <arch/sun60iw2p1/cpu_autogen.h>
#include <arch/sun60iw2p1/clock_autogen.h>

#define A7S_BIT(n)			(1U << (n))
#define A7S_PLL_ENABLE			A7S_BIT(31)
#define A7S_PLL_LDO_ENABLE		A7S_BIT(30)
#define A7S_PLL_LOCK_ENABLE		A7S_BIT(29)
#define A7S_PLL_LOCK			A7S_BIT(28)
#define A7S_REF_OUTPUT_GATE		A7S_BIT(27)
#define A7S_REF_REG_ENABLE		A7S_BIT(24)
#define A7S_PERI_OUTPUT_GATES		(A7S_BIT(27) | A7S_BIT(26) | A7S_BIT(25))
#define A7S_PERI_REF_PLL		A7S_BIT(24)

#define A7S_RTC_XO_CTRL0		(SUNXI_RTC_BASE + 0x160U)
#define A7S_RTC_DCXO_STATUS_MASK	(3U << 14)
#define A7S_RTC_DCXO_STATUS_SHIFT	14

#define A7S_PLL_LOCK_TIMEOUT_US		1000U
#define A7S_PLL_LOCK_STABLE_READS	3U

#define A7S_CLOCK_SOURCE_SHIFT		24
#define A7S_AHB_APB_SOURCE_MASK		(3U << A7S_CLOCK_SOURCE_SHIFT)
#define A7S_UART_SOURCE_MASK		(7U << A7S_CLOCK_SOURCE_SHIFT)
#define A7S_BUS_DIV_MASK		0x1fU

#define A7S_AHB_SOURCE_PERI0_600	(3U << A7S_CLOCK_SOURCE_SHIFT)
#define A7S_APB_SOURCE_PERI0_600	(3U << A7S_CLOCK_SOURCE_SHIFT)
#define A7S_UART_SOURCE_PERI0_480	(4U << A7S_CLOCK_SOURCE_SHIFT)

#define A7S_AHB_DIV_3			(3U - 1U)
#define A7S_APB_DIV_6			(6U - 1U)
#define A7S_UART_DIV_20			(20U - 1U)

/* All documented SW and no-auto gate bits; reserved bits stay untouched. */
#define A7S_PERI_DISTRIBUTION_GATES	0x8fff0fffU

struct a7s_pll_config {
	unsigned long reg;
	unsigned long pattern_reg;
	unsigned int config_mask;
	unsigned int config_value;
	unsigned int output_mask;
	unsigned int supply_mask;
};

static unsigned long a7s_ccu_reg(unsigned int offset)
{
	return SUNXI_CCMU_BASE + offset;
}

static int a7s_pll_wait_stable(unsigned long reg)
{
	unsigned int stable = 0;
	unsigned int elapsed;

	/* The manual requires the first sample after 3 us and three good reads. */
	udelay(3);
	for (elapsed = 0; elapsed < A7S_PLL_LOCK_TIMEOUT_US; elapsed++) {
		if (readl(reg) & A7S_PLL_LOCK) {
			if (++stable == A7S_PLL_LOCK_STABLE_READS)
				return 0;
		} else {
			stable = 0;
		}
		udelay(1);
	}

	return -1;
}

static int a7s_pll_is_running(const struct a7s_pll_config *pll,
			      int check_config)
{
	unsigned int required = A7S_PLL_ENABLE | A7S_PLL_LDO_ENABLE |
				A7S_PLL_LOCK_ENABLE | pll->output_mask |
				pll->supply_mask;
	unsigned int value = readl(pll->reg);

	if ((value & required) != required)
		return 0;
	if (check_config &&
	    (value & pll->config_mask) != pll->config_value)
		return 0;
	if (pll->pattern_reg && (readl(pll->pattern_reg) & A7S_BIT(31)))
		return 0;

	return a7s_pll_wait_stable(pll->reg) == 0;
}

static int a7s_pll_program(const struct a7s_pll_config *pll)
{
	unsigned int value = readl(pll->reg);
	int was_enabled = !!(value & A7S_PLL_ENABLE);

	if (a7s_pll_is_running(pll, 1))
		return 0;

	if (pll->pattern_reg) {
		value = readl(pll->pattern_reg);
		writel(value & ~A7S_BIT(31), pll->pattern_reg);
	}

	value = readl(pll->reg);
	if (was_enabled) {
		/* Frequency-modulation sequence: isolate consumers first. */
		writel(value & ~pll->output_mask, pll->reg);
		value = readl(pll->reg) & ~A7S_PLL_LOCK_ENABLE;
		writel(value, pll->reg);
	} else {
		/* Startup method 1 from the A733 manual. */
		value |= A7S_PLL_LDO_ENABLE | pll->supply_mask;
		writel(value, pll->reg);
		value |= pll->output_mask;
		writel(value, pll->reg);
	}

	value = readl(pll->reg);
	value &= ~pll->config_mask;
	value |= pll->config_value | A7S_PLL_LDO_ENABLE | pll->supply_mask;
	writel(value, pll->reg);

	value = readl(pll->reg) | A7S_PLL_ENABLE;
	writel(value, pll->reg);
	value |= A7S_PLL_LOCK_ENABLE;
	writel(value, pll->reg);

	if (a7s_pll_wait_stable(pll->reg)) {
		value = readl(pll->reg) & ~pll->output_mask;
		writel(value, pll->reg);
		return -1;
	}

	udelay(20);
	if (was_enabled) {
		value = readl(pll->reg) | pll->output_mask;
		writel(value, pll->reg);
	}

	return 0;
}

static unsigned int a7s_read_dcxo_rate(void)
{
	unsigned int first;
	unsigned int second;
	unsigned int third;
	unsigned int attempt;

	for (attempt = 0; attempt < 8; attempt++) {
		first = readl(A7S_RTC_XO_CTRL0) & A7S_RTC_DCXO_STATUS_MASK;
		udelay(1);
		second = readl(A7S_RTC_XO_CTRL0) & A7S_RTC_DCXO_STATUS_MASK;
		udelay(1);
		third = readl(A7S_RTC_XO_CTRL0) & A7S_RTC_DCXO_STATUS_MASK;
		if (first == second && second == third)
			break;
	}
	if (attempt == 8)
		return 0;

	switch (first >> A7S_RTC_DCXO_STATUS_SHIFT) {
	case 1:
		return 19200000U;
	case 2:
		return 26000000U;
	default:
		return 24000000U;
	}
}

static int a7s_ref_pll_init(void)
{
	struct a7s_pll_config pll = {
		.reg = a7s_ccu_reg(PLL_REF_CTRL_REG),
		.pattern_reg = 0,
		.config_mask = (0x7fU << 16) | (0xffU << 8) | A7S_BIT(1),
		.output_mask = A7S_REF_OUTPUT_GATE,
		.supply_mask = A7S_REF_REG_ENABLE,
	};
	unsigned int dcxo_rate;
	unsigned int n;
	unsigned int output_div;

	/* REFPLL is architecturally fixed at 24 MHz; preserve a stable one. */
	if (a7s_pll_is_running(&pll, 0))
		return 0;

	dcxo_rate = a7s_read_dcxo_rate();
	switch (dcxo_rate) {
	case 19200000U:
		n = 100;
		output_div = 80;
		break;
	case 24000000U:
		n = 100;
		output_div = 100;
		break;
	case 26000000U:
		n = 96;
		output_div = 104;
		break;
	default:
		return -1;
	}

	pll.config_value = ((output_div - 1U) << 16) | ((n - 1U) << 8);
	return a7s_pll_program(&pll);
}

static int a7s_peri_pll_init(unsigned int offset, unsigned int n)
{
	struct a7s_pll_config pll = {
		.reg = a7s_ccu_reg(offset),
		.pattern_reg = a7s_ccu_reg(offset + 8U),
		.config_mask = A7S_PERI_REF_PLL | (7U << 20) | (7U << 16) |
			       (0xffU << 8) | (7U << 2) | A7S_BIT(1),
		.config_value = A7S_PERI_REF_PLL | (1U << 20) | (2U << 16) |
				 ((n - 1U) << 8) | (4U << 2),
		.output_mask = A7S_PERI_OUTPUT_GATES,
		.supply_mask = 0,
	};

	return a7s_pll_program(&pll);
}

static void a7s_bus_switch_to_sys24(unsigned int offset,
				    unsigned int source_mask)
{
	unsigned long reg = a7s_ccu_reg(offset);
	unsigned int value = readl(reg);

	/* Higher to lower: source first, divider second. */
	value &= ~source_mask;
	writel(value, reg);
	value = readl(reg) & ~A7S_BUS_DIV_MASK;
	writel(value, reg);
}

static int a7s_bus_use_sys24(void)
{
	a7s_bus_switch_to_sys24(AHB_CLK_REG, A7S_AHB_APB_SOURCE_MASK);
	a7s_bus_switch_to_sys24(APB0_CLK_REG, A7S_AHB_APB_SOURCE_MASK);
	a7s_bus_switch_to_sys24(APB1_CLK_REG, A7S_AHB_APB_SOURCE_MASK);
	a7s_bus_switch_to_sys24(APB_UART_CLK_REG, A7S_UART_SOURCE_MASK);

	if (readl(a7s_ccu_reg(AHB_CLK_REG)) &
	    (A7S_AHB_APB_SOURCE_MASK | A7S_BUS_DIV_MASK))
		return -1;
	if (readl(a7s_ccu_reg(APB0_CLK_REG)) &
	    (A7S_AHB_APB_SOURCE_MASK | A7S_BUS_DIV_MASK))
		return -1;
	if (readl(a7s_ccu_reg(APB1_CLK_REG)) &
	    (A7S_AHB_APB_SOURCE_MASK | A7S_BUS_DIV_MASK))
		return -1;
	if (readl(a7s_ccu_reg(APB_UART_CLK_REG)) &
	    (A7S_UART_SOURCE_MASK | A7S_BUS_DIV_MASK))
		return -1;

	return 0;
}

static void a7s_bus_switch_up(unsigned int offset, unsigned int source_mask,
			      unsigned int source, unsigned int divider)
{
	unsigned long reg = a7s_ccu_reg(offset);
	unsigned int value = readl(reg);

	/* Lower to higher: divider first, source second. */
	value &= ~A7S_BUS_DIV_MASK;
	value |= divider;
	writel(value, reg);
	value = readl(reg) & ~source_mask;
	value |= source;
	writel(value, reg);
}

static int a7s_bus_use_nominal_rates(void)
{
	a7s_bus_switch_up(AHB_CLK_REG, A7S_AHB_APB_SOURCE_MASK,
			  A7S_AHB_SOURCE_PERI0_600, A7S_AHB_DIV_3);
	a7s_bus_switch_up(APB0_CLK_REG, A7S_AHB_APB_SOURCE_MASK,
			  A7S_APB_SOURCE_PERI0_600, A7S_APB_DIV_6);
	a7s_bus_switch_up(APB1_CLK_REG, A7S_AHB_APB_SOURCE_MASK,
			  A7S_APB_SOURCE_PERI0_600, A7S_APB_DIV_6);
	a7s_bus_switch_up(APB_UART_CLK_REG, A7S_UART_SOURCE_MASK,
			  A7S_UART_SOURCE_PERI0_480, A7S_UART_DIV_20);

	if ((readl(a7s_ccu_reg(AHB_CLK_REG)) &
	     (A7S_AHB_APB_SOURCE_MASK | A7S_BUS_DIV_MASK)) !=
	    (A7S_AHB_SOURCE_PERI0_600 | A7S_AHB_DIV_3))
		return -1;
	if ((readl(a7s_ccu_reg(APB0_CLK_REG)) &
	     (A7S_AHB_APB_SOURCE_MASK | A7S_BUS_DIV_MASK)) !=
	    (A7S_APB_SOURCE_PERI0_600 | A7S_APB_DIV_6))
		return -1;
	if ((readl(a7s_ccu_reg(APB1_CLK_REG)) &
	     (A7S_AHB_APB_SOURCE_MASK | A7S_BUS_DIV_MASK)) !=
	    (A7S_APB_SOURCE_PERI0_600 | A7S_APB_DIV_6))
		return -1;
	if ((readl(a7s_ccu_reg(APB_UART_CLK_REG)) &
	     (A7S_UART_SOURCE_MASK | A7S_BUS_DIV_MASK)) !=
	    (A7S_UART_SOURCE_PERI0_480 | A7S_UART_DIV_20))
		return -1;

	return 0;
}

static int a7s_peri_distribution_enable(unsigned int offset)
{
	unsigned long reg = a7s_ccu_reg(offset);
	unsigned int value = readl(reg) | A7S_PERI_DISTRIBUTION_GATES;

	writel(value, reg);
	return (readl(reg) & A7S_PERI_DISTRIBUTION_GATES) ==
	       A7S_PERI_DISTRIBUTION_GATES ? 0 : -1;
}

void a7s_clock_reset(void)
{
	(void)a7s_bus_use_sys24();
}

int a7s_clock_init(void)
{
	if (a7s_bus_use_sys24())
		return A7S_CLOCK_ERR_BUS;
	if (a7s_ref_pll_init())
		return A7S_CLOCK_ERR_REF_PLL;
	if (a7s_peri_pll_init(PLL_PERI0_CTRL_REG, 100))
		return A7S_CLOCK_ERR_PERI0_PLL;
	if (a7s_peri_pll_init(PLL_PERI1_CTRL_REG, 104)) {
		a7s_bus_use_sys24();
		return A7S_CLOCK_ERR_PERI1_PLL;
	}
	if (a7s_peri_distribution_enable(PERI0PLL_GATE_EN_REG) ||
	    a7s_peri_distribution_enable(PERI1PLL_GATE_EN_REG)) {
		a7s_bus_use_sys24();
		return A7S_CLOCK_ERR_BUS;
	}
	if (a7s_bus_use_nominal_rates()) {
		a7s_bus_use_sys24();
		return A7S_CLOCK_ERR_BUS;
	}

	return A7S_CLOCK_OK;
}
