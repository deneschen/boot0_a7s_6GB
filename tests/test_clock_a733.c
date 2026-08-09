#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <clock_a733.h>
#include <clock_diag_a733.h>
#include <arch/sun60iw2p1/cpu_autogen.h>
#include <arch/sun60iw2p1/clock_autogen.h>

#define BIT(n)			(1U << (n))
#define CCU_WORDS		(0x2000U / sizeof(uint32_t))
#define CPU_PLL_WORDS		(0x4000U / sizeof(uint32_t))
#define MAX_WRITES		128U
#define RTC_XO_CTRL0		(SUNXI_RTC_BASE + 0x160U)
#define PLL_STATUS		(BIT(31) | BIT(30) | BIT(29) | BIT(28))
#define REF_STATUS		(PLL_STATUS | BIT(27) | BIT(24))
#define PERI_STATUS		(PLL_STATUS | BIT(27) | BIT(26) | BIT(25))

struct write_event {
	unsigned long address;
	uint32_t value;
};

static uint32_t ccu[CCU_WORDS];
static uint32_t cpu_pll[CPU_PLL_WORDS];
static uint32_t rtc_xo_ctrl0;
static unsigned long lock_failure_address;
static unsigned long total_delay_us;
static struct write_event writes[MAX_WRITES];
static unsigned int write_count;
static unsigned int clock_printf_tracking;
static unsigned int clock_printf_calls;
static unsigned int clock_printf_bad_lines;

int test_printf(const char *format, ...)
{
	const char *cursor;
	unsigned int newlines = 0;
	va_list args;
	int result;

	if (clock_printf_tracking) {
		clock_printf_calls++;
		for (cursor = format; *cursor; cursor++) {
			if (*cursor == '\n')
				newlines++;
		}
		if (newlines != 1U || cursor == format || cursor[-1] != '\n')
			clock_printf_bad_lines++;
	}

	va_start(args, format);
	result = vprintf(format, args);
	va_end(args);
	return result;
}

static int is_pll(unsigned long address)
{
	return address == SUNXI_CCMU_BASE + PLL_REF_CTRL_REG ||
	       address == SUNXI_CCMU_BASE + PLL_PERI0_CTRL_REG ||
	       address == SUNXI_CCMU_BASE + PLL_PERI1_CTRL_REG;
}

uint32_t test_readl(unsigned long address)
{
	if (address == RTC_XO_CTRL0)
		return rtc_xo_ctrl0;
	if (address >= SUNXI_CPU_PLL_CFG_BASE &&
	    address < SUNXI_CPU_PLL_CFG_BASE + sizeof(cpu_pll))
		return cpu_pll[(address - SUNXI_CPU_PLL_CFG_BASE) /
			       sizeof(uint32_t)];
	assert(address >= SUNXI_CCMU_BASE);
	assert(address < SUNXI_CCMU_BASE + sizeof(ccu));
	return ccu[(address - SUNXI_CCMU_BASE) / sizeof(uint32_t)];
}

void test_writel(uint32_t value, unsigned long address)
{
	assert(address >= SUNXI_CCMU_BASE);
	assert(address < SUNXI_CCMU_BASE + sizeof(ccu));
	if (is_pll(address)) {
		value &= ~BIT(28);
		if (address != lock_failure_address &&
		    (value & (BIT(31) | BIT(29))) == (BIT(31) | BIT(29)))
			value |= BIT(28);
	}
	ccu[(address - SUNXI_CCMU_BASE) / sizeof(uint32_t)] = value;
	if (write_count < MAX_WRITES) {
		writes[write_count].address = address;
		writes[write_count].value = value;
		write_count++;
	}
}

void udelay(unsigned long usec)
{
	total_delay_us += usec;
}

static uint32_t reg_value(unsigned int offset)
{
	return ccu[offset / sizeof(uint32_t)];
}

static void reset_fixture(unsigned int dcxo_status)
{
	memset(ccu, 0, sizeof(ccu));
	memset(cpu_pll, 0, sizeof(cpu_pll));
	memset(writes, 0, sizeof(writes));
	rtc_xo_ctrl0 = dcxo_status << 14;
	lock_failure_address = 0;
	total_delay_us = 0;
	write_count = 0;

	/* Exercise the required high-to-low transition before PLL changes. */
	ccu[AHB_CLK_REG / 4] = (3U << 24) | 2U;
	ccu[APB0_CLK_REG / 4] = (3U << 24) | 5U;
	ccu[APB1_CLK_REG / 4] = (3U << 24) | 5U;
	ccu[APB_UART_CLK_REG / 4] = (4U << 24) | 19U;
}

