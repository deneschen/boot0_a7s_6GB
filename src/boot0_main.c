/*
 * (C) Copyright 2018
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <private_tee.h>
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

__u8 uboot_backup;

struct fip_boot_images {
	const void *scp_source;
	u32 scp_size;
	u32 dtb_base;
};

extern const u8 fip_handoff_start[];
extern const u8 fip_handoff_end[];

static void update_uboot_info(phys_addr_t uboot_base, phys_addr_t optee_base,
		phys_addr_t monitor_base, phys_addr_t rtos_base, u32 dram_size,
		u32 boot_package_size, u16 pmu_type, u16 uart_input, u16 key_input);
static int load_fip_images(phys_addr_t *uboot_base,
		phys_addr_t *monitor_base, u32 *boot_package_size,
		struct fip_boot_images *images);
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
	u32 boot_package_size = 0;
	struct fip_boot_images fip_images = { 0 };
	phys_addr_t  uboot_base = 0, optee_base = 0, monitor_base = 0, \
				rtos_base = 0, opensbi_base = 0, cpus_rtos_base = 0;
	u16 pmu_type = 0, key_input = 0; /* TODO: set real value */

	a7s_early_uart_init();
	a7s_early_uart_puts("\r\nA7S BOOT0: early uart0 alive\r\n");

	sunxi_board_init_early();
	sunxi_serial_init(BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 2);
	a7s_early_uart_init();
	print_commit_log();

	status = sunxi_board_init();
	if (status)
		goto _BOOT_ERROR;

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

	status = load_fip_images(&uboot_base, &monitor_base,
				 &boot_package_size, &fip_images);
	if (status != 0)
		goto _BOOT_ERROR;

	update_uboot_info(uboot_base, optee_base, monitor_base, rtos_base, dram_size,
			boot_package_size, pmu_type, uart_input_value, key_input);

	if (load_and_run_fastboot(uboot_base, optee_base, monitor_base, rtos_base, opensbi_base, cpus_rtos_base))
		goto _BOOT_ERROR;

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
	return mmc_bread_ext(start_sector, sector_count, destination);
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
	if (destination == SUNXI_FIP_HW_CONFIG_BASE)
		images->dtb_base = destination;
	return 0;
}

static int load_fip_images(phys_addr_t *uboot_base,
		phys_addr_t *monitor_base, u32 *boot_package_size,
		struct fip_boot_images *images)
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

	*uboot_base = CONFIG_UBOOT_BASE;
	*monitor_base = CONFIG_MONITOR_BASE;
	*boot_package_size = (u32)fip_size;
	uboot_backup = used_sector != primary_sector &&
			used_sector == backup_sector ? UBOOTB : UBOOTA;
	printf("FIP: loaded sector=%u, size=%u\n", used_sector,
	       *boot_package_size);
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

static int run_fip_handoff(const struct fip_boot_images *images)
{
	size_t handoff_size = fip_handoff_end - fip_handoff_start;
	fip_handoff_fn handoff;

	if (!images->scp_source || !images->scp_size ||
	    handoff_size > CONFIG_FIP_HANDOFF_SIZE) {
		printf("FIP: invalid handoff layout\n");
		return -1;
	}

	memcpy((void *)(phys_addr_t)CONFIG_FIP_HANDOFF_BASE,
	       fip_handoff_start, handoff_size);
	v7_flush_dcache_all();
	invalidate_instruction_cache();

	handoff = (fip_handoff_fn)(phys_addr_t)(CONFIG_FIP_HANDOFF_BASE | 1U);
	handoff(images->scp_source, images->scp_size, images->dtb_base);
	return -1;
}

static void update_uboot_info(phys_addr_t uboot_base, phys_addr_t optee_base,
		phys_addr_t monitor_base, phys_addr_t rtos_base, u32 dram_size,
		u32 boot_package_size, u16 pmu_type, u16 uart_input, u16 key_input)
{
	if (rtos_base)
		return;

	uboot_head_t  *header = (uboot_head_t *) uboot_base;
	if (uboot_base) {

#ifdef CFG_SUNXI_BOOT_PARAM
		sunxi_bootparam_format_and_transfer(
			(void *)(uboot_base + SUNXI_BOOTPARAM_OFFSET));
#endif

		header->boot_data.boot_package_size = boot_package_size;
		header->boot_data.dram_scan_size = dram_size;
		memcpy((void *)header->boot_data.dram_para,
			(void *)sunxi_bootparam_get_dram_buf(), 32 * sizeof(int));

		if (monitor_base)
			header->boot_data.monitor_exist = 1;

		if (optee_base) {
			struct spare_optee_head *tee_head =
				(struct spare_optee_head *)optee_base;
			header->boot_data.secureos_exist = 1;
			tee_head->dram_size		 = dram_size;
			tee_head->drm_size = BT0_head.secure_dram_mbytes;
			tee_head->uart_port = BT0_head.prvt_head.uart_port;
		}

#ifndef CONFIG_RISCV
		header->boot_data.func_mask |= get_uboot_func_mask(UBOOT_FUNC_MASK_ALL);
#endif
		update_flash_para(uboot_base);

		header->boot_data.uart_port = BT0_head.prvt_head.uart_port;
		memcpy((void *)header->boot_data.uart_gpio, BT0_head.prvt_head.uart_ctrl, 2*sizeof(normal_gpio_cfg));
		header->boot_data.pmu_type = pmu_type;
		header->boot_data.uart_input = uart_input;
		header->boot_data.key_input = key_input;
		header->boot_data.debug_mode = sunxi_get_debug_mode_for_uboot();
		if (get_card_work_mode() != NOUSE_CARDMODE)
			header->boot_data.work_mode = get_card_work_mode();
		header->boot_data.uboot_backup = uboot_backup;
	}
}

static int boot0_clear_env(void)
{
	sunxi_board_exit();
	sunxi_board_clock_reset();
	mmu_disable();
	mdelay(10);

	return 0;
}
