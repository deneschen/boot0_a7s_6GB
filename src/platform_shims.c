/*
 * Platform hooks needed by the closed A733 DRAM library.
 *
 * The supplied A733 board blob is an FPGA build and has no AXP8191 support, so
 * the real board GPIO and DRAM-rail setup live here.
 */
#include <common.h>
#include <arch/gpio.h>
#include <axp8191_voltage.h>

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

#define AXP8191_DEVICE_ADDR	400000
#define AXP8191_RUNTIME_ADDR	0x36
#define AXP8191_CHIP_ID		0x0e
#define AXP8191_CHIP_ID_A	0x03
#define AXP8191_DCDC_CTRL1	0x10
#define AXP8191_DCDC_CTRL2	0x11
#define AXP8191_DC1SW2_ENABLE	(1U << 4)
#define AXP8191_DCDC6_VOL	0x17
#define AXP8191_DCDC7_VOL	0x18
#define AXP8191_DCDC8_VOL	0x19
#define AXP8191_LDO_CTRL3	0x22
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
};

int pmic_bus_init(u32 device_addr, u32 runtime_addr);
int pmic_bus_read(u8 runtime_addr, u8 reg, u8 *value);
int pmic_bus_write(u8 runtime_addr, u8 reg, u8 value);

static int a7s_axp8191_ready;

extern void *mmc_devices[];
int mmc_init(void *mmc);

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

int sunxi_board_init(void)
{
	u32 value = readl(0x08020000);

	/* Preserve the SoC setup performed by the supplied FPGA board object. */
	writel(value | 1U, 0x08020000);
	if (a7s_axp8191_init())
		return -1;

	return a7s_axp8191_enable_sd_power();
}

int set_ddr_voltage_ext(char *name, int set_vol, int onoff)
{
	return a7s_axp8191_set_rail(name, set_vol, onoff);
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

	return mmc_init(mmc);
}
