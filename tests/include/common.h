#ifndef _A7S_CLOCK_TEST_COMMON_H_
#define _A7S_CLOCK_TEST_COMMON_H_

#include <stdint.h>
#include <stdio.h>

uint32_t test_readl(unsigned long address);
void test_writel(uint32_t value, unsigned long address);
void udelay(unsigned long usec);
int test_printf(const char *format, ...);

#define readl(address) test_readl((unsigned long)(address))
#define writel(value, address) test_writel((uint32_t)(value), \
					  (unsigned long)(address))
#define printf(format, ...) test_printf((format), ##__VA_ARGS__)

#endif
