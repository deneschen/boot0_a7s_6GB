/*
 * Platform hooks needed by the closed A733 DRAM library.
 *
 * The supplied A733 board blob is an FPGA build and has no AXP8191 support, so
 * the real board GPIO and DRAM-rail setup live here.
 */
#include <common.h>
#include <arch/gpio.h>
#include <axp8191_voltage.h>
#include <boot_param.h>
#include <dram_cache_a733.h>
#include <private_boot0.h>

#define A7S_MAIN_PIO_BANK_SIZE	0x80
#define A7S_MAIN_PIO_CFG_BASE	0x80
#define A7S_MAIN_PIO_DATA	0x90
#define A7S_MAIN_PIO_DRV_BASE	0xa0
#define A7S_MAIN_PIO_PULL_BASE	0xb0

#define A7S_R_PIO_FIRST_PORT	12
#define A7S_R_PIO_BANK_SIZE	0x30
#define A7S_R_PIO_CFG_BASE	0x00
#define A7S_R_PIO_DATA		0x10
#define A7S_R_PIO_DRV_BASE	0x14
#define A7S_R_PIO_PULL_BASE	0x24

#define A7S_MMC0_PF_CFG		(SUNXI_PIO_BASE + 0x300)
#define A7S_MMC0_PF_DRV		(SUNXI_PIO_BASE + 0x320)
#define A7S_MMC0_PF_PULL		(SUNXI_PIO_BASE + 0x330)

#define A7S_SOC_VERSION_REG		0x03000024
#define A7S_SOC_VERSION_MASK		0x7
#define A7S_SOC_VERSION_B		1

#define A7S_PIO_A_POWER_MODE_SELECT	(SUNXI_PIO_BASE + GPIO_POW_MODE_REG)
#define A7S_PIO_A_POWER_MODE_CONTROL	(SUNXI_PIO_BASE + GPIO_POW_MS_CTL)
#define A7S_PIO_A_POWER_MODE_VALUE	(SUNXI_PIO_BASE + GPIO_POW_MODE_VAL_REG)
#define A7S_PIO_A_POWER_CONTROL		(SUNXI_PIO_BASE + 0x390)

#define A7S_PIO_B_POWER_MODE_SELECT	(SUNXI_PIO_BASE + 0x40)
#define A7S_PIO_B_POWER_MODE_VALUE	(SUNXI_PIO_BASE + 0x48)
#define A7S_PIO_B_POWER_CONTROL		(SUNXI_PIO_BASE + 0x70)

/*
 * GPIO_POW_MOD_SEL bit fields (manual 18.14.6.1): two bits per port,
 * 00/10 = 3.3 V manual, 01 = adaptive, 11 = 1.8 V manual.  Reset default is
 * 10 (3.3 V) for every port.  Port C carries the 1.8 V eMMC bus and must not
 * stay in the 3.3 V withstand mode, otherwise high speed modes fail with
 * response CRC errors while everything below ~50 MHz still works.  The vendor
 * U-Boot puts every group it configures into adaptive mode; nothing later
 * does (the kernel pinctrl runs in auto_hard mode and skips these registers).
 */
#define A7S_PIO_PC_WITHSTAND_SHIFT	4
#define A7S_PIO_PC_WITHSTAND_MASK	(0x3U << A7S_PIO_PC_WITHSTAND_SHIFT)
#define A7S_PIO_PC_WITHSTAND_AUTO	(0x1U << A7S_PIO_PC_WITHSTAND_SHIFT)

#define AXP8191_DEVICE_ADDR	400000
#define AXP8191_RUNTIME_ADDR	0x36
#define AXP8191_CHIP_ID		0x0e
#define AXP8191_CHIP_ID_A	0x03
#define AXP8191_DCDC_CTRL1	0x10
#define AXP8191_DCDC_CTRL2	0x11
#define AXP8191_DC1SW2_ENABLE	(1U << 4)
#define AXP8191_DCDC2_VOL	0x13
#define AXP8191_DCDC2_840MV	0x22
#define AXP8191_DCDC6_VOL	0x17
#define AXP8191_DCDC7_VOL	0x18
#define AXP8191_DCDC8_VOL	0x19
#define AXP8191_LDO_CTRL2	0x21
#define AXP8191_LDO_CTRL3	0x22
#define AXP8191_CLDO5_VOL	0x33
#define AXP8191_ELDO1_VOL	0x3a
#define AXP8191_ELDO2_VOL	0x3b
#define AXP8191_AP_RESET_CTRL	0x55
#define AXP8191_WRITE_LOCK	0xf0
#define AXP8191_EFUSE_CTRL	0xf1
#define AXP8191_EXT_ADDR		0xff

