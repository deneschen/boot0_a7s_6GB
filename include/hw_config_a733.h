/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _BOOT0_HW_CONFIG_A733_H_
#define _BOOT0_HW_CONFIG_A733_H_

int a733_hw_config_update_dram(void *fdt, unsigned long loaded_size,
		const unsigned int *dram_para, unsigned int count);

#endif
