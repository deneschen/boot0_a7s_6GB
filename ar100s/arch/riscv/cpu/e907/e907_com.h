/*
 * c906_com.h
 *
 *  Created on: 2020-6-29
 *      Author: Administrator
 */

#ifndef TERRACE_INCLUDE_C906_COM_H_
#define TERRACE_INCLUDE_C906_COM_H_

#include "include.h"

#ifndef __RV64_REV
#define __RV64_REV               0x0000U
#endif

#ifndef __VIC_PRIO_BITS
#define __VIC_PRIO_BITS           2U
#endif

#ifndef __Vendor_SysTickConfig
#define __Vendor_SysTickConfig    1U
#endif

#ifndef __MPU_PRESENT
#define __MPU_PRESENT             1U
#endif

#ifndef __ICACHE_PRESENT
#define __ICACHE_PRESENT          1U
#endif

#ifndef __DCACHE_PRESENT
#define __DCACHE_PRESENT          0U
#endif

#ifndef __L2CACHE_PRESENT
#define __L2CACHE_PRESENT          0U
#endif

typedef void (*irqhandle)(void);
typedef struct {
//    u32 RESERVED0;                 /*!< Offset: 0x000 (R/W)  CLINT configure register */
	volatile u32 PLIC_PRIO[1024];
	volatile u32 PLIC_IP[32];
	u32 RESERVED1[3972 / 4 - 1];
	volatile u32 PLIC_H0_MIE[32];
	volatile u32 PLIC_H0_SIE[32];
	volatile u32 PLIC_H1_MIE[32];
	volatile u32 PLIC_H1_SIE[32];
	volatile u32 PLIC_H2_MIE[32];
	volatile u32 PLIC_H2_SIE[32];
	volatile u32 PLIC_H3_MIE[32];
	volatile u32 PLIC_H3_SIE[32];
	u32 RESERVED2[(0x01FFFFC - 0x00023FC) / 4 - 1];
	volatile u32 PLIC_PER;
	volatile u32 PLIC_H0_MTH;
	volatile u32 PLIC_H0_MCLAIM;
	u32 RESERVED3[0xFFC / 4 - 1];
	volatile u32 PLIC_H0_STH;
	volatile u32 PLIC_H0_SCLAIM;
	u32 RESERVED4[0xFFC / 4 - 1];

	volatile u32 PLIC_H1_MTH;
	volatile u32 PLIC_H1_MCLAIM;
	u32 RESERVED5[0xFFC / 4 - 1];
	volatile u32 PLIC_H1_STH;
	volatile u32 PLIC_H1_SCLAIM;
	u32 RESERVED6[0xFFC / 4 - 1];

	volatile u32 PLIC_H2_MTH;
	volatile u32 PLIC_H2_MCLAIM;
	u32 RESERVED7[0xFFC / 4 - 1];
	volatile u32 PLIC_H2_STH;
	volatile u32 PLIC_H2_SCLAIM;
	u32 RESERVED8[0xFFC / 4 - 1];

	volatile u32 PLIC_H3_MTH;
	volatile u32 PLIC_H3_MCLAIM;
	u32 RESERVED9[0xFFC / 4 - 1];
	volatile u32 PLIC_H3_STH;
	volatile u32 PLIC_H3_SCLAIM;
	u32 RESERVED10[0xFFC / 4 - 1];
} PLIC_Type;

/**
 \ingroup    CSI_core_register
 \defgroup   CSI_PMP Physical Memory Protection (PMP)
 \brief      Type definitions for the PMP Registers
 @{
 */

#define PMP_PMPCFG_R_Pos                       0U                                    /*!< PMP PMPCFG: R Position */
#define PMP_PMPCFG_R_Msk                       (0x1UL << PMP_PMPCFG_R_Pos)           /*!< PMP PMPCFG: R Mask */

