/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <dram_cache_a733.h>

typedef char a7s_dram_cache_record_is_one_sector
	[sizeof(struct a7s_dram_cache_record) == 512 ? 1 : -1];

/* Standard reflected CRC32 (zlib.crc32), bitwise to keep boot0 small. */
unsigned int a7s_dram_cache_crc32(const void *data, unsigned int length)
{
	const unsigned char *p = data;
	unsigned int crc = 0xffffffffU;
	unsigned int bit;

	while (length--) {
		crc ^= *p++;
		for (bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ (0xedb88320U & -(crc & 1U));
	}
	return ~crc;
}

static int a7s_dram_cache_valid(const struct a7s_dram_cache_record *record)
{
	return record->magic[0] == A7S_DRAM_CACHE_MAGIC0 &&
	       record->magic[1] == A7S_DRAM_CACHE_MAGIC1 &&
	       record->version == A7S_DRAM_CACHE_VERSION &&
	       record->trained_count == A7S_DRAM_TRAINED_WORDS &&
	       record->crc == a7s_dram_cache_crc32(record,
				sizeof(*record) - sizeof(record->crc));
}

int a7s_dram_cache_newest(const struct a7s_dram_cache_record *slots,
			  unsigned int count)
{
	int newest = -1;
	unsigned int index;

	for (index = 0; index < count; index++) {
		if (!a7s_dram_cache_valid(&slots[index]))
			continue;
		/* Serial-number comparison: a wrapped sequence is still newer. */
		if (newest < 0 || (int)(slots[index].sequence -
					slots[newest].sequence) > 0)
			newest = index;
	}
	return newest;
}

enum a7s_dram_cache_state
a7s_dram_cache_check(const struct a7s_dram_cache_record *record,
		     unsigned int chipid_crc, unsigned int base_crc)
{
	/*
	 * The identity checks come first: a record from another board or from
	 * another boot0 parameter set must not be able to switch the cache off.
	 */
	if (record->base_crc != base_crc)
		return A7S_DRAM_CACHE_MISS;
	if (record->chipid_crc != chipid_crc)
		return A7S_DRAM_CACHE_MISS;
	if (record->flags & A7S_DRAM_CACHE_FLAG_DISABLED)
		return A7S_DRAM_CACHE_DISABLED;
	return A7S_DRAM_CACHE_HIT;
}

void a7s_dram_cache_apply(unsigned int *dram_para,
			  const struct a7s_dram_cache_record *record)
{
	unsigned int index;

	for (index = 0; index < A7S_DRAM_TRAINED_WORDS; index++)
		dram_para[A7S_DRAM_TRAINED_FIRST + index] = record->trained[index];
	/*
	 * Both bits are needed.  With tpr10 bit29 clear the PHY skips 2D
	 * training and mctl_phy_init() replays words 32-49.  tpr1 bit31 makes
	 * lpddr5_phyinit_F_loadDMEM() program VrefDQ: from words 50-51 for
	 * Pstate 0 and from mr14 for the others.  Without it the message block
	 * VrefDQ stays unset and the Pstate 1 training never completes.
	 */
	dram_para[A7S_DRAM_PARA_TPR1] |= A7S_DRAM_TPR1_SKIP_TRAINING;
	dram_para[A7S_DRAM_PARA_TPR10] &= ~A7S_DRAM_TPR10_TRAIN_2D;
}
