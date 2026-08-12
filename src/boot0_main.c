/*
 * (C) Copyright 2018
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <clock_a733.h>
#include <clock_diag_a733.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/rtc.h>
#include <arch/gpio.h>
#include <board_helper.h>
#include <config.h>
#include <boot_param.h>
#include <sunxi_fip.h>
#include <sunxi_flashmap.h>
#include <hw_config_a733.h>

#ifndef CONFIG_SUNXI_FIP
#error "This A733 boot0 build requires the FIP-only boot path"
#endif
#if CONFIG_FIP_STAGING_SIZE != SUNXI_FIP_MAX_SIZE
#error "A733 FIP staging size must match the parser limit"
#endif
#if CONFIG_MONITOR_BASE != SUNXI_FIP_BL31_BASE
#error "A733 BL31 address does not match the FIP copy policy"
#endif
#if CONFIG_UBOOT_BASE != SUNXI_FIP_BL33_BASE
#error "A733 BL33 address does not match the FIP copy policy"
#endif
#if SCP_SRAM_BASE != SUNXI_FIP_SCP_BL2_BASE
#error "A733 SCP_BL2 address does not match the FIP copy policy"
#endif
#if SCP_SRAM_SIZE != SUNXI_FIP_SCP_BL2_MAX_SIZE
#error "A733 SCP_BL2 size does not match the SRAM copy window"
#endif
#if SCP_DTS_BASE != SUNXI_FIP_HW_CONFIG_BASE
#error "A733 SCP DTB address does not match the FIP copy policy"
#endif
#if SCP_DTS_SIZE != SUNXI_FIP_HW_CONFIG_MAX_SIZE
#error "A733 SCP DTB size does not match the little-endian access window"
#endif
#if SUNXI_FIP_BL31_BASE + SUNXI_FIP_BL31_MAX_SIZE > SUNXI_FIP_HW_CONFIG_BASE
#error "A733 BL31 and HW_CONFIG load windows overlap"
#endif
#if SUNXI_FIP_HW_CONFIG_BASE + SUNXI_FIP_HW_CONFIG_MAX_SIZE > SUNXI_FIP_BL33_BASE
#error "A733 HW_CONFIG and BL33 load windows overlap"
#endif

struct fip_boot_images {
	const void *scp_source;
	u32 scp_size;
	u32 dtb_base;
	u32 dtb_size;
};

extern const u8 fip_handoff_start[];
extern const u8 fip_handoff_end[];

static int load_fip_images(phys_addr_t *uboot_base,
		phys_addr_t *monitor_base, struct fip_boot_images *images);
static int run_fip_handoff(const struct fip_boot_images *images);
static int boot0_clear_env(void);
__maybe_unused int load_kernel_from_spinor(u32 *);
__maybe_unused void startup_kernel(u32, u32);
void a7s_early_uart_init(void);
void a7s_early_uart_puts(const char *s);

static void print_commit_log(void)
{
	printf("HELLO! BOOT0 is starting!\n");
	printf("BOOT0 commit : %s\n", BT0_head.hash);
	sunxi_set_printf_debug_mode(BT0_head.prvt_head.debug_mode, 0);
}

void main(void)
{
	int dram_size;
	int status;
	struct fip_boot_images fip_images = { 0 };
	phys_addr_t uboot_base = 0, monitor_base = 0;

	a7s_early_uart_init();
	a7s_early_uart_puts("\r\nA7S BOOT0: early uart0 alive\r\n");

	sunxi_board_init_early();
	sunxi_serial_init(BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 2);
	a7s_early_uart_init();
	print_commit_log();

	status = sunxi_board_init();
	if (status)
		goto _BOOT_ERROR;

	status = a7s_clock_init();
	if (status) {
		printf("A7S clock init failed: %d (24 MHz fallback requested)\n",
		       status);
		goto _BOOT_ERROR;
	}
	if (rtc_probe_fel_flag()) {
		rtc_clear_fel_flag();
		goto _BOOT_ERROR;
#ifdef CFG_SUNXI_PHY_KEY
#ifdef CFG_LRADC_KEY
	} else if (check_update_key(&key_input)) {
		goto _BOOT_ERROR;
#endif
#endif
	}

	if (BT0_head.prvt_head.enable_jtag) {
		printf("enable_jtag\n");
		boot_set_gpio((normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio, 5, 1);
	}

	char uart_input_value = get_uart_input(); /* Prevent DRAM jamming */
	if (uart_input_value == '2') {
		printf("detected_f user input 2\n");
		goto _BOOT_ERROR;
	}
	sunxi_bootparam_load();
