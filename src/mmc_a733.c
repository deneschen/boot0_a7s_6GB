/* A733 SDC2 SDR clock setup for the bundled FPGA MMC core. */
#include <common.h>

#define SDC2_BASE 0x04022000U
#define SDC2_CLK  0x02002d20U
#define REG(off) (SDC2_BASE + (off))

/* Offsets are the ABI of blobs/board_sdcard.o, not U-Boot's struct mmc. */
#define MMC_WORD(mmc, off) (*(u32 *)((char *)(mmc) + (off)))

static int a7s_mmc2_update_clock(u32 clock_control)
{
	u32 start = timer_get_us();

	/* Mask DAT0 only while latching the clock; never enable power saving. */
	writel(clock_control | (1U << 31), REG(0x04));
	writel(0x80202000, REG(0x18));
	while (readl(REG(0x18)) & (1U << 31)) {
		if ((u32)(timer_get_us() - start) >= 100000) {
			printf("A7S SDC2: clock update timeout\n");
			return -1;
		}
	}
	writel(readl(REG(0x38)), REG(0x38));
	writel(clock_control, REG(0x04));
	return 0;
}

static void a7s_mmc2_set_ios(void *mmc)
{
	u32 requested = MMC_WORD(mmc, 0x48);
	u32 width = MMC_WORD(mmc, 0x44);
	u32 divider, value;
	void *host = *(void **)((char *)mmc + 0x20);

	if (a7s_mmc2_update_clock(0))
		goto failed;
	if (!requested)
		return;
	/* This boot-only path supports SDR at 400 kHz..12 MHz from SYS24M.
	 * CCU M is linear, and TM4 has an additional fixed /2 divider.
	 * No FPGA clock assumptions or unverified peripheral PLL rate.
	 */
	if (requested < 400000 || requested > 12000000 ||
	    MMC_WORD(mmc, 0x104) > 1 ||
	    (width != 1 && width != 4 && width != 8))
		goto failed;
	divider = (12000000 + requested - 1) / requested;
	writel(0, SDC2_CLK);
	writel(0x80000000U | (divider - 1), SDC2_CLK);
	writel(readl(REG(0x5c)) & ~(1U << 31), REG(0x5c));
	writel(readl(REG(0x00)) & ~(1U << 10), REG(0x00));
	writel(readl(REG(0x10c)) & ~(1U << 31), REG(0x10c));
	writel(width == 8 ? 2 : width == 4 ? 1 : 0, REG(0x0c));
	/* CMD 180 degrees, DAT 90 degrees; FPGA select bit 7 must be clear. */
	value = readl(REG(0x140)) & ~((3U << 16) | (1U << 7));
	/* Like the vendor sunxi_r_op: latch drive phase with module clock off. */
	writel(divider - 1, SDC2_CLK);
	writel(value | (1U << 16), REG(0x140));
	writel(0x80000000U | (divider - 1), SDC2_CLK);
	writel((readl(REG(0x144)) & ~0x3fU) | 0x80, REG(0x144));
	/* Vendor A733 SDR workaround: sample FIFO bypass + continuous clock. */
	writel(readl(REG(0x104)) | 1, REG(0x104));
	if (a7s_mmc2_update_clock(1U << 16))
		goto failed;
	MMC_WORD(mmc, 0x48) = 12000000 / divider;
	printf("A7S SDC2: req=%u actual=%u width=%u CCU=%08x CLK=%08x SFC=%08x\n",
	       requested, MMC_WORD(mmc, 0x48), width, readl(SDC2_CLK),
	       readl(REG(0x04)), readl(REG(0x104)));
	return;
failed:
	/* The blob's send_cmd checks host->fatal_err before issuing commands. */
	MMC_WORD(host, 0x1c) = 1;
	printf("A7S SDC2: rejected clock setup req=%u width=%u\n", requested, width);
}

void a7s_mmc2_install_clock(void *mmc)
{
	MMC_WORD(mmc, 0x34) = 400000;
	MMC_WORD(mmc, 0x38) = 12000000;
	MMC_WORD(mmc, 0x3c) = 12000000;
	*(void (**)(void *))((char *)mmc + 0xac) = a7s_mmc2_set_ios;
}