struct a7s_axp8191_rail {
	const char *name;
	u8 voltage_reg;
	u8 voltage_mask;
	u8 enable_reg;
	u8 enable_bit;
	enum a7s_axp8191_voltage_rail voltage_rail;
};

static const struct a7s_axp8191_rail a7s_dram_rails[] = {
	{ "dcdc6", AXP8191_DCDC6_VOL, 0x7f, AXP8191_DCDC_CTRL1, 5,
	  A7S_AXP8191_DCDC6 },
	{ "dcdc7", AXP8191_DCDC7_VOL, 0x7f, AXP8191_DCDC_CTRL1, 6,
	  A7S_AXP8191_DCDC7 },
	{ "dcdc8", AXP8191_DCDC8_VOL, 0x7f, AXP8191_DCDC_CTRL1, 7,
	  A7S_AXP8191_DCDC8 },
	{ "eldo1", AXP8191_ELDO1_VOL, 0x3f, AXP8191_LDO_CTRL3, 6,
	  A7S_AXP8191_ELDO },
	{ "eldo2", AXP8191_ELDO2_VOL, 0x3f, AXP8191_LDO_CTRL3, 7,
	  A7S_AXP8191_ELDO },
	{ "cldo5", AXP8191_CLDO5_VOL, 0x1f, AXP8191_LDO_CTRL2, 7,
	  A7S_AXP8191_CLDO },
};

int pmic_bus_init(u32 device_addr, u32 runtime_addr);
int pmic_bus_read(u8 runtime_addr, u8 reg, u8 *value);
int pmic_bus_write(u8 runtime_addr, u8 reg, u8 value);

static int a7s_axp8191_ready;

extern void *mmc_devices[];
int mmc_init(void *mmc);
int mmc_read_blocks(void *mmc, void *dst, u32 start, u32 blkcnt);

/*
 * Vendor blob scratch area (one sector).  It holds the optional "acmm"
 * eMMC timing region read from LBA 24504, and the blob's mmc_init() takes
 * the card media type from byte offset 0x68.  Our images never write that
 * region, so the media type is always 0 and the blob prints
 * "Wrong media type 0x0" before probing.  The SMHC number fixes the media:
 * SDC0 is the microSD slot and SDC2 is the soldered eMMC.
 */
extern unsigned char mmc_arg_addr[];

#define A7S_MMC_INFO_SECTOR		24504
#define A7S_MMC_INFO_MAGIC		0x6d6d6361U	/* "acmm" */
#define A7S_MMC_MEDIA_OFFSET		0x68
#define A7S_MMC_MEDIA_SD		0x08000001U
#define A7S_MMC_MEDIA_MMC		0x08000000U

static void a7s_mmc_set_media(u32 media)
{
	unsigned char *p = mmc_arg_addr + A7S_MMC_MEDIA_OFFSET;

	p[0] = media & 0xff;
	p[1] = (media >> 8) & 0xff;
	p[2] = (media >> 16) & 0xff;
	p[3] = (media >> 24) & 0xff;
}

/*
 * Replaces the blob's mmc_read_info() (first definition wins at link time).
 * The vendor version retries four times and prints "region magic is not
 * right" / "RMCA FAIL!" / "mmc read timing info fail" whenever the optional
 * region is absent, which is the normal state on this board.  Read it once,
 * keep a valid region for the consumers (they check the magic themselves),
 * and invalidate a region with a bad checksum.  Absence is not an error.
 */