#ifdef FPGA_PLATFORM
	dram_size = sunxi_fpga_dram_init((void *)sunxi_bootparam_get_dram_buf());
#else
	dram_size = init_DRAM(0, (void *)sunxi_bootparam_get_dram_buf());
#endif
	if (!dram_size) {
		printf("init dram fail\n");
		goto _BOOT_ERROR;
	} else {
		if (BT0_head.dram_size > 0)
			dram_size = BT0_head.dram_size;
		printf("dram size =%d\n", dram_size);
	}
	a7s_clock_dump();
#ifdef CFG_SUNXI_STANDBY_WORKAROUND
	handler_super_standby();
#endif

	uart_input_value = get_uart_input();
	if (uart_input_value == '2') {
		sunxi_set_printf_debug_mode(3, 0);
		printf("detected_r user input 2\n");
		goto _BOOT_ERROR;
	} else if (uart_input_value == 'd') {
		sunxi_set_printf_debug_mode(8, 1);
		printf("detected user input d\n");
	} else if (uart_input_value == 'q') {
		printf("detected user input q\n");
		sunxi_set_printf_debug_mode(0, 1);
	}

	mmu_enable(dram_size);
	malloc_init(CONFIG_HEAP_BASE, CONFIG_HEAP_SIZE);
	status = sunxi_board_late_init();
	if (status)
		goto _BOOT_ERROR;

	status = load_fip_images(&uboot_base, &monitor_base, &fip_images);
	if (status != 0)
		goto _BOOT_ERROR;

	/*
	 * BL33 is a raw mainline U-Boot image whose first byte is executable
	 * code, not a vendor spare header. Treat it as immutable and only pass
	 * its load address through the monitor header. Do not call
	 * v7_flush_dcache_all()/mmu_disable() here: on this A733 boot path the
	 * vendor flush hangs, and RMR restarts the CPU in a clean MMU state.
	 */
	if (monitor_base) {
		struct spare_monitor_head *monitor_head =
			(struct spare_monitor_head *)(phys_addr_t)monitor_base;
		monitor_head->secureos_base = 0;
		monitor_head->nboot_base = uboot_base;
	}

	printf("Jump to second Boot.\n");
	if (run_fip_handoff(&fip_images) != 0)
		goto _BOOT_ERROR;
_BOOT_ERROR:
	boot0_clear_env();
	boot0_jmp(FEL_BASE);

}

static int fip_mmc_read(u32 start_sector, u32 sector_count,
			void *destination, void *context)
{
	(void)context;
	/*
	 * Vendor mmc_bread_ext returns 0 on success and non-zero on failure.
	 * sunxi_fip_read_fn expects 1 on success (U-Boot block_read style),
	 * so convert here instead of changing the FIP parser contract.
	 */
	return mmc_bread_ext(start_sector, sector_count, destination) == 0 ? 1 : 0;
}

static int fip_copy_image(u32 destination, const void *source, size_t size,
			  void *context)
{
	struct fip_boot_images *images = context;

	if (destination == SUNXI_FIP_SCP_BL2_BASE) {
		images->scp_source = source;
		images->scp_size = size;
		return 0;
	}

	memcpy((void *)(phys_addr_t)destination, source, size);
	if (destination == SUNXI_FIP_HW_CONFIG_BASE) {
		images->dtb_base = destination;
		images->dtb_size = size;
	}
	return 0;
}

static int load_fip_images(phys_addr_t *uboot_base,
		phys_addr_t *monitor_base, struct fip_boot_images *images)
{
	size_t fip_size = 0;
	u32 primary_sector;
	u32 backup_sector;
	u32 used_sector = 0;

	if (sunxi_mmc_init_ext() != 0) {
		printf("FIP: MMC init failed\n");
		return -1;
	}

	primary_sector = sunxi_flashmap_toc1_start(SUNXI_FLASHMAP_SDMMC);
	backup_sector = sunxi_flashmap_toc1_bak_start(SUNXI_FLASHMAP_SDMMC);
	printf("FIP: primary=%u, backup=%u\n", primary_sector, backup_sector);

	if (sunxi_fip_load_redundant(primary_sector, backup_sector,
			(void *)(phys_addr_t)CONFIG_BOOTPKG_BASE,
			CONFIG_FIP_STAGING_SIZE, fip_mmc_read, NULL,
			fip_copy_image, images, &fip_size,
			&used_sector) != 0) {
		printf("FIP: no valid package found\n");
		return -1;
	}

	if (!images->scp_source || !images->scp_size) {
		printf("FIP: SCP_BL2 was not loaded\n");
		return -1;
	}
	if (images->dtb_base &&
	    a733_hw_config_update_dram(
			(void *)(phys_addr_t)images->dtb_base,
			images->dtb_size,
			sunxi_bootparam_get_dram_buf()->dram_para,
			MAX_DRAMPARA_SIZE) != 0) {
		printf("FIP: invalid AR100S HW_CONFIG; disabling DTB services\n");
		images->dtb_base = 0;
		images->dtb_size = 0;
	} else if (images->dtb_base) {
		printf("FIP: patched 96 trained DRAM parameters into HW_CONFIG\n");
	}

	*uboot_base = CONFIG_UBOOT_BASE;
	*monitor_base = CONFIG_MONITOR_BASE;
	printf("FIP: loaded sector=%u, size=%u\n", used_sector,
	       (u32)fip_size);
	return 0;
}

