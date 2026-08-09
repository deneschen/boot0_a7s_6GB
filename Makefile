# =============================================================================
# boot0-A7S : standalone, self-contained build of a real, bootable boot0 for
#             Allwinner A733 (sun60iw2p1), SD/MMC boot medium.
#
# It compiles the vendor boot0 sources and links them against the real
# pre-built board/DRAM blobs.  The legacy TOC1 loader is replaced by a strict
# TF-A FIP loader and a DRAM-resident BL31 handoff for the A733 boot chain.
#
# Low-level DRAM, MMC, PMIC, clock, and CPU-release operations still use the
# genuine vendor implementations or instruction sequences verified from them.
# =============================================================================

CROSS   ?= $(if $(shell command -v arm-linux-gnueabi-gcc 2>/dev/null),arm-linux-gnueabi-,arm-none-eabi-)
HOSTCC  ?= cc
FIPTOOL ?= $(abspath $(TOP)/../arm-trusted-firmware/tools/fiptool/fiptool)
DTC     ?= dtc
CC       = $(CROSS)gcc
LD       = $(CROSS)ld
OBJCOPY  = $(CROSS)objcopy
OBJDUMP  = $(CROSS)objdump
NM       = $(CROSS)nm
CPP      = $(CC) -E

TOP      := $(CURDIR)
BUILD    := $(TOP)/build
PLATFORM := sun60iw2p1

# Mainline U-Boot BL33. Keep its output under this project's build directory
# while leaving the independently upgradeable source tree untouched.
UBOOT_TOP       ?= $(abspath $(TOP)/../u-boot)
UBOOT_BUILD     ?= $(BUILD)/u-boot-a733
UBOOT_CROSS     ?= arm-none-eabi-
UBOOT_DEFCONFIG ?= cubie_a7s_defconfig
UBOOT_ELF       := $(UBOOT_BUILD)/u-boot
UBOOT_BIN       := $(UBOOT_BUILD)/u-boot.bin
UBOOT_DTB       := $(UBOOT_BUILD)/u-boot.dtb

# ---- run/size layout --------------------------------------------------------
# The flashable A733 SD-card boot0 image in device-a733/bin is linked and
# advertised to BROM at 0x47000.  Keep this standalone build on that runtime
# address so absolute references to BT0_head and early code match the image
# header used by the ROM loader.
BOOT0ADDR := 0x47000
BOOT0SIZE := 0x40000

# ---- include search path (mirrors spl-pub mk/config.mk SPLINCLUDE) ----------
INC := -I$(TOP)/include \
       -I$(TOP)/include/arch/arm \
       -I$(TOP)/include/configs \
       -I$(TOP)/include/arch/$(PLATFORM)

LIBGCCINC := -isystem $(shell dirname `$(CC) -print-libgcc-file-name`)/include

# ---- common flags (verbatim from spl-pub mk/config.mk, arm branch) ----------
COMM_FLAGS := -nostdinc $(LIBGCCINC) $(INC) \
	-g -Os -fno-common -mfpu=neon -msoft-float \
	-ffunction-sections \
	-fno-builtin -ffreestanding \
	-D__KERNEL__ \
	-DCONFIG_ARM -D__ARM__ \
	-D__NEON_SIMD__ \
	-march=armv7-a \
	-mabi=aapcs-linux \
	-mthumb-interwork \
	-fno-stack-protector \
	-Wall \
	-Wno-format-security \
	-Wno-format-nonliteral \
	-fno-delete-null-pointer-checks \
	-pipe \
	-mno-unaligned-access \
	-D__LINUX_ARM_ARCH__=7

# The A733 executes Thumb-2 code natively.  Keep the reset vector in ARM
# state, while compiling the board-specific C wrappers as Thumb-2 to reduce
# their footprint; -mthumb-interwork lets the linker create the required
# state-switch veneers at the assembly/C boundary.
CFLAGS := $(COMM_FLAGS) -mthumb -fdata-sections
AFLAGS := $(COMM_FLAGS) -D__ASSEMBLY__

LDFLAGS_GC := --gc-sections
LDFLAGS_MULTI_DEF := --allow-multiple-definition
PLATFORM_LIBGCC := -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

# ---- objects ----------------------------------------------------------------
# Real compiled-from-source objects:
OBJS := $(BUILD)/boot0_entry.o \
        $(BUILD)/fip_handoff.o \
        $(BUILD)/boot0_head.o \
        $(BUILD)/early_uart.o \
        $(BUILD)/boot0_main.o \
        $(BUILD)/sunxi_fip.o \
        $(BUILD)/platform_shims.o