int mmc_read_info(void *mmc, void *buf, u32 count)
{
	unsigned char *p = buf;
	u32 *w = buf;
	u32 len, stored, sum = 0, i;

	if (mmc_read_blocks(mmc, buf, A7S_MMC_INFO_SECTOR, count) < 0)
		return 0;
	if (w[5] != A7S_MMC_INFO_MAGIC)
		return 0;

	len = w[7];
	stored = w[6];
	w[6] = 0;
	if (len <= count * 512)
		for (i = 0; i < len; i++)
			sum += p[i];
	if (len > count * 512 || sum != stored)
		w[5] = 0;

	return 0;
}

static void a7s_gpio_registers(unsigned int port, unsigned int pin,
		unsigned long *cfg, unsigned long *pull, unsigned long *drv,
		unsigned long *data)
{
	unsigned long bank;

	if (port < A7S_R_PIO_FIRST_PORT) {
		bank = SUNXI_PIO_BASE + (port - 1) * A7S_MAIN_PIO_BANK_SIZE;
		*cfg = bank + A7S_MAIN_PIO_CFG_BASE + (pin >> 3) * 4;
		*pull = bank + A7S_MAIN_PIO_PULL_BASE + (pin >> 4) * 4;
		*drv = bank + A7S_MAIN_PIO_DRV_BASE + (pin >> 3) * 4;
		*data = bank + A7S_MAIN_PIO_DATA;
	} else {
		bank = SUNXI_R_PIO_BASE +
		       (port - A7S_R_PIO_FIRST_PORT) * A7S_R_PIO_BANK_SIZE;
		*cfg = bank + A7S_R_PIO_CFG_BASE + (pin >> 3) * 4;
		*pull = bank + A7S_R_PIO_PULL_BASE + (pin >> 4) * 4;
		*drv = bank + A7S_R_PIO_DRV_BASE + (pin >> 3) * 4;
		*data = bank + A7S_R_PIO_DATA;
	}
}

int a7s_boot_set_gpio_v3(void *user_gpio_list, u32 group_count_max, int set_gpio)
{
	normal_gpio_cfg *gpio_list = user_gpio_list;
	u32 index;

	if (!gpio_list || !group_count_max)
		return -1;

	for (index = 0; index < group_count_max; index++) {
		normal_gpio_cfg *gpio = &gpio_list[index];
		unsigned int port = (unsigned char)gpio->port;
		unsigned int pin = (unsigned char)gpio->port_num;
		unsigned int cfg_shift = (pin & 0x7) * 4;
		unsigned int pull_shift = (pin & 0xf) * 2;
		unsigned int drv_shift = (pin & 0x7) * 4;
		int pull = (signed char)gpio->pull;
		int drv = (signed char)gpio->drv_level;
		int pin_data = (signed char)gpio->data;
		unsigned long cfg_reg, pull_reg, drv_reg, data_reg;
		unsigned int val;

		if (!port)
			continue;

		a7s_gpio_registers(port, pin, &cfg_reg, &pull_reg, &drv_reg,
				   &data_reg);

		val = readl(cfg_reg);
		val &= ~(0xfU << cfg_shift);
		if (set_gpio)
			val |= ((unsigned char)gpio->mul_sel & 0xfU) << cfg_shift;
		writel(val, cfg_reg);

		if (pull >= 0) {
			val = readl(pull_reg);
			val &= ~(0x3U << pull_shift);
			val |= ((unsigned int)pull & 0x3U) << pull_shift;
			writel(val, pull_reg);
		}

		if (drv >= 0) {
			val = readl(drv_reg);
			val &= ~(0x3U << drv_shift);
			val |= ((unsigned int)drv & 0x3U) << drv_shift;
			writel(val, drv_reg);
		}

		if ((unsigned char)gpio->mul_sel == 1 && pin_data >= 0) {
			val = readl(data_reg);
			val &= ~(1U << pin);
			val |= ((unsigned int)pin_data & 1U) << pin;
			writel(val, data_reg);
		}
	}

	return 0;
}

int boot_set_gpio(void *user_gpio_list, u32 group_count_max, int set_gpio)
{
	return a7s_boot_set_gpio_v3(user_gpio_list, group_count_max, set_gpio);
}

static int a7s_axp8191_update_bits(u8 reg, u8 mask, u8 value)
{
	u8 reg_value;

	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, reg, &reg_value))
		return -1;

	reg_value = (reg_value & ~mask) | (value & mask);
	return pmic_bus_write(AXP8191_RUNTIME_ADDR, reg, reg_value);
}