#define PMP_PMPCFG_W_Pos                       1U                                    /*!< PMP PMPCFG: W Position */
#define PMP_PMPCFG_W_Msk                       (0x1UL << PMP_PMPCFG_W_Pos)           /*!< PMP PMPCFG: W Mask */

#define PMP_PMPCFG_X_Pos                       2U                                    /*!< PMP PMPCFG: X Position */
#define PMP_PMPCFG_X_Msk                       (0x1UL << PMP_PMPCFG_X_Pos)           /*!< PMP PMPCFG: X Mask */

#define PMP_PMPCFG_A_Pos                       3U                                    /*!< PMP PMPCFG: A Position */
#define PMP_PMPCFG_A_Msk                       (0x3UL << PMP_PMPCFG_A_Pos)           /*!< PMP PMPCFG: A Mask */

#define PMP_PMPCFG_L_Pos                       7U                                    /*!< PMP PMPCFG: L Position */
#define PMP_PMPCFG_L_Msk                       (0x1UL << PMP_PMPCFG_L_Pos)           /*!< PMP PMPCFG: L Mask */

typedef enum {
	REGION_SIZE_4B = -1,
	REGION_SIZE_8B = 0,
	REGION_SIZE_16B = 1,
	REGION_SIZE_32B = 2,
	REGION_SIZE_64B = 3,
	REGION_SIZE_128B = 4,
	REGION_SIZE_256B = 5,
	REGION_SIZE_512B = 6,
	REGION_SIZE_1KB = 7,
	REGION_SIZE_2KB = 8,
	REGION_SIZE_4KB = 9,
	REGION_SIZE_8KB = 10,
	REGION_SIZE_16KB = 11,
	REGION_SIZE_32KB = 12,
	REGION_SIZE_64KB = 13,
	REGION_SIZE_128KB = 14,
	REGION_SIZE_256KB = 15,
	REGION_SIZE_512KB = 16,
	REGION_SIZE_1MB = 17,
	REGION_SIZE_2MB = 18,
	REGION_SIZE_4MB = 19,
	REGION_SIZE_8MB = 20,
	REGION_SIZE_16MB = 21,
	REGION_SIZE_32MB = 22,
	REGION_SIZE_64MB = 23,
	REGION_SIZE_128MB = 24,
	REGION_SIZE_256MB = 25,
	REGION_SIZE_512MB = 26,
	REGION_SIZE_1GB = 27,
	REGION_SIZE_2GB = 28,
	REGION_SIZE_4GB = 29,
	REGION_SIZE_8GB = 30,
	REGION_SIZE_16GB = 31
} region_size_e;

typedef enum {
	ADDRESS_MATCHING_TOR = 1, ADDRESS_MATCHING_NAPOT = 3
} address_matching_e;

typedef struct {
	u32 r :1; /* readable enable */
	u32 w :1; /* writeable enable */
	u32 x :1; /* execable enable */
	address_matching_e a : 2; /* address matching mode */
	u32 reserved :2; /* reserved */
	u32 l :1; /* lock enable */
} mpu_region_attr_t;

/*@} end of group CSI_PMP */

/* CACHE Register Definitions */
#define CACHE_MHCR_L0BTB_Pos                   12U                                           /*!< CACHE MHCR: L0BTB Position */
#define CACHE_MHCR_L0BTB_Msk                   (0x1UL << CACHE_MHCR_L0BTB_Pos)               /*!< CACHE MHCR: WA Mask */

#define CACHE_MHCR_BPE_Pos                     5U                                            /*!< CACHE MHCR: BPE Position */
#define CACHE_MHCR_BPE_Msk                     (0x1UL << CACHE_MHCR_BPE_Pos)                 /*!< CACHE MHCR: BPE Mask */

#define CACHE_MHCR_RS_Pos                      4U                                            /*!< CACHE MHCR: RS Position */
#define CACHE_MHCR_RS_Msk                      (0x1UL << CACHE_MHCR_RS_Pos)                  /*!< CACHE MHCR: RS Mask */