# Real pre-built closed blob.
#
#   board_sdcard.o : the genuine vendor SD/MMC boot blob for a733/sun60iw2p1.
#                    It provides MMC / UART / legacy PMIC / clock drivers and
#                    the boot package loader. AXP8191 support is supplied by
#                    platform_shims.c because this blob is the FPGA variant.
#
#   libdram.o     : the A733 real-silicon DRAM training blob. It is linked
#                    before board_sdcard.o so its init_DRAM() wins over the
#                    small FPGA/bring-up routine in board_sdcard.o.
BLOBS := $(TOP)/blobs/libdram.o \
         $(BUILD)/board_sdcard.o

LDS_IN  := $(TOP)/arch/armv7/boot0.lds
LDS_OUT := $(BUILD)/boot0.lds

NAME := boot0_sdcard_$(PLATFORM)
FIPTOOL_TEST_IMAGE := $(BUILD)/test_fiptool.bin
FIPTOOL_TEST_IMAGE_NO_DTB := $(BUILD)/test_fiptool_no_dtb.bin
FIPTOOL_TEST_DTB := $(BUILD)/test_hw_config.dtb
SANITIZED_FIP_TEST := $(BUILD)/test_sunxi_fip_sanitize

.PHONY: all clean test test-sanitize test-fiptool verify scp scp-clean \
	uboot verify-all
all: $(BUILD)/$(NAME).bin

# ---- U-Boot BL33 ------------------------------------------------------------
uboot: | $(BUILD)
	@test -d $(UBOOT_TOP) || { echo "missing U-Boot tree: $(UBOOT_TOP)" >&2; exit 1; }
	@test -f $(UBOOT_TOP)/configs/$(UBOOT_DEFCONFIG) || { \
		echo "missing U-Boot defconfig: $(UBOOT_DEFCONFIG)" >&2; exit 1; }
	$(MAKE) -C $(UBOOT_TOP) O=$(UBOOT_BUILD) CROSS_COMPILE=$(UBOOT_CROSS) \
		$(UBOOT_DEFCONFIG)
	$(MAKE) -C $(UBOOT_TOP) O=$(UBOOT_BUILD) CROSS_COMPILE=$(UBOOT_CROSS)
	@python3 $(TOP)/tools/check_a733_uboot.py \
		--elf $(UBOOT_ELF) --binary $(UBOOT_BIN) --dtb $(UBOOT_DTB)

# ---- AR100S / SCP firmware --------------------------------------------------
# Transplant of u-boot-aw2501/arisc/ar100s, built the same way as
# device-a733/*/scp.bin (sun60iw2p1_defconfig + LICHEE_DRAMLIB_PATH).
# Output: ar100s/scp.bin and a staged copy at build/scp.bin (FIP SCP_BL2).
scp: | $(BUILD)
	$(TOP)/ar100s/build.sh
	@test -f $(BUILD)/scp.bin

scp-clean:
	-$(TOP)/ar100s/build.sh clean
	rm -f $(BUILD)/scp.bin $(BUILD)/scp.elf

test: $(BUILD)/test_sunxi_fip
	$<

test-sanitize: $(SANITIZED_FIP_TEST)
	ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 $<

$(BUILD)/test_sunxi_fip: $(TOP)/tests/test_sunxi_fip.c \
				 $(TOP)/src/sunxi_fip.c \
				 $(TOP)/include/sunxi_fip.h | $(BUILD)
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-I$(TOP)/include -I$(TOP)/include/arch/arm -o $@ \
		$(TOP)/tests/test_sunxi_fip.c $(TOP)/src/sunxi_fip.c

$(SANITIZED_FIP_TEST): $(TOP)/tests/test_sunxi_fip.c \
				$(TOP)/src/sunxi_fip.c \
				$(TOP)/include/sunxi_fip.h | $(BUILD)
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror -O1 -g \
		-fsanitize=address,undefined -fno-omit-frame-pointer \
		-I$(TOP)/include -I$(TOP)/include/arch/arm -o $@ \
		$(TOP)/tests/test_sunxi_fip.c $(TOP)/src/sunxi_fip.c

ifneq ($(wildcard $(FIPTOOL)),)
$(FIPTOOL_TEST_IMAGE): $(BUILD)/test_sunxi_fip \
			       $(TOP)/tests/test_sunxi_fip.c \
			       $(FIPTOOL_TEST_DTB) $(FIPTOOL) | $(BUILD)
	$(FIPTOOL) create --align 512 \
		--scp-fw $(TOP)/tests/test_sunxi_fip.c \
		--soc-fw $(TOP)/tests/test_sunxi_fip.c \
		--nt-fw $(TOP)/tests/test_sunxi_fip.c \
		--hw-config $(FIPTOOL_TEST_DTB) $@

