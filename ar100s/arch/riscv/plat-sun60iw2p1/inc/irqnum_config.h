/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : intc.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-4-27
* Descript: interrupt controller public header.
* Update  : date                auther      ver     notes
*           2012-4-27 10:52:56  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __IRQNUM_CONFIG_H__
#define __IRQNUM_CONFIG_H__

/*
 * ------------------------------------------------------------------------------
 * r_intc interrupt source
 * ------------------------------------------------------------------------------
 */
#define INTC_R_USB_IRQ		16
#define INTC_R_TIMER0_IRQ	20
#define INTC_R_TIMER1_IRQ	21
#define INTC_R_TIMER2_IRQ	22
#define INTC_R_TIMER3_IRQ	23
#define INTC_R_ALM0_IRQ		24
#define INTC_R_GPIOL_S_IRQ	25
#define INTC_R_GPIOL_NS_IRQ	26
#define INTC_R_GPIOM_S_IRQ	27
#define INTC_R_GPIOM_NS_IRQ	28
#define INTC_R_UART_IRQ		29
#define INTC_R_IRRX_IRQ		34
#define IRQ_SOUCE_MAX           (INTC_R_IRRX_IRQ + 1)

/*
 * ------------------------------------------------------------------------------
 * gic interrupt source
 * ------------------------------------------------------------------------------
 */
#define GIC_USB0_EHCI_IRQ	189
#define GIC_USB0_OHCI_IRQ	190
#define GIC_USB1_EHCI_IRQ	191
#define GIC_USB1_OHCI_IRQ	192
#define GIC_R_EXTERNAL_NMI_IRQ	220
#define GIC_R_ALARM0_IRQ	228
#define GIC_R_GPIOL_S_IRQ	229
#define GIC_R_GPIOL_NS_IRQ	230
#define GIC_R_GPIOM_S_IRQ	231
#define GIC_R_GPIOM_NS_IRQ	232
#define GIC_R_IR_IRQ		238

#endif	/*__IRQNUM_CONFIG_H__*/