static void assert_bus_rates(void)
{
	assert((reg_value(AHB_CLK_REG) & 0x0300001fU) == 0x03000002U);
	assert((reg_value(APB0_CLK_REG) & 0x0300001fU) == 0x03000005U);
	assert((reg_value(APB1_CLK_REG) & 0x0300001fU) == 0x03000005U);
	assert((reg_value(APB_UART_CLK_REG) & 0x0700001fU) == 0x04000013U);
}

static unsigned int collect_writes(unsigned long address,
				   struct write_event *events,
				   unsigned int capacity)
{
	unsigned int count = 0;
	unsigned int index;

	assert(write_count < MAX_WRITES);
	for (index = 0; index < write_count; index++) {
		if (writes[index].address != address)
			continue;
		assert(count < capacity);
		events[count++] = writes[index];
	}
	return count;
}

static void assert_programming_order(void)
{
	struct write_event events[8];
	unsigned int count;

	count = collect_writes(SUNXI_CCMU_BASE + AHB_CLK_REG,
			       events, sizeof(events) / sizeof(events[0]));
	assert(count == 4);
	assert((events[0].value & 0x0300001fU) == 2U);
	assert((events[1].value & 0x0300001fU) == 0U);
	assert((events[2].value & 0x0300001fU) == 2U);
	assert((events[3].value & 0x0300001fU) == 0x03000002U);

	count = collect_writes(SUNXI_CCMU_BASE + PLL_REF_CTRL_REG,
			       events, sizeof(events) / sizeof(events[0]));
	assert(count == 5);
	assert((events[0].value & (BIT(31) | BIT(27) | BIT(24) | BIT(30))) ==
	       (BIT(24) | BIT(30)));
	assert((events[1].value & (BIT(31) | BIT(27) | BIT(24) | BIT(30))) ==
	       (BIT(27) | BIT(24) | BIT(30)));
	assert((events[2].value & BIT(31)) == 0);
	assert((events[3].value & BIT(31)) != 0);
	assert((events[4].value & BIT(29)) != 0);
}

static void test_dcxo_configuration(unsigned int status, unsigned int n,
				    unsigned int output_div)
{
	uint32_t ref;

	reset_fixture(status);
	assert(a7s_clock_init() == A7S_CLOCK_OK);
	ref = reg_value(PLL_REF_CTRL_REG);
	assert((ref & REF_STATUS) == REF_STATUS);
	assert(((ref >> 8) & 0xffU) == n - 1U);
	assert(((ref >> 16) & 0x7fU) == output_div - 1U);
	assert((reg_value(PLL_PERI0_CTRL_REG) & PERI_STATUS) == PERI_STATUS);
	assert((reg_value(PLL_PERI1_CTRL_REG) & PERI_STATUS) == PERI_STATUS);
	assert(((reg_value(PLL_PERI0_CTRL_REG) >> 8) & 0xffU) == 99U);
	assert(((reg_value(PLL_PERI1_CTRL_REG) >> 8) & 0xffU) == 103U);
	assert((reg_value(PERI0PLL_GATE_EN_REG) & 0x8fff0fffU) == 0x8fff0fffU);
	assert((reg_value(PERI1PLL_GATE_EN_REG) & 0x8fff0fffU) == 0x8fff0fffU);
	assert_bus_rates();
	assert(total_delay_us >= 60U);
	assert_programming_order();
	a7s_clock_reset();
	assert((reg_value(AHB_CLK_REG) & 0x0300001fU) == 0);
	assert((reg_value(APB0_CLK_REG) & 0x0300001fU) == 0);
	assert((reg_value(APB1_CLK_REG) & 0x0300001fU) == 0);
	assert((reg_value(APB_UART_CLK_REG) & 0x0700001fU) == 0);
}

static void test_stable_ref_pll_is_preserved(void)
{
	uint32_t original = REF_STATUS | (90U << 16) | (88U << 8);

	reset_fixture(2);
	ccu[PLL_REF_CTRL_REG / 4] = original;
	assert(a7s_clock_init() == A7S_CLOCK_OK);
	assert(reg_value(PLL_REF_CTRL_REG) == original);
}

