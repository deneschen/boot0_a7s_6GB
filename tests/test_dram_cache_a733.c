#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <dram_cache_a733.h>

#define CHIPID_CRC 0x89abcdefU
#define BASE_CRC 0x12345678U

static void seal(struct a7s_dram_cache_record *record)
{
	record->crc = a7s_dram_cache_crc32(record,
				sizeof(*record) - sizeof(record->crc));
}

static void make_record(struct a7s_dram_cache_record *record,
			unsigned int sequence)
{
	unsigned int index;

	memset(record, 0, sizeof(*record));
	record->magic[0] = A7S_DRAM_CACHE_MAGIC0;
	record->magic[1] = A7S_DRAM_CACHE_MAGIC1;
	record->version = A7S_DRAM_CACHE_VERSION;
	record->sequence = sequence;
	record->chipid_crc = CHIPID_CRC;
	record->base_crc = BASE_CRC;
	record->trained_count = A7S_DRAM_TRAINED_WORDS;
	for (index = 0; index < A7S_DRAM_TRAINED_WORDS; index++)
		record->trained[index] = 0x30303030U + index;
	seal(record);
}

static void test_crc32(void)
{
	/* zlib.crc32(b"123456789"); the Linux writer uses zlib. */
	assert(a7s_dram_cache_crc32("123456789", 9) == 0xcbf43926U);
	assert(a7s_dram_cache_crc32("", 0) == 0);
}

static void test_layout(void)
{
	/* Offsets shared with debian-a7s/tools/system/a7s-dram-cache. */
	assert(sizeof(struct a7s_dram_cache_record) == 512);
	assert(offsetof(struct a7s_dram_cache_record, chipid_crc) == 0x18);
	assert(offsetof(struct a7s_dram_cache_record, base_crc) == 0x28);
	assert(offsetof(struct a7s_dram_cache_record, trained) == 0x30);
	assert(offsetof(struct a7s_dram_cache_record, crc) == 0x1fc);
}

static void test_newest(void)
{
	struct a7s_dram_cache_record slots[2];

	make_record(&slots[0], 7);
	make_record(&slots[1], 8);
	assert(a7s_dram_cache_newest(slots, 2) == 1);

	/* A torn write in the newer slot falls back to the older one. */
	slots[1].trained[3] ^= 1;
	assert(a7s_dram_cache_newest(slots, 2) == 0);

	/* Sequence numbers compare across the 32-bit wrap. */
	make_record(&slots[0], 0xffffffffU);
	make_record(&slots[1], 0);
	assert(a7s_dram_cache_newest(slots, 2) == 1);

	memset(slots, 0, sizeof(slots));
	assert(a7s_dram_cache_newest(slots, 2) == -1);

	make_record(&slots[0], 1);
	slots[0].version = A7S_DRAM_CACHE_VERSION + 1;
	seal(&slots[0]);
	assert(a7s_dram_cache_newest(slots, 1) == -1);

	make_record(&slots[0], 1);
	slots[0].trained_count = 19;
	seal(&slots[0]);
	assert(a7s_dram_cache_newest(slots, 1) == -1);
}

static void test_check(void)
{
	struct a7s_dram_cache_record record;

	make_record(&record, 1);
	assert(a7s_dram_cache_check(&record, CHIPID_CRC, BASE_CRC) ==
	       A7S_DRAM_CACHE_HIT);
	assert(a7s_dram_cache_check(&record, CHIPID_CRC + 1, BASE_CRC) ==
	       A7S_DRAM_CACHE_MISS);
	assert(a7s_dram_cache_check(&record, CHIPID_CRC, BASE_CRC + 1) ==
	       A7S_DRAM_CACHE_MISS);
	record.flags = A7S_DRAM_CACHE_FLAG_DISABLED;
	assert(a7s_dram_cache_check(&record, CHIPID_CRC, BASE_CRC) ==
	       A7S_DRAM_CACHE_DISABLED);
	/* A record from another board must not switch this board's cache off. */
	assert(a7s_dram_cache_check(&record, CHIPID_CRC + 1, BASE_CRC) ==
	       A7S_DRAM_CACHE_MISS);
	assert(a7s_dram_cache_check(&record, CHIPID_CRC, BASE_CRC + 1) ==
	       A7S_DRAM_CACHE_MISS);
}

static void test_apply(void)
{
	struct a7s_dram_cache_record record;
	unsigned int para[96];
	unsigned int index;

	for (index = 0; index < 96; index++)
		para[index] = 0xdead0000U + index;
	para[A7S_DRAM_PARA_TPR1] = 0;
	para[A7S_DRAM_PARA_TPR10] = 0x325f0000U;

	make_record(&record, 1);
	a7s_dram_cache_apply(para, &record);

	assert(para[A7S_DRAM_PARA_TPR1] == 0x80000000U);
	assert(para[A7S_DRAM_PARA_TPR10] == 0x125f0000U);
	for (index = 0; index < A7S_DRAM_TRAINED_WORDS; index++)
		assert(para[32 + index] == 0x30303030U + index);
	assert(para[31] == 0xdead0000U + 31);
	assert(para[52] == 0xdead0000U + 52);
}

int main(void)
{
	test_crc32();
	test_layout();
	test_newest();
	test_check();
	test_apply();
	puts("DRAM training cache tests: OK");
	return 0;
}
