#ifndef _A7S_AXP8191_VOLTAGE_H_
#define _A7S_AXP8191_VOLTAGE_H_

enum a7s_axp8191_voltage_rail {
	A7S_AXP8191_DCDC6,
	A7S_AXP8191_DCDC7,
	A7S_AXP8191_DCDC8,
	A7S_AXP8191_ELDO,
	A7S_AXP8191_VOLTAGE_RAIL_COUNT,
};

int a7s_axp8191_voltage_selector(enum a7s_axp8191_voltage_rail rail,
				 unsigned int millivolts,
				 unsigned int *selector);

#endif
