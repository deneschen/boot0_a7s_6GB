#!/usr/bin/env python3
"""Fail-closed validation for the mainline A733 U-Boot BL33 image."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import subprocess
import struct


ELF32_HEADER_SIZE = 52
ELFCLASS32 = 1
ELFDATA2LSB = 1
ET_EXEC = 2
EM_ARM = 40
BL33_ENTRY = 0x4A000000
BL33_MAX_SIZE = 0x180000


def fail(message: str) -> None:
    raise SystemExit(f"U-Boot verify: {message}")


MMC_NODE = "/soc/mmc@4020000"


def fdtget(dtb_path: Path, value_type: str, node: str, prop: str) -> list[str]:
    try:
        result = subprocess.run(
            ["fdtget", "-t", value_type, str(dtb_path), node, prop],
            check=True,
            capture_output=True,
            text=True,
        )
    except FileNotFoundError:
        fail("fdtget is required to validate u-boot.dtb")
    except subprocess.CalledProcessError as error:
        fail(f"cannot read {node}/{prop}: {error.stderr.strip()}")

    return result.stdout.split()


def verify(elf_path: Path, binary_path: Path, dtb_path: Path) -> None:
    for path in (elf_path, binary_path, dtb_path):
        if not path.is_file():
            fail(f"missing file: {path}")

    elf = elf_path.read_bytes()
    if len(elf) < ELF32_HEADER_SIZE or elf[:4] != b"\x7fELF":
        fail(f"not an ELF image: {elf_path}")
    if elf[4] != ELFCLASS32 or elf[5] != ELFDATA2LSB:
        fail("ELF is not 32-bit little-endian")

    elf_type, machine = struct.unpack_from("<HH", elf, 16)
    entry = struct.unpack_from("<I", elf, 24)[0]
    if elf_type != ET_EXEC or machine != EM_ARM:
        fail("ELF is not an executable for ARM")
    if entry != BL33_ENTRY:
        fail(f"ELF entry is 0x{entry:08x}, expected 0x{BL33_ENTRY:08x}")

    binary = binary_path.read_bytes()
    if not binary:
        fail("u-boot.bin is empty")
    if len(binary) > BL33_MAX_SIZE:
        fail(f"u-boot.bin is {len(binary)} bytes, limit is {BL33_MAX_SIZE}")

    clocks = fdtget(dtb_path, "x", MMC_NODE, "clocks")
    clock_names = fdtget(dtb_path, "s", MMC_NODE, "clock-names")
    resets = fdtget(dtb_path, "x", MMC_NODE, "resets")
    if len(clocks) != 4 or clock_names != ["ahb", "mmc"]:
        fail("MMC0 requires ahb and mmc clock phandles")
    if len(resets) != 2:
        fail("MMC0 requires one reset phandle")

    digest = hashlib.sha256(binary).hexdigest()
    print(
        "U-Boot verify: OK "
        f"entry=0x{entry:08x} file={len(binary)} sha256={digest[:12]}"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--elf", type=Path, required=True)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--dtb", type=Path, required=True)
    args = parser.parse_args()
    verify(args.elf, args.binary, args.dtb)


if __name__ == "__main__":
    main()
