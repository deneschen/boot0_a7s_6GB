#!/usr/bin/env python3
"""Run the actual SDC2 callback against fake MMIO, including clock timeout."""
import pathlib
import hashlib
import subprocess
import tempfile

source = pathlib.Path(__file__).resolve().parents[1] / 'src/mmc_a733.c'
blob = source.parents[1] / 'blobs/board_sdcard.o'
assert hashlib.sha256(blob.read_bytes()).hexdigest() == (
    'e0eedaf36da36fae7c64af00c210f7b890d1f4bb142a6ab7bb04807cf44a83b2'
), 'MMC blob changed: recheck callback and field offsets before using the shim'
with tempfile.TemporaryDirectory() as tmp:
    tmp = pathlib.Path(tmp)
    (tmp / 'common.h').write_text('''
#include <stdint.h>
#include <stdio.h>
typedef uint32_t u32;
u32 readl(u32 address);
void writel(u32 value, u32 address);
u32 timer_get_us(void);
''')
    (tmp / 'test.c').write_text('''
#include <assert.h>
#include <string.h>
#include "common.h"
static u32 regs[256], ccu, ticks;
static int stuck, drive_writes;
u32 timer_get_us(void) { return ticks += 1000; }
u32 readl(u32 a) {
    return a == 0x02002d20 ? ccu : regs[(a - 0x04022000) / 4];
}
void writel(u32 v, u32 a) {
    if (a == 0x02002d20) { ccu = v; return; }
    if (a == 0x04022140) { assert(!(ccu >> 31)); drive_writes++; }
    if (a == 0x04022018 && !stuck) v &= ~(1U << 31);
    regs[(a - 0x04022000) / 4] = v;
}
''' + '#include "' + str(source) + '"\n' + '''
int main(void) {
    union { void *align; unsigned char bytes[280]; } m = {0}, h = {0};
    void *mmc = m.bytes;
    *(void **)(m.bytes + 0x20) = h.bytes;
    a7s_mmc2_install_clock(mmc);
    assert(MMC_WORD(mmc, 0x34) == 400000);
    assert(MMC_WORD(mmc, 0x38) == 12000000);
    /* memcpy avoids an unaligned 64-bit function-pointer load in this
     * host test; the actual ARM32 blob field at 0xac is 4-byte aligned. */
    void (*callback)(void *);
    memcpy(&callback, m.bytes + 0xac, sizeof(callback));
    MMC_WORD(mmc, 0x44) = 1;
    MMC_WORD(mmc, 0x48) = 400000;
    callback(mmc);
    assert(ccu == 0x8000001d); /* SYS24M / 30 / 2 = 400 kHz */
    assert(readl(REG(0x04)) == 0x10000);
    assert(readl(REG(0x104)) & 1);
    assert(MMC_WORD(h.bytes, 0x1c) == 0);
    MMC_WORD(mmc, 0x48) = 12000000;
    MMC_WORD(mmc, 0x44) = 8;
    MMC_WORD(mmc, 0x104) = 1;
    callback(mmc);
    assert(ccu == 0x80000000);
    assert(readl(REG(0x0c)) == 2);
    assert(drive_writes == 2);
    MMC_WORD(mmc, 0x48) = 0;
    callback(mmc);
    assert(readl(REG(0x04)) == 0);
    MMC_WORD(mmc, 0x48) = 26000000;
    callback(mmc);
    assert(MMC_WORD(h.bytes, 0x1c) == 1);
    MMC_WORD(h.bytes, 0x1c) = 0;
    MMC_WORD(mmc, 0x48) = 400000;
    ticks = 0xffff0000; /* Timeout arithmetic must survive wraparound. */
    stuck = 1;
    callback(mmc);
    assert(MMC_WORD(h.bytes, 0x1c) == 1);
    puts("A733 SDC2 clock callback: PASS");
}
''')
    subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-I', str(tmp),
                    str(tmp / 'test.c'), '-o', str(tmp / 'test')], check=True)
    subprocess.run([str(tmp / 'test')], check=True)
