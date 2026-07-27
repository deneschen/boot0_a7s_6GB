#!/usr/bin/env python3
"""Validate the self-contained A7S boot0 image produced by this tree."""

from __future__ import annotations

import argparse
import re
import struct
import sys
from pathlib import Path


BOOT0_MAGIC = b"eGON.BT0"
STAMP_VALUE = 0x5F0A6C39

PUB_HEAD_SIZE_OFF = 0x14
PRVT_HEAD_OFF = 0x30
DEBUG_MODE_OFF = PRVT_HEAD_OFF + 4
POWER_MODE_OFF = PRVT_HEAD_OFF + 5
DRAM_PARA_OFF = PRVT_HEAD_OFF + 8
UART_PORT_OFF = DRAM_PARA_OFF + 32 * 4
UART_GPIO_OFF = UART_PORT_OFF + 4
STORAGE_GPIO_OFF = UART_GPIO_OFF + 2 * 8 + 4 + 5 * 8
I2C_GPIO_OFF = 0x308

A7S_DRAM_PARA = [
    2400,
    9,
    0x0E0E0E0E,
    0x0F0F0F0F,
    0xEC030E0F,
    0,
    0xA10A,
    0x1001,
    0,
    0,
    0,
    0x6,
    0,
    0,
    0,
    0x13,
    0x44,
    0,
    0x2E,
    0,
    0x6,
    0,
    0x4040,
    0,
    0x170B0703,
    0x3800,
    0x3514,
    0x325F0000,
    0,
    0,
    0x10065,
    0,
]

EXPECTED_UART_PORT = 0
EXPECTED_BOOT0_RUN_ADDR = 0x47000
EXPECTED_BOOT0_TEXT_ADDR = EXPECTED_BOOT0_RUN_ADDR + 0xB00
EXPECTED_NBOOT_STACK_MOV = 0xE3A0DA8D
EXPECTED_UART_GPIOS = [
    (2, 9, 2, 1, 0xFF, 0xFF),
    (2, 10, 2, 1, 0xFF, 0xFF),
]

EXPECTED_EARLY_UART_MARKERS = [
    b"A7S BOOT0: early uart0 alive\r\n",
]

EXPECTED_SDMMC_GPIOS = [
    (6, 2, 2, 1, 1, 0xFF),
    (6, 3, 2, 1, 1, 0xFF),
    (6, 1, 2, 1, 1, 0xFF),
    (6, 0, 2, 1, 1, 0xFF),
    (6, 5, 2, 1, 1, 0xFF),
    (6, 4, 2, 1, 1, 0xFF),
]

EXPECTED_I2C_GPIOS = [
    (12, 0, 2, 1, 0xFF, 0xFF),
    (12, 1, 2, 1, 0xFF, 0xFF),
]

MMC_REGISTER_MAP_RE = re.compile(
    r"\.text\.mmc_register\s*\n"
    r"\s*0x[0-9a-f]+\s+0x[0-9a-f]+\s+.*?/platform_shims\.o\s*\n"
    r"\s*0x[0-9a-f]+\s+mmc_register\b"
)

REQUIRED_FIP_SYMBOLS = [
    "fip_handoff_start",
    "fip_handoff_end",
    "sunxi_fip_read_image",
    "sunxi_fip_read_redundant",
    "sunxi_fip_copy_images",
    "sunxi_fip_load_redundant",
]

FORBIDDEN_TOC1_LOADER_SYMBOLS = [
    "load_package",
    "load_image",
]


def u32(buf: bytes, off: int) -> int:
    return struct.unpack_from("<I", buf, off)[0]


def u32_array(buf: bytes, off: int, count: int) -> list[int]:
    return list(struct.unpack_from(f"<{count}I", buf, off))


def gpio_array(buf: bytes, off: int, count: int) -> list[tuple[int, int, int, int, int, int]]:
    out = []
    for index in range(count):
        base = off + index * 8
        out.append(tuple(buf[base : base + 6]))
    return out


def boot0_checksum_ok(buf: bytes) -> bool:
    length = u32(buf, 0x10)
    words = list(struct.unpack_from(f"<{length // 4}I", buf, 0))
    words[3] = STAMP_VALUE
    total = sum(words) & 0xFFFFFFFF
    return total == u32(buf, 0x0C)


