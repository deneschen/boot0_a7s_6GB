/*
 * include/cfgs.h
 *
 * Descript: system configure header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: superm <superm@allwinnertech.com>
 *
 */
#ifndef __PLATFORM_CFGS_H__
#define __PLATFORM_CFGS_H__


#define TWI_CLOCK_FREQ		(200 * 1000)	/* the twi source clock freq */
#define TICK_PER_SEC		(100)

#define RSB_RTSADDR_AXP8191	(0x36)
#define RSB_RTSADDR_AXP515	(0x34)

/* uart config */
#define UART_BAUDRATE		(115200 / 2)

/* devices define */
#define ARISC_DTS_SIZE		(0x00100000)
/* E902-visible DRAM window (identity-mapped to the CPUX DRAM view) */
#define ARISC_DRAM_BASE		(0x40000000)
#define ARISC_DRAM_END		(0x80000000)

#endif
