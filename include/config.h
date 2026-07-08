#ifndef _CONFIG_H_
#define _CONFIG_H_
/*
 * boot0-A7S standalone build configuration.
 *
 * This mirrors what spl-pub's mk/checkconf.mk (check-conf-h) generates:
 *   #include <PLATFORM.h>  followed by  #define CFG_xxx 1  for each "=y" flag.
 *
 * Flags below are taken from spl-pub/board/a733/common.mk + mmc.mk, but with
 * the FPGA-only flags (CFG_FPGA_PLATFORM / CFG_SUNXI_FPGA_BTIFILE_INIT_DRAM)
 * REMOVED so that the real-silicon init_DRAM() path is compiled (matching the
 * vendor real boot0_sdcard_sun60iw2p1.bin), not the FPGA bitfile path.
 */
#include <sun60iw2p1.h>

/* storage / boot medium (SD/MMC) */
#define CFG_SUNXI_SDMMC      1

/* gpio */
#define CFG_SUNXI_GPIO_V3    1

/* power / pmic */
#define CFG_SUNXI_POWER      1
#define CFG_SUNXI_TWI        1
#define CFG_SUNXI_PMIC       1
#define CFG_AXP81X_POWER     1
#define CFG_AXP858_POWER     1
#define CFG_AXP806_POWER     1
#define CFG_AXP2202_POWER    1

/* chipid / efuse */
#define CFG_SUNXI_CHIPID     1
#define CFG_SUNXI_EFUSE      1

/* nsi */
#define CFG_SUNXI_NSI        1

/* multi dram para select */
#define CFG_SUNXI_SELECT_DRAM_PARA 1

/* secure monitor call */
#define CFG_SUNXI_SMC_30     1

#endif /* _CONFIG_H_ */
