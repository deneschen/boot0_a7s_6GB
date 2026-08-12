#include <assert.h>
#include <stdio.h>

#include <axp8191_voltage.h>

static void expect_selector(enum a7s_axp8191_voltage_rail rail,
			    unsigned int millivolts,
			    unsigned int expected)
{
	unsigned int selector = ~0U;

	assert(a7s_axp8191_voltage_selector(rail, millivolts, &selector) == 0);
	assert(selector == expected);
}

static void test_dcdc6(void)
{
	expect_selector(A7S_AXP8191_DCDC6, 0, 0x00);
	expect_selector(A7S_AXP8191_DCDC6, 500, 0x00);
	expect_selector(A7S_AXP8191_DCDC6, 1200, 0x46);
	expect_selector(A7S_AXP8191_DCDC6, 1201, 0x47);
	expect_selector(A7S_AXP8191_DCDC6, 1220, 0x47);
	expect_selector(A7S_AXP8191_DCDC6, 1540, 0x57);
	expect_selector(A7S_AXP8191_DCDC6, 1541, 0x58);
	expect_selector(A7S_AXP8191_DCDC6, 1800, 0x58);
	expect_selector(A7S_AXP8191_DCDC6, 2100, 0x67);
	expect_selector(A7S_AXP8191_DCDC6, 2101, 0x68);
	expect_selector(A7S_AXP8191_DCDC6, 2440, 0x68);
	expect_selector(A7S_AXP8191_DCDC6, 2441, 0x69);
	expect_selector(A7S_AXP8191_DCDC6, 2760, 0x70);
	expect_selector(A7S_AXP8191_DCDC6, 5000, 0x70);
}

static void test_dcdc7(void)
{
	expect_selector(A7S_AXP8191_DCDC7, 1200, 0x46);
	expect_selector(A7S_AXP8191_DCDC7, 1201, 0x47);
	expect_selector(A7S_AXP8191_DCDC7, 1540, 0x57);
	expect_selector(A7S_AXP8191_DCDC7, 1840, 0x57);
}

static void test_dcdc8(void)
{
	expect_selector(A7S_AXP8191_DCDC8, 1540, 0x57);
	expect_selector(A7S_AXP8191_DCDC8, 1541, 0x58);
	expect_selector(A7S_AXP8191_DCDC8, 1900, 0x58);
	expect_selector(A7S_AXP8191_DCDC8, 1901, 0x59);
	expect_selector(A7S_AXP8191_DCDC8, 3400, 0x67);
	expect_selector(A7S_AXP8191_DCDC8, 4000, 0x67);
}

static void test_eldo(void)
{
	expect_selector(A7S_AXP8191_ELDO, 500, 0x00);
	expect_selector(A7S_AXP8191_ELDO, 501, 0x01);
	expect_selector(A7S_AXP8191_ELDO, 800, 0x0c);
	expect_selector(A7S_AXP8191_ELDO, 1500, 0x28);
}

int main(void)
{
	unsigned int selector;

	test_dcdc6();
	test_dcdc7();
	test_dcdc8();
	test_eldo();
	assert(a7s_axp8191_voltage_selector(A7S_AXP8191_VOLTAGE_RAIL_COUNT,
					     1000, &selector) == -1);
	assert(a7s_axp8191_voltage_selector(A7S_AXP8191_DCDC6, 1000, NULL) ==
	       -1);
	puts("AXP8191 voltage selector tests: OK");
	return 0;
}
