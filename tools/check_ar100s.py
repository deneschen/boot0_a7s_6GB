#!/usr/bin/env python3
"""Fail-closed validation for the A733 AR100S firmware image."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile


EXPECTED_BLOB_SHA256 = (
    "e1d0b91d4a3c8c4b67b65a03b901de5eb01b46b32743d245f287e9f1d57069e8"
)
SCP_ORIGIN = 0x40004000
SCP_FILE_MAX = 0x00028000
SCP_LINK_END = 0x40030000


def fail(message: str) -> None:
    raise SystemExit(f"AR100S verify: {message}")


def run(tool: Path | str, *args: str) -> str:
    result = subprocess.run(
        [str(tool), *args],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    if result.returncode != 0:
        fail(f"{' '.join([str(tool), *args])} failed:\n{result.stdout}")
    return result.stdout


def find_tool(root: Path, name: str) -> Path | str:
    local = root / "ar100s/tools/riscv64-elf-x86_64-20201104/bin" / name
    if local.is_file() and local.stat().st_mode & 0o111:
        return local
    system = shutil.which(name)
    if system:
        return system
    fail(f"required tool not found: {name}")


def symbol_values(nm_output: str) -> dict[str, int]:
    values: dict[str, int] = {}
    for line in nm_output.splitlines():
        fields = line.split()
        if len(fields) >= 3 and re.fullmatch(r"[0-9a-fA-F]+", fields[0]):
            values[fields[-1]] = int(fields[0], 16)
    return values


def verify(args: argparse.Namespace) -> None:
    root = Path(__file__).resolve().parent.parent
    elf = Path(args.elf)
    binary = Path(args.binary)
    blob = Path(args.blob)
    staged = Path(args.staged) if args.staged else None

    for path in (elf, binary, blob):
        if not path.is_file():
            fail(f"missing file: {path}")

    readelf = find_tool(root, "riscv64-unknown-elf-readelf")
    nm = find_tool(root, "riscv64-unknown-elf-nm")
    objdump = find_tool(root, "riscv64-unknown-elf-objdump")
    objcopy = find_tool(root, "riscv64-unknown-elf-objcopy")
    ar = find_tool(root, "riscv64-unknown-elf-ar")

    header = run(readelf, "-h", str(elf))
    if "ELF32" not in header or "little endian" not in header or "RISC-V" not in header:
        fail("ELF is not 32-bit little-endian RISC-V")
    entry_match = re.search(r"Entry point address:\s*(0x[0-9a-fA-F]+)", header)
    if not entry_match or int(entry_match.group(1), 16) != SCP_ORIGIN:
        fail("ELF entry point is not _start at 0x40004000")
    if "RVE" not in header or "RVC" not in header:
        fail("ELF flags do not advertise the required RV32E/RVC ABI")

    attributes = run(readelf, "-A", str(elf)).lower()
    arch_match = re.search(r'tag_riscv_arch:\s*"([^"]+)"', attributes)
    if not arch_match:
        fail("ELF has no Tag_RISCV_arch attribute")
    arch_tokens = arch_match.group(1).split("_")
    base_isa = arch_tokens[0]
    if not base_isa.startswith("rv32e"):
        fail(f"ELF base ISA is not RV32E: {base_isa}")
    for extension in ("m", "c"):
        if not any(re.fullmatch(rf"{extension}[0-9].*", token) for token in arch_tokens[1:]):
            fail(f"ELF attributes are missing ISA extension {extension}")
    for extension in ("zicsr", "zifencei"):
        if not any(token.startswith(extension) for token in arch_tokens[1:]):
            fail(f"ELF attributes are missing ISA extension {extension}")

    symbols = symbol_values(run(nm, "-n", str(elf)))
    required = (
        "_start",
        "__bss_end",
        "metal_segment_stack_begin",
        "metal_segment_stack_end",
    )
    missing = [name for name in required if name not in symbols]
    if missing:
        fail(f"missing linker symbols: {', '.join(missing)}")
    if symbols["_start"] != SCP_ORIGIN:
        fail("_start does not match the CPUX/E902 SRAM alias contract")
    if symbols["__bss_end"] > symbols["metal_segment_stack_begin"]:
        fail("allocated image overlaps the runtime stack")
    if not (
        symbols["metal_segment_stack_begin"]
        < symbols["metal_segment_stack_end"]
        <= SCP_LINK_END
    ):
        fail("runtime stack is outside the reserved SRAM window")

    program_headers = run(readelf, "-W", "-l", str(elf))
    load_segments = []
    for line in program_headers.splitlines():
        fields = line.split()
        if not fields or fields[0] != "LOAD" or len(fields) < 8:
            continue
        vaddr = int(fields[2], 16)
        file_size = int(fields[4], 16)
        memory_size = int(fields[5], 16)
        flags = "".join(fields[6:-1])
        if "W" in flags and "E" in flags:
            fail("ELF contains a writable and executable load segment")
        load_segments.append((vaddr, file_size, memory_size))
    if not load_segments:
        fail("ELF has no load segments")
    if min(segment[0] for segment in load_segments) != SCP_ORIGIN:
        fail("first ELF load segment does not start at 0x40004000")
    file_end = max(vaddr + file_size for vaddr, file_size, _ in load_segments)
    memory_end = max(vaddr + memory_size for vaddr, _, memory_size in load_segments)
    if file_end - SCP_ORIGIN != binary.stat().st_size:
        fail("raw image size does not match the ELF file-backed address span")
    if binary.stat().st_size > SCP_FILE_MAX:
        fail("raw image exceeds the boot0 SRAM copy window")
    if memory_end > symbols["metal_segment_stack_begin"]:
        fail("ELF load memory crosses the runtime stack boundary")

    if run(nm, "-u", str(elf)).strip():
        fail("ELF contains undefined symbols")

    set_sp = run(objdump, "-d", "--disassemble=__set_SP", str(elf))
    instructions = [
        line
        for line in set_sp.splitlines()
        if re.match(r"^\s*[0-9a-fA-F]+:\s", line)
    ]
    if (
        len(instructions) != 2
        or not re.search(r"\bmv\s+sp,a0\s*$", instructions[0])
        or not re.search(r"\bret\s*$", instructions[1])
    ):
        fail("__set_SP contains a compiler-generated stack frame")

    build_dir = root / "build"
    build_dir.mkdir(exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=build_dir) as raw:
        run(objcopy, "-O", "binary", str(elf), raw.name)
        if Path(raw.name).read_bytes() != binary.read_bytes():
            fail("scp.bin is stale or differs from scp.elf")

    blob_hash = hashlib.sha256(blob.read_bytes()).hexdigest()
    if blob_hash != EXPECTED_BLOB_SHA256:
        fail(f"unexpected libar100s.a SHA256: {blob_hash}")
    members = [line for line in run(ar, "t", str(blob)).splitlines() if line]
    if members != ["obj-in.o"]:
        fail(f"unexpected libar100s.a members: {members}")

    if staged is not None:
        if not staged.is_file() or staged.read_bytes() != binary.read_bytes():
            fail("staged build/scp.bin is missing or stale")

    headroom = symbols["metal_segment_stack_begin"] - symbols["__bss_end"]
    print(
        "AR100S verify: OK "
        f"entry=0x{SCP_ORIGIN:08x} file={binary.stat().st_size} "
        f"stack_headroom={headroom} blob={blob_hash[:12]}"
    )


def main() -> None:
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser()
    parser.add_argument("--elf", default=root / "ar100s/scp.elf")
    parser.add_argument("--binary", default=root / "ar100s/scp.bin")
    parser.add_argument("--blob", default=root / "blobs/libar100s.a")
    parser.add_argument("--staged")
    verify(parser.parse_args())


if __name__ == "__main__":
    main()
