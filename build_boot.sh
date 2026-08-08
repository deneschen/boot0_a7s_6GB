#!/usr/bin/env bash
# =============================================================================
# boot0-A7S/build_boot.sh
#
# One-shot build + package for the A733 (sun60iw2p1) boot chain.  Verifies
# local changes in:
#   - arm-trusted-firmware        (BL31, --soc-fw)
#   - boot0-A7S/ar100s            (SCP,  --scp-fw)
#   - u-boot-aw2501               (BL33, --nt-fw)
#
# Steps:
#   1. build boot0  (this tree)               -> build/boot0_sdcard_sun60iw2p1.bin
#   2. build SCP    (ar100s)                  -> build/scp.bin
#   3. build BL31   (arm-trusted-firmware)    -> <atf>/build/sun60i_a733/release/bl31.bin
#   4. prepend the "monitor" header           -> build/bl31-monitor.bin
#      (BL31 is linked at 0x48001000; boot0 copies the monitor header plus
#       BL31 to 0x48000000 and enters at 0x48001000, so the FIP must carry
#       the header + BL31, not the bare bl31.bin)
#   5. package FIP with fiptool               -> build/fip.bin
#
# Flashing is handled by a separate script (../Share/flash.sh), which writes
# boot0 + FIP to an SD/MMC device.
#
# Usage:
#   ./build_boot.sh [options]
#
# Options:
#   --skip-boot0     keep existing build/boot0_sdcard_sun60iw2p1.bin
#   --skip-scp       keep existing build/scp.bin
#   --skip-bl31      keep the selected monitor image
#   --atf DIR        arm-trusted-firmware tree (default: ../arm-trusted-firmware)
#   --uboot FILE     BL33 image (default: ../u-boot-aw2501/src/u-boot.bin)
#   --jobs N         parallel make jobs (default: nproc)
#   -h, --help       show this help
# =============================================================================
set -euo pipefail

# ---- defaults ---------------------------------------------------------------
TOP="$(cd "$(dirname "$0")" && pwd)"
ATF="$(cd "${TOP}/.." && pwd)/arm-trusted-firmware"
UBOOT_BIN="$(cd "${TOP}/.." && pwd)/u-boot-aw2501/src/u-boot.bin"
BUILD="${TOP}/build"
BL31_PLAT="sun60i_a733"
BL31_LOAD_ADDR="0x48000000"

JOBS="$(nproc 2>/dev/null || echo 4)"

SKIP_BOOT0=0
SKIP_SCP=0
SKIP_BL31=0

usage() {
	sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//'
}

# ---- options ----------------------------------------------------------------
while [ $# -gt 0 ]; do
	case "$1" in
	--skip-boot0) SKIP_BOOT0=1 ;;
	--skip-scp) SKIP_SCP=1 ;;
	--skip-bl31) SKIP_BL31=1 ;;
	--atf)
		[ $# -ge 2 ] || { echo "error: --atf needs a directory" >&2; exit 1; }
		ATF="$2"; shift ;;
	--uboot)
		[ $# -ge 2 ] || { echo "error: --uboot needs a file" >&2; exit 1; }
		UBOOT_BIN="$2"; shift ;;
	--jobs)
		[ $# -ge 2 ] || { echo "error: --jobs needs a number" >&2; exit 1; }
		JOBS="$2"; shift ;;
	-h|--help) usage; exit 0 ;;
	*) echo "error: unknown option: $1" >&2; usage; exit 1 ;;
	esac
	shift
done

ATF_BUILD_BASE="${ATF}/build"
BL31_MONITOR="${BUILD}/bl31-monitor.bin"
FIP_OUTPUT="${BUILD}/fip.bin"

BL31_BIN="${ATF_BUILD_BASE}/${BL31_PLAT}/release/bl31.bin"
FIPTOOL="${ATF_BUILD_BASE}/${BL31_PLAT}/release/tools/fiptool/fiptool"
GEN_MONITOR="${ATF}/plat/allwinner/${BL31_PLAT}/gen_monitor_img.py"

# ---- sanity checks ----------------------------------------------------------
[ -d "${ATF}" ] || { echo "error: ATF tree not found: ${ATF}" >&2; exit 1; }
[ -f "${UBOOT_BIN}" ] || { echo "error: BL33 not found: ${UBOOT_BIN}" >&2; exit 1; }

require_file() {
	[ -f "$1" ] || { echo "error: missing $1 (build it or drop the matching --skip-*)" >&2; exit 1; }
}

if [ ! -x "${FIPTOOL}" ]; then
	echo "==> building fiptool"
	make -C "${ATF}" BUILD_BASE="${ATF_BUILD_BASE}" \
		PLAT="${BL31_PLAT}" fiptool
fi
[ -x "${FIPTOOL}" ] || { echo "error: fiptool build failed: ${FIPTOOL}" >&2; exit 1; }

# ---- 1. boot0 ---------------------------------------------------------------
if [ "${SKIP_BOOT0}" -eq 1 ]; then
	echo "==> skipping boot0 build"
else
	echo "==> building boot0"
	make -C "${TOP}" -j"${JOBS}" all
fi
require_file "${BUILD}/boot0_sdcard_sun60iw2p1.bin"

# ---- 2. SCP -----------------------------------------------------------------
if [ "${SKIP_SCP}" -eq 1 ]; then
	echo "==> skipping SCP build"
else
	echo "==> building SCP (ar100s)"
	make -C "${TOP}" -j"${JOBS}" scp
fi
require_file "${BUILD}/scp.bin"

# ---- 3+4. BL31 + monitor header ---------------------------------------------
if [ "${SKIP_BL31}" -eq 1 ]; then
	echo "==> skipping BL31 build"
else
	echo "==> building BL31 (${BL31_PLAT})"
	make -C "${ATF}" BUILD_BASE="${ATF_BUILD_BASE}" \
		PLAT="${BL31_PLAT}" -j"${JOBS}"
	require_file "${BL31_BIN}"
	echo "==> applying monitor header -> ${BL31_MONITOR}"
	python3 "${GEN_MONITOR}" "${BL31_BIN}" "${BL31_MONITOR}" "${BL31_LOAD_ADDR}"
fi
require_file "${BL31_MONITOR}"

# ---- 5. FIP -----------------------------------------------------------------
echo "==> packaging FIP"
"${FIPTOOL}" create --align 512 \
	--scp-fw "${BUILD}/scp.bin" \
	--soc-fw "${BL31_MONITOR}" \
	--nt-fw "${UBOOT_BIN}" \
	"${FIP_OUTPUT}"
echo "==> FIP contents:"
"${FIPTOOL}" info "${FIP_OUTPUT}"

echo
echo "=== build done ==="
ls -l "${BUILD}/boot0_sdcard_sun60iw2p1.bin" "${BUILD}/scp.bin" \
	"${BL31_MONITOR}" "${FIP_OUTPUT}"
echo "=== flash with: sudo ../Share/flash.sh /dev/sdX ==="