$(FIPTOOL_TEST_IMAGE_NO_DTB): $(BUILD)/test_sunxi_fip \
				      $(TOP)/tests/test_sunxi_fip.c \
				      $(FIPTOOL) | $(BUILD)
	$(FIPTOOL) create --align 512 \
		--scp-fw $(TOP)/tests/test_sunxi_fip.c \
		--soc-fw $(TOP)/tests/test_sunxi_fip.c \
		--nt-fw $(TOP)/tests/test_sunxi_fip.c $@

$(FIPTOOL_TEST_DTB): $(TOP)/tests/test_hw_config.dts | $(BUILD)
	$(DTC) -I dts -O dtb -o $@ $<

test-fiptool: test $(FIPTOOL_TEST_IMAGE) $(FIPTOOL_TEST_IMAGE_NO_DTB)
	$(BUILD)/test_sunxi_fip $(FIPTOOL_TEST_IMAGE)
	$(BUILD)/test_sunxi_fip $(FIPTOOL_TEST_IMAGE_NO_DTB)
else
test-fiptool: test
	@echo "SKIP: TF-A fiptool not found at $(FIPTOOL)"
endif

# ---- compile rules ----------------------------------------------------------
$(BUILD)/boot0_entry.o: $(TOP)/arch/armv7/boot0_entry.S | $(BUILD)
	$(CC) $(AFLAGS) -c -o $@ $<

$(BUILD)/fip_handoff.o: $(TOP)/arch/armv7/fip_handoff.S \
				 $(TOP)/include/configs/sun60iw2p1.h | $(BUILD)
	$(CC) $(AFLAGS) -c -o $@ $<

$(BUILD)/boot0_head.o: $(TOP)/src/boot0_head.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/boot0_main.o: $(TOP)/src/boot0_main.c \
			       $(TOP)/include/sunxi_fip.h \
			       $(TOP)/include/sunxi_flashmap.h \
			       $(TOP)/include/configs/sun60iw2p1.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/early_uart.o: $(TOP)/src/early_uart.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/sunxi_fip.o: $(TOP)/src/sunxi_fip.c \
			      $(TOP)/include/sunxi_fip.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/platform_shims.o: $(TOP)/src/platform_shims.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

# Export the blob's local MMC device table so the real-silicon registration
# shim can preserve the vendor bookkeeping while applying the card0 pin setup.
$(BUILD)/board_sdcard.o: $(TOP)/blobs/board_sdcard.o | $(BUILD)
	$(OBJCOPY) --globalize-symbol=mmc_devices $< $@

# ---- preprocess linker script (mirrors nboot/Makefile boot0.lds rule) -------
$(LDS_OUT): $(LDS_IN) | $(BUILD)
	$(CPP) $(AFLAGS) -DBOOT0ADDR=$(BOOT0ADDR) -DBOOT0SIZE=$(BOOT0SIZE) \
		-DCPUDIR=$(BUILD) -DSOLUTIONDIR=$(BUILD) \
		-ansi -D__ASSEMBLY__ -P - < $< > $@

# ---- link + objcopy + header/checksum ---------------------------------------
$(BUILD)/boot0.elf: $(OBJS) $(BLOBS) $(LDS_OUT)
	$(LD) $(OBJS) $(BLOBS) $(PLATFORM_LIBGCC) $(LDFLAGS_GC) $(LDFLAGS_MULTI_DEF) \
		-T $(LDS_OUT) -o $@ -Map $(BUILD)/boot0.map

$(BUILD)/boot0.bin: $(BUILD)/boot0.elf
	$(OBJCOPY) -O binary $< $@

# gen_check_sum writes the BROM-required length + checksum into the head,
# producing the final flashable image (same step as the vendor build).
$(BUILD)/$(NAME).bin: $(BUILD)/boot0.bin
	$(TOP)/tools/gen_check_sum $< $@
	@echo "==> built $@"

$(BUILD):
	@mkdir -p $(BUILD)

verify: all test-fiptool test-sanitize
	@echo "=== ELF arch ==="; $(OBJDUMP) -f $(BUILD)/boot0.elf | head
	@echo "=== size ==="; ls -l $(BUILD)/$(NAME).bin
	@echo "=== entry head (first 32 bytes) ==="; xxd -l 32 $(BUILD)/$(NAME).bin
	@python3 $(TOP)/tools/check_a7s_boot0.py $(BUILD)/$(NAME).bin $(BUILD)/boot0.map

# boot0 + SCP firmware + U-Boot BL33 (A7S FIP ingredients)
verify-all: verify scp uboot
	@echo "=== SCP ==="; file $(BUILD)/scp.bin $(TOP)/ar100s/scp.elf
	@ls -l $(BUILD)/scp.bin $(TOP)/ar100s/scp.bin
	@$(TOP)/tools/check_ar100s.py --staged $(BUILD)/scp.bin
	@echo "=== U-Boot BL33 ==="; file $(UBOOT_ELF) $(UBOOT_BIN)

clean: scp-clean
	rm -rf $(BUILD)