#define CACHE_MHCR_WB_Pos                      3U                                            /*!< CACHE MHCR: WB Position */
#define CACHE_MHCR_WB_Msk                      (0x1UL << CACHE_MHCR_WB_Pos)                  /*!< CACHE MHCR: WB Mask */

#define CACHE_MHCR_WA_Pos                      2U                                            /*!< CACHE MHCR: WA Position */
#define CACHE_MHCR_WA_Msk                      (0x1UL << CACHE_MHCR_WA_Pos)                  /*!< CACHE MHCR: WA Mask */

#define CACHE_MHCR_DE_Pos                      1U                                            /*!< CACHE MHCR: DE Position */
#define CACHE_MHCR_DE_Msk                      (0x1UL << CACHE_MHCR_DE_Pos)                  /*!< CACHE MHCR: DE Mask */

#define CACHE_MHCR_IE_Pos                      0U                                            /*!< CACHE MHCR: IE Position */
#define CACHE_MHCR_IE_Msk                      (0x1UL << CACHE_MHCR_IE_Pos)                  /*!< CACHE MHCR: IE Mask */

#define CACHE_INV_ADDR_Pos                     5U
#define CACHE_INV_ADDR_Msk                     (0xFFFFFFFFUL << CACHE_INV_ADDR_Pos)

/*@} end of group CSI_CACHE */

/**
 \ingroup  CSI_core_register
 \defgroup CSI_SysTick     System Tick Timer (CORET)
 \brief    Type definitions for the System Timer Registers.
 @{
 */

/**
 \brief  The data structure of the access system timer.
 */
typedef struct {
	volatile u32 MSIP0;
	volatile u32 MSIP1;
	volatile u32 MSIP2;
	volatile u32 MSIP3;
	u32 RESERVED0[(0x4004000 - 0x400000C) / 4 - 1];
	volatile u32 MTIMECMPL0;
	volatile u32 MTIMECMPH0;
	volatile u32 MTIMECMPL1;
	volatile u32 MTIMECMPH1;
	volatile u32 MTIMECMPL2;
	volatile u32 MTIMECMPH2;
	volatile u32 MTIMECMPL3;
	volatile u32 MTIMECMPH3;
	u32 RESERVED1[(0x400C000 - 0x400401C) / 4 - 1];
	volatile u32 SSIP0;
	volatile u32 SSIP1;
	volatile u32 SSIP2;
	volatile u32 SSIP3;
	u32 RESERVED2[(0x400D000 - 0x400C00C) / 4 - 1];
	volatile u32 STIMECMPL0;
	volatile u32 STIMECMPH0;
	volatile u32 STIMECMPL1;
	volatile u32 STIMECMPH1;
	volatile u32 STIMECMPL2;
	volatile u32 STIMECMPH2;
	volatile u32 STIMECMPL3;
	volatile u32 STIMECMPH3;
} CORET_Type;
/*@} end of group CSI_SysTick */

/**
 \ingroup    CSI_core_register
 \defgroup   CSI_core_bitfield     Core register bit field macros
 \brief      Macros for use with bit field definitions (xxx_Pos, xxx_Msk).
 @{
 */

/**
 \brief   Mask and shift a bit field value for use in a register bit range.
 \param[in] field  Name of the register bit field.
 \param[in] value  Value of the bit field.
 \return           Masked and shifted value.
 */
#define _VAL2FLD(field, value)    ((value << field ## _Pos) & field ## _Msk)

/**
 \brief     Mask and shift a register value to extract a bit filed value.
 \param[in] field  Name of the register bit field.
 \param[in] value  Value of register.
 \return           Masked and shifted bit field value.
 */
#define _FLD2VAL(field, value)    ((value & field ## _Msk) >> field ## _Pos)

/*@} end of group CSI_core_bitfield */

#endif /* TERRACE_INCLUDE_C906_COM_H_ */