static int a7s_axp8191_init(void)
{
	u8 chip_id;
	u8 ext_cfg;
	int status = 0;
	int ret;

	if (a7s_axp8191_ready)
		return 0;

	if (pmic_bus_init(AXP8191_DEVICE_ADDR, AXP8191_RUNTIME_ADDR)) {
		printf("A7S PMU: TWI6 init failed\n");
		return -1;
	}

	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_CHIP_ID, &chip_id)) {
		printf("A7S PMU: no device at 0x36\n");
		return -1;
	}
	if (chip_id != AXP8191_CHIP_ID_A) {
		printf("A7S PMU: unexpected chip id 0x%x\n", chip_id);
		return -1;
	}

	/* Match the vendor AXP8191 probe sequence before changing regulators. */
	status = pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_WRITE_LOCK, 0x06);
	if (status)
		goto restore_normal_page;
	status = pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_EFUSE_CTRL, 0x04);
	if (status)
		goto restore_normal_page;
	status = pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_EXT_ADDR, 0x01);
	if (status)
		goto restore_normal_page;
	status = pmic_bus_read(AXP8191_RUNTIME_ADDR, 0x00, &ext_cfg);
	if (status)
		goto restore_normal_page;
	status = pmic_bus_write(AXP8191_RUNTIME_ADDR, 0x00,
				ext_cfg | (1U << 6));

restore_normal_page:
	/* A failed extended-page access must not poison later regulator traffic. */
	ret = pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_EXT_ADDR, 0x00);
	if (!status && ret)
		status = ret;
	ret = pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_EFUSE_CTRL, 0x00);
	if (!status && ret)
		status = ret;
	ret = pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_WRITE_LOCK, 0x00);
	if (!status && ret)
		status = ret;
	if (status) {
		printf("A7S PMU: extended configuration failed (%d)\n", status);
		return status;
	}

	status = a7s_axp8191_update_bits(AXP8191_AP_RESET_CTRL, 1U << 3,
					  1U << 3);
	if (status)
		return status;

	a7s_axp8191_ready = 1;
	printf("A7S PMU: AXP8191 ready on TWI6\n");
	return 0;
}

static int a7s_axp8191_enable_sd_power(void)
{
	int status;

	status = a7s_axp8191_update_bits(AXP8191_DCDC_CTRL2,
					     AXP8191_DC1SW2_ENABLE,
					     AXP8191_DC1SW2_ENABLE);
	if (status) {
		printf("A7S PMU: enable SD power dc1sw2 failed\n");
		return status;
	}

	return 0;
}

static const struct a7s_axp8191_rail *a7s_axp8191_find_rail(const char *name)
{
	u32 index;

	for (index = 0; index < sizeof(a7s_dram_rails) / sizeof(a7s_dram_rails[0]);
	     index++) {
		if (!strcmp(name, a7s_dram_rails[index].name))
			return &a7s_dram_rails[index];
	}

	return 0;
}

static int a7s_axp8191_set_rail(const char *name, int set_vol, int onoff)
{
	const struct a7s_axp8191_rail *rail = a7s_axp8191_find_rail(name);
	unsigned int selector;

	if (!rail) {
		printf("A7S PMU: unsupported DRAM rail %s\n", name);
		return -1;
	}
	if (a7s_axp8191_init())
		return -1;

	if (set_vol > 0) {
		if (a7s_axp8191_voltage_selector(rail->voltage_rail, set_vol,
					  &selector) ||
		    selector & ~rail->voltage_mask) {
			printf("A7S PMU: invalid voltage %d for %s\n", set_vol,
			       name);
			return -1;
		}
		if (a7s_axp8191_update_bits(rail->voltage_reg,
					     rail->voltage_mask, selector))
			return -1;
	}

	if (onoff < 0)
		return 0;

	return a7s_axp8191_update_bits(rail->enable_reg,
					 1U << rail->enable_bit,
					 onoff ? 1U << rail->enable_bit : 0);
}

