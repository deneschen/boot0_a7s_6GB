/*
 * (C) Copyright 2013-2019
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
#ifndef _SUN60IW2_H
#define _SUN60IW2_H

#include <arch/sun60iw2p1/cpu_sun60iw2.h>

#define BOOT_PUB_HEAD_VERSION  "3000"
#define CONFIG_ARCH_SUN60IW2
#define CONFIG_ARCH_SUN60IW2P1
#define CONFIG_DRAM_PARA_V1
#define CONFIG_MONITOR
#define CONFIG_SUNXI_FIP
#define CONFIG_SUNXI_VERSION_AOTUGEN 1
/*#define FPGA_PLATFORM*/

/* dram layout*/
#define SDRAM_OFFSET(x)                   (0x40000000+(x))
#define CONFIG_SYS_DRAM_BASE              SDRAM_OFFSET(0)
#define DRAM_PARA_STORE_ADDR              SDRAM_OFFSET(0x00800000) /*fel*/
#define CONFIG_HEAP_BASE                  SDRAM_OFFSET(0x00800000) /*secure */
#define CONFIG_HEAP_SIZE                  (16 * 1024 * 1024)

#define CONFIG_BOOTPKG_BASE               SDRAM_OFFSET(0x02e00000)
#define CONFIG_FIP_STAGING_SIZE           (0x00200000)
#define CONFIG_FIP_HANDOFF_BASE            (CONFIG_BOOTPKG_BASE - 0x1000)
#define CONFIG_FIP_HANDOFF_SIZE            (0x1000)
#define CONFIG_MONITOR_BASE               SDRAM_OFFSET(0x08000000)
/*
 * bl31-monitor.bin layout:
 *   +0x0000  AArch32 monitor header (gen_monitor_img.py)
 *   +0x1000  real AArch64 BL31 entry (linked at CONFIG_MONITOR_BASE+0x1000)
 *
 * RMR warm-reset enters AArch64, so RVBAR must point at the BL31 entry,
 * not at the AArch32 header that only a direct AArch32 branch would use.
 */
#define CONFIG_MONITOR_HEAD_SIZE          (0x1000)
#define CONFIG_MONITOR_ENTRY              (CONFIG_MONITOR_BASE + CONFIG_MONITOR_HEAD_SIZE)
/* Cluster INITARCH / alternate per-core RVBAR (CPUSUBSYS). */
#define SUNXI_INITARCH_REG                (SUNXI_CPUXCFG_BASE + 0x1000)
#define SUNXI_ALT_RVBAR_LO                (SUNXI_CPUXCFG_BASE + 0x40)
#define SUNXI_ALT_RVBAR_HI                (SUNXI_CPUXCFG_BASE + 0x44)
#define CONFIG_UBOOT_BASE                 SDRAM_OFFSET(0x0a000000)
#define SCP_DRAM_BASE                     SDRAM_OFFSET(0x08100000)
#define SCP_DTS_BASE                      SDRAM_OFFSET(0x08100000)

#define SUNXI_DRAM_PARA_MAX               32

#define SUNXI_LOGO_COMPRESSED_LOGO_SIZE_ADDR            SDRAM_OFFSET(0x03000000)
#define SUNXI_LOGO_COMPRESSED_LOGO_BUFF                 SDRAM_OFFSET(0x03000010)
#define SUNXI_SHUTDOWN_CHARGE_COMPRESSED_LOGO_SIZE_ADDR SDRAM_OFFSET(0x03100000)
#define SUNXI_SHUTDOWN_CHARGE_COMPRESSED_LOGO_BUFF  	SDRAM_OFFSET(0x03100010)
#define SUNXI_ANDROID_CHARGE_COMPRESSED_LOGO_SIZE_ADDR  SDRAM_OFFSET(0x03200000)
#define SUNXI_ANDROID_CHARGE_COMPRESSED_LOGO_BUFF   	SDRAM_OFFSET(0x03200010)


/* scp mem layout */
#define SCP_DRAM_SIZE                    (0x0000) /* no cpus dram code on sun50iw10 */
#define SCP_DTS_SIZE                     (0x100000)
#define SCP_CODE_DRAM_OFFSET		     (0x14000)
#define SCP_SRAM_BASE                    (CONFIG_SYS_SRAMA2_BASE)
#define SCP_SRAM_SIZE                    (CONFIG_SYS_SRAMA2_SIZE)

/* Documented A733 E902 clock, configuration, and reset-vector controls. */
#define SUNXI_RISCV_24M_CLK_REG          (SUNXI_RPRCM_BASE + 0x0210)
#define SUNXI_RISCV_BGR_REG              (SUNXI_RPRCM_BASE + 0x021c)
#define SUNXI_E902_RST_START_ADDR_REG    (0x07032000 + 0x0204)
#define SUNXI_E902_RESET_CTRL_REG        (0x07050000 + 0x0008)
#define SUNXI_RTC_DTB_BASE_STORE_REG     (SUNXI_RTC_BASE + 0x010c)
#define SUNXI_AR100S_BOOT0_TRACE_REG      (SUNXI_RTC_BASE + 0x0118)
#define SUNXI_AR100S_E902_TRACE_REG       (SUNXI_RTC_BASE + 0x011c)
#define SUNXI_E902_RESET_VECTOR          (0x40004000)
#define SUNXI_AR100S_BOOT0_TRACE_MAGIC    (0xb0070000)

#define RISCV_CLK_GATING                 (1U << 31)
#define RISCV_CLK_SOURCE_MASK            (3U << 24)
#define RISCV_CFG_RESET                  (1U << 16)
#define RISCV_CFG_GATING                 (1U << 1)
#define RISCV_BUS_GATING                 (1U << 0)
#define E902_RESET_DEASSERT              (1U << 0)


/* boot run addr */
#define FEL_BASE                          0x20
#define SECURE_FEL_BASE                  (0x64)
#define CONFIG_BOOT0_RUN_ADDR            (0x47000)
#define CONFIG_NBOOT_STACK               (0x8d000)

#define CONFIG_TOC0_RUN_ADDR             (CONFIG_SYS_SRAMA2_BASE + 0x480)
#define CONFIG_HASH_TABLE_STACK_GAP      (4)
#define CONFIG_HASH_INFO_TABLE_SIZE      (512 - CONFIG_HASH_TABLE_STACK_GAP)/*to pass hash info to optee*/
#define CONFIG_HASH_INFO_TABLE_BASE      (CONFIG_SYS_SRAMA2_BASE + CONFIG_SYS_SRAMA2_SIZE - CONFIG_HASH_INFO_TABLE_SIZE)
#define CONFIG_SBOOT_STACK               (CONFIG_HASH_INFO_TABLE_BASE - CONFIG_HASH_TABLE_STACK_GAP)
#define CONFIG_BOOT0_RET_ADDR            (CONFIG_BOOT0_RUN_ADDR)
#define CONFIG_TOC0_HEAD_BASE            (CONFIG_SYS_SRAMA2_BASE)
#define CONFIG_TOC0_CFG_ADDR             (CONFIG_TOC0_HEAD_BASE + 0x80)


#define CONFIG_SYS_INIT_RAM_SIZE 0x8000

/* FES */
#define CONFIG_FES1_RUN_ADDR             (CONFIG_SYS_SRAMA2_BASE + 0x8000)
#define CONFIG_FES1_RET_ADDR             (CONFIG_SYS_SRAMA2_BASE + 0x7210)

/*CPU vol for boot*/
#define CONFIG_SUNXI_CORE_VOL           1100
#define CONFIG_SUNXI_SYS_VOL           950

#endif
