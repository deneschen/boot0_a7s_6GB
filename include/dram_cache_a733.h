/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _BOOT0_DRAM_CACHE_A733_H_
#define _BOOT0_DRAM_CACHE_A733_H_

/*
 * Per-board DRAM training cache.
 *
 * libdram (V0.601) runs a full 2D training of the 2400 MHz Pstate on every
 * boot unless the DRAM parameters carry the result of an earlier training.
 * The vendor chain keeps that result in its flash bootparam area; this board
 * keeps it in two raw sectors on the boot medium.  boot0 only reads them;
 * the Linux tool a7s-dram-cache writes them.
 *
 * Both sides must agree on this layout.  All fields are little-endian u32.
 */

#define A7S_DRAM_CACHE_LBA		24320U	/* slot 0; slot 1 follows */
#define A7S_DRAM_CACHE_SLOTS		2U
#define A7S_DRAM_CACHE_VERSION		2U
#define A7S_DRAM_CACHE_MAGIC0		0x44533741U	/* "A7SD" */
#define A7S_DRAM_CACHE_MAGIC1		0x434d4152U	/* "RAMC" */
#define A7S_DRAM_CACHE_FLAG_DISABLED	0x1U

/* dram_para words that hold the training result (save_phy_2d_training_result). */
#define A7S_DRAM_PARA_BASE_WORDS	32U
#define A7S_DRAM_TRAINED_FIRST		32U
#define A7S_DRAM_TRAINED_WORDS		20U
#define A7S_DRAM_PARA_TPR1		23U
#define A7S_DRAM_PARA_TPR10		27U
/* tpr1 bit31: use the stored result (vendor sunxi_bootparam_set_dram_flag). */
#define A7S_DRAM_TPR1_SKIP_TRAINING	0x80000000U
/* tpr10 bit29: run 2D training and store the result. */
#define A7S_DRAM_TPR10_TRAIN_2D		0x20000000U

struct a7s_dram_cache_record {
	unsigned int magic[2];
	unsigned int version;
	unsigned int sequence;
	unsigned int flags;
	unsigned int fail_count;
	unsigned int chipid_crc;	/* CRC32 of the four SID chip id words */
	unsigned int reserved[3];
	unsigned int base_crc;		/* CRC32 of the boot0 header dram_para[32] */
	unsigned int trained_count;
	unsigned int trained[A7S_DRAM_TRAINED_WORDS];
	unsigned int reserved2[95];
	unsigned int crc;		/* CRC32 of all preceding bytes */
};

/*
 * Boot state reported to Linux in RTC general-purpose register 5:
 * A7S_DRAM_STATE_MAGIC | card << 8 | state.
 */
#define A7S_DRAM_STATE_MAGIC		0xa7dc0000U
#define A7S_DRAM_STATE_MAGIC_MASK	0xffff0000U
enum a7s_dram_cache_state {
	A7S_DRAM_CACHE_ATTEMPT = 1,	/* cached init running (watchdog armed) */
	A7S_DRAM_CACHE_HIT = 2,		/* trained with the cached result */
	A7S_DRAM_CACHE_MISS = 3,	/* no usable record: full training */
	A7S_DRAM_CACHE_FALLBACK = 4,	/* last cached init failed: full training */
	A7S_DRAM_CACHE_DISABLED = 5,	/* record disables the cache */
};

/*
 * RTC general-purpose registers, at SUNXI_RTC_BASE + 0x100 + n * 4.  boot0
 * writes them so that Linux can build a record that this boot0 accepts
 * without having to read the SID or the boot0 header itself.
 *
 * GPR3 is the vendor record cell: fip_handoff.S puts the AR100 device tree
 * address there, and the AR100 daemon later overwrites it with a state flag.
 * The kernel RTC driver owns GPR6 and GPR7, and GPR0 is borrowed by its
 * register access check, so the cache uses GPR1, GPR4 and GPR5.
 */
#define A7S_DRAM_GPR_CHIPID_CRC	1U	/* GPR1: CRC32 of the SID chip id */
#define A7S_DRAM_GPR_BASE_CRC	4U	/* GPR4: CRC32 of the header dram_para */
#define A7S_DRAM_GPR_STATE	5U	/* GPR5: magic | card << 8 | state */

unsigned int a7s_dram_cache_crc32(const void *data, unsigned int length);

/*
 * Pick the newest valid slot.  Returns its index, or -1 when no slot holds
 * a well-formed record.  It does not check the chip or the base parameters.
 */
int a7s_dram_cache_newest(const struct a7s_dram_cache_record *slots,
			  unsigned int count);

/*
 * Decide what the record means for this boot.  Returns A7S_DRAM_CACHE_HIT
 * when the record can be applied, A7S_DRAM_CACHE_DISABLED, or
 * A7S_DRAM_CACHE_MISS.
 */
enum a7s_dram_cache_state
a7s_dram_cache_check(const struct a7s_dram_cache_record *record,
		     unsigned int chipid_crc, unsigned int base_crc);

/* Load the cached result into a dram_para array of at least 52 words. */
void a7s_dram_cache_apply(unsigned int *dram_para,
			  const struct a7s_dram_cache_record *record);

/* boot0 glue (platform_shims.c): call right after init_DRAM(). */
void a7s_dram_cache_finish(int dram_size);

#endif