static void a7s_pio_emmc_withstand_auto(void)
{
	u32 before = readl(A7S_PIO_B_POWER_MODE_SELECT);
	u32 after = (before & ~A7S_PIO_PC_WITHSTAND_MASK) |
		    A7S_PIO_PC_WITHSTAND_AUTO;

	writel(after, A7S_PIO_B_POWER_MODE_SELECT);
	after = readl(A7S_PIO_B_POWER_MODE_SELECT);
	printf("A7S PIO: PC withstand mode %08x -> %08x%s\n", before, after,
	       (after & A7S_PIO_PC_WITHSTAND_MASK) == A7S_PIO_PC_WITHSTAND_AUTO ?
		       "" : " (readback failed)");
}

static int a7s_axp8191_configure_emmc_io(void)
{
	u8 enable;
	u8 selector;

	if (a7s_axp8191_set_rail("cldo5", 1800, 1)) {
		printf("A7S PMU: set eMMC I/O cldo5 to 1800 mV failed\n");
		return -1;
	}
	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_CLDO5_VOL,
			  &selector) ||
	    pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_LDO_CTRL2,
			  &enable) ||
	    (selector & 0x1f) != 0x0d || !(enable & (1U << 7))) {
		printf("A7S PMU: eMMC I/O cldo5 readback failed\n");
		return -1;
	}

	/* Allow the PIO hardware voltage detector to settle after enabling CLDO5. */
	mdelay(1);
	a7s_pio_emmc_withstand_auto();
	printf("A7S PMU: eMMC I/O cldo5 = 1800 mV\n");
	return 0;
}

/*
 * Match the vendor boot-chain NPU rail.  The kernel leaves npu-setvol disabled
 * and relies on boot0 to set DCDC2 (0x22 = 840 mV, 10 mV/LSB).
 */
static int a7s_axp8191_set_npu_rail(void)
{
	u8 value;

	if (a7s_axp8191_update_bits(AXP8191_DCDC_CTRL1, 1U << 1, 1U << 1)) {
		printf("A7S PMU: enable NPU rail dcdc2 failed\n");
		return -1;
	}
	if (pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_DCDC2_VOL,
			   AXP8191_DCDC2_840MV)) {
		printf("A7S PMU: set NPU rail dcdc2 failed\n");
		return -1;
	}
	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_DCDC2_VOL, &value) ||
	    (value & 0x7f) != AXP8191_DCDC2_840MV) {
		printf("A7S PMU: NPU rail dcdc2 readback failed\n");
		return -1;
	}
	return 0;
}

int sunxi_board_init(void)
{
	u32 value = readl(0x08020000);

	/* Preserve the SoC setup performed by the supplied FPGA board object. */
	writel(value | 1U, 0x08020000);
	if (a7s_axp8191_init())
		return -1;
	if (a7s_axp8191_configure_emmc_io())
		return -1;
	if (a7s_axp8191_set_npu_rail())
		return -1;

	return a7s_axp8191_enable_sd_power();
}

int set_ddr_voltage_ext(char *name, int set_vol, int onoff)
{
	return a7s_axp8191_set_rail(name, set_vol, onoff);
}

/*
 * Replace the blob's weak bootparam hooks.  Its get_dram_buf() returns
 * BT0_head.dram_para, which has only 32 words, but init_DRAM() stores the
 * 2D training result in words 32-95 and overwrote the UART, JTAG and SDC0
 * storage GPIO entries that follow it in the header.
 */
static boot_dram_info_t a7s_dram_info;

boot_dram_info_t *sunxi_bootparam_get_dram_buf(void)
{
	return &a7s_dram_info;
}

/*
 * DRAM training cache (see dram_cache_a733.h).  Every boot that may read the
 * cache runs under the R_WDOG watchdog with ATTEMPT recorded in RTC GPR5.
 * A hang therefore resets the board, and the next boot sees ATTEMPT, skips
 * the cache read entirely and trains fully.  The cache can cost one reset,
 * never a dead board.
 */
