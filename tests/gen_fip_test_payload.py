#!/usr/bin/env python3
"""Generate deterministic minimal payloads for the fiptool integration test."""

import argparse
import struct
from pathlib import Path


def put_le32(payload: bytearray, offset: int, value: int) -> None:
    struct.pack_into("<I", payload, offset, value)


def make_scp() -> bytes:
    payload = bytearray(64)
    put_le32(payload, 0, 0x30047073)  # csrci mstatus, 8
    put_le32(payload, 4, 0x070902B7)  # lui t0, 0x7090
    put_le32(payload, 8, 0x11C28293)  # addi t0, t0, 0x11c
    put_le32(payload, 12, 0xE9020337)  # lui t1, 0xe9020
    put_le32(payload, 16, 0xA0230305)  # c.addi t1, 1; sw t1, 0(t0)
    put_le32(payload, 20, 0x40810062)  # tail of sw; c.li x1, 0
    return bytes(payload)


def make_bl31() -> bytes:
    payload = bytearray(0x1004)
    put_le32(payload, 0, 0xEA0003FE)
    payload[4:12] = b"monitor\0"
    put_le32(payload, 0x2C, 0x48000000)
    put_le32(payload, 0x1000, 0xAA0003F4)
    return bytes(payload)


def make_bl33() -> bytes:
    payload = bytearray(64)
    put_le32(payload, 0, 0xEA000002)  # ARM B from +0 to +0x10
    return bytes(payload)


PAYLOADS = {
    "scp": make_scp,
    "bl31": make_bl31,
    "bl33": make_bl33,
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("payload", choices=PAYLOADS)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    args.output.write_bytes(PAYLOADS[args.payload]())


if __name__ == "__main__":
    main()