static void test_peri0_timeout_keeps_safe_buses(void)
{
	reset_fixture(2);
	lock_failure_address = SUNXI_CCMU_BASE + PLL_PERI0_CTRL_REG;
	assert(a7s_clock_init() == A7S_CLOCK_ERR_PERI0_PLL);
	assert((reg_value(AHB_CLK_REG) & 0x0300001fU) == 0);
	assert((reg_value(APB0_CLK_REG) & 0x0300001fU) == 0);
	assert((reg_value(APB1_CLK_REG) & 0x0300001fU) == 0);
	assert((reg_value(APB_UART_CLK_REG) & 0x0700001fU) == 0);
	assert((reg_value(PLL_PERI0_CTRL_REG) &
		(BIT(27) | BIT(26) | BIT(25))) == 0);
}

static void test_read_only_clock_rates(void)
{
	struct a7s_clock_rates rates;
	unsigned int writes_before;

	reset_fixture(2);
	assert(a7s_clock_init() == A7S_CLOCK_OK);

	ccu[PLL_DDR_CTRL_REG / 4] = PLL_STATUS | BIT(27) | BIT(24) |
					(1U << 20) | (99U << 8);
	ccu[PLL_NPU_CTRL_REG / 4] = PLL_STATUS | BIT(27) | BIT(24) |
					(1U << 20) | (69U << 8);
	ccu[PLL_DE_CTRL_REG / 4] = PLL_STATUS | BIT(27) | BIT(26) |
				       BIT(24) | (1U << 20) | (2U << 16) |
				       (86U << 8);
	ccu[PLL_CCI_CTRL_REG / 4] = PLL_STATUS | BIT(27) | BIT(24) |
					(1U << 20) | (49U << 8);
	ccu[GIC_CLK_REG / 4] = BIT(31) | (2U << 24) | 2U;
	ccu[NSI_CLK_REG / 4] = BIT(31) | (1U << 24) | 1U;
	ccu[MBUS_CLK_REG / 4] = BIT(31) | 2U;

	cpu_pll[0x0000U / 4] = PLL_STATUS | BIT(27) |
				      (1U << 20) | (74U << 8);
	cpu_pll[0x1000U / 4] = PLL_STATUS | BIT(27) | (40U << 8);
	cpu_pll[0x2000U / 4] = PLL_STATUS | BIT(27) | (48U << 8);
	cpu_pll[0x3000U / 4] = PLL_STATUS | BIT(27) | (36U << 8);
	cpu_pll[0x101cU / 4] = 3U << 24;
	cpu_pll[0x1020U / 4] = 1U;
	cpu_pll[0x201cU / 4] = 4U << 24;
	cpu_pll[0x2020U / 4] = 1U;
	cpu_pll[0x301cU / 4] = (5U << 24) | (1U << 8) | (1U << 2) | 1U;
	cpu_pll[0x3020U / 4] = 1U;

	writes_before = write_count;
	a7s_clock_read_rates(&rates);
	assert(write_count == writes_before);
	assert(rates.dcxo == 26000U);
	assert(rates.ref == 24000U);
	assert(rates.cpu_back_pll == 975000U);
	assert(rates.cpu_a_pll == 1040000U);
	assert(rates.cpu_b_pll == 1248000U);
	assert(rates.dsu_pll == 936000U);
	assert(rates.cpu_a == 1040000U);
	assert(rates.cpu_b == 600000U);
	assert(rates.dsu == 600000U);
	assert(rates.dsu_axi == 300000U);
	assert(rates.dsu_apb == 300000U);
	assert(rates.dsu_gic == 300000U);
	assert(rates.ddr == 1200000U);
	assert(rates.peri0_2x == 1200000U);
	assert(rates.peri0_800 == 800000U);
	assert(rates.peri0_480 == 480000U);
	assert(rates.npu == 840000U);
	assert(rates.de_3x == 696000U);
	assert(rates.cci == 600000U);
	assert(rates.ahb == 200000U);
	assert(rates.apb0 == 100000U);
	assert(rates.apb1 == 100000U);
	assert(rates.uart == 24000U);
	assert(rates.gic == 200000U);
	assert(rates.nsi == 400000U);
	assert(rates.mbus == 200000U);

	clock_printf_calls = 0;
	clock_printf_bad_lines = 0;
	clock_printf_tracking = 1;
	a7s_clock_dump();
	clock_printf_tracking = 0;
	assert(clock_printf_calls != 0U);
	assert(clock_printf_bad_lines == 0U);
	assert(write_count == writes_before);
}

int main(void)
{
	test_dcxo_configuration(0, 100, 100);
	test_dcxo_configuration(1, 100, 80);
	test_dcxo_configuration(2, 96, 104);
	test_dcxo_configuration(3, 100, 100);
	test_stable_ref_pll_is_preserved();
	test_peri0_timeout_keeps_safe_buses();
	test_read_only_clock_rates();
	puts("A733 clock tests passed");
	return 0;
}