#define A7S_DRAM_GPR_BASE	(SUNXI_RTC_BASE + 0x100)
#define A7S_DRAM_STATE_REG	(A7S_DRAM_GPR_BASE + A7S_DRAM_GPR_STATE * 4)
#define A7S_SID_CHIPID		(SUNXI_SID_BASE + 0x200)
#define A7S_R_WDOG_BASE		0x07021000U
#define A7S_WDOG_SRST		(A7S_R_WDOG_BASE + 0x08)
#define A7S_WDOG_CFG		(A7S_R_WDOG_BASE + 0x10)
#define A7S_WDOG_MODE		(A7S_R_WDOG_BASE + 0x14)
#define A7S_WDOG_OCFG		(A7S_R_WDOG_BASE + 0x18)
#define A7S_WDOG_KEY		(0x16aaU << 16)
/*
 * A full training takes 4.5 s on this board, so 16 s separates a slow but
 * working boot from a hang.  It is only ever paid once, because a boot that
 * sees ATTEMPT does not arm the watchdog again.
 */
#define A7S_WDOG_INTERVAL_16S	(0xBU << 4)

int get_card_num(void);

static struct a7s_dram_cache_record a7s_dram_cache_slots[A7S_DRAM_CACHE_SLOTS];
static u32 a7s_dram_cache_card;
static u32 a7s_dram_cache_base_crc;
static enum a7s_dram_cache_state a7s_dram_cache_state;

static void a7s_dram_cache_report(enum a7s_dram_cache_state state)
{
	writel(A7S_DRAM_STATE_MAGIC | (a7s_dram_cache_card & 0xff) << 8 | state,
	       A7S_DRAM_STATE_REG);
}

static void a7s_wdog_arm(void)
{
	/* Same register sequence and values as BL31's sunxi_system_reset(). */
	writel(A7S_WDOG_KEY | 0x3f, A7S_WDOG_OCFG);
	writel(A7S_WDOG_KEY | 1, A7S_WDOG_CFG);
	writel(A7S_WDOG_KEY, A7S_WDOG_MODE);
	writel(A7S_WDOG_KEY | A7S_WDOG_INTERVAL_16S | 1, A7S_WDOG_MODE);
}

static void a7s_wdog_disarm(void)
{
	writel(A7S_WDOG_KEY, A7S_WDOG_MODE);
}

/*
 * Read the record one sector at a time.  Measured on this board: a two-sector
 * read (CMD18) issued before init_DRAM() never returns, while single-sector
 * reads (CMD17) at the same point return the data.  The same multi-sector
 * call works once DRAM is up, which is how the FIP below is loaded.
 */
static int a7s_dram_cache_read(void)
{
	unsigned int index;

	for (index = 0; index < A7S_DRAM_CACHE_SLOTS; index++)
		if (mmc_bread_ext(A7S_DRAM_CACHE_LBA + index, 1,
				  &a7s_dram_cache_slots[index]) != 0)
			return -1;
	return 0;
}

static enum a7s_dram_cache_state a7s_dram_cache_load(u32 *dram_para,
						     u32 chipid_crc)
{
	const struct a7s_dram_cache_record *record;
	u32 previous = readl(A7S_DRAM_STATE_REG);
	int slot;

	if (previous == (A7S_DRAM_STATE_MAGIC |
			 (a7s_dram_cache_card & 0xff) << 8 |
			 A7S_DRAM_CACHE_ATTEMPT)) {
		/*
		 * The previous boot stopped inside the window below.  Train
		 * fully without touching the cache, and tell Linux so that it
		 * counts the failure and eventually disables the cache.
		 */
		printf("A7S DRAM: last cached boot did not finish\n");
		a7s_dram_cache_report(A7S_DRAM_CACHE_FALLBACK);
		return A7S_DRAM_CACHE_FALLBACK;
	}

	a7s_dram_cache_report(A7S_DRAM_CACHE_ATTEMPT);
	a7s_wdog_arm();

	/* The MMC driver reads by CPU into SRAM, so it works before DRAM. */
	if (sunxi_mmc_init_ext() != 0 || a7s_dram_cache_read() != 0)
		return A7S_DRAM_CACHE_MISS;

	slot = a7s_dram_cache_newest(a7s_dram_cache_slots,
				     A7S_DRAM_CACHE_SLOTS);
	if (slot < 0)
		return A7S_DRAM_CACHE_MISS;
	record = &a7s_dram_cache_slots[slot];

	switch (a7s_dram_cache_check(record, chipid_crc,
				     a7s_dram_cache_base_crc)) {
	case A7S_DRAM_CACHE_HIT:
		break;
	case A7S_DRAM_CACHE_DISABLED:
		return A7S_DRAM_CACHE_DISABLED;
	default:
		return A7S_DRAM_CACHE_MISS;
	}

	a7s_dram_cache_apply(dram_para, record);
	printf("A7S DRAM: cached training slot %d seq %u\n", slot,
	       record->sequence);
	return A7S_DRAM_CACHE_HIT;
}

