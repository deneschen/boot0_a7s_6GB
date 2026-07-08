/*
 * Minimal A733/A7S UART0 bring-up for the first bytes of boot0.
 *
 * This intentionally does not depend on the closed board blob's serial init:
 * the blob was built with hard-coded UART pins, while Cubie A7S uses PB9/PB10.
 */

#include <common.h>
#include <arch/clock.h>
#include <arch/uart.h>

#define A7S_UART0_TX_BANK	1
#define A7S_UART0_TX_PIN	9
#define A7S_UART0_RX_BANK	1
#define A7S_UART0_RX_PIN	10
#define A7S_UART0_MUX		2
#define A7S_GPIO_PULL_UP	1
#define A7S_GPIO_DRV_LEVEL	1

/* A733 main PIO uses the V3 register layout. */
#define A7S_GPIO_BANK_SIZE	0x80
#define A7S_GPIO_CFG_BASE	0x80
#define A7S_GPIO_DRV_BASE	0xa0
#define A7S_GPIO_PULL_BASE	0xb0

#define A7S_UART_THR		0x00
#define A7S_UART_DLL		0x00
#define A7S_UART_DLH		0x04
#define A7S_UART_IER		0x04
#define A7S_UART_FCR		0x08
#define A7S_UART_LCR		0x0c
#define A7S_UART_LSR		0x14

#define A7S_UART_LCR_DLAB	0x80
#define A7S_UART_LCR_8N1	0x03
#define A7S_UART_FCR_ENABLE	0x01
#define A7S_UART_FCR_RXRST	0x02
#define A7S_UART_FCR_TXRST	0x04
#define A7S_UART_LSR_THRE	0x20

static void a7s_gpio_set_cfg(unsigned int bank, unsigned int pin, unsigned int mux)
{
	unsigned long reg = SUNXI_PIO_BASE + bank * A7S_GPIO_BANK_SIZE +
			    A7S_GPIO_CFG_BASE + ((pin >> 3) << 2);
	unsigned int shift = (pin & 0x7) << 2;
	unsigned int val = readl(reg);

	val &= ~(0xfU << shift);
	val |= (mux & 0xfU) << shift;
	writel(val, reg);
}

static void a7s_gpio_set_pull(unsigned int bank, unsigned int pin, unsigned int pull)
{
	unsigned long reg = SUNXI_PIO_BASE + bank * A7S_GPIO_BANK_SIZE +
			    A7S_GPIO_PULL_BASE + ((pin >> 4) << 2);
	unsigned int shift = (pin & 0xf) << 1;
	unsigned int val = readl(reg);

	val &= ~(0x3U << shift);
	val |= (pull & 0x3U) << shift;
	writel(val, reg);
}

static void a7s_gpio_set_drv(unsigned int bank, unsigned int pin, unsigned int drv)
{
	unsigned long reg = SUNXI_PIO_BASE + bank * A7S_GPIO_BANK_SIZE +
			    A7S_GPIO_DRV_BASE + ((pin >> 3) << 2);
	unsigned int shift = (pin & 0x7) << 2;
	unsigned int val = readl(reg);

	val &= ~(0x3U << shift);
	val |= (drv & 0x3U) << shift;
	writel(val, reg);
}

static void a7s_uart0_clock_init(void)
{
	unsigned int val = readl(CCMU_UART_BGR_REG);

	val &= ~(1U << CCM_UART_RST_OFFSET);
	writel(val, CCMU_UART_BGR_REG);

	val |= (1U << CCM_UART_RST_OFFSET);
	writel(val, CCMU_UART_BGR_REG);

	val &= ~(1U << CCM_UART_GATING_OFFSET);
	writel(val, CCMU_UART_BGR_REG);

	val |= (1U << CCM_UART_GATING_OFFSET);
	writel(val, CCMU_UART_BGR_REG);
}

void a7s_early_uart_init(void)
{
	a7s_gpio_set_cfg(A7S_UART0_TX_BANK, A7S_UART0_TX_PIN, A7S_UART0_MUX);
	a7s_gpio_set_cfg(A7S_UART0_RX_BANK, A7S_UART0_RX_PIN, A7S_UART0_MUX);
	a7s_gpio_set_pull(A7S_UART0_TX_BANK, A7S_UART0_TX_PIN, A7S_GPIO_PULL_UP);
	a7s_gpio_set_pull(A7S_UART0_RX_BANK, A7S_UART0_RX_PIN, A7S_GPIO_PULL_UP);
	a7s_gpio_set_drv(A7S_UART0_TX_BANK, A7S_UART0_TX_PIN, A7S_GPIO_DRV_LEVEL);
	a7s_gpio_set_drv(A7S_UART0_RX_BANK, A7S_UART0_RX_PIN, A7S_GPIO_DRV_LEVEL);

	a7s_uart0_clock_init();

	writel(0, SUNXI_UART0_BASE + A7S_UART_IER);
	writel(A7S_UART_LCR_DLAB, SUNXI_UART0_BASE + A7S_UART_LCR);
	writel(13, SUNXI_UART0_BASE + A7S_UART_DLL);
	writel(0, SUNXI_UART0_BASE + A7S_UART_DLH);
	writel(A7S_UART_LCR_8N1, SUNXI_UART0_BASE + A7S_UART_LCR);
	writel(A7S_UART_FCR_ENABLE | A7S_UART_FCR_RXRST | A7S_UART_FCR_TXRST,
	       SUNXI_UART0_BASE + A7S_UART_FCR);
}

static void a7s_early_uart_putc(char c)
{
	unsigned int timeout = 100000;

	while (timeout--) {
		if (readl(SUNXI_UART0_BASE + A7S_UART_LSR) & A7S_UART_LSR_THRE)
			break;
	}

	writel((unsigned int)c, SUNXI_UART0_BASE + A7S_UART_THR);
}

void a7s_early_uart_puts(const char *s)
{
	char prev = 0;

	while (*s) {
		if (*s == '\n' && prev != '\r')
			a7s_early_uart_putc('\r');
		a7s_early_uart_putc(*s);
		prev = *s++;
	}
}
