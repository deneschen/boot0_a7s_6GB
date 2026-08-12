/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <libfdt.h>
#include <hw_config_a733.h>

#define A733_DRAM_PARA_COUNT 96U

int a733_hw_config_update_dram(void *fdt, unsigned long loaded_size,
		const unsigned int *dram_para, unsigned int count)
{
	char property[] = "dram_para00";
	int node;
	unsigned int index;

	if (!fdt || !dram_para || count != A733_DRAM_PARA_COUNT ||
	    fdt_check_header(fdt) != 0 ||
	    fdt_totalsize(fdt) > loaded_size)
		return -1;

	node = fdt_path_offset(fdt, "/dram");
	if (node < 0)
		return -1;

	for (index = 0; index < A733_DRAM_PARA_COUNT; index++) {
		const fdt32_t *value;
		int length;

		property[9] = '0' + index / 10;
		property[10] = '0' + index % 10;
		value = fdt_getprop(fdt, node, property, &length);
		if (!value || length != sizeof(*value) ||
		    fdt_setprop_inplace_u32(fdt, node, property,
					    dram_para[index]) != 0)
			return -1;
	}

	return 0;
}