int sunxi_bootparam_load(void)
{
	u32 chipid[4];
	u32 chipid_crc;
	unsigned int index;

	memcpy(a7s_dram_info.dram_para, BT0_head.prvt_head.dram_para,
	       sizeof(BT0_head.prvt_head.dram_para));

	for (index = 0; index < 4; index++)
		chipid[index] = readl(A7S_SID_CHIPID + index * 4);
	chipid_crc = a7s_dram_cache_crc32(chipid, sizeof(chipid));
	a7s_dram_cache_base_crc =
		a7s_dram_cache_crc32(BT0_head.prvt_head.dram_para,
				     sizeof(BT0_head.prvt_head.dram_para));

	a7s_dram_cache_card = get_card_num();

	/* Publish what this boot compares a record against. */
	writel(chipid_crc,
	       A7S_DRAM_GPR_BASE + A7S_DRAM_GPR_CHIPID_CRC * 4);
	writel(a7s_dram_cache_base_crc,
	       A7S_DRAM_GPR_BASE + A7S_DRAM_GPR_BASE_CRC * 4);

	a7s_dram_cache_state =
		a7s_dram_cache_load(a7s_dram_info.dram_para, chipid_crc);
	return 0;
}

/* Called right after init_DRAM(); dram_size is 0 when it failed. */
void a7s_dram_cache_finish(int dram_size)
{
	if (a7s_dram_cache_state == A7S_DRAM_CACHE_FALLBACK)
		return;			/* the watchdog was not armed */

	if (!dram_size) {
		/*
		 * Keep ATTEMPT: the reset brings boot0 back on the FALLBACK
		 * path, which trains fully.
		 */
		printf("A7S DRAM: DRAM init failed, resetting\n");
		writel(A7S_WDOG_KEY | 1, A7S_WDOG_SRST);
		for (;;)
			;
	}

	a7s_wdog_disarm();
	a7s_dram_cache_report(a7s_dram_cache_state);
}

void sunxi_smc_en_with_glitch_workaround(void)
{
}

static void a7s_mmc0_config_pins(void)
{
	u32 value;

	/* PF0-PF5 are SDC0; PF2 (CLK) uses the stronger board-DTS setting. */
	value = readl(A7S_MMC0_PF_CFG);
	value = (value & ~0x00ffffffU) | 0x00222222U;
	writel(value, A7S_MMC0_PF_CFG);

	value = readl(A7S_MMC0_PF_DRV);
	value = (value & ~0x00ffffffU) | 0x00111211U;
	writel(value, A7S_MMC0_PF_DRV);

	value = readl(A7S_MMC0_PF_PULL);
	value = (value & ~0x00000fffU) | 0x00000555U;
	writel(value, A7S_MMC0_PF_PULL);
}

int mmc_register(int sdc_no, void *mmc)
{
	u32 *timeout = (u32 *)((char *)mmc + 0xe8);

	mmc_devices[sdc_no] = mmc;
	if (!*timeout)
		*timeout = 0xffff;

	if (sdc_no == 0)
		a7s_mmc0_config_pins();
	else if (sdc_no == 2)
		a7s_mmc2_install_clock(mmc);

	if (sdc_no != 0) {
		a7s_mmc_set_media(A7S_MMC_MEDIA_MMC);
		return mmc_init(mmc);
	}

	/* Keep the vendor SD -> MMC fallback order for the removable slot. */
	a7s_mmc_set_media(A7S_MMC_MEDIA_SD);
	if (mmc_init(mmc) >= 0)
		return 0;
	a7s_mmc_set_media(A7S_MMC_MEDIA_MMC);
	return mmc_init(mmc);
}
