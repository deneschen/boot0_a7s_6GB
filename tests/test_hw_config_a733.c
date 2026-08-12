/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libfdt.h>
#include <hw_config_a733.h>

#define DRAM_PARA_COUNT 96U

static void fail(const char *message)
{
	fprintf(stderr, "hw-config test: %s\n", message);
	exit(1);
}

static void *read_file(const char *path, size_t *size)
{
	FILE *file = fopen(path, "rb");
	long length;
	void *buffer;

	if (!file || fseek(file, 0, SEEK_END) != 0 ||
	    (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0)
		fail("cannot size input DTB");
	buffer = malloc((size_t)length);
	if (!buffer || fread(buffer, 1, (size_t)length, file) != (size_t)length)
		fail("cannot read input DTB");
	fclose(file);
	*size = (size_t)length;
	return buffer;
}

static void check_single_cell(const void *fdt, int node, const char *name)
{
	int length;

	if (!fdt_getprop(fdt, node, name, &length) || length != sizeof(fdt32_t))
		fail("standby property is missing or is not one cell");
}

int main(int argc, char **argv)
{
	static const char *const standby_properties[] = {
		"vcc-io", "vcc-efuse", "vdd-cpub", "vdd-cpu",
		"vdd-sys", "vcc-pll", "osc24m-on",
	};
	uint32_t dram_para[DRAM_PARA_COUNT];
	char property[] = "dram_para00";
	void *fdt;
	size_t size;
	unsigned int index;
	int node;

	if (argc != 2)
		fail("usage: test_hw_config_a733 FILE.dtb");
	fdt = read_file(argv[1], &size);
	for (index = 0; index < DRAM_PARA_COUNT; index++)
		dram_para[index] = 0xa7330000U + index;

	if (a733_hw_config_update_dram(fdt, size, dram_para,
					DRAM_PARA_COUNT) != 0)
		fail("valid HW_CONFIG was rejected");
	if (a733_hw_config_update_dram(fdt, size - 1, dram_para,
					DRAM_PARA_COUNT) == 0 ||
	    a733_hw_config_update_dram(fdt, size, dram_para,
					DRAM_PARA_COUNT - 1) == 0)
		fail("invalid HW_CONFIG bounds were accepted");

	node = fdt_path_offset(fdt, "/dram");
	if (node < 0)
		fail("missing /dram node");
	for (index = 0; index < DRAM_PARA_COUNT; index++) {
		const fdt32_t *value;
		int length;

		property[9] = '0' + index / 10;
		property[10] = '0' + index % 10;
		value = fdt_getprop(fdt, node, property, &length);
		if (!value || length != sizeof(*value) ||
		    fdt32_to_cpu(*value) != dram_para[index])
			fail("trained DRAM parameter was not written in place");
	}

	node = fdt_path_offset(fdt, "/standby_param");
	if (node < 0)
		fail("missing /standby_param node");
	for (index = 0; index < sizeof(standby_properties) /
					  sizeof(standby_properties[0]); index++)
		check_single_cell(fdt, node, standby_properties[index]);

	free(fdt);
	puts("A733 AR100S HW_CONFIG checks passed");
	return 0;
}