typedef void (*fip_handoff_fn)(const void *scp_source, u32 scp_size,
			       u32 dtb_base);

static void invalidate_instruction_cache(void)
{
	u32 zero = 0;

	__asm__ volatile(
		"dsb sy\n\t"
		"mcr p15, 0, %0, c7, c5, 0\n\t"
		"isb sy"
		:
		: "r" (zero)
		: "memory");
}

/*
 * Clean D-cache by VA to PoC for a small relocated region.
 * Avoid v7_flush_dcache_all() on this path: the vendor set/way flush hangs
 * after FIP load on A733.
 */
static void clean_dcache_range(unsigned long start, unsigned long size)
{
	unsigned long end = start + size;
	unsigned long line = 32;
	unsigned long addr;

	start &= ~(line - 1);
	for (addr = start; addr < end; addr += line) {
		__asm__ volatile("mcr p15, 0, %0, c7, c10, 1"
				 :
				 : "r" (addr)
				 : "memory");
	}
	__asm__ volatile("dsb sy" ::: "memory");
}

static int run_fip_handoff(const struct fip_boot_images *images)
{
	/*
	 * fip_handoff_start is a Thumb function symbol, so its ELF value has
	 * bit0 set (e.g. 0x47b65). That is correct for BLX, but memcpy of
	 * the machine code must start at the even address. Clear bit0 on both
	 * ends before computing size / copying bytes.
	 */
	const u8 *handoff_src =
		(const u8 *)((unsigned long)fip_handoff_start & ~1UL);
	const u8 *handoff_end_addr =
		(const u8 *)((unsigned long)fip_handoff_end & ~1UL);
	size_t handoff_size = (size_t)(handoff_end_addr - handoff_src);
	fip_handoff_fn handoff;

	printf("FIP: prepare handoff size=%u src=0x%lx\n",
	       (u32)handoff_size, (unsigned long)handoff_src);

	if (!images->scp_source || !images->scp_size ||
	    handoff_size == 0 || handoff_size > CONFIG_FIP_HANDOFF_SIZE) {
		printf("FIP: invalid handoff layout\n");
		return -1;
	}

	/*
	 * SCP_SRAM is 0x44000..0x6c000 and overlaps boot0 @ 0x47000. The
	 * handoff stub must run from DRAM so clearing/copying SCP cannot
	 * overwrite the code that is still executing.
	 */
	memcpy((void *)(phys_addr_t)CONFIG_FIP_HANDOFF_BASE,
	       handoff_src, handoff_size);
	clean_dcache_range(CONFIG_FIP_HANDOFF_BASE, handoff_size);
	clean_dcache_range(CONFIG_MONITOR_BASE, SUNXI_FIP_BL31_MAX_SIZE);
	clean_dcache_range(CONFIG_UBOOT_BASE, SUNXI_FIP_BL33_MAX_SIZE);
	clean_dcache_range((unsigned long)images->scp_source, images->scp_size);
	if (images->dtb_base)
		clean_dcache_range(images->dtb_base, images->dtb_size);
	invalidate_instruction_cache();

	printf("FIP: handoff scp=%u dtb=0x%x entry=0x%x dest=0x%x\n",
	       images->scp_size, images->dtb_base, CONFIG_MONITOR_ENTRY,
	       CONFIG_FIP_HANDOFF_BASE);
	/* Dest is even; set Thumb bit only on the call target. */
	handoff = (fip_handoff_fn)(phys_addr_t)(CONFIG_FIP_HANDOFF_BASE | 1U);
	handoff(images->scp_source, images->scp_size, images->dtb_base);
	return -1;
}

static int boot0_clear_env(void)
{
	sunxi_board_exit();
	a7s_clock_reset();
	mmu_disable();
	mdelay(10);

	return 0;
}
