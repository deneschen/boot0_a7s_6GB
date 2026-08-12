#include <axp8191_voltage.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

struct a7s_axp8191_linear_range {
	unsigned int min_mv;
	unsigned int min_selector;
	unsigned int max_selector;
	unsigned int step_mv;
};

struct a7s_axp8191_voltage_desc {
	const struct a7s_axp8191_linear_range *ranges;
	unsigned int range_count;
};

/* Keep these selector ranges in sync with the A733 Linux AXP8191 driver. */
static const struct a7s_axp8191_linear_range dcdc6_ranges[] = {
	{ 500,  0x00, 0x46, 10 },
	{ 1220, 0x47, 0x57, 20 },
	{ 1800, 0x58, 0x67, 20 },
	{ 2440, 0x68, 0x70, 40 },
};

static const struct a7s_axp8191_linear_range dcdc7_ranges[] = {
	{ 500,  0x00, 0x46, 10 },
	{ 1220, 0x47, 0x57, 20 },
};

static const struct a7s_axp8191_linear_range dcdc8_ranges[] = {
	{ 500,  0x00, 0x46, 10 },
	{ 1220, 0x47, 0x57, 20 },
	{ 1900, 0x58, 0x67, 100 },
};

static const struct a7s_axp8191_linear_range eldo_ranges[] = {
	{ 500, 0x00, 0x28, 25 },
};

static const struct a7s_axp8191_voltage_desc voltage_descs[] = {
	[A7S_AXP8191_DCDC6] = { dcdc6_ranges, ARRAY_SIZE(dcdc6_ranges) },
	[A7S_AXP8191_DCDC7] = { dcdc7_ranges, ARRAY_SIZE(dcdc7_ranges) },
	[A7S_AXP8191_DCDC8] = { dcdc8_ranges, ARRAY_SIZE(dcdc8_ranges) },
	[A7S_AXP8191_ELDO] = { eldo_ranges, ARRAY_SIZE(eldo_ranges) },
};

static unsigned int range_max_mv(const struct a7s_axp8191_linear_range *range)
{
	return range->min_mv +
	       (range->max_selector - range->min_selector) * range->step_mv;
}

int a7s_axp8191_voltage_selector(enum a7s_axp8191_voltage_rail rail,
				 unsigned int millivolts,
				 unsigned int *selector)
{
	const struct a7s_axp8191_voltage_desc *desc;
	const struct a7s_axp8191_linear_range *range;
	unsigned int max_mv;
	unsigned int index;
	unsigned int offset;

	if ((unsigned int)rail >= ARRAY_SIZE(voltage_descs) || !selector)
		return -1;

	desc = &voltage_descs[rail];
	if (!desc->ranges || !desc->range_count)
		return -1;

	if (millivolts < desc->ranges[0].min_mv)
		millivolts = desc->ranges[0].min_mv;
	max_mv = range_max_mv(&desc->ranges[desc->range_count - 1]);
	if (millivolts > max_mv)
		millivolts = max_mv;

	for (index = 0; index < desc->range_count; index++) {
		range = &desc->ranges[index];
		if (!range->step_mv || range->min_selector > range->max_selector)
			return -1;
		if (millivolts > range_max_mv(range))
			continue;

		if (millivolts <= range->min_mv) {
			*selector = range->min_selector;
			return 0;
		}

		offset = (millivolts - range->min_mv + range->step_mv - 1) /
			 range->step_mv;
		if (range->min_selector + offset > range->max_selector)
			continue;
		*selector = range->min_selector + offset;
		return 0;
	}

	return -1;
}
