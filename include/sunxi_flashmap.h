#ifndef _SUNXI_FLASHMAP_H_
#define _SUNXI_FLASHMAP_H_

enum sunxi_flash_type {
	SUNXI_FLASHMAP_SPI_NOR,
	SUNXI_FLASHMAP_SDMMC,
};

int sunxi_flashmap_toc1_start(enum sunxi_flash_type flash_type);
int sunxi_flashmap_toc1_bak_start(enum sunxi_flash_type flash_type);

#endif