def linked_symbol(text: str, symbol: str) -> bool:
    return re.search(
        rf"^\s*0x[0-9a-f]+\s+{re.escape(symbol)}\s*$", text, re.MULTILINE
    ) is not None


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def validate_image(image: Path) -> None:
    buf = image.read_bytes()

    if buf[4:12] != BOOT0_MAGIC:
        fail(f"{image} is missing {BOOT0_MAGIC!r}")

    length = u32(buf, 0x10)
    if length != len(buf):
        fail(f"header length 0x{length:x} does not match file size 0x{len(buf):x}")

    if length > 0x40000:
        fail(f"boot0 length 0x{length:x} exceeds the 256 KiB A733 boot0 window")

    if u32(buf, PUB_HEAD_SIZE_OFF) != 0x30:
        fail("unexpected public header size")

    if u32(buf, 0x1C) != EXPECTED_BOOT0_RUN_ADDR:
        fail(f"boot0 ret_addr is 0x{u32(buf, 0x1C):x}, expected 0x{EXPECTED_BOOT0_RUN_ADDR:x}")

    if u32(buf, 0x20) != EXPECTED_BOOT0_RUN_ADDR:
        fail(f"boot0 run_addr is 0x{u32(buf, 0x20):x}, expected 0x{EXPECTED_BOOT0_RUN_ADDR:x}")

    if u32(buf, 0xB34) != EXPECTED_NBOOT_STACK_MOV:
        fail("entry code does not set the official A733 boot0 stack at 0x8d000")

    if not boot0_checksum_ok(buf):
        fail("boot0 checksum is invalid")

    if buf[DEBUG_MODE_OFF] != 1:
        fail(f"debug_mode is {buf[DEBUG_MODE_OFF]}, expected 1")

    if buf[POWER_MODE_OFF] != 1:
        fail(f"power_mode is {buf[POWER_MODE_OFF]}, expected 1")

    dram_para = u32_array(buf, DRAM_PARA_OFF, 32)
    if dram_para != A7S_DRAM_PARA:
        fail("DRAM parameters do not match the confirmed A7S 6GB LPDDR5 2400 configuration")

    if u32(buf, UART_PORT_OFF) != EXPECTED_UART_PORT:
        fail(f"UART port is {u32(buf, UART_PORT_OFF)}, expected {EXPECTED_UART_PORT}")

    if gpio_array(buf, UART_GPIO_OFF, 2) != EXPECTED_UART_GPIOS:
        fail("UART GPIO table does not match cubie_a7s PB9/PB10")

    for marker in EXPECTED_EARLY_UART_MARKERS:
        if marker not in buf:
            fail(f"early UART marker {marker!r} is missing from the image")

    if gpio_array(buf, STORAGE_GPIO_OFF, 6) != EXPECTED_SDMMC_GPIOS:
        fail("SD/MMC GPIO table does not match the A7S card0 boot pins")

    if gpio_array(buf, I2C_GPIO_OFF, 2) != EXPECTED_I2C_GPIOS:
        fail("I2C GPIO table does not match AXP8191 TWI6 pins PL0/PL1")

    for index in range(2):
        if buf[I2C_GPIO_OFF + index * 8 + 6] != 6:
            fail("I2C GPIO table does not select TWI6")


def validate_map(map_file: Path) -> None:
    text = map_file.read_text(encoding="utf-8", errors="replace")

    if "blobs/libdram.o" not in text:
        fail("link map does not include blobs/libdram.o")

    if "                sunxi_fpga_dram_init" in text:
        fail("link map still includes the FPGA DRAM path")

    if ".text.init_DRAM" not in text:
        fail("link map does not include init_DRAM")

    if "a7s_early_uart_init" not in text:
        fail("link map does not include the early A7S UART initializer")

    if "a7s_boot_set_gpio_v3" not in text:
        fail("link map does not include the A733 GPIO V3 override")

    if "a7s_axp8191_init" not in text:
        fail("link map does not include the AXP8191 PMIC implementation")

    # Do not inspect instruction bytes: ARM and Thumb-2 encode the same MMC0
    # register writes differently.  The map proves the A7S replacement is
    # linked instead of the FPGA blob's mmc_register implementation.
    if not MMC_REGISTER_MAP_RE.search(text):
        fail("link map does not include the A7S MMC registration override")

    if b"A7S PMU: AXP8191 ready on TWI6" not in map_file.with_name("boot0.bin").read_bytes():
        fail("boot0 image does not include the AXP8191 DRAM rail implementation")

    if f"0x{EXPECTED_BOOT0_RUN_ADDR:08x}                BT0_head" not in text:
        fail(f"BT0_head is not linked at 0x{EXPECTED_BOOT0_RUN_ADDR:x}")

    if f"0x{EXPECTED_BOOT0_TEXT_ADDR:08x}                _start" not in text:
        fail(f"_start is not linked at 0x{EXPECTED_BOOT0_TEXT_ADDR:x}")

    for symbol in REQUIRED_FIP_SYMBOLS:
        if not linked_symbol(text, symbol):
            fail(f"link map does not include the active FIP loader symbol {symbol}")

    for symbol in FORBIDDEN_TOC1_LOADER_SYMBOLS:
        if linked_symbol(text, symbol):
            fail(f"link map still includes the legacy TOC1 loader symbol {symbol}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("image", type=Path)
    parser.add_argument("map", type=Path)
    args = parser.parse_args()

    validate_image(args.image)
    validate_map(args.map)
    print("A7S boot0 image checks passed")


if __name__ == "__main__":
    main()
