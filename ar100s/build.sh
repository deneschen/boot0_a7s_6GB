#!/usr/bin/env bash
# =============================================================================
# boot0-A7S/ar100s/build.sh
#
# Build Allwinner A733 (sun60iw2p1) AR100S / SCP firmware.
#
# Default toolchain: Ubuntu/mainline riscv64-unknown-elf-gcc
#   sudo apt install gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf \
#                    picolibc-riscv64-unknown-elf
#
# Optional local vendor tree (if present):
#   tools/riscv64-elf-x86_64-20201104/
#
# OEM blob  : ../blobs/libar100s.a (verified canonical input)
# Output    : scp.bin / scp.elf  (+ staged to ../build/scp.bin)
# =============================================================================
set -euo pipefail
cd "$(dirname "$0")"

PLAT=sun60iw2p1
DEFCONFIG=${PLAT}_defconfig
ROOT="$(cd .. && pwd)"
DRAMLIB_PATH="$(pwd)/dramlib"
LOCAL_GCC="tools/riscv64-elf-x86_64-20201104/bin/riscv64-unknown-elf-gcc"
BLOB="${ROOT}/blobs/libar100s.a"
BLOB_SHA256="e1d0b91d4a3c8c4b67b65a03b901de5eb01b46b32743d245f287e9f1d57069e8"

# ---- OEM / DRAM closed blob -------------------------------------------------
if [ ! -f "${BLOB}" ]; then
	echo "error: missing canonical AR100S blob: ${BLOB}" >&2
	exit 1
fi
ACTUAL_BLOB_SHA256="$(sha256sum "${BLOB}" | awk '{print $1}')"
if [ "${ACTUAL_BLOB_SHA256}" != "${BLOB_SHA256}" ]; then
	echo "error: unexpected libar100s.a SHA256: ${ACTUAL_BLOB_SHA256}" >&2
	exit 1
fi
mkdir -p "${DRAMLIB_PATH}/${PLAT}/arisc_liboem"
if ! cmp -s "${BLOB}" "${DRAMLIB_PATH}/${PLAT}/arisc_liboem/libar100s.a"; then
	cp -f "${BLOB}" "${DRAMLIB_PATH}/${PLAT}/arisc_liboem/libar100s.a"
fi

# ---- Toolchain --------------------------------------------------------------
if [ -x "${LOCAL_GCC}" ]; then
	TOOLCHAIN="$("${LOCAL_GCC}" --version | head -1)"
elif command -v riscv64-unknown-elf-gcc >/dev/null 2>&1; then
	TOOLCHAIN="$(riscv64-unknown-elf-gcc --version | head -1)"
else
	echo "error: riscv64-unknown-elf-gcc not found" >&2
	echo "       install: sudo apt install gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf picolibc-riscv64-unknown-elf" >&2
	exit 1
fi

# Skip vendor auto-extract path in Makefile (we no longer ship that tarball)
mkdir -p tools
touch tools/.toolchain.flag

JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
MAKE_JOBS=(-j"${JOBS}")
case " ${MAKEFLAGS:-} " in
*--jobserver-auth=*|*--jobserver-fds=*)
	MAKE_JOBS=()
	;;
esac

# ---- Build ------------------------------------------------------------------
STAGE_OUTPUT=0
if [ "$#" -eq 0 ]; then
	echo "arisc defconfig: ${DEFCONFIG}"
	echo "dramlib: ${DRAMLIB_PATH}"
	echo "toolchain: ${TOOLCHAIN}"
	make "${MAKE_JOBS[@]}" "${DEFCONFIG}"
	make "${MAKE_JOBS[@]}" \
		LICHEE_DRAMLIB_PATH="${DRAMLIB_PATH}" \
		CFG_CHIP_PLATFORM="${PLAT}"
	STAGE_OUTPUT=1
else
	make "${MAKE_JOBS[@]}" \
		LICHEE_DRAMLIB_PATH="${DRAMLIB_PATH}" \
		CFG_CHIP_PLATFORM="${PLAT}" \
		"$@"
	for target in "$@"; do
		case "${target}" in
		all|scp.bin)
			STAGE_OUTPUT=1
			;;
		esac
	done
fi

if [ "${STAGE_OUTPUT}" -eq 1 ]; then
	if [ ! -f scp.bin ] || [ ! -f scp.elf ]; then
		echo "error: requested firmware target did not produce scp.bin/scp.elf" >&2
		exit 1
	fi
	"${ROOT}/tools/check_ar100s.py" --elf scp.elf --binary scp.bin --blob "${BLOB}"
	mkdir -p "${ROOT}/build"
	cp -f scp.bin "${ROOT}/build/scp.bin"
	cp -f scp.elf "${ROOT}/build/scp.elf"
	echo
	echo "=== build done ==="
	file scp.bin scp.elf 2>/dev/null || true
	ls -l scp.bin
	echo "staged: ${ROOT}/build/scp.bin"
fi
